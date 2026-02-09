# STM32 Drone İzleme Projesi

Bu proje, STM32 mikrodenetleyici kullanarak geliştirdiğim bir drone izleme çalışmasıdır.
Projeyi yaparken hem donanım hem de yazılım tarafında kendimi geliştirmeyi amaçladım.

FreeRTOS kullanarak sensör okuma ve ekran işlemlerini ayrı görevler (task) halinde çalıştırdım.

## Projede Neler Var?
- Ultrasonik sensör ile mesafe ölçümü
- DHT11 ile sıcaklık ve nem okuma
- SSD1306 OLED ekranda bilgileri gösterme
- UART üzerinden bilgisayara veri gönderme
- Mesafe limitine göre uyarı sistemi (LED)

## Kullandığım Teknolojiler
- STM32 (HAL kütüphanesi)
- FreeRTOS
- UART / I2C / Timer
- STM32CubeIDE

## Proje Yapısı
- **SensorTask**  
  Sensörlerden verileri okur ve sistemi kontrol eder.
- **DisplayTask**  
  OLED ekrana bilgileri yazar ve UART üzerinden veri gönderir.

## Karşılaştığım Sorunlar ve Çözümler

Bu projeyi geliştirirken yazılımdan çok donanım kaynaklı birçok problemle karşılaştım. 
Özellikle kullanılan parçaların orijinal olmaması, internetteki kaynaklarla birebir uyuşmayan durumlara sebep oldu.

### 1. Orijinal Olmayan Donanımlar Kaynaklı Sorunlar

**DHT11 Sensörü (Bacak Dizilimi ve Besleme Problemi)**  
Sipariş ettiğim DHT11 sensörünün bacak dizilimi internetteki hiçbir kaynakla uyuşmuyordu. 
İnternette ve yapay zeka kaynaklarında verilen bilgiler bu sensör için yanlış yönlendirmelere sebep oldu.

- Sensörün normalde **3.3V ile çalışması gerektiği** belirtilmesine rağmen,
- Kullandığım sensör **3.3V’ta hiç çalışmadı**, yalnızca **5V besleme ile kararlı veri üretebildi**.

Bu durumu tamamen **deneme–yanılma** yöntemiyle tespit ettim.  
Ancak bu yöntemin donanım için **riskli ve tehlikeli** olabileceğini özellikle belirtmek isterim.

---

**PL2303 USB-TTL Dönüştürücü Problemi**  
Kullandığım PL2303 modeli orijinal değildi ve bilgisayar tarafından algılanmıyordu.

- Güncel sürücülerle cihaz hiç tanınmadı.
- Çözüm olarak **eski model PL2303 sürücüsü** yüklenerek cihaz çalıştırılabildi.

Bu süreç, donanım uyumluluğunun ne kadar kritik olduğunu fark etmemi sağladı.

---

**OLED Ekran Uyumsuzluğu**  
Sipariş ettiğim ekran ile gelen ekran farklıydı.

- Pin isimleri kart üzerinde yazmıyordu.
- İnternetteki şemalarla birebir uyuşmadığı için bağlantı aşamasında ciddi zaman kaybı yaşadım.

Pinleri datasheet ve deneme yöntemiyle tek tek tespit ederek bağlantıyı gerçekleştirdim.

---

**Jumper Kablo Temassızlık Sorunu**  
Kullandığım jumper kabloların uçları uzun olduğu için **STM32F407 kartına tam oturmuyordu**.

- En ufak bir oynamada bağlantı kopuyordu.
- Bu durum sensörlerin bazen çalışıp bazen çalışmamasına sebep oldu.

**Çözüm:** Daha kısa uçlu ve kaliteli jumper kablolar tercih edilmesi gerektiğini deneyimledim.

---

### 2. Ekran Güncelleme ve RTOS Senkronizasyonu

**Sorun:**  
OLED ekranda verilerin çok hızlı ve düzensiz güncellenmesi, I2C hattının yorulması.

**Neden:**  
FreeRTOS görevleri içerisinde yeterli bekleme süresinin olmaması.

**Çözüm:**  
`StartDisplayTask` döngüsüne `osDelay(1000)` eklenerek:
- Ekran güncelleme süresi **1 saniyeye sabitlendi**
- Okunabilirlik arttı
- I2C hattındaki yoğunluk azaldı

---

### 3. DHT11 Sensöründen Veri Alınamaması

**Sorun:**  
Sıcaklık ve nem değerlerinin sürekli `0` gelmesi veya hiç değişmemesi.

**Nedenler:**
1. DHT11’in mikrosaniye hassasiyetindeki zamanlamasının RTOS kesmeleriyle bozulması  
2. Sensörün 3.3V hattında düşük akım nedeniyle kararsız çalışması  

**Çözüm:**
- Kritik okuma anında kesmeler durdurularak:
  `__disable_irq()` ve `__enable_irq()` kullanıldı
- Sensör beslemesi **5V hattına taşındı**

Bu sayede sensör kararlı şekilde veri üretmeye başladı.

---

### 4. Derleme Uyarıları ve Bellek Taşması (Buffer Overflow)

**Sorun:**  
`sprintf` kullanımı sırasında derleme uyarıları ve sistemin bir süre sonra donması.

**Neden:**  
UART üzerinden gönderilen veri uzunluğu (77–99 byte),
tanımlanan `buff[64]` dizisinden büyüktü ve bellek taşması oluşuyordu.

**Çözüm:**
- Buffer boyutu `buff[128]` olarak güncellendi
- Kullanılmayan değişkenler temizlendi

Sonuçta proje **0 Error, 0 Warning** seviyesine getirildi.

---

### 5. I2C Hattı Kilitlenmesi (I2C Bus Lockup)

**Sorun:**  
OLED ekranın donması, reset atılsa bile görüntünün gelmemesi.

**Neden:**  
I2C haberleşmesi sırasında ekranın SDA hattını "Low" seviyesinde bırakması.
Bu durumda işlemci hattı sürekli "Busy" görüyordu.

**Çözüm:**  
`I2C_Bus_Recovery` fonksiyonu yazıldı.

- Sistem açılışında SCL hattı **9 kez manuel olarak tetiklendi**
- I2C hattı zorla serbest bırakıldı

Bu sayede ekran her reset sonrası düzgün çalıştı.

---

### 6. UART Üzerinden Anlamsız Karakterler Gelmesi

**Sorun:**  
Terminale gelen verilerin okunamaz karakterlerden oluşması.

**Nedenler:**
1. STM32 ve PL2303 arasında ortak GND bağlantısının olmaması  
2. Sistem saatinin UART baud hızıyla uyumsuz olması  

**Çözüm:**
- Tüm cihazların GND uçları birleştirildi
- Sistem saati, harici kristal yerine **HSI (Internal Oscillator)** kullanacak şekilde ayarlandı  
  (16 MHz → PLL ile 168 MHz)

UART haberleşmesi stabil hale getirildi.

---

### 7. UART Komutlarının Algılanmaması (+ / -)

**Sorun:**  
Terminalden gönderilen `+` ve `-` komutlarının eşik değerini değiştirmemesi.

**Nedenler:**
1. TX ve RX hatlarının çapraz bağlanmaması  
2. USART2 kesmesinin CubeMX üzerinde aktif edilmemesi  

**Çözüm:**
- TX–RX bağlantıları doğru şekilde çaprazlandı
- USART2 NVIC kesmesi aktif edildi
- `HAL_UART_Receive_IT` callback sonunda tekrar çağrılarak sürekli dinleme sağlandı


## Amaç
Bu proje bir öğrenme projesidir.
Amacım gerçek zamanlı sistemler, RTOS ve gömülü yazılım konularında deneyim kazanmaktır.

> Not: Bu projeyi geliştirirken amacım öğrenmekti.
> İleride IMU sensörleri ve uçuş kontrol algoritmaları ekleyerek
> projeyi geliştirmeyi planlıyorum.

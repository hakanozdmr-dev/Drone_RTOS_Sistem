#ifndef SSD1306_H
#define SSD1306_H

#include "stm32f4xx_hal.h"

// Ekran Ayarları
#define SSD1306_I2C_ADDR        0x78 // Senin ekranın adresi
#define SSD1306_WIDTH           128
#define SSD1306_HEIGHT          64

// Fonksiyon Tanımları
void SSD1306_Init(void);
void SSD1306_Fill(uint8_t color); // 0: Siyah, 1: Beyaz
void SSD1306_UpdateScreen(void);
void SSD1306_DrawPixel(uint8_t x, uint8_t y, uint8_t color);
void SSD1306_WriteString(char* str, uint8_t x, uint8_t y, uint8_t color);
void SSD1306_SetCursor(uint8_t x, uint8_t y);

#endif

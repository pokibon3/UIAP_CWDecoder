/*
 * Single-File-Header for using SPI OLED
 * 05-05-2023 E. Brombaugh
 */

#ifndef _SSD1306_H
#define _SSD1306_H

#include <stdint.h>
#include <string.h>
#include "font_8x8.h"

// comfortable packet size for this OLED
#define SSD1306_PSZ 32

// characteristics of each type
#if !defined (SSD1306_64X32) && !defined (SSD1306_128X32) && !defined (SSD1306_128X64) && !defined (SH1107_128x128)
	#error "Please define the SSD1306_WXH resolution used in your application"
#endif

#ifdef SSD1306_64X32
#define SSD1306_W 64
#define SSD1306_H 32
#define SSD1306_FULLUSE
#define SSD1306_OFFSET 32
#endif

#ifdef SSD1306_128X32
#define SSD1306_W 128
#define SSD1306_H 32
#define SSD1306_OFFSET 0
#endif

#ifdef SSD1306_128X64
#define SSD1306_W 128
#define SSD1306_H 64
#define SSD1306_FULLUSE
#define SSD1306_OFFSET 0
#endif

#ifdef SH1107_128x128
#define SH1107
#define SSD1306_FULLUSE
#define SSD1306_W 128
#define SSD1306_H 128
#define SSD1306_FULLUSE
#define SSD1306_OFFSET 0
#endif

#define SSD1306_SETCONTRAST 0x81
#define SSD1306_SEGREMAP 0xA0
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF
#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA
#define SSD1306_SETVCOMDETECT 0xDB
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9
#define SSD1306_SETMULTIPLEX 0xA8
#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10
#define SSD1306_SETSTARTLINE 0x40
#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR   0x22
#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8
#define SSD1306_CHARGEPUMP 0x8D
#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2
#define SSD1306_TERMINATE_CMDS 0xFF

/* choose VCC mode */
#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2
//#define vccstate SSD1306_EXTERNALVCC
#define vccstate SSD1306_SWITCHCAPVCC

/*
 * enum for font size
 */
typedef enum {
    fontsize_8x8 = 1,
    fontsize_16x16 = 2,
    fontsize_32x32 = 4,
	fontsize_64x64 = 8,
} font_size_t;

void ssd1306_setbuf(uint8_t color);
void ssd1306_refresh(void);
void ssd1306_drawPixel(uint8_t x, uint8_t y, uint8_t color);
void ssd1306_xorPixel(uint8_t x, uint8_t y);
void ssd1306_drawImage(uint8_t x, uint8_t y, const unsigned char* input, uint8_t width, uint8_t height, uint8_t color_mode);
void ssd1306_drawFastVLine(uint8_t x, uint8_t y, uint8_t h, uint8_t color);
void ssd1306_drawFastHLine(uint8_t x, uint8_t y, uint8_t w, uint8_t color);
void ssd1306_drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t color);
void ssd1306_drawCircle(int16_t x, int16_t y, int16_t radius, int8_t color);
void ssd1306_fillCircle(int16_t x, int16_t y, int16_t radius, int8_t color);
void ssd1306_drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
void ssd1306_fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
void ssd1306_xorrect(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void ssd1306_drawchar(uint8_t x, uint8_t y, uint8_t chr, uint8_t color);
void ssd1306_drawstr(uint8_t x, uint8_t y, char *str, uint8_t color);
void ssd1306_drawchar_sz(uint8_t x, uint8_t y, uint8_t chr, uint8_t color, font_size_t font_size);
void ssd1306_drawstr_sz(uint8_t x, uint8_t y, char *str, uint8_t color, font_size_t font_size);
uint8_t ssd1306_init(void);

#endif

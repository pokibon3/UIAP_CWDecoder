/*
 * Single-File-Header for SSD1306 SPI interface
 * 05-05-2023 E. Brombaugh
 */

#ifndef _SSD1306_SPI_H
#define _SSD1306_SPI_H

// control pins
#define SSD1306_RST_PORT GPIOA			// change C -> A
#define SSD1306_RST_PIN 6				// change 2 -> 6  for 203
#define SSD1306_RST_HIGH() SSD1306_RST_PORT->BSHR = (1<<(SSD1306_RST_PIN))
#define SSD1306_RST_LOW() SSD1306_RST_PORT->BSHR = (1<<(16+SSD1306_RST_PIN))
#define SSD1306_CS_PORT GPIOA			// change C -> A
#define SSD1306_CS_PIN 3
#define SSD1306_CS_HIGH() SSD1306_CS_PORT->BSHR = (1<<(SSD1306_CS_PIN))
#define SSD1306_CS_LOW() SSD1306_CS_PORT->BSHR = (1<<(16+SSD1306_CS_PIN))
#define SSD1306_DC_PIN 4
#define SSD1306_DC_PORT GPIOA			// change C -> A
#define SSD1306_DC_HIGH() SSD1306_DC_PORT->BSHR = (1<<(SSD1306_DC_PIN))
#define SSD1306_DC_LOW() SSD1306_DC_PORT->BSHR = (1<<(16+SSD1306_DC_PIN))

uint8_t ssd1306_spi_init(void);
void ssd1306_rst(void);
uint8_t ssd1306_pkt_send(uint8_t *data, uint8_t sz, uint8_t cmd);

#endif

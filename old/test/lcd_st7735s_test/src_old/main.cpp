//
//	CW Decoder and Audio Spectrum AnalyzerVer 1.2
//
//  2024.11.08 New Create
//
#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#include "ch32v003fun.h"
#include "ch32v003_GPIO_branchless.h"
#include "morse_decoder.h"
#include "docode.h"
#include "gpio.h"
#include "fft.h"

// what type of OLED - uncomment just one
//#define SSD1306_64X32
//#define SSD1306_128X32
#define SSD1306_128X64
#include "ssd1306_spi.h"
#include "ssd1306.h"

// function prototype (declaration), definition in "ch32v003fun.c"
extern "C" int mini_snprintf(char* buffer, unsigned int buffer_len, const char *fmt, ...);

static char title1[]   = " CW Decoder V1.2";
static uint16_t function = 0;
enum func {
	cw_decode = 0,
	fft = 1
};

//=========================================================
//	check GPIO SW
//=========================================================
static void check_function() 
{

	if (!GPIO_digitalRead(SW1_PIN)) {
		function = fft;
	} else {
		function = cw_decode;
	}
}

//=========================================================
//	setup
//=========================================================
void setup()
{
	SystemInit();			// ch32v003 sETUP
    // 各GPIOの有効化
    GPIO_port_enable(GPIO_port_D);
    GPIO_port_enable(GPIO_port_C);
    GPIO_port_enable(GPIO_port_B);
    // SWピンの設定
    GPIO_pinMode(SW1_PIN, GPIO_pinMode_I_pullUp, GPIO_Speed_10MHz); /// GPIO_Speed_In? @see https://github.com/cnlohr/ch32v003fun/blob/master/examples/GPIO/GPIO.c#L55
    GPIO_pinMode(SW2_PIN, GPIO_pinMode_I_pullUp, GPIO_Speed_10MHz);
    GPIO_pinMode(SW3_PIN, GPIO_pinMode_I_pullUp, GPIO_Speed_10MHz);
    
	// LED pinの設定　B11
	GPIOB->CFGHR &= ~(0xf<<(4 * (11 - 8)));
	GPIOB->CFGHR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4 * (11 - 8));

	ssd1306_spi_init();		// i2c Setup
	ssd1306_init();			// SSD1306 Setup
	ssd1306_setbuf(0);		// Clear Screen
	ssd1306_drawstr_sz(0, 32, title1, 1, fontsize_8x8);
	ssd1306_refresh();
	check_function();
	Delay_Ms( 2000 );
	if (function == cw_decode) {
		setupDecoder();
	} else {
		setupFFT();
	}
	ssd1306_setbuf(0);	// Clear Screen
	ssd1306_refresh();
}

//=========================================================
//	main procedure
//=========================================================
int main()
{
	setup();				// gpio etc Setup;
	if (function == cw_decode) {
		morseDecoder();
	} else {
		dispFFT();
	}
}

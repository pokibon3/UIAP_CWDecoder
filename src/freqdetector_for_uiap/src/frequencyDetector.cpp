#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
//#include <math.h>
#include "ch32v003_GPIO_branchless.h"
#include "common.h"
#include "frequencyDetector.h"
#include "fix_fft.h"
#include "st7735.h"
#include "ch32v003fun.h"

static char title1[] = "Freq.Detector";
static char title2[] = " Version 1.0u";
static int8_t vReal[SAMPLES];
static int8_t vImag[SAMPLES];

int fd_setup()
{
    sampling_period_us = 900000L / SAMPLING_FREQUENCY; 

	tft_init();
	tft_fill_rect(0, 0, ST7735_WIDTH, ST7735_HEIGHT, BLACK);
	tft_set_color(WHITE);
	tft_set_cursor(0, 10);
	tft_print(title1, FONT_SCALE_16X16);
	tft_set_color(BLUE);
	tft_set_cursor(0, 50);
	tft_print(title2, FONT_SCALE_16X16);
	Delay_Ms( 2000 );	

    return 0;
}

//==================================================================
//  adc and fft for freq counter
//==================================================================
int freqDetector() {
	uint16_t peakFrequency = 0;
	uint16_t oldFreequency = 0;
	char buf[16];

	tft_fill_rect(0, 0, ST7735_WIDTH, ST7735_HEIGHT, BLACK);
	tft_draw_rect(0, 0, ST7735_WIDTH, ST7735_HEIGHT - 8, BLUE);
//	tft_draw_line(0, 19, ST7735_WIDTH, 19, BLUE);
	tft_set_cursor(0, 73);
	tft_set_color(BLUE);
	tft_print("0Hz      1KHz       2KHz", FONT_SCALE_8X8);

	while(1) {
		uint16_t ave = 0;
		uint8_t  val = 0;
		unsigned long t = 0;
TEST_HIGH
		// input audio
		for (int i = 0; i < SAMPLES; i++) {
			t = micros();
			val = (uint8_t)(GPIO_analogRead(GPIO_Ain0_A2) >> 2);
			ave += val;
			vImag[i] = val;
			while ((micros() - t) < sampling_period_us);
		}
TEST_LOW
		ave = ave / SAMPLES;
		//printf("ave = %d\n", ave);
		for (int i = 0; i < SAMPLES; i++) {
			vReal[i] = (int8_t)(vImag[i] - ave);
			vImag[i] = 0; // Imaginary partは0に初期化
		}
//TEST_HIGH
		// FFT
  		fix_fft((char *)vReal, (char *)vImag, 7, 0); // SAMPLES = 256なので、log2(SAMPLES) = 8
//TEST_HIGH
  		// Magnitude Calculation
		for (int i = 0; i < SAMPLES / 2; i++) {
			vReal[i] = abs(vReal[i]) + abs(vImag[i]); // Magnitude calculation without sqrt
			//vReal[i] = sqrt(vReal[i] * vReal[i] + vImag[i] * vImag[i]);
		}
		// draw FFT result
		uint8_t maxIndex = 0;
		uint8_t maxValue = 0;
		tft_fill_rect(1, 19, ST7735_WIDTH - 2, 51, BLACK);
		tft_draw_line(64,  1,  64, 71, DARKBLUE); // 1.0kHz line
		tft_draw_line(128, 1, 128, 71, DARKBLUE); // 2.0kHz line

		for (int i = 1; i < (((SAMPLES / 2) < 52) ? SAMPLES /2 : 52); i++) {
			int8_t val = (vReal[i] * SCALE < 50) ? vReal[i] * SCALE : 50;
			tft_draw_line(i * 3,     70       , i * 3,     70 - val, WHITE);
			tft_draw_line(i * 3 + 1, 70 - val , i * 3 + 2, 70 - val, WHITE);
			tft_draw_line(i * 3 + 3, 70       , i * 3 + 3, 70 - val, WHITE);
			if (vReal[i] > maxValue) {
				maxValue = vReal[i];
				maxIndex = i;
			}
		}
		// disp freqeuncy
		peakFrequency = (SAMPLING_FREQUENCY / SAMPLES) * maxIndex;
		if (peakFrequency != oldFreequency) {
			if (maxValue >= 4 ) {
				mini_snprintf(buf, sizeof(buf), "%4dHz", peakFrequency + SAMPLING_FREQUENCY / SAMPLES / 2);
			} else {
//				strcpy(buf, "   0Hz");
				strcpy(buf, "    Hz");
			}
			tft_set_cursor ((6 - strlen(buf)) * 12 + 82, 3);
			tft_set_color(YELLOW);
			tft_fill_rect(80, 1, ST7735_WIDTH - 82, 18, BLACK);
			tft_print(buf, FONT_SCALE_16X16);
			oldFreequency = peakFrequency;
		}
//TEST_LOW
	}
	return 0;
}

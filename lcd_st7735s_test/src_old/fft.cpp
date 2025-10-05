//
//	freqcounter Ver 1.1
//
//  2024.07.31 New Create
//
#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#include "fix_fft.h"
#include "ch32v003fun.h"
#include "ch32v003_GPIO_branchless.h"
#include  "adc.h"
#include  "gpio.h"

// what type of OLED - uncomment just one
//#define SSD1306_64X32
//#define SSD1306_128X32
#define SSD1306_128X64
#define SCALE 3
#include "ssd1306_spi.h"
#include "ssd1306.h"

#define micros() (SysTick->CNT / DELAY_US_TIME)
#define microsx18() (SysTick->CNT)
#define millis() (SysTick->CNT / DELAY_MS_TIME)

#define SAMPLES 256 // 2の累乗である必要があります
#define SAMPLING_FREQUENCY 6000

uint16_t sampling_period_us;
int8_t vReal[SAMPLES];
int8_t vImag[SAMPLES];

// function prototype (declaration), definition in "ch32v003fun.c"
extern "C" int mini_snprintf(char* buffer, unsigned int buffer_len, const char *fmt, ...);

void setupFFT(void)
{
	adc_init();
	sampling_period_us = 1000000 / SAMPLING_FREQUENCY;
}

void dispFFT(void)
{
	char buf[16];

	SystemInit();			// ch32v003 SETUP
	setupFFT();
	while(1) {
		uint16_t ave = 0;
		uint8_t  val = 0;
		ssd1306_setbuf(0);	// Clear Screen
		for (int i = 0; i < SAMPLES; i++) {
			uint32_t t = micros();
			val = (uint8_t)(adc_get() >> 4);
						
			ave += val;
			vImag[i] = val;
			while ((micros() - t) < sampling_period_us);
		}
		ave = ave / SAMPLES;
		//printf("ave = %d\n", ave);
		for (int i = 0; i < SAMPLES; i++) {
			vReal[i] = (int8_t)(vImag[i] - ave);
			vImag[i] = 0; // Imaginary partは0に初期化
		}

  		fix_fft((char *)vReal, (char *)vImag, 8, 0); // SAMPLES = 32なので、log2(SAMPLES) = 5

  		/* Magnitude Calculation */
		for (int i = 0; i < SAMPLES / 2; i++) {
			//vReal[i] = abs(vReal[i]) + abs(vImag[i]); // Magnitude calculation without sqrt
			vReal[i] = sqrt(vReal[i] * vReal[i] + vImag[i] * vImag[i]);
		}
		/* Find Peak */
		uint8_t maxIndex = 0;
		uint8_t maxValue = 0;
		for (int i = 8; i < SAMPLES / 2; i++) {
			int8_t val = (vReal[i] * SCALE < 48) ? vReal[i] * SCALE : 48;  
			ssd1306_drawFastVLine(i, 63 - val + 1, val + 1, 1);
			if (vReal[i] > maxValue) {
				maxValue = vReal[i];
				maxIndex = i;
			}
		}
		// disp freqeuncy
		int peakFrequency = (SAMPLING_FREQUENCY / SAMPLES) * maxIndex;
 		if (maxValue >= 4 ) {
//			uint16_to_string(peakFrequency + 12, buf, sizeof(buf));
			mini_snprintf(buf, sizeof(buf), " %4dHz", peakFrequency + 12);
		    //printf("%d\n", peakFrequency); // Print out what frequency is the most dominant.
  		} else {
			strcpy(buf, "    0Hz");
		}
//		strcat(buf, "Hz");
		ssd1306_drawstr_sz((6 - strlen(buf)) * 16 + 16, 1, buf, 1, fontsize_16x16);
		ssd1306_drawFastHLine(  0,   0,  128, 1);
		ssd1306_drawFastHLine(  0,  16,  128, 1);
		ssd1306_drawFastHLine(  0,  63,  128, 1);
		ssd1306_drawFastVLine(  0,   0,   64, 1);
		ssd1306_drawFastVLine(127,   0,   64, 2);

		ssd1306_refresh();
	}
}

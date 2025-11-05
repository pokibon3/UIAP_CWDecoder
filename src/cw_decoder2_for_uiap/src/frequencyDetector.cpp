//
//	Frequency Detector functions
//
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "ch32v003_GPIO_branchless.h"
#include "frequencyDetector.h"
#include "fix_fft.h"
#include "st7735.h"
#include "ch32v003fun.h"

#define FD_SAMPLING_FREQUENCY 6000	// Hz	+10%

// 画面レイアウト用マクロ
#define FFT_AREA_Y_TOP      19
#define FFT_AREA_Y_BOTTOM   70
#define FFT_AREA_HEIGHT     (FFT_AREA_Y_BOTTOM - FFT_AREA_Y_TOP + 1)
#define FFT_LABEL_Y         73

// タイトル文字列
static char title1[]   = "Freq.Detector";
static char title2[]   = "  for UIAP   ";
static char title3[]   = " Version 1.2";


//==================================================================
//	chack switch
//==================================================================
static int check_sw()
{
    int ret = 0;
	int val = check_input();

	if (val == 1) {
		Delay_Ms(300);
        ret = 1;
	}

    return ret;
}


//==================================================================
//	freq_detector setup
//==================================================================
int fd_setup()
{
    sampling_period_us = 900000L / FD_SAMPLING_FREQUENCY;
    //sampling_period_us = 1000000L / FD_SAMPLING_FREQUENCY;

	// display title
	tft_fill_rect(0, 0, ST7735_WIDTH, ST7735_HEIGHT, BLACK);

	tft_set_color(BLUE);
	tft_set_cursor(0, 10);
	tft_print(title1, FONT_SCALE_16X16);

	tft_set_color(RED);
	tft_set_cursor(0, 30);
	tft_print(title2, FONT_SCALE_16X16);

	tft_set_cursor(0, 50);
	tft_set_color(GREEN);
	tft_print(title3, FONT_SCALE_16X16);

	tft_set_color(WHITE);

	Delay_Ms( 1000 );

    return 0;
}

//==================================================================
//  adc and fft for freq counter
//==================================================================
int freqDetector(int8_t *vReal, int8_t *vImag) 
{
	uint16_t peakFrequency = 0;
	uint16_t oldFreequency = 0;
	char buf[16];

	tft_fill_rect(0, 0, ST7735_WIDTH, ST7735_HEIGHT, BLACK);
	tft_draw_rect(0, 0, ST7735_WIDTH, ST7735_HEIGHT - 8, BLUE);

	tft_set_cursor(0, FFT_LABEL_Y);
	tft_set_color(BLUE);
	tft_print("0Hz      1KHz       2KHz", FONT_SCALE_8X8);

	while(1) {
		uint16_t ave = 0;
		uint8_t  val = 0;
		unsigned long t = 0;

		if (check_sw() == 1) {
            break;
        }
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
		tft_fill_rect(1, FFT_AREA_Y_TOP, ST7735_WIDTH - 2, FFT_AREA_HEIGHT, BLACK);
		tft_draw_line(64,  1,  64, 71, DARKBLUE); // 1.0kHz line
		tft_draw_line(128, 1, 128, 71, DARKBLUE); // 2.0kHz line

		for (int i = 1; i < (((SAMPLES / 2) < 52) ? SAMPLES /2 : 52); i++) {
			int8_t val = (vReal[i] * SCALE < 50) ? vReal[i] * SCALE : 50;
			tft_draw_line(i * 3,     FFT_AREA_Y_BOTTOM       , i * 3,     FFT_AREA_Y_BOTTOM - val, WHITE);
			tft_draw_line(i * 3 + 1, FFT_AREA_Y_BOTTOM - val , i * 3 + 2, FFT_AREA_Y_BOTTOM - val, WHITE);
			tft_draw_line(i * 3 + 3, FFT_AREA_Y_BOTTOM       , i * 3 + 3, FFT_AREA_Y_BOTTOM - val, WHITE);
			if (vReal[i] > maxValue) {
				maxValue = vReal[i];
				maxIndex = i;
			}
		}
		// disp freqeuncy
		peakFrequency = (FD_SAMPLING_FREQUENCY / SAMPLES) * maxIndex;
		if (peakFrequency != oldFreequency) {
			if (maxValue >= 4 ) {
				mini_snprintf(buf, sizeof(buf), "%4dHz", peakFrequency + FD_SAMPLING_FREQUENCY / SAMPLES / 2);
			} else {
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

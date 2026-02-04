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
#include "ch32v003fun.h"

#define FD_SAMPLING_FREQUENCY 6000	// Hz	+10%

// 画面レイアウト用マクロ
#define FFT_FRAME_HEIGHT    (TFT_HEIGHT - 8)
#define FFT_AREA_Y_TOP      ((TFT_HEIGHT * 19) / 80)
#define FFT_AREA_Y_BOTTOM   ((TFT_HEIGHT * 70) / 80)
#define FFT_AREA_HEIGHT     (FFT_AREA_Y_BOTTOM - FFT_AREA_Y_TOP + 1)
#define FFT_LABEL_Y         (FFT_FRAME_HEIGHT + 1)

// タイトル文字列
//static char title1[]   = "Freq.Detector";
//static char title2[]   = "  for UIAP   ";
//static char title3[]   = " Version 1.0";


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
    tim1_pwm_stop();
    sampling_period_us = 900000L / FD_SAMPLING_FREQUENCY;
    //sampling_period_us = 1000000L / FD_SAMPLING_FREQUENCY;

	// display title
	tft_fill_rect(0, 0, TFT_WIDTH, TFT_HEIGHT, BLACK);

/*
	tft_set_color(BLUE);
	tft_set_cursor(0, 10);
	tft_print(title1, FONT_SCALE_16X16);

	tft_set_color(RED);
	tft_set_cursor(0, 30);
	tft_print(title2, FONT_SCALE_16X16);

	tft_set_cursor(0, 50);
	tft_set_color(GREEN);
	tft_print(title3, FONT_SCALE_16X16);
	Delay_Ms( 1000 );
*/
	tft_set_color(WHITE);


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

	tft_fill_rect(0, 0, TFT_WIDTH, TFT_HEIGHT, BLACK);
	tft_draw_rect(0, 0, TFT_WIDTH, FFT_FRAME_HEIGHT, BLUE);

	tft_set_cursor((TFT_WIDTH - (uint16_t)(strlen("0Hz      1KHz       2KHz") * 8)) / 2, FFT_LABEL_Y);
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
			val = (uint8_t)((GPIO_analogRead(GPIO_Ain0_A2) >> 2) & 0xFF); // 0-255
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
		uint8_t fft_bins = ((SAMPLES / 2) < 52) ? (SAMPLES / 2) : 52;
		uint16_t bin_step = (TFT_WIDTH - 1) / fft_bins;
		if (bin_step < 3) bin_step = 3;
		uint16_t plot_width = bin_step * fft_bins;
		if (plot_width > (TFT_WIDTH - 2)) {
			plot_width = TFT_WIDTH - 2;
		}
		uint16_t plot_left = (TFT_WIDTH - plot_width) / 2;
		uint16_t line1_x = plot_left + (plot_width * 2) / 5;
		uint16_t line2_x = plot_left + (plot_width * 4) / 5;
		uint16_t value_area_x = plot_left + (plot_width / 2);
		uint16_t value_text_base = value_area_x + 2;

		tft_fill_rect(1, FFT_AREA_Y_TOP, TFT_WIDTH - 2, FFT_AREA_HEIGHT, BLACK);
		tft_draw_line(line1_x, 1, line1_x, FFT_FRAME_HEIGHT - 1, DARKBLUE); // 1.0kHz line
		tft_draw_line(line2_x, 1, line2_x, FFT_FRAME_HEIGHT - 1, DARKBLUE); // 2.0kHz line

		for (int i = 1; i < fft_bins; i++) {
			int16_t val = vReal[i] * SCALE;
			if (val > (int16_t)(FFT_AREA_HEIGHT - 1)) val = (int16_t)(FFT_AREA_HEIGHT - 1);
			uint16_t x = plot_left + (i * bin_step);
			uint16_t bar_width = (bin_step > 2) ? (bin_step - 1) : 1;
			for (uint16_t dx = 0; dx < bar_width; dx++) {
				tft_draw_line(x + dx, FFT_AREA_Y_BOTTOM, x + dx, FFT_AREA_Y_BOTTOM - val, WHITE);
			}
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
			tft_set_cursor((6 - strlen(buf)) * 12 + value_text_base, 3);
			tft_set_color(YELLOW);
			tft_fill_rect(value_area_x, 1, (plot_left + plot_width) - value_area_x - 2, 18, BLACK);
			tft_print(buf, FONT_SCALE_16X16);
			
			oldFreequency = peakFrequency;
		}
//TEST_LOW
	}
	return 0;
}

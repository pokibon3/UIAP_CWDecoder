//
//	freq detector Ver 1.0u
//
//  2024.08.12 New Create
//  2024.08.21 integer version
//  2025.09.30 port to UIAPduino Pro Micro(CH32V003)
//
//  Hardware Connections
//
//	UIAP	CH32V003  	SSD1306(SPI)	SW		MIC    	 ETC.
//	10	    PD0            DC
//	 8      PC6            MOSI
//   9      PC7            RES
//   7      PC5            SCK
//   5      PC3            CS
//	A1	    PA1                         SW1
//	A2      PC4                         SW2
//  A3      PD2							SW3
//	A0		PA2									OUT
//  A6		PD6											TEST
//	3.3V                  3.3V           		VCC(Via 78L33 3.3V)
//  GND                    GND          		GND
//

//#define SERIAL_OUT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <math.h>
#include "fix_fft.h"
#include "ch32v003fun.h"

#include "st7735.h"

#define GPIO_ADC_MUX_DELAY 100
#define GPIO_ADC_sampletime GPIO_ADC_sampletime_43cy
#define SCALE 3
#include "ch32v003_GPIO_branchless.h"

#define micros() (SysTick->CNT / DELAY_US_TIME)
#define millis() (SysTick->CNT / DELAY_MS_TIME)

//#define LED_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 5)
#define SW1_PIN GPIOv_from_PORT_PIN(GPIO_port_A, 1)		// for uiap
#define SW2_PIN GPIOv_from_PORT_PIN(GPIO_port_C, 4)		// for uiap
#define SW3_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 2)
#define ADC_PIN GPIOv_from_PORT_PIN(GPIO_port_A, 2)		// for uiap
#define LED_PIN GPIOv_from_PORT_PIN(GPIO_port_C, 0)		// for uiap
#define UART_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 5)
#define TEST_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 6)

#define TEST_HIGH			GPIO_digitalWrite(TEST_PIN, high);
#define TEST_LOW			GPIO_digitalWrite(TEST_PIN, low);

#define SAMPLES 128
#define SAMPLING_FREQUENCY 6000	// Hz	+10%

uint16_t sampling_period_us;
int8_t vReal[SAMPLES];
int8_t vImag[SAMPLES];
static char title1[] = "Freq.Detector";
static char title2[] = " Version 1.0u";

// function prototype (declaration), definition in "ch32v003fun.c"
extern "C" int mini_snprintf(char* buffer, unsigned int buffer_len, const char *fmt, ...);


//==================================================================
//  adc and fft for freq counter
//==================================================================
int freqCounter() {
	uint16_t peakFrequency = 0;
	uint16_t oldFreequency = 0;
	char buf[16];

	tft_draw_rect(0, 0, ST7735_WIDTH, ST7735_HEIGHT, BLUE);
	tft_draw_line(0, 19, ST7735_WIDTH, 19, BLUE);

	while(1) {
		uint16_t ave = 0;
		uint8_t  val = 0;
		unsigned long t = 0;
TEST_HIGH
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
  		fix_fft((char *)vReal, (char *)vImag, 7, 0); // SAMPLES = 256なので、log2(SAMPLES) = 8
//TEST_HIGH
  		/* Magnitude Calculation */
		for (int i = 0; i < SAMPLES / 2; i++) {
			vReal[i] = abs(vReal[i]) + abs(vImag[i]); // Magnitude calculation without sqrt
			//vReal[i] = sqrt(vReal[i] * vReal[i] + vImag[i] * vImag[i]);
		}
		/* Find Peak */
		uint8_t maxIndex = 0;
		uint8_t maxValue = 0;
		tft_fill_rect(1, 20, ST7735_WIDTH - 2, 59, BLACK);
		for (int i = 1; i < SAMPLES / 2; i++) {
			int8_t val = (vReal[i] * SCALE < 58) ? vReal[i] * SCALE : 58;
			tft_draw_line(i * 2,     78, i * 2,     78 - val, WHITE);
			tft_draw_line(i * 2 + 1, 78, i * 2 + 1, 78 - val, WHITE);
			if (vReal[i] > maxValue) {
				maxValue = vReal[i];
				maxIndex = i;
			}
		}
		// disp freqeuncy
		peakFrequency = (SAMPLING_FREQUENCY / SAMPLES) * maxIndex;
		if (peakFrequency != oldFreequency) {
			if (maxValue >= 4 ) {
				mini_snprintf(buf, sizeof(buf), " %4dHz", peakFrequency + SAMPLING_FREQUENCY / SAMPLES / 2);
			} else {
				strcpy(buf, "    0Hz");
			}
			tft_set_cursor ((6 - strlen(buf)) * 16 + 48, 3);
			tft_set_color(YELLOW);
			tft_fill_rect(1, 1, ST7735_WIDTH - 2, 18, BLACK);
			tft_print(buf, FONT_SCALE_16X16);
			oldFreequency = peakFrequency;
		}
//TEST_LOW
	}
}

//==================================================================
//	setup
//==================================================================
void setup()
{
    // 各GPIOの有効化
    GPIO_port_enable(GPIO_port_A);
    GPIO_port_enable(GPIO_port_C);
    GPIO_port_enable(GPIO_port_D);
    // 各ピンの設定
    GPIO_pinMode(SW1_PIN, GPIO_pinMode_I_pullUp, GPIO_Speed_10MHz);
    GPIO_pinMode(SW2_PIN, GPIO_pinMode_I_pullUp, GPIO_Speed_10MHz);
    GPIO_pinMode(SW3_PIN, GPIO_pinMode_I_pullUp, GPIO_Speed_10MHz);
    GPIO_pinMode(ADC_PIN, GPIO_pinMode_I_analog, GPIO_Speed_10MHz);

	GPIO_pinMode(LED_PIN, GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
	GPIO_digitalWrite(LED_PIN, low);

	GPIO_pinMode(TEST_PIN, GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
	GPIO_digitalWrite(TEST_PIN, low);

	GPIO_pinMode(UART_PIN, GPIO_pinMode_O_pushPullMux, GPIO_Speed_10MHz);
	RCC->APB2PCENR |= RCC_APB2Periph_AFIO;

	GPIO_ADCinit();
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
}

//==================================================================
//	main
//==================================================================
int main()
{
	SystemInit();			// ch32v003 Setup
	setup();				// gpio Setup;
	freqCounter();			// run freq counter
}

//
//	freqcounter Ver 1.1u
//
//  2024.07.31 New Create
//  2025.09.30 port to UIAPduino Pro(CH32V003)
//
//  Hardware Connections
//
//	UIAP	CH32V003  	SSD1306(SPI)	SW1		SW2		MIC    	 
//	10	    PD0            DC
//	 8      PC6            MOSI
//   9      PC7            RES
//   7      PC5            SCK
//   5      PC3            CS
//	 3	    PC1                         SW1
//	 4      PC2                                 SW2
//	A0		PA2											OUT
//	5V                     5V           				VCC(Via 78L33 3.3V)
//  GND                    GND          				GND
//
#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#include "fix_fft.h"
#include "ch32v003fun.h"

#define micros() (SysTick->CNT / DELAY_US_TIME)
#define millis() (SysTick->CNT / DELAY_MS_TIME)

// what type of OLED - uncomment just one
#define SSD1306_128X64
#define SCALE 3

#define GPIO_ADC_MUX_DELAY 100
#define GPIO_ADC_sampletime GPIO_ADC_sampletime_43cy
#include "ch32v003_GPIO_branchless.h"
#include "ssd1306_spi.h"
#include "ssd1306.h"

#define SW1_PIN GPIOv_from_PORT_PIN(GPIO_port_C, 1)		// for uiap
#define SW2_PIN GPIOv_from_PORT_PIN(GPIO_port_C, 2)		// for uiap
#define ADC_PIN GPIOv_from_PORT_PIN(GPIO_port_A, 2)		// for uiap

#define SAMPLES 256 // 2の累乗である必要があります
#define SAMPLING_FREQUENCY 6000

uint16_t sampling_period_us;
int8_t vReal[SAMPLES];
int8_t vImag[SAMPLES];

// function prototype (declaration), definition in "ch32v003fun.c"
extern "C" int mini_snprintf(char* buffer, unsigned int buffer_len, const char *fmt, ...);
char title1[] = "FrequencyCounter";
char title2[] = "  Version 1.1u  ";

//
// setup function
//
void setup()
{
    // 各GPIOの有効化
    GPIO_port_enable(GPIO_port_A);
    GPIO_port_enable(GPIO_port_C);
    GPIO_port_enable(GPIO_port_D);
    // 各ピンの設定
    GPIO_pinMode(SW1_PIN, GPIO_pinMode_I_pullDown, GPIO_Speed_10MHz);
    GPIO_pinMode(SW2_PIN, GPIO_pinMode_I_pullDown, GPIO_Speed_10MHz);
    GPIO_pinMode(ADC_PIN, GPIO_pinMode_I_analog, GPIO_Speed_10MHz);

	GPIO_ADCinit();
	sampling_period_us = 1000000 / SAMPLING_FREQUENCY;
	ssd1306_spi_init();		// spi Setup
	ssd1306_init();			// SSD1306 Setup
	ssd1306_setbuf(0);		// Clear Screen
	ssd1306_drawstr_sz(0, 24, title1, 1, fontsize_8x8);		// print title
	ssd1306_drawstr_sz(0, 40, title2, 1, fontsize_8x8);		// print tittle
	ssd1306_refresh();
}


//
//  adc and fft for freq counter
//
int freqCounter() {
	char buf[16];

	while(1) {
		uint16_t ave = 0;
		uint8_t  val = 0;
		ssd1306_setbuf(0);	// Clear Screen
		for (int i = 0; i < SAMPLES; i++) {
			unsigned long t = micros();
			val = (uint8_t)(GPIO_analogRead(GPIO_Ain0_A2) >> 2);
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
			vReal[i] = abs(vReal[i]) + abs(vImag[i]); // Magnitude calculation without sqrt
			//vReal[i] = sqrt(vReal[i] * vReal[i] + vImag[i] * vImag[i]);
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
			mini_snprintf(buf, sizeof(buf), " %4dHz", peakFrequency + 12);
  		} else {
			strcpy(buf, "    0Hz");
		}
		ssd1306_drawstr_sz((6 - strlen(buf)) * 16 + 16, 1, buf, 1, fontsize_16x16);
		ssd1306_drawFastHLine(  0,   0,  128, 1);
		ssd1306_drawFastHLine(  0,  16,  128, 1);
		ssd1306_drawFastHLine(  0,  63,  128, 1);
		ssd1306_drawFastVLine(  0,   0,   64, 1);
		ssd1306_drawFastVLine(127,   0,   64, 2);

		ssd1306_refresh();
	}
}

//
// main function
//
int main()
{
	uint8_t exitStatus;
	uint8_t mode;

	SystemInit();			// ch32v003 Setup
	setup();				// gpio Setup;
	Delay_Ms( 2000 );
	freqCounter();			// run freq counter
}

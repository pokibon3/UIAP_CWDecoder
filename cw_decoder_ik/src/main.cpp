//
//	CW Decoder Ver 1.0u
//
//  2024.08.12 New Create
//  2024.08.21 integer version
//  2025.09.30 port to UIAPduino Pro Micro(CH32V003)
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
#define OLED_SPI
#define SERIAL_OUT

#include <stdio.h>
#include <stdlib.h> 
//#include <math.h>
#include "goertzel.h"
#include "docode.h"
#include "ch32v003fun.h"

// what type of OLED - uncomment just one
//#define SSD1306_64X32
//#define SSD1306_128X32
#define SSD1306_128X64
#ifdef OLED_SPI
#include "ssd1306_spi.h"
#else
#include "ssd1306_i2c.h"
#endif
#include "ssd1306.h"

#define GPIO_ADC_MUX_DELAY 100
#define GPIO_ADC_sampletime GPIO_ADC_sampletime_43cy
#include "ch32v003_GPIO_branchless.h"

#define micros() (SysTick->CNT / DELAY_US_TIME)
#define millis() (SysTick->CNT / DELAY_MS_TIME)

//#define LED_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 5)
#define SW1_PIN GPIOv_from_PORT_PIN(GPIO_port_C, 1)		// for uiap
#define SW2_PIN GPIOv_from_PORT_PIN(GPIO_port_C, 2)		// for uiap
#define ADC_PIN GPIOv_from_PORT_PIN(GPIO_port_A, 2)		// for uiap
//#define LED_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 5)
#define LED_PIN GPIOv_from_PORT_PIN(GPIO_port_C, 0)		// for uiap
#define UART_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 5)

#define SAMPLES 48 
#define SAMPLING_FREQUENCY 8000

// function prototype (declaration), definition in "ch32v003fun.c"
extern "C" int mini_snprintf(char* buffer, unsigned int buffer_len, const char *fmt, ...);

static uint16_t sampling_period_us;
static int16_t 	morseData[SAMPLES];

static char title1[]   = "CW Decoder V1.1u";
static uint16_t magnitudelimit = 100;
static uint16_t magnitudelimit_low = 100;
static uint16_t realstate = low;
static uint16_t realstatebefore = low;
static uint16_t filteredstate = low;
static uint16_t filteredstatebefore = low;
static uint32_t starttimehigh;
static uint32_t highduration;
static uint32_t lasthighduration;
static uint32_t hightimesavg;
static uint32_t startttimelow;
static uint32_t lowduration;
static uint32_t laststarttime = 0;
#define  nbtime 	6  /// ms noise blanker         

static char code[20];
static uint16_t stop = low;
static uint16_t wpm;

static char		sw = MODE_US;
static int16_t 	speed = 1;

char *tone[] = {
	" 600",
	" 800",
	"1000"
};

/////////////////////////////////////
// here we update the upper line   //
// with the speed.                 //
/////////////////////////////////////
void updateinfolinelcd()
{
	char buf[17];
	char *mode;
	uint16_t w;

	if (wpm < 10) {
		w = 0;
	} else {
		w = wpm;
	}
	if (sw == 0) {
		mode = "US";
	} else {
		mode = "JP";
	}
	mini_snprintf(buf, sizeof(buf), "%2d WPM[%s%sHz]", w, mode, tone[speed]);
	ssd1306_drawstr_sz(0 ,2 , buf, 1, fontsize_8x8);
	ssd1306_drawFastHLine(0, 13, 128, 1);
	ssd1306_refresh();
}

const int colums = 10; /// have to be 16 or 20
const int rows = 4;  /// have to be 2 or 4

int lcdindex = 0;
uint8_t line1[colums];
uint8_t line2[colums];
uint8_t lastChar = 0;
/////////////////////////////////////
// print the ascii code to the lcd //
// one a time so we can generate   //
// special letters                 //
/////////////////////////////////////
#define FONT_WIDTH 12
void printascii(int16_t asciinumber)
{
	int fail = 0;

	if (asciinumber == 0) return;
	if (lastChar == 32 && asciinumber == 32) return;
#ifdef SERIAL_OUT
	printf("%c", asciinumber);
#endif
	if (rows == 4 and colums == 20)fail = -4; /// to fix the library problem with 4*16 display http://forum.arduino.cc/index.php/topic,14604.0.html
	if (lcdindex > colums-1){
		lcdindex = 0;
		if (rows == 4){
			for (int i = 0; i <= colums - 1 ; i++){
				ssd1306_drawchar_sz(i * FONT_WIDTH ,(rows - 3) * 16 , line2[i], 1, fontsize_16x16);
				line2[i]=line1[i];
			}
		}
		for (int i = 0; i <= colums-1 ; i++){
			ssd1306_drawchar_sz((i + fail)* FONT_WIDTH ,(rows - 2) * 16 , line1[i], 1, fontsize_16x16);
			ssd1306_drawchar_sz((i + fail)* FONT_WIDTH ,(rows - 1) * 16 , 32, 1, fontsize_16x16);
		}
 	}
	line1[lcdindex]=asciinumber;
	ssd1306_drawchar_sz((lcdindex + fail)* FONT_WIDTH ,(rows - 1) * 16 , asciinumber, 1, fontsize_16x16);
	lcdindex += 1;
	ssd1306_refresh();
	lastChar = asciinumber;
}

void check_input() 
{
	if (!GPIO_digitalRead(SW2_PIN)) {
		speed -= 1;
		if (speed < 0) speed = 2;
		setSpeed(speed);
		updateinfolinelcd();
//		printf("speed + %d\n", speed + 1);
		Delay_Ms(300);
	} else if (!GPIO_digitalRead(SW1_PIN)) {
		sw ^= 1;
		updateinfolinelcd();
		Delay_Ms(300);
	}
}


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
    
	GPIO_pinMode(LED_PIN, GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
	GPIO_digitalWrite(LED_PIN, low);

	GPIO_pinMode(UART_PIN, GPIO_pinMode_O_pushPullMux, GPIO_Speed_10MHz);
	RCC->APB2PCENR |= RCC_APB2Periph_AFIO;

#ifdef OLED_SPI
	ssd1306_spi_init();		// i2c Setup
#else
	ssd1306_i2c_init();		// i2c Setup
#endif
	ssd1306_init();			// SSD1306 Setup
	ssd1306_setbuf(0);		// Clear Screen
	ssd1306_drawstr_sz(0, 32, title1, 1, fontsize_8x8);
	ssd1306_refresh();
	GPIO_ADCinit();
	initGoertzel(speed);
	sampling_period_us = 1000000 / SAMPLING_FREQUENCY;
	for (int i = 0; i < colums; i++){
    	line1[i] = 32;
		line2[i] = 32;
	}           
}

int main()
{
	int32_t magnitude;

	SystemInit();			// ch32v003 sETUP
	setup();				// gpio Setup;
	Delay_Ms( 2000 );
	ssd1306_setbuf(0);	// Clear Screen
	ssd1306_refresh();
	while(1) {
 	  	int16_t ave = 0;

		check_input();
		for (int i = 0; i < SAMPLES; i++) {
			uint32_t t = micros();
			morseData[i] = GPIO_analogRead(GPIO_Ain0_A2);
			ave += morseData[i];
			while ((micros() - t) < sampling_period_us);
		}
		ave = ave / SAMPLES;
		for (int i = 0; i < SAMPLES; i++) {
			morseData[i] -= ave;
		}
		// calc goertzel
		magnitude = goertzel(morseData, SAMPLES) / 400;
#ifndef SERIAL_OUT
		printf("mag = %d\n", magnitude);
#endif
		/////////////////////////////////////////////////////////// 
		// here we will try to set the magnitude limit automatic //
		///////////////////////////////////////////////////////////
  		if (magnitude > magnitudelimit_low){
    		magnitudelimit = (magnitudelimit +((magnitude - magnitudelimit)/6));  /// moving average filter
  		}
  		if (magnitudelimit < magnitudelimit_low) {
			magnitudelimit = magnitudelimit_low;
		}

		////////////////////////////////////
		// now we check for the magnitude //
		////////////////////////////////////
		if(magnitude > magnitudelimit*0.6) {  // just to have some space up 
     		realstate = high; 
		} else {
    		realstate = low; 
		}

		///////////////////////////////////////////////////// 
		// here we clean up the state with a noise blanker //
		/////////////////////////////////////////////////////
		if (realstate != realstatebefore){
			laststarttime = millis();
		}
		if ((millis()-laststarttime)> nbtime) {
			if (realstate != filteredstate) {
				filteredstate = realstate;
			}
		}

		////////////////////////////////////////////////////////////
		// Then we do want to have some durations on high and low //
		////////////////////////////////////////////////////////////
		if (filteredstate != filteredstatebefore) {
			if (filteredstate == high) {
				starttimehigh = millis();
				lowduration = (millis() - startttimelow);
			}
			if (filteredstate == low) {
				startttimelow = millis();
				highduration = (millis() - starttimehigh);
				if (highduration < (2*hightimesavg) || hightimesavg == 0) {
					hightimesavg = (highduration+hightimesavg+hightimesavg) / 3;     // now we know avg dit time ( rolling 3 avg)
				}
				if (highduration > (5*hightimesavg) ) {
					hightimesavg = highduration+hightimesavg;     // if speed decrease fast ..
				}
			}
		}

		///////////////////////////////////////////////////////////////
		// now we will check which kind of baud we have - dit or dah //
		// and what kind of pause we do have 1 - 3 or 7 pause        //
		// we think that hightimeavg = 1 bit                         //
		///////////////////////////////////////////////////////////////
		if (filteredstate != filteredstatebefore){
			stop = low;
			if (filteredstate == low){  //// we did end a HIGH
				if (highduration < (hightimesavg*2) && highduration > (hightimesavg*0.6)){ /// 0.6 filter out false dits
					strcat(code,".");
//					printf(".");
				}
				if (highduration > (hightimesavg*2) && highduration < (hightimesavg*6)){ 
					strcat(code,"-");
//					printf("-");
					wpm = (wpm + (1200/((highduration)/3)))/2;  //// the most precise we can do ;o)
				}
			}
		}

		if (filteredstate == high){  //// we did end a LOW
	
			int16_t lacktime = 8;
			if(wpm > 25)lacktime=10; ///  when high speeds we have to have a little more pause before new letter or new word 
			if(wpm > 30)lacktime=12;
			if(wpm > 35)lacktime=15;
		
			if (lowduration > ((hightimesavg * (2 * lacktime) / 10)) && lowduration < (hightimesavg * (5 * lacktime)) / 10){ // letter space
				if (strlen(code) > 0) {
					printascii(docode(code, &sw));
					code[0] = '\0';
//	 				printf("/");
				}
			}
			if (lowduration >= (hightimesavg * (5 * lacktime)) / 10){ // word space
				if (strlen(code) > 0) {
					printascii(docode(code, &sw));
					code[0] = '\0';
					printascii(32);
	//				printf(" ");
	//				printf("\n");
				}
			}
		}

		//////////////////////////////
		// write if no more letters //
		//////////////////////////////
		if ((millis() - startttimelow) > (highduration * 6) && stop == low){
			printascii(docode(code, &sw));
			code[0] = '\0';
			stop = high;
		}

		/////////////////////////////////////
		// we will turn on and off the LED //
		// and the speaker                 //
		/////////////////////////////////////
		if(filteredstate == high){ 
			GPIO_digitalWrite(LED_PIN, high);
		} else {
			GPIO_digitalWrite(LED_PIN, low);
		}
 
		//////////////////////////////////
		// the end of main loop clean up//
		/////////////////////////////////
		updateinfolinelcd();
		realstatebefore = realstate;
		lasthighduration = highduration;
		filteredstatebefore = filteredstate;
	}
}

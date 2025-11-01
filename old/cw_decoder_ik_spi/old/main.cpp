//
//	CW Decoder Ver 1.0
//
//  2024.08.12 New Create
//
#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#include "goertzel.h"
#include "ch32v003fun.h"
#include "cwdecoder.h"

// what type of OLED - uncomment just one
//#define SSD1306_64X32
//#define SSD1306_128X32
#define SSD1306_128X64
#include "ssd1306_spi.h"
#include "ssd1306.h"

#define GPIO_ADC_MUX_DELAY 100
#define GPIO_ADC_sampletime GPIO_ADC_sampletime_43cy
#include "ch32v003_GPIO_branchless.h"

#define micros() (SysTick->CNT / DELAY_US_TIME)
#define millis() (SysTick->CNT / DELAY_MS_TIME)
extern "C" int mini_snprintf(char* buffer, unsigned int buffer_len, const char *fmt, ...);


//#define LED_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 5)
#define SW1_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 0)
#define SW2_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 4)
#define SW3_PIN GPIOv_from_PORT_PIN(GPIO_port_C, 0)
#define ADC_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 3)
#define SPK_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 2)
#define LED_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 5)
#define UART_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 6)


#define SAMPLES 48 // 
#define SAMPLING_FREQUENCY 8000

static uint16_t magnitudelimit = 100;
static uint16_t magnitudelimit_low = 100;
//static uint16_t magnitude;
static uint16_t realstate = low;
static uint16_t realstatebefore = low;
static uint16_t filteredstate = low;
static uint16_t filteredstatebefore = low;
static uint32_t starttimehigh;
static uint32_t highduration;
static uint32_t lasthighduration;
static uint32_t hightimesavg;
//static uint32_t lowtimesavg;
static uint32_t startttimelow;
static uint32_t lowduration;
static uint32_t laststarttime = 0;
static uint16_t nbtime = 6;  /// ms noise blanker         

static uint16_t sampling_period_us;
int16_t morseData[SAMPLES];


char title1[]   = "   CW Decoder   ";
char title2[]   = "   Version 1.1  ";

static char code[20];
uint16_t stop = low;
uint16_t wpm;

/////////////////////////////////////
// print the ascii code to the lcd //
// one a time so we can generate   //
// special letters                 //
/////////////////////////////////////
void printascii(int asciinumber)
{
	printf("%c", asciinumber);
	ssd1306_drawchar_sz(31 ,0 , asciinumber, 1, fontsize_64x64);
	ssd1306_refresh();
/*
	int fail = 0;
	if (rows == 4 and colums == 16)fail = -4; /// to fix the library problem with 4*16 display http://forum.arduino.cc/index.php/topic,14604.0.html
	if (lcdindex > colums-1){
		lcdindex = 0;
		if (rows==4){
			for (int i = 0; i <= colums-1 ; i++){
				lcd.setCursor(i,rows-3);
				lcd.write(line2[i]);
				line2[i]=line1[i];
			}
		}
		for (int i = 0; i <= colums-1 ; i++){
			lcd.setCursor(i+fail,rows-2);
			lcd.write(line1[i]);
			lcd.setCursor(i+fail,rows-1);
			lcd.write(32);
		}
 	}
	line1[lcdindex]=asciinumber;
	lcd.setCursor(lcdindex+fail,rows-1);
	lcd.write(asciinumber);
	lcdindex += 1;
*/
}

void setup()
{
    // 各GPIOの有効化
    GPIO_port_enable(GPIO_port_D);
    GPIO_port_enable(GPIO_port_C);
    // 各ピンの設定
    GPIO_pinMode(SW1_PIN, GPIO_pinMode_I_pullDown, GPIO_Speed_10MHz); /// GPIO_Speed_In? @see https://github.com/cnlohr/ch32v003fun/blob/master/examples/GPIO/GPIO.c#L55
    GPIO_pinMode(SW2_PIN, GPIO_pinMode_I_pullDown, GPIO_Speed_10MHz);
    GPIO_pinMode(SW3_PIN, GPIO_pinMode_I_pullDown, GPIO_Speed_10MHz);
    GPIO_pinMode(ADC_PIN, GPIO_pinMode_I_analog, GPIO_Speed_10MHz);
    
	GPIO_pinMode(LED_PIN, GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
	GPIO_digitalWrite(LED_PIN, low);

	GPIO_pinMode(UART_PIN, GPIO_pinMode_O_pushPullMux, GPIO_Speed_10MHz);
	RCC->APB2PCENR |= RCC_APB2Periph_AFIO;

	AFIO->PCFR1 &= ~GPIO_PartialRemap1_USART1;
	AFIO->PCFR1 &= ~GPIO_PartialRemap2_USART1;
	AFIO->PCFR1 |=  GPIO_PartialRemap2_USART1;

	ssd1306_spi_init();		// i2c Setup
	ssd1306_init();			// SSD1306 Setup
	ssd1306_setbuf(0);		// Clear Screen
	ssd1306_drawstr_sz(0, 24, title1, 1, fontsize_8x8);
	ssd1306_drawstr_sz(0, 40, title2, 1, fontsize_8x8);
	ssd1306_refresh();
	GPIO_ADCinit();
	initGoertzel();
	sampling_period_us = 1000000 / SAMPLING_FREQUENCY;
}

int main()
{
	float magnitude;

	SystemInit();			// ch32v003 sETUP
	setup();				// gpio Setup;
	Delay_Ms( 2000 );
	printf("Start CW Decoder Version 1.0\n");
	ssd1306_setbuf(0);	// Clear Screen
	ssd1306_refresh();
	while(1) {
 	  	int16_t ave = 0										;

//		GPIO_digitalWrite(LED_PIN, high);

		for (int i = 0; i < SAMPLES; i++) {
//		GPIO_digitalWrite(LED_PIN, high);
			uint32_t t = micros();

			morseData[i] = GPIO_analogRead(GPIO_Ain4_D3);

			ave += morseData[i];
//		GPIO_digitalWrite(LED_PIN, low);
			while ((micros() - t) < sampling_period_us);
		}
//		GPIO_digitalWrite(LED_PIN, low);

		ave = ave / SAMPLES;
		for (int i = 0; i < SAMPLES; i++) {
			morseData[i] -= ave;
		}
//		for (int i = 0; i < SAMPLES; i++) {
//			printf("%d\n", morseData[i]);
//		}
		magnitude = goertzel(morseData, SAMPLES);
//		printf("%8lu : mag = %ld\n", micros(), (int32_t)magnitude);
//		magnitude = (uint16_t)goertzel(morseData, SAMPLES);
		/////////////////////////////////////////////////////////// 
		// here we will try to set the magnitude limit automatic //
		///////////////////////////////////////////////////////////
  		if (magnitude > magnitudelimit_low){
    		magnitudelimit = (magnitudelimit +((magnitude - magnitudelimit)/6));  /// moving average filter
  		}
 
  		if (magnitudelimit < magnitudelimit_low)
			magnitudelimit = magnitudelimit_low;

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
		if ((millis()-laststarttime)> nbtime){
			if (realstate != filteredstate){
				filteredstate = realstate;
			}
		}

		////////////////////////////////////////////////////////////
		// Then we do want to have some durations on high and low //
		////////////////////////////////////////////////////////////
		if (filteredstate != filteredstatebefore){
			if (filteredstate == high){
				starttimehigh = millis();
				lowduration = (millis() - startttimelow);
			}

			if (filteredstate == low){
				startttimelow = millis();
				highduration = (millis() - starttimehigh);
				if (highduration < (2*hightimesavg) || hightimesavg == 0){
					hightimesavg = (highduration+hightimesavg+hightimesavg)/3;     // now we know avg dit time ( rolling 3 avg)
				}
				if (highduration > (5*hightimesavg) ){
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
	
			float lacktime = 1;
			if(wpm > 25)lacktime=1.0; ///  when high speeds we have to have a little more pause before new letter or new word 
			if(wpm > 30)lacktime=1.2;
			if(wpm > 35)lacktime=1.5;
		
			if (lowduration > (hightimesavg*(2*lacktime)) && lowduration < hightimesavg*(5*lacktime)){ // letter space
				printascii(docode(code));
				code[0] = '\0';
// 				printf("/");
			}
			if (lowduration >= hightimesavg*(5*lacktime)){ // word space
				printascii(docode(code));
				code[0] = '\0';
				printascii(32);
//				printf("\n");
				printf(" ");
			}
		}

		//////////////////////////////
		// write if no more letters //
		//////////////////////////////
		if ((millis() - startttimelow) > (highduration * 6) && stop == low){
			printascii(docode(code));
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
	//	updateinfolinelcd();
		realstatebefore = realstate;
		lasthighduration = highduration;
		filteredstatebefore = filteredstate;
	}
}

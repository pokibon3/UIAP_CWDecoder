///////////////////////////////////////////////////////////////////////
// CW Decoder made by Hjalmar Skovholm Hansen OZ1JHM  VER 1.01       //
// Feel free to change, copy or what ever you like but respect       //
// that license is http://www.gnu.org/copyleft/gpl.html              //
// Discuss and give great ideas on                                   //
// https://groups.yahoo.com/neo/groups/oz1jhm/conversations/messages //
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Read more here http://en.wikipedia.org/wiki/Goertzel_algorithm        //
// if you want to know about FFT the http://www.dspguide.com/pdfbook.htm //
///////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdint.h>
#include "ch32v003fun.h"
#include "ch32v003_GPIO_branchless.h"
#define   SSD1306_128X64
#include "ssd1306_spi.h"
#include "ssd1306.h"
#include "docode.h"
#include "goertzel.h"
#include "adc.h"
#include "gpio.h"

//#define LED_ON 		GPIOC->BSHR = 1 << 15	 // Turn on GPIOs
//#define LED_OFF		GPIOC->BSHR = (1 << 31) // Turn off GPIODs
#define LED_ON 		GPIOB->BSHR = 1 << 11	 // Turn on GPIOs
#define LED_OFF		GPIOB->BSHR = (1 << 27) // Turn off GPIODs
#define micros() (SysTick->CNT / DELAY_US_TIME)
#define millis() (SysTick->CNT / DELAY_MS_TIME)
#define SAMPLES 48 
#define SAMPLING_FREQUENCY 8000

//#define PRINT_CODE
//#define PRINT_ADC
//#define PRINT_MAG

// function prototype (declaration), definition in "ch32v003fun.c"
extern "C" int mini_snprintf(char* buffer, unsigned int buffer_len, const char *fmt, ...);

static uint16_t sampling_period_us;
static int16_t 	morseData[SAMPLES];

static uint16_t magnitudelimit 		= 1800;				// default 100
static uint16_t magnitudelimit_low 	= 1800;				// default 100
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
#define  nbtime 	6  /// ms noise blanker         

static char code[20];
static uint16_t stop = low;
static uint16_t wpm;

static char     langMode;
static char		sw = MODE_US;
static int16_t 	speed = 1;

static uint16_t morseSpeed = 1;

static const int colums = 10; /// have to be 16 or 20
static const int rows = 4;  /// have to be 2 or 4

static int lcdindex = 0;
static uint8_t line1[colums];
static uint8_t line2[colums];
static uint8_t lastChar = 0;

static const char *tone[] = {
	" 600",
	" 800",
	"1000"
};

//=========================================================
//	 setspeed
//=========================================================
static void setSpeed(int16_t speed)
{
    switch(speed) {
        case 0 :
            initGoertzel(666.7);
            break;
        case 1 :
            initGoertzel(888.8);
            break;
        case 2 :
            initGoertzel(1000.0);
            break;
        default :
            initGoertzel(888.8);
    }
}

//=========================================================
//	 print states
//=========================================================
void updateinfolinelcd(char sw, uint16_t speed)
{
	char buf[17];
	char *modeString;
	uint16_t w;

	if (wpm < 10) {
		w = 0;
	} else {
		w = wpm;
	}
    langMode = sw;
    morseSpeed = speed;
	if (sw == 0) {
		modeString = (char *)"US";
	} else {
		modeString = (char *)"JP";
	}
	mini_snprintf(buf, sizeof(buf), "%2d WPM[%s%sHz]", w, modeString, tone[speed]);
	ssd1306_drawstr_sz(0 ,2 , buf, 1, fontsize_8x8);
	ssd1306_drawFastHLine(0, 13, 128, 1);
	ssd1306_refresh();
}

//=========================================================
//	check GPIO SW
//=========================================================
static void check_input() 
{

	if (!GPIO_digitalRead(SW2_PIN)) {
		speed -= 1;
		if (speed < 0) speed = 2;
		setSpeed(speed);
		updateinfolinelcd(sw, speed);
//		printf("speed + %d\n", speed + 1);
		Delay_Ms(300);
	} else if (!GPIO_digitalRead(SW3_PIN)) {
		speed += 1;
		if (speed > 2) speed = 0;
		setSpeed(speed);
		updateinfolinelcd(sw, speed);
		Delay_Ms(300);
	} else if (!GPIO_digitalRead(SW1_PIN)) {
		sw ^= 1;
		updateinfolinelcd(sw, speed);
		Delay_Ms(300);
	}
}


//=========================================================
//	 setup
//=========================================================
void setupDecoder(void)
{
	initGoertzel(888.8);
   	sampling_period_us = 1000000 / SAMPLING_FREQUENCY;
	adc_init();

	for (int i = 0; i < colums; i++){
    	line1[i] = 32;
		line2[i] = 32;
	}           
}

//=========================================================
//	 print ascii code to OLED
//=========================================================
#define FONT_WIDTH 12
static void printascii(int16_t asciinumber)
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

//=========================================================
//	morse decoder
//=========================================================
void morseDecoder(void)
{
	float magnitude;

	while(1) {
 	  	int16_t ave = 0;

		check_input();
		for (int i = 0; i < SAMPLES; i++) {
			uint32_t t = micros();
//			morseData[i] = GPIO_analogRead(GPIO_Ain4_D3);
			morseData[i] = adc_get() >> 2;
			ave += morseData[i];
			while ((micros() - t) < sampling_period_us);
		}
		ave = ave / SAMPLES;
		for (int i = 0; i < SAMPLES; i++) {
			morseData[i] -= ave;
#ifdef PRINT_ADC
			printf("%4d\n", morseData[i]);
#endif
		}
		// calc goertzel
		magnitude = goertzel(morseData, SAMPLES);
#ifdef PRINT_MAG
		printf("%8ld\n", (int32_t)magnitude);
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
#ifdef PRINT_CODE
					printf(".");
#endif
				}
				if (highduration > (hightimesavg*2) && highduration < (hightimesavg*6)){ 
					strcat(code,"-");
#ifdef PRINT_CODE
					printf("-");
#endif
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
				if (strlen(code) > 0) {
					printascii(docode(code, &langMode));
					code[0] = '\0';
//#ifdef PRINT_CODE
//	 				printf("/");
//#endif
				}
			}
			if (lowduration >= hightimesavg*(5*lacktime)){ // word space
				if (strlen(code) > 0) {
					printascii(docode(code, &langMode));
					code[0] = '\0';
					printascii(32);
//#ifdef PRINT_CODE
//					printf(" ");
//					printf("\n");
//#endif
				}
			}
		}

		//////////////////////////////
		// write if no more letters //
		//////////////////////////////
		if ((millis() - startttimelow) > (highduration * 6) && stop == low){
			printascii(docode(code, &langMode));
			code[0] = '\0';
			stop = high;
		}

		/////////////////////////////////////
		// we will turn on and off the LED //
		// and the speaker                 //
		/////////////////////////////////////
		if(filteredstate == high){ 
//			GPIO_digitalWrite(LED_PIN, high);
			LED_ON;
		} else {
//			GPIO_digitalWrite(LED_PIN, low);
			LED_OFF;
		}
 
		//////////////////////////////////
		// the end of main loop clean up//
		/////////////////////////////////
		updateinfolinelcd(langMode, morseSpeed);
		realstatebefore = realstate;
		lasthighduration = highduration;
		filteredstatebefore = filteredstate;
	}
}

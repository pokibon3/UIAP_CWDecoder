///////////////////////////////////////////////////////////////////////
// CW Decoder made by Hjalmar Skovholm Hansen OZ1JHM  VER 1.01       //
// Feel free to change, copy or what ever you like but respect       //
// that license is <a href="http://www.gnu.org/copyleft/gpl.html" title="http://www.gnu.org/copyleft/gpl.html" rel="nofollow">http://www.gnu.org/copyleft/gpl.html</a>              //
// Discuss and give great ideas on                                   //
// <a href="https://groups.yahoo.com/neo/groups/oz1jhm/conversations/messages" title="https://groups.yahoo.com/neo/groups/oz1jhm/conversations/messages" rel="nofollow">https://groups.yahoo.com/neo/groups/oz1jhm/conversations/messages</a> //
//                                                                   //
// Modifications by KC2UEZ. Bumped to VER 1.2:                       //
// Changed to work with the Arduino NANO.                            //
// Added selection of "Target Frequency" and "Bandwith" at power up. //
///////////////////////////////////////////////////////////////////////
 
///////////////////////////////////////////////////////////////////////////
// Read more here <a href="http://en.wikipedia.org/wiki/Goertzel_algorithm" title="http://en.wikipedia.org/wiki/Goertzel_algorithm" rel="nofollow">http://en.wikipedia.org/wiki/Goertzel_algorithm</a>        //
// if you want to know about FFT the <a href="http://www.dspguide.com/pdfbook.htm" title="http://www.dspguide.com/pdfbook.htm" rel="nofollow">http://www.dspguide.com/pdfbook.htm</a> //
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Modified version by Kimio Ohe
// Modifications:
//  - port to UIAPduino Pro Micro(CH32V003)
//  - add freq detector function
//  - Refactoring of all source files
// Date: 2025-11-07
//
// このソフトウェアは GNU General Public License (GPL) に基づき配布されています。
// 改変版も同じ GPL ライセンスで再配布してください。
///////////////////////////////////////////////////////////////////////////
//
//	CW Decoder functions
//
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "goertzel.h"
#include "docode.h"
#include "st7735.h"
#include "ch32v003fun.h"
#include "st7735.h"
#include "ch32v003_GPIO_branchless.h"

#define TEST_HIGH			GPIO_digitalWrite(TEST_PIN, high);
#define TEST_LOW			GPIO_digitalWrite(TEST_PIN, low);

#define GOERTZEL_SAMPLES 48
#define GOERTZEL_SAMPLING_FREQUENCY 8000
#define FONT_WIDTH 12
#define LINE_HEIGHT 20
static char title1[]   = " CW Decoder  ";
static char title2[]   = "  for UIAP   ";
static char title3[]   = " Version 1.0 ";

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

static const char *tone[] = {
	" 600",
	" 800",
	"1000"
};

const int colums = 13; /// have to be 16 or 20

int lcdindex = 0;
uint8_t line1[colums];
uint8_t line2[colums];
uint8_t lastChar = 0;

int16_t 	morseData[GOERTZEL_SAMPLES];

//==================================================================
// gap を 1単位 hightimesavg の相対値で分類
//==================================================================
typedef enum {
	GAP_INTRA = 0, // 文字内
	GAP_CHAR  = 1, // 文字間
	GAP_WORD  = 2  // 単語間
} gap_type_t;

static gap_type_t classify_gap(uint32_t gap, uint32_t unit)
{
	if (unit == 0) return GAP_INTRA; // まだ学習前の保護

	// gap < 1.5 * unit → 同一文字内
	if (gap < (unit * 3) / 2) {
		return GAP_INTRA;
	}
	// 1.5〜4.5 * unit → 文字間
	if (gap < (unit * 9) / 2) {
		return GAP_CHAR;
	}
	// 6 * unit 以上 → 単語間
	if (gap >= unit * 6) {
		return GAP_WORD;
	}

	// 4.5〜6 * unit は微妙ゾーン → とりあえず文字間に寄せる
	return GAP_CHAR;
}
//==================================================================
//	updateinfolinelcd() : print info field
//==================================================================
static void updateinfolinelcd()
{
	char buf[17];
	const char *mode;
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
	mini_snprintf(buf, sizeof(buf), "%2dWPM %s%s", w, mode, tone[speed]);
	tft_set_cursor(0, 0);
	tft_set_color(BLUE);
	tft_print(buf, FONT_SCALE_16X16);
	tft_draw_line(0, 17, 159, 17, GREEN);
	tft_set_color(WHITE);
}

//==================================================================
//	printascii : print the ascii code to the lcd
//==================================================================
static void printAscii(int16_t asciinumber)
{
#ifdef SERIAL_OUT
	printf("%c", asciinumber);
#endif
	if (lcdindex > colums - 1){
		lcdindex = 0;
		for (int i = 0; i <= colums - 1 ; i++){
			tft_set_cursor(i * FONT_WIDTH , LINE_HEIGHT);
			tft_print_char(line2[i], FONT_SCALE_16X16);

			line2[i] = line1[i];
		}
		for (int i = 0; i <= colums - 1 ; i++){
			tft_set_cursor(i * FONT_WIDTH , LINE_HEIGHT * 2);
			tft_print_char(line1[i], FONT_SCALE_16X16);
			tft_set_cursor(i * FONT_WIDTH , LINE_HEIGHT * 3);
			tft_print_char(32, FONT_SCALE_16X16);
		}
 	}
	line1[lcdindex] = asciinumber;
	tft_set_cursor(lcdindex * FONT_WIDTH , LINE_HEIGHT * 3);
	tft_print_char(asciinumber, FONT_SCALE_16X16);
	lcdindex += 1;
}

//==================================================================
//	chack switch
//==================================================================
static int check_sw()
{
    int ret = 0;
	int val = check_input();

	if (val == 3) {
		speed -= 1;
		if (speed < 0) speed = 2;
		setSpeed(speed);
		updateinfolinelcd();
		Delay_Ms(300);
	} else if (val == 2) {
		sw ^= 1;
		updateinfolinelcd();
		Delay_Ms(300);
    } else if (val == 1) {
		Delay_Ms(300);
        ret = 1;
	}
    return ret;
}

//==================================================================
//	cw_decoder setup
//==================================================================
int cwd_setup()
{
	tft_fill_rect(0, 0, ST7735_WIDTH, ST7735_HEIGHT, BLACK);
	tft_set_cursor(0, 0);
	tft_set_color(BLUE);
	tft_set_cursor(0, 10);
	tft_print(title1, FONT_SCALE_16X16);
	tft_set_color(RED);
	tft_set_cursor(0, 30);
	tft_print(title2, FONT_SCALE_16X16);
	tft_set_cursor(0, 50);
	tft_set_color(GREEN);
	tft_print(title3, FONT_SCALE_16X16)	;
	tft_set_color(WHITE);

	initGoertzel(speed);
	sampling_period_us = 900000 / GOERTZEL_SAMPLING_FREQUENCY;
	for (int i = 0; i < colums; i++){
    	line1[i] = 32;
		line2[i] = 32;
	}
    Delay_Ms( 1000 );	
    return 0;
}

//==================================================================
//	cwDecoder : deocder main
//==================================================================
static int decodeAscii(int16_t asciinumber)
{
	if (asciinumber == 0) return 0;
	if (lastChar == 32 && asciinumber == 32) return 0;

	if        (asciinumber == 1) {			// AR
		printAscii('A');
		printAscii('R');
	} else if (asciinumber == 2) {			// KN
		printAscii('K');
		printAscii('N');
	} else if (asciinumber == 3) {			// BT
		printAscii('B');
		printAscii('T');
	} else if (asciinumber == 4) {			// VA
		printAscii('V');
		printAscii('A');
	} else {
		printAscii(asciinumber);
	}
	lastChar = asciinumber;

	return 0;
}

//==================================================================
//	cwDecoder : deocder main
//==================================================================
int cwDecoder(int16_t *morseData)
{
	int32_t magnitude;

	tft_fill_rect(0, 0, ST7735_WIDTH, ST7735_HEIGHT, BLACK);

	while(1) {
TEST_HIGH
 	  	int16_t ave = 0;

		if (check_sw() == 1) {
            break;
        }
		for (int i = 0; i < GOERTZEL_SAMPLES; i++) {
			uint32_t t = micros();
			morseData[i] = GPIO_analogRead(GPIO_Ain0_A2);
			ave += morseData[i];
			while ((micros() - t) < sampling_period_us);
		}
		ave = ave / GOERTZEL_SAMPLES;
		for (int i = 0; i < GOERTZEL_SAMPLES; i++) {
			morseData[i] -= ave;
		}
TEST_LOW

		// calc goertzel
		magnitude = goertzel(morseData, GOERTZEL_SAMPLES) / 400;
		tft_draw_line(159, 79, 159, 0 , BLACK);
		int16_t w = 80 - magnitude / 50;
		if (w < 0) w = 0;
		if (w > 80) w = 80;

		tft_draw_line(159, 79, 159, w , YELLOW);
#ifdef SERIAL_OUT
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
		if (filteredstate == high) {  //// we did end a LOW

			if (hightimesavg > 0) {
				gap_type_t g = classify_gap(lowduration, hightimesavg);

				if (g == GAP_CHAR) {          // 文字間
					if (strlen(code) > 0) {
						decodeAscii(docode(code, &sw));
						code[0] = '\0';
					}
				} else if (g == GAP_WORD) {   // 単語間
					if (strlen(code) > 0) {
						decodeAscii(docode(code, &sw));
						code[0] = '\0';
					}
					decodeAscii(32);           // スペース出力
				}
			}
		}

		//////////////////////////////
		// write if no more letters //
		//////////////////////////////
		uint32_t unit = (hightimesavg > 0) ? hightimesavg : highduration;
		if ((millis() - startttimelow) > unit * 6 && stop == low) {
			decodeAscii(docode(code, &sw));
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
    return 0;
}

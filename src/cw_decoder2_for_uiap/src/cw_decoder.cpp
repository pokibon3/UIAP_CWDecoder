///////////////////////////////////////////////////////////////////////
// CWデコーダ (Hjalmar Skovholm Hansen OZ1JHM) バージョン 1.01       //
// 自由に改変・複製できますが、GPL を遵守してください。       //
// ライセンス: <a href="http://www.gnu.org/copyleft/gpl.html" title="http://www.gnu.org/copyleft/gpl.html" rel="nofollow">http://www.gnu.org/copyleft/gpl.html</a>              //
// 議論・提案はこちら:                                               //
// <a href="https://groups.yahoo.com/neo/groups/oz1jhm/conversations/messages" title="https://groups.yahoo.com/neo/groups/oz1jhm/conversations/messages" rel="nofollow">https://groups.yahoo.com/neo/groups/oz1jhm/conversations/messages</a> //
//                                                                   //
// KC2UEZ による改変 (バージョン 1.2):                                     //
// - Arduino NANO に対応                                            //
// - 起動時に「ターゲット周波数」と「帯域」を選択可能               //
///////////////////////////////////////////////////////////////////////
 
///////////////////////////////////////////////////////////////////////////
// Goertzel 法の解説: <a href="http://en.wikipedia.org/wiki/Goertzel_algorithm" title="http://en.wikipedia.org/wiki/Goertzel_algorithm" rel="nofollow">http://en.wikipedia.org/wiki/Goertzel_algorithm</a>     //
// FFT の参考: <a href="http://www.dspguide.com/pdfbook.htm" title="http://www.dspguide.com/pdfbook.htm" rel="nofollow">http://www.dspguide.com/pdfbook.htm</a>                //
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// 改変版 (Kimio Ohe)
// 変更点:
//  - UIAPduino Pro Micro(CH32V003) に移植
//  - 周波数検出機能を追加
//  - 全ソースをリファクタリング
// 日付: 2025-11-07 バージョン 1.0
//  - DFT を Goertzel 法に変更
// 日付: 2025.12.05 バージョン 1.1
//  - オーディオ入力ダイナミックレンジ拡大
// 日付: 2025.12.13 バージョン 1.2
//  - DFTの周波数を調整
//  - DFTの入力レベルを調整
// 日付: 2025.12.20 バージョン 1.3
//  - デコードタイミング微調整
//
// このソフトウェアは GNU General Public License (GPL) に基づき配布されています。
// 改変版も同じ GPL ライセンスで再配布してください。
///////////////////////////////////////////////////////////////////////////
//
//	CWデコーダ関数
//
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "goertzel.h"
#include "decode.h"
#include "st7735.h"
#include "ch32v003fun.h"
#include "st7735.h"
#include "ch32v003_GPIO_branchless.h"

//#define SERIAL_OUT

#define TEST_HIGH			GPIO_digitalWrite(TEST_PIN, high);
#define TEST_LOW			GPIO_digitalWrite(TEST_PIN, low);

#define GOERTZEL_SAMPLES 48
#define GOERTZEL_SAMPLING_FREQUENCY 8000
#define FONT_WIDTH 12
#define LINE_HEIGHT 20
static char title1[]   = " CW Decoder  ";
static char title2[]   = "  for UIAP   ";
static char title3[]   = " Version 1.3 ";
static uint8_t first_flg = 1;

static uint16_t magnitudelimit = 140;  		// 以前は 140
static uint16_t magnitudelimit_low = 140;
static uint16_t realstate = low;
static uint16_t realstatebefore = low;
static uint16_t filteredstate = low;
static uint16_t filteredstatebefore = low;
static uint32_t starttimehigh;
static uint32_t highduration;
static uint32_t lasthighduration;
static uint32_t hightimesavg = 60;
static uint32_t startttimelow;
static uint32_t lowduration;
static uint32_t laststarttime = 0;
#define  nbtime 	6  /// ノイズブランカの時間(ms)

static char code[20];
static uint16_t stop = low;
static uint16_t wpm;

static char		sw = MODE_US;
static int16_t 	speed = 0;

static const char *tone[] = {
	" 700",
	" 800",
	"1000"
};

const int colums = 13; /// 表示列数(本来は16/20向け)

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
//	updateinfolinelcd() : 情報行をLCDに表示
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
//	printascii : ASCII文字をLCDに表示
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
//	スイッチ入力の確認
//==================================================================
static int check_sw()
{
    int ret = 0;
	int val = check_input();

	if (val == 3) {
		speed += 1;
		if (speed > 2) speed = 0;
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
//	cw_decoder 初期化
//==================================================================
int cwd_setup()
{
	if (first_flg == 1) {
		first_flg = 0;
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
	    Delay_Ms( 1000 );	
	}
	tft_set_color(WHITE);

	initGoertzel(speed);
	sampling_period_us = 900000 / GOERTZEL_SAMPLING_FREQUENCY;
	for (int i = 0; i < colums; i++){
    	line1[i] = 32;
		line2[i] = 32;
	}
    return 0;
}

//==================================================================
//	cwDecoder : デコーダ本体
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
	} else if (asciinumber == 7) {			// HH (訂正)
		printAscii('H');
		printAscii('H');
	} else {
		printAscii(asciinumber);
	}
	lastChar = asciinumber;

	return 0;
}

//==================================================================
//	cwDecoder : デコーダ本体
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
			morseData[i] = GPIO_analogRead(GPIO_Ain0_A2) >> 1;
			ave += morseData[i];
			while ((micros() - t) < sampling_period_us);
		}
		ave = ave / GOERTZEL_SAMPLES;
		for (int i = 0; i < GOERTZEL_SAMPLES; i++) {
			morseData[i] -= ave;
		}
TEST_LOW

		// Goertzel 計算
		magnitude = goertzel(morseData, GOERTZEL_SAMPLES);
		tft_draw_line(159, 79, 159, 0 , BLACK);
		int16_t w = 80 - magnitude / 8;
		if (w < 0) w = 0;
		if (w > 80) w = 80;

		tft_draw_line(159, 79, 159, w , YELLOW);
#ifdef SERIAL_OUT
		printf("mag = %d\n", magnitude);
#endif
		///////////////////////////////////////////////////////////
		// 振幅しきい値を自動更新
		///////////////////////////////////////////////////////////
  		if (magnitude > magnitudelimit_low){
			magnitudelimit = (magnitudelimit +((magnitude - magnitudelimit)/6));  /// 移動平均フィルタ
  		}
  		if (magnitudelimit < magnitudelimit_low) {
			magnitudelimit = magnitudelimit_low;
		}

		////////////////////////////////////
		// 振幅でしきい値判定
		////////////////////////////////////
		if(magnitude > magnitudelimit*0.6) {  // 余裕を持たせる
     		realstate = high;
		} else {
    		realstate = low;
		}

		/////////////////////////////////////////////////////
		// ノイズブランカで状態を安定化
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
		// HIGH/LOW の継続時間を計測
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
				hightimesavg = (highduration+hightimesavg+hightimesavg) / 3;     // 短点平均を更新 (3点移動平均)
				}
				if (highduration > (5*hightimesavg) ) {
				hightimesavg = highduration+hightimesavg;     // 速度低下が急な場合に追従
				}
				if (hightimesavg < 24) {
					hightimesavg = 24;
				}
			}
		}

		///////////////////////////////////////////////////////////////
		// 短点/長点判定と休止(1/3/7単位)の判定
		// 1/3/7 単位の休止を判定
		// hightimesavg を 1単位(短点)とみなす
		///////////////////////////////////////////////////////////////
		if (filteredstate != filteredstatebefore){
			stop = low;
			if (filteredstate == low){  //// HIGH 終了
				if (highduration < (hightimesavg*2) && highduration > (hightimesavg*0.6)){ /// 0.6 未満はノイズ除外
					strcat(code,".");
//					printf(".");
				}
				if (highduration > (hightimesavg*2) && highduration < (hightimesavg*6)){
					strcat(code,"-");
//					printf("-");
				wpm = (wpm + (1200/((highduration)/3)))/2;  //// 可能な限り精度の高い推定
					if (wpm > 50) {
						wpm = 50;
					}
				}
			}
		}
		if (filteredstate == high) {  //// LOW 終了

			if (hightimesavg > 0) {
				gap_type_t g = classify_gap(lowduration, hightimesavg);

				if (g == GAP_CHAR) {          // 文字間
					if (strlen(code) > 0) {
						decodeAscii(decode(code, &sw));
						code[0] = '\0';
					}
				} else if (g == GAP_WORD) {   // 単語間
					if (strlen(code) > 0) {
						decodeAscii(decode(code, &sw));
						code[0] = '\0';
					}
					decodeAscii(32);           // スペース出力
				}
			}
		}

		//////////////////////////////
		// 一定時間無音なら確定出力
		//////////////////////////////
		uint32_t unit = (hightimesavg > 0) ? hightimesavg : highduration;
		if ((millis() - startttimelow) > unit * 6 && stop == low) {
			decodeAscii(decode(code, &sw));
			code[0] = '\0';
			stop = high;
		}

		/////////////////////////////////////
		// LED の点灯/消灯
		// スピーカ制御(未使用)
		/////////////////////////////////////
		if(filteredstate == high){
			GPIO_digitalWrite(LED_PIN, high);
		} else {
			GPIO_digitalWrite(LED_PIN, low);
		}

		//////////////////////////////////
		// ループ終端の状態更新
		/////////////////////////////////
		updateinfolinelcd();
		realstatebefore = realstate;
		lasthighduration = highduration;
		filteredstatebefore = filteredstate;
	}
    return 0;
}

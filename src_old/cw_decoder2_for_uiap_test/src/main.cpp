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
// このソフトウェアは GNU General Public License (GPL) に基づき配布されています。
// 改変版も同じ GPL ライセンスで再配布してください。
///////////////////////////////////////////////////////////////////////////
//
//
//	CW Decoder Ver for UIAPduino Pro Micro
//
//  2024.08.12 New Create
//  2024.08.21 integer version
//  2025.09.30 port to UIAPduino Pro Micro(CH32V003)
//  2025.10.04 add freq detector
//  2025.11.07 first release Version 1.0
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "ch32v003fun.h"
#include "st7735.h"
#include "cw_decoder.h"
//#include "frequencyDetector.h"

uint16_t sampling_period_us;
uint8_t  buf[BUFSIZE];

//==================================================================
//	main
//==================================================================
int main()
{
	int16_t *morseData;
	int8_t *vReal;
	int8_t *vImag;

	SystemInit();				// ch32v003 Setup
	GPIO_setup();				// gpio Setup;
    tft_init();					// LCD init

	vReal = (int8_t *)&buf[0];
	vImag = (int8_t *)&buf[128];
	morseData = (int16_t *)&buf[0];

	while (1) {
		// cw decoder
		cwd_setup();			// freq detector Setup
		cwDecoder(morseData);	// run cw decoder
		// frequency detector
		//fd_setup();				// freq detector Setup
		//freqDetector(vReal, vImag);			// run freq counter
	}
	return 0;
}



//
//	CW Decoder Ver 2.0 for UIAPduino Pro Micro
//
//  2024.08.12 New Create
//  2024.08.21 integer version
//  2025.09.30 port to UIAPduino Pro Micro(CH32V003)
//  2025.10.04 add freq detector
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
#include "frequencyDetector.h"

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

	vReal = (int8_t *)&buf[0];
	vImag = (int8_t *)&buf[128];
	morseData = (int16_t *)&buf[0];

	SystemInit();				// ch32v003 Setup
	GPIO_setup();				// gpio Setup;
    tft_init();					// LCD init
	while (1) {
		cwd_setup();			// freq detector Setup
		cwDecoder(morseData);	// run cw decoder
		fd_setup();				// freq detector Setup
		freqDetector(vReal, vImag);			// run freq counter
	}
}



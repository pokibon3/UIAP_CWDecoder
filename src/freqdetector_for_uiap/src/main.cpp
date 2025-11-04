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
#include "common.h"
#include "frequencyDetector.h"
#include "ch32v003fun.h"
#include "st7735.h"
#include "ch32v003_GPIO_branchless.h"


uint16_t sampling_period_us;

//==================================================================
//	main
//==================================================================
int main()
{
	SystemInit();			// ch32v003 Setup
	GPIO_setup();				// gpio Setup;
	fd_setup();				// freq detector Setup
	freqDetector();			// run freq counter
}

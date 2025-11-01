#include "ch32v003fun.h"
#define GPIO_ADC_MUX_DELAY 100
#define GPIO_ADC_sampletime GPIO_ADC_sampletime_43cy

//==========================================================
// init adc
//==========================================================
void adc_init( void )
{
	// ADCCLK = 24 MHz => RCC_ADCPRE = 0: divide by 2
	// ADCCLK = 144 / ADCPRE(b'10:6) / PB2(b'100:2) 
	// ADCCLK = 144 / ADCPRE(b'11:8) / PB2(b'000:1) 
	// b'10100 = 0x14
	RCC->CFGR0 &= ~(0x1f<<11);
	RCC->CFGR0 |=  (0x18<<11);
	
	// Enable GPIOA and ADC
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1;
	
	// PA0 is analog input chl 0
	GPIOA->CFGLR &= ~(0xf<<(4 * 0));	// CNF = 00: Analog, MODE = 00: Input
	
	// Reset the ADC to init all regs
	RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;
	
	// Set up single conversion on chl 7
	ADC1->RSQR1 = 0;
	ADC1->RSQR2 = 0;
	ADC1->RSQR3 = 0;	// 0-9 for 8 ext inputs and two internals
	
	// set sampling time for chl 0
	ADC1->SAMPTR2 &= ~(ADC_SMP0 << (3 * 0));
	ADC1->SAMPTR2 |= 4 << (3 * 0);	// 0:7 => 3/9/15/30/43/57/73/241 cycles
		
	// turn on ADC and set rule group to sw trig
//	ADC1->CTLR2 |= ADC_ADON | ADC_EXTSEL;
	ADC1->CTLR2 |= ADC_EXTSEL;
	ADC1->CTLR2 |= ADC_ADON;

	// Reset calibration
	ADC1->CTLR2 |= ADC_RSTCAL;
	while(ADC1->CTLR2 & ADC_RSTCAL);
	
	// Calibrate
	ADC1->CTLR2 |= ADC_CAL;
	while(ADC1->CTLR2 & ADC_CAL);
	
	// should be ready for SW conversion now
}

//==========================================================
//  start conversion, wait and return result
//==========================================================
uint16_t adc_get( void )
{
	ADC1->CTLR2 |= ADC_ADON;
	// start sw conversion (auto clears)
	ADC1->CTLR2 |= ADC_SWSTART;
	
	// wait for conversion complete
	while(!(ADC1->STATR & ADC_EOC));
	
	// get result
	return ADC1->RDATAR;
}

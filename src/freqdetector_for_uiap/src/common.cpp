#include "ch32v003_GPIO_branchless.h"
#include "common.h"

//==================================================================
//	setup
//==================================================================
int GPIO_setup()
{
    // Enable GPIO Ports A, C, D
    GPIO_port_enable(GPIO_port_A);
    GPIO_port_enable(GPIO_port_C);
    GPIO_port_enable(GPIO_port_D);
    // Set Pin Modes
    GPIO_pinMode(SW1_PIN, GPIO_pinMode_I_pullUp, GPIO_Speed_10MHz);
    GPIO_pinMode(SW2_PIN, GPIO_pinMode_I_pullUp, GPIO_Speed_10MHz);
    GPIO_pinMode(SW3_PIN, GPIO_pinMode_I_pullUp, GPIO_Speed_10MHz);
    GPIO_pinMode(ADC_PIN, GPIO_pinMode_I_analog, GPIO_Speed_10MHz);
	GPIO_pinMode(LED_PIN, GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
	GPIO_digitalWrite(LED_PIN, low);

	GPIO_pinMode(TEST_PIN, GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
	GPIO_digitalWrite(TEST_PIN, low);

	GPIO_pinMode(UART_PIN, GPIO_pinMode_O_pushPullMux, GPIO_Speed_10MHz);
	RCC->APB2PCENR |= RCC_APB2Periph_AFIO;

	GPIO_ADCinit();

    return 0;
}

//
//	CW Decoder Common header
//
#pragma once
#define GPIO_ADC_MUX_DELAY 100
#define GPIO_ADC_sampletime GPIO_ADC_sampletime_43cy

#define SCALE 3

#define micros() (SysTick->CNT / DELAY_US_TIME)
#define millis() (SysTick->CNT / DELAY_MS_TIME)

#define SW1_PIN GPIOv_from_PORT_PIN(GPIO_port_A, 1)		// for uiap
#define SW2_PIN GPIOv_from_PORT_PIN(GPIO_port_C, 4)		// for uiap
#define SW3_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 2)
#define ADC_PIN GPIOv_from_PORT_PIN(GPIO_port_A, 2)		// for uiap
#define LED_PIN GPIOv_from_PORT_PIN(GPIO_port_C, 0)		// for uiap
#define UART_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 5)
#define TEST_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 6)

#define TEST_HIGH			GPIO_digitalWrite(TEST_PIN, high);
#define TEST_LOW			GPIO_digitalWrite(TEST_PIN, low);

#define SAMPLES 128
#define BUFSIZE (SAMPLES * 2)

extern uint16_t sampling_period_us;

extern "C" int mini_snprintf(char* buffer, unsigned int buffer_len, const char *fmt, ...);
extern int GPIO_setup();
extern int check_input();

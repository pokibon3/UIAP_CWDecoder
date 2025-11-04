#if defined(GPIO_ADC_MUX_DELAY)
    #undef GPIO_ADC_MUX_DELAY
    #define GPIO_ADC_MUX_DELAY 100
#endif

#if defined(GPIO_ADC_sampletime)
    #undef GPIO_ADC_sampletime
    #define GPIO_ADC_sampletime GPIO_ADC_sampletime_43cy
#endif

#define SCALE 3

#define micros() (SysTick->CNT / DELAY_US_TIME)
#define millis() (SysTick->CNT / DELAY_MS_TIME)

//#define LED_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 5)
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
#define SAMPLING_FREQUENCY 6000	// Hz	+10%

extern uint16_t sampling_period_us;
//extern int8_t vReal[SAMPLES];
//extern int8_t vImag[SAMPLES];

// function prototype (declaration), definition in "ch32v003fun.c"
extern "C" int mini_snprintf(char* buffer, unsigned int buffer_len, const char *fmt, ...);

extern int GPIO_setup();
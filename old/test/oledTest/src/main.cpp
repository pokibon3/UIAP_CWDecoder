//
//  freqcounter Ver 1.1u (page-buffered SSD1306: RAM 128B)
//  2025-09-30 port to UIAPduino Pro Micro(CH32V003)
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fix_fft.h"
#include "ch32v003fun.h"

#define micros() (SysTick->CNT / DELAY_US_TIME)
#define millis() (SysTick->CNT / DELAY_MS_TIME)

// OLED select
#define SSD1306_128X64
#define SCALE 3

#define GPIO_ADC_MUX_DELAY 100
#define GPIO_ADC_sampletime GPIO_ADC_sampletime_43cy
#include "ch32v003_GPIO_branchless.h"
#include "ssd1306_spi.h"
#include "ssd1306.h"

#define SW1_PIN GPIOv_from_PORT_PIN(GPIO_port_C, 1)   // for uiap
#define SW2_PIN GPIOv_from_PORT_PIN(GPIO_port_C, 2)   // for uiap
#define ADC_PIN GPIOv_from_PORT_PIN(GPIO_port_A, 2)   // for uiap

#define SAMPLES 256
#define SAMPLING_FREQUENCY 6000

uint16_t sampling_period_us;
int8_t vReal[SAMPLES];
int8_t vImag[SAMPLES];

// from ch32v003fun.c
extern "C" int mini_snprintf(char* buffer, unsigned int buffer_len, const char *fmt, ...);
static char title1[] = "FrequencyCounter";
static char title2[] = "  Version 1.1u  ";

/* ==== helpers for rendering ==== */
static void draw_axes_cb(uint8_t page, uint8_t* buf)
{
    // horizontal lines: y=0,16,63
    if((0>>3)==page)  ssd1306p_hline_in_page(buf, 0, 127, 0);
    if((16>>3)==page) ssd1306p_hline_in_page(buf, 0, 127, 16);
    if((63>>3)==page) ssd1306p_hline_in_page(buf, 0, 127, 63);

    // vertical lines: x=0,127 (span pages)
    ssd1306p_vline_in_page(buf, 0,   0, 63, page);
    ssd1306p_vline_in_page(buf, 127, 0, 63, page);
}

static void draw_text_center_8(uint8_t page, uint8_t* buf, int y, const char* s)
{
    // naive center: 8px per char
    int len = 0; for(const char* p=s; *p; ++p) len++;
    int px = (128 - len*8)/2; if(px < 0) px = 0;
    ssd1306p_draw_str8_in_page(buf, page, px, y, s);
}

/* ==== splash ==== */
static void splash_screen(void)
{
//    auto splash_cb = [](uint8_t page, uint8_t *buf){
//        draw_text_center_8(page, buf, 24, title1);
//        draw_text_center_8(page, buf, 40, title2);
//    };
//    ssd1306_present(splash_cb);
	auto splash_cb = [](uint8_t page, uint8_t *buf){
  	  ssd1306p_draw_str8_in_page_rm(buf, page, (128 - (int)strlen(title1)*8)/2, 24, title1);
    	ssd1306p_draw_str8_in_page_rm(buf, page, (128 - (int)strlen(title2)*8)/2, 40, title2);
	};
	ssd1306_present(splash_cb);
}

/* ==== ADC + FFT + render ==== */
// 置き換え先: main.cpp の render_fft_and_freq()
static void render_fft_and_freq(uint16_t peakHz, uint8_t maxVal)
{
    // 0..15px を「ヘッダ」（16px文字）として使用
    // 17..63px をバー領域にする（グリッドは 0,16,63 に引く）
    char linebuf[16];
    if (maxVal >= 4) mini_snprintf(linebuf, sizeof(linebuf), " %4dHz", peakHz + 12);
    else             mini_snprintf(linebuf, sizeof(linebuf), "    0Hz");

    auto cb = [&](uint8_t page, uint8_t *buf)
    {
        // 1) グリッド/枠
        // y=0,16,63 の水平線 / x=0,127 の縦線
        if ((0>>3)  == page) ssd1306p_hline_in_page(buf, 0, 127, 0);
        if ((16>>3) == page) ssd1306p_hline_in_page(buf, 0, 127, 16);
        if ((63>>3) == page) ssd1306p_hline_in_page(buf, 0, 127, 63);
        ssd1306p_vline_in_page(buf, 0,   0, 63, page);
        ssd1306p_vline_in_page(buf, 127, 0, 63, page);

        // 2) FFTバー（バー上端を y=17 にクリップして、ヘッダと重ならないようにする）
        for (int i = 8; i < (SAMPLES/2); i++)
        {
            int val = vReal[i];
            if (val < 0) val = -val;
            val += (vImag[i] < 0 ? -vImag[i] : vImag[i]);

            int h = val * SCALE;           // 最大 48 程度を想定
            if (h <= 0) continue;
            if (h > 47) h = 47;            // 上エッジ留め：63 - 47 = 16 → 17 にクリップする

            int y1 = 63 - h;               // バー上端
            if (y1 < 17) y1 = 17;          // ヘッダ(0..15) と罫線16を避ける
            int y2 = 63;                   // 下端
            ssd1306p_vline_in_page(buf, i, y1, y2, page);
        }

        // 3) 周波数文字（8x16 に拡大。y=0 に置けばページ 0/1 のみで完結）
        int px = (SSD1306_W - (int)strlen(linebuf) * 16) / 2;
        if (px < 0) px = 0;
        ssd1306p_draw_str16_in_page_rm(buf, page, px, 0, linebuf);
    };

    ssd1306_present(cb);
}


/* ==== setup ==== */
static void setup(void)
{
    GPIO_port_enable(GPIO_port_A);
    GPIO_port_enable(GPIO_port_C);
    GPIO_port_enable(GPIO_port_D);

    GPIO_pinMode(SW1_PIN, GPIO_pinMode_I_pullDown, GPIO_Speed_10MHz);
    GPIO_pinMode(SW2_PIN, GPIO_pinMode_I_pullDown, GPIO_Speed_10MHz);
    GPIO_pinMode(ADC_PIN, GPIO_pinMode_I_analog,   GPIO_Speed_10MHz);

    GPIO_ADCinit();
    sampling_period_us = 1000000 / SAMPLING_FREQUENCY;

    ssd1306_spi_init();   // SPI
    ssd1306_init();       // OLED (page mode) & clear
    splash_screen();
}

/* ==== sampling + FFT loop ==== */
static void freqCounter_loop(void)
{
    while(1)
    {
        // sample
        uint16_t ave = 0;
        for (int i = 0; i < SAMPLES; i++)
        {
            unsigned long t = micros();
            uint8_t val = (uint8_t)(GPIO_analogRead(GPIO_Ain0_A2) >> 2);
            ave += val;
            vImag[i] = val;
            while ((micros() - t) < sampling_period_us) { /* wait */ }
        }
        ave = ave / SAMPLES;

        for (int i = 0; i < SAMPLES; i++)
        {
            vReal[i] = (int8_t)(vImag[i] - ave);
            vImag[i] = 0;
        }

        fix_fft((char *)vReal, (char *)vImag, 8, 0); // log2(256) = 8

        // magnitude reuse (rect)
        for (int i = 0; i < SAMPLES/2; i++)
            vReal[i] = ( (vReal[i] < 0 ? -vReal[i] : vReal[i]) +
                         (vImag[i] < 0 ? -vImag[i] : vImag[i]) );

        // find peak (skip DC..)
        uint8_t maxIndex = 0, maxValue = 0;
        for (int i = 8; i < SAMPLES/2; i++)
        {
            uint8_t m = (uint8_t)vReal[i];
            if (m > maxValue) { maxValue = m; maxIndex = (uint8_t)i; }
        }

        int peakFrequency = (SAMPLING_FREQUENCY / SAMPLES) * maxIndex;

        // render one frame (compose per page -> send)
        render_fft_and_freq((uint16_t)peakFrequency, maxValue);
    }
}

/* ==== main ==== */
int main(void)
{
    SystemInit();
    setup();
    Delay_Ms(2000);
    freqCounter_loop();
}

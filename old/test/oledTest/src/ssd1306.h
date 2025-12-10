/*
 * Single-File-Header for using SPI OLED (page-buffered: 128B)
 * 2025-10-01: Remove 1KB framebuffer; add 128B page composition API
 * Base: original ssd1306.h by E. Brombaugh (SPI glue kept)
 */

#ifndef _SSD1306_H
#define _SSD1306_H

#include <stdint.h>
#include <string.h>
#include "font_8x8.h"
#include "ssd1306_spi.h"

/* =========================================
 * Panel selection (keep original macros)
 * ========================================= */
#define SSD1306_PSZ 32

#if !defined (SSD1306_64X32) && !defined (SSD1306_72X40) && !defined (SSD1306_128X32) && !defined (SSD1306_128X64) && !defined (SH1107_128x128) && !(defined(SSD1306_W) && defined(SSD1306_H) && defined(SSD1306_OFFSET))
  #error "Please define the SSD1306_WXH resolution used in your application"
#endif

#ifdef SSD1306_64X32
  #define SSD1306_W 64
  #define SSD1306_H 32
  #define SSD1306_FULLUSE
  #define SSD1306_OFFSET 32
#endif

#ifdef SSD1306_72X40
  #define SSD1306_W 72
  #define SSD1306_H 40
  #define SSD1306_FULLUSE
  #define SSD1306_OFFSET 28
#endif

#ifdef SSD1306_128X32
  #define SSD1306_W 128
  #define SSD1306_H 32
  #define SSD1306_OFFSET 0
#endif

#ifdef SSD1306_128X64
  #define SSD1306_W 128
  #define SSD1306_H 64
  #define SSD1306_FULLUSE
  #define SSD1306_OFFSET 0
#endif

#ifdef SH1107_128x128
  #define SH1107
  #define SSD1306_FULLUSE
  #define SSD1306_W 128
  #define SSD1306_H 128
  #define SSD1306_OFFSET 0
#endif

/* =========================================
 * Low-level tx (reuse your SPI driver)
 * ========================================= */
static inline uint8_t ssd1306_cmd(uint8_t cmd)
{
    return ssd1306_pkt_send(&cmd, 1, 1);
}

static inline uint8_t ssd1306_data(uint8_t *data, int sz)
{
    return ssd1306_pkt_send(data, (uint8_t)sz, 0);
}

/* =========================================
 * Init sequence (original kept)
 * ========================================= */
#define SSD1306_SETCONTRAST 0x81
#define SSD1306_SEGREMAP 0xA0
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF
#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA
#define SSD1306_SETVCOMDETECT 0xDB
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9
#define SSD1306_SETMULTIPLEX 0xA8
#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10
#define SSD1306_SETSTARTLINE 0x40
#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR   0x22
#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8
#define SSD1306_CHARGEPUMP 0x8D
#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2
#define SSD1306_TERMINATE_CMDS 0xFF

#ifndef vccstate
  #define vccstate SSD1306_SWITCHCAPVCC
#endif

#if !defined(SSD1306_CUSTOM_INIT_ARRAY) || !SSD1306_CUSTOM_INIT_ARRAY
static const uint8_t ssd1306_init_array[] =
{
#ifdef SH1107
    SSD1306_DISPLAYOFF,
    0x00, 0x10, 0xb0,
    0xdc, 0x00,
    SSD1306_SETCONTRAST, 0x6f,
    SSD1306_COLUMNADDR,
    SSD1306_DISPLAYALLON_RESUME,
    SSD1306_SETMULTIPLEX, (SSD1306_H-1),
    SSD1306_SETDISPLAYOFFSET, 0x00,
    SSD1306_SETDISPLAYCLOCKDIV, 0xf0,
    SSD1306_SETPRECHARGE, 0x1d,
    SSD1306_SETVCOMDETECT, 0x35,
    SSD1306_SETSTARTLINE | 0x0,
    0xad, 0x80,
    SSD1306_SEGREMAP, 0x01,
    SSD1306_SETPRECHARGE, 0x06,
    SSD1306_SETCONTRAST, 0xfe,
    SSD1306_SETVCOMDETECT, 0xfe,
    SSD1306_SETMULTIPLEX, (SSD1306_H-1),
    SSD1306_DISPLAYON,
#else
    SSD1306_DISPLAYOFF,
    SSD1306_SETDISPLAYCLOCKDIV, 0x80,
    SSD1306_SETMULTIPLEX,
  #if defined(SSD1306_64X32)
    0x1F,
  #elif defined(SSD1306_72X40)
    0x27,
  #else
    0x3F,
  #endif
    SSD1306_SETDISPLAYOFFSET, 0x00,
    SSD1306_SETSTARTLINE | 0x0,
    SSD1306_CHARGEPUMP, 0x14,
    SSD1306_MEMORYMODE, 0x00,
    SSD1306_SEGREMAP | 0x1,
    SSD1306_COMSCANDEC,
    SSD1306_SETCOMPINS,
  #if defined(SSD1306_FULLUSE)
    0x12,
  #else
    0x22,
  #endif
    SSD1306_SETCONTRAST,
  #ifdef SSD1306_72X40
    0xAF,
  #else
    0x8F,
  #endif
    SSD1306_SETPRECHARGE, 0xF1,
    SSD1306_SETVCOMDETECT, 0x40,
    SSD1306_DISPLAYALLON_RESUME,
    SSD1306_NORMALDISPLAY,
    SSD1306_DISPLAYON,
#endif
    SSD1306_TERMINATE_CMDS
};
#endif

/* =========================================
 * 128B Page API (NEW) — no global framebuffer
 * ========================================= */
#define SSD1306_PAGES (SSD1306_H/8)

typedef void (*ssd1306_render_page_cb_t)(uint8_t page, uint8_t *buf128);

static inline void ssd1306p_set_page(uint8_t page)
{
    ssd1306_cmd(0xB0 | (page & 0x07));
}

static inline void ssd1306p_set_col(uint8_t col)
{
    ssd1306_cmd(SSD1306_SETLOWCOLUMN  | (col & 0x0F));
    ssd1306_cmd(SSD1306_SETHIGHCOLUMN | (col >> 4));
}

static inline void ssd1306p_send_page(uint8_t page, const uint8_t *buf128)
{
    ssd1306p_set_page(page);
    ssd1306p_set_col(0);
    ssd1306_data((uint8_t*)buf128, 128);
}

static inline void ssd1306p_clear(void)
{
    uint8_t zero[SSD1306_W];
    memset(zero, 0x00, sizeof(zero));
    for(uint8_t p=0; p<SSD1306_PAGES; ++p)
        ssd1306p_send_page(p, zero);
}

/* pixel set helper for a given page buffer */
static inline void ssd1306p_set_px_in_page(uint8_t *buf128, int x, int y)
{
    if((unsigned)x < SSD1306_W && (unsigned)y < SSD1306_H)
        buf128[x] |= (1u << (y & 7));
}

/* horizontal line helper (clipped) for one page */
static inline void ssd1306p_hline_in_page(uint8_t *buf128, int x1, int x2, int y)
{
    if((unsigned)y >= SSD1306_H) return;
    if(x1 > x2){ int t=x1; x1=x2; x2=t; }
    if(x2 < 0 || x1 >= SSD1306_W) return;
    if(x1 < 0) x1 = 0;
    if(x2 >= SSD1306_W) x2 = SSD1306_W-1;
    uint8_t mask = (1u << (y & 7));
    for(int x=x1; x<=x2; ++x) buf128[x] |= mask;
}

/* vertical line helper (clipped) for one page */
static inline void ssd1306p_vline_in_page(uint8_t *buf128, int x, int y1, int y2, uint8_t page)
{
    if((unsigned)x >= SSD1306_W) return;
    if(y1 > y2){ int t=y1; y1=y2; y2=t; }
    if(y2 < 0 || y1 >= SSD1306_H) return;
    if(y1 < 0) y1 = 0;
    if(y2 >= SSD1306_H) y2 = SSD1306_H-1;
    int ylo = page*8, yhi = ylo+7;
    if(y2 < ylo || y1 > yhi) return;
    int ys = (y1<ylo)?ylo:y1;
    int ye = (y2>yhi)?yhi:y2;
    uint8_t mask = ((uint8_t)((0xFFu << (ys&7)) & (0xFFu >> (7-(ye&7)))));
    buf128[x] |= mask;
}

/* Draw 8x8 ASCII string at (x,y). Only draws when page matches. */
static inline void ssd1306p_draw_str8_in_page(uint8_t *buf128, uint8_t page, int x, int y, const char *s)
{
    if(((unsigned)y>>3) != page) return;
    int xx = x;
    while(*s)
    {
        if((unsigned)xx > SSD1306_W-8) break;
        const uint8_t *g = &fontdata[((uint8_t)*s) << 3];
        for(int k=0;k<8;k++) buf128[xx+k] |= g[k];
        xx += 8; s++;
    }
}

/* 8x8 glyph (row-major in fontdata) -> draw correctly onto one page */
static inline void ssd1306p_draw_str8_in_page_rm(uint8_t *buf128, uint8_t page, int x, int y, const char *s)
{
    if(((unsigned)y>>3) != page) return;   // yがこのpageに乗る時だけ描く（等倍は1ページ内に収まる想定）

    int xx = x;
    while(*s)
    {
        if((unsigned)xx > SSD1306_W-8) break;
        const uint8_t *rows = &fontdata[((uint8_t)*s) << 3]; // rows[0..7], 1bit=1px, MSB=左端と仮定

        // 列ごとに1バイトを作る（縦8bit = ページのビット並び）
        for(int c=0; c<8; ++c)
        {
            uint8_t col = 0;
            for(int r=0; r<8; ++r)
            {
                // rows[r] の (7-c) ビット目を取り、縦ビット r に配置
                col |= ((rows[r] >> (7 - c)) & 1u) << r;
            }
            buf128[xx + c] |= col;
        }

        ++s;
        xx += 8;
    }
}

/* 8x16 (2x scale) using 8x8 glyph, split across two pages (y must be multiple of 8) */
static inline void ssd1306p_draw_str16_in_page_rm(uint8_t *buf128, uint8_t page, int x, int y, const char *s)
{
    // yは8の倍数推奨（page境界に合わせる）
    uint8_t page_top = (uint8_t)(y >> 3);

    int xx = x;
    while(*s)
    {
        if((unsigned)xx > SSD1306_W-16) break;
        const uint8_t *rows = &fontdata[((uint8_t)*s) << 3]; // 8 rows

        for(int c=0; c<8; ++c)
        {
            // まず1列（8bit縦）を作る
            uint8_t col8 = 0;
            for(int r=0; r<8; ++r)
                col8 |= ((rows[r] >> (7 - c)) & 1u) << r;

            // 縦2倍：各ビットを上下に複製 → 16bit列を想定
            // 上位8bit(下半分)と下位8bit(上半分)に分ける
            uint16_t col16 = 0;
            for(int b=0; b<8; ++b)          // b: 0..7 (元の縦bit)
            {
                if(col8 & (1u<<b))
                {
                    int t = (b<<1);         // 2倍拡大
                    col16 |= (1u<<t);
                    col16 |= (1u<<(t+1));
                }
            }

            // 横2倍：同じ列を2カラムに展開
            // 上8ビット（col16の下位8）と下8ビット（上位8）を、pageに応じて書き分け
            uint8_t upper8 =  (uint8_t)( col16       & 0xFFu);
            uint8_t lower8 =  (uint8_t)((col16 >> 8) & 0xFFu);

            if(page == page_top) {
                buf128[xx + (c<<1) + 0] |= upper8;
                buf128[xx + (c<<1) + 1] |= upper8;
            } else if(page == (uint8_t)(page_top + 1)) {
                buf128[xx + (c<<1) + 0] |= lower8;
                buf128[xx + (c<<1) + 1] |= lower8;
            }
        }

        ++s;
        xx += 16;
    }
}

/* Present whole screen by composing each page on the fly (RAM=128B) */
static inline void ssd1306_present(ssd1306_render_page_cb_t cb)
{
    static uint8_t pagebuf[SSD1306_W];  /* 128B working RAM */
    for(uint8_t p=0; p<SSD1306_PAGES; ++p)
    {
        memset(pagebuf, 0x00, sizeof(pagebuf));
        if(cb) cb(p, pagebuf);
        ssd1306p_send_page(p, pagebuf);
    }
}
#ifdef __cplusplus
template <class Fn>
static inline void ssd1306_present(Fn cb)
{
    static uint8_t pagebuf[SSD1306_W];  // 128B固定RAM（関数内静的）
    for(uint8_t p = 0; p < SSD1306_PAGES; ++p)
    {
        memset(pagebuf, 0x00, sizeof(pagebuf));
        cb(p, pagebuf);                 // 任意のコール可能オブジェクトを呼び出し
        ssd1306p_send_page(p, pagebuf);
    }
}
#endif

/* =========================================
 * Init (call once after SPI init)
 * ========================================= */
static inline uint8_t ssd1306_init(void)
{
    ssd1306_rst();

#if !defined(SSD1306_CUSTOM_INIT_ARRAY) || !SSD1306_CUSTOM_INIT_ARRAY
    const uint8_t *cmd_list = (const uint8_t *)ssd1306_init_array;
    while(*cmd_list != SSD1306_TERMINATE_CMDS)
    {
        if(ssd1306_cmd(*cmd_list++))
            return 1;
    }
#endif
    ssd1306p_clear();
    return 0;
}

#endif /* _SSD1306_H */

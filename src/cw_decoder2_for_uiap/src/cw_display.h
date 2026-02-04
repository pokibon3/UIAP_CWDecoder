//
//	CW Decoder display helpers
//
#pragma once
#include <stdint.h>

void cw_display_setup(void);
void cw_display_reset_decoder_view(void);
void cw_display_update_info(uint16_t wpm, uint8_t sw, int16_t speed);
void cw_display_enqueue_char(int16_t asciinumber);
void cw_display_tick(void);
void cw_display_draw_magnitude(int32_t magnitude);

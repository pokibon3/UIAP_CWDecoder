//
//	CW Decoder display helpers
//
#include <string.h>
#include "common.h"
#include "ch32v003fun.h"
#include "cw_display.h"

#if (TFT_WIDTH >= 240)
#define TEXT_SCALE 3
#define FONT_WIDTH 24
#define TEXT_ADVANCE 20
#define LINE_HEIGHT 28
static const int colums = 12;
#else
#define TEXT_SCALE 2
#define FONT_WIDTH 12
#define TEXT_ADVANCE 12
#define LINE_HEIGHT 20
static const int colums = 13;
#endif

static const char title1[] = " CW Decoder  ";
static const char title2[] = "  for UIAP   ";
static const char title3[] = " Version 1.4 ";
static uint8_t first_flg = 1;

static const uint16_t tone_hz[] = { 700, 800, 1000 };
#if !defined(TFT_ST7739)
static const char *tone[] = { " 700", " 800", "1000" };
#endif
static char info_last_buf[24];
static uint8_t info_sep_drawn = 0;
static uint16_t info_last_wpm = 0xFFFF;
static uint8_t info_last_sw = 0xFF;
static int16_t info_last_speed = -1;

static uint8_t draw_div = 0;
static int lcdindex = 0;
#define DISPLAY_QUEUE_SIZE 16
static int16_t display_queue[DISPLAY_QUEUE_SIZE];
static uint8_t queue_head = 0;
static uint8_t queue_tail = 0;
static uint8_t queue_count = 0;
static uint8_t scroll_in_progress = 0;
static uint8_t scroll_col = 0;
static int16_t pending_char = 0;
static uint8_t pending_valid = 0;
#if defined(TFT_ST7739)
static const uint16_t text_top = (uint16_t)((8 * FONT_SCALE_16X16 + 1) + 2);
static const uint16_t scroll_area_height = (uint16_t)(LINE_HEIGHT * 4);
#endif
#if defined(TFT_ST7739)
static uint8_t current_line = 0;
static uint8_t line0[colums];
static uint8_t line1[colums];
static uint8_t line2[colums];
static uint8_t line3[colums];
static uint8_t* const linebufs[] = { line0, line1, line2, line3 };
#else
static uint8_t current_line = 0;
static uint8_t line0[colums];
static uint8_t line1[colums];
static uint8_t line2[colums];
static uint8_t* const linebufs[] = { line0, line1, line2 };
#endif

static void display_queue_reset(void)
{
	queue_head = 0;
	queue_tail = 0;
	queue_count = 0;
	scroll_in_progress = 0;
	scroll_col = 0;
	pending_char = 0;
	pending_valid = 0;
}

static void display_queue_push(int16_t asciinumber)
{
	if (queue_count >= DISPLAY_QUEUE_SIZE) {
		queue_head = (uint8_t)((queue_head + 1) % DISPLAY_QUEUE_SIZE);
		queue_count--;
	}
	display_queue[queue_tail] = asciinumber;
	queue_tail = (uint8_t)((queue_tail + 1) % DISPLAY_QUEUE_SIZE);
	queue_count++;
}

static int display_queue_pop(int16_t *out)
{
	if (queue_count == 0) {
		return 0;
	}
	*out = display_queue[queue_head];
	queue_head = (uint8_t)((queue_head + 1) % DISPLAY_QUEUE_SIZE);
	queue_count--;
	return 1;
}

static void display_start_scroll(void)
{
	lcdindex = 0;
	current_line++;
#if defined(TFT_ST7739)
	if (current_line >= 4) {
		current_line = 3;
		for (int i = 0; i <= colums - 1 ; i++){
			line0[i] = line1[i];
			line1[i] = line2[i];
			line2[i] = line3[i];
			line3[i] = 32;
		}
		scroll_in_progress = 1;
		scroll_col = 0;
	}
#else
	if (current_line >= 3) {
		current_line = 2;
		for (int i = 0; i <= colums - 1 ; i++){
			line0[i] = line1[i];
			line1[i] = line2[i];
			line2[i] = 32;
		}
		scroll_in_progress = 1;
		scroll_col = 0;
	}
#endif
}

static void display_draw_scroll_step(void)
{
#if defined(TFT_ST7739)
	uint16_t line_y[4];
	{
		for (uint8_t i = 0; i < 4; i++) {
			line_y[i] = (uint16_t)(text_top + (LINE_HEIGHT * i));
		}
	}
	if (scroll_col < colums) {
		uint16_t x = (uint16_t)(scroll_col * TEXT_ADVANCE);
		for (uint8_t i = 0; i < 4; i++) {
			tft_set_cursor(x, line_y[i]);
			tft_print_char((char)linebufs[i][scroll_col], TEXT_SCALE);
		}
		scroll_col++;
	}
	if (scroll_col >= colums) {
		scroll_in_progress = 0;
	}
#else
	const uint16_t base_y = (uint16_t)(8 * FONT_SCALE_16X16 + 3);
	const uint16_t line_y[] = { base_y, (uint16_t)(base_y + LINE_HEIGHT), (uint16_t)(base_y + LINE_HEIGHT * 2) };
	if (scroll_col < colums) {
		uint16_t x = (uint16_t)(scroll_col * TEXT_ADVANCE);
		for (uint8_t i = 0; i < 3; i++) {
			tft_set_cursor(x, line_y[i]);
			tft_print_char((char)linebufs[i][scroll_col], TEXT_SCALE);
		}
		scroll_col++;
	}
	if (scroll_col >= colums) {
		scroll_in_progress = 0;
	}
#endif
}

void cw_display_setup(void)
{
	if (first_flg == 1) {
		const uint8_t title_gap = 4;
		uint8_t max_title_len = (uint8_t)strlen(title1);
		uint8_t title_scale_w;
		uint8_t title_scale_h;
		uint8_t title_scale;
		uint8_t title_char_w;
		uint8_t title_char_h;
		uint16_t title_block_h;
		uint16_t title_top;
		uint16_t title_y1;
		uint16_t title_y2;
		uint16_t title_y3;

		if ((uint8_t)strlen(title2) > max_title_len) max_title_len = (uint8_t)strlen(title2);
		if ((uint8_t)strlen(title3) > max_title_len) max_title_len = (uint8_t)strlen(title3);
		if (max_title_len == 0) max_title_len = 1;

		title_scale_w = (uint8_t)(TFT_WIDTH / (max_title_len * (8 - 2)));
		if (title_scale_w < 1) title_scale_w = 1;
		if (title_scale_w > 3) title_scale_w = 3;

		title_scale_h = (uint8_t)((TFT_HEIGHT - (title_gap * 2)) / (8 * 3));
		if (title_scale_h < 1) title_scale_h = 1;
		if (title_scale_h > 3) title_scale_h = 3;

		title_scale = (title_scale_w < title_scale_h) ? title_scale_w : title_scale_h;
		title_char_w = (uint8_t)((8 - 2) * title_scale);
		title_char_h = (uint8_t)(8 * title_scale);
		title_block_h = (uint16_t)(title_char_h * 3 + title_gap * 2);
		title_top = (TFT_HEIGHT > title_block_h) ? (uint16_t)((TFT_HEIGHT - title_block_h) / 2) : 0;
		title_y1 = title_top;
		title_y2 = title_top + title_char_h + title_gap;
		title_y3 = title_top + (title_char_h + title_gap) * 2;

		first_flg = 0;
		tft_fill_rect(0, 0, TFT_WIDTH, TFT_HEIGHT, BLACK);
		tft_set_cursor(0, 0);
		tft_set_color(BLUE);
		tft_set_cursor((TFT_WIDTH - (uint16_t)(strlen(title1) * title_char_w)) / 2, title_y1);
		tft_print(title1, title_scale);
		tft_set_color(RED);
		tft_set_cursor((TFT_WIDTH - (uint16_t)(strlen(title2) * title_char_w)) / 2, title_y2);
		tft_print(title2, title_scale);
		tft_set_cursor((TFT_WIDTH - (uint16_t)(strlen(title3) * title_char_w)) / 2, title_y3);
		tft_set_color(GREEN);
		tft_print(title3, title_scale);
		Delay_Ms(1000);
	}

	tft_set_color(WHITE);
	current_line = 0;
	lcdindex = 0;
#if defined(TFT_ST7739)
	for (int i = 0; i < colums; i++) {
		line0[i] = 32;
		line1[i] = 32;
		line2[i] = 32;
		line3[i] = 32;
	}
#else
	for (int i = 0; i < colums; i++) {
		line0[i] = 32;
		line1[i] = 32;
		line2[i] = 32;
	}
#endif

	info_last_buf[0] = '\0';
	info_sep_drawn = 0;
	info_last_wpm = 0xFFFF;
	info_last_sw = 0xFF;
	info_last_speed = -1;
	draw_div = 0;
	display_queue_reset();
#if defined(TFT_ST7739)
#endif
}

void cw_display_reset_decoder_view(void)
{
	tft_fill_rect(0, 0, TFT_WIDTH, TFT_HEIGHT, BLACK);
	info_last_buf[0] = '\0';
	info_sep_drawn = 0;
	info_last_wpm = 0xFFFF;
	info_last_sw = 0xFF;
	info_last_speed = -1;
	draw_div = 0;
	tft_set_color(WHITE);
	display_queue_reset();
#if defined(TFT_ST7739)
#endif
}

void cw_display_update_info(uint16_t wpm, uint8_t sw, int16_t speed)
{
	char buf[24];
	const char *mode;
	uint16_t w;
	uint8_t buf_len;
	uint8_t last_len;
	uint8_t max_len;
	uint8_t force_full = 0;
	const uint16_t info_advance = (uint16_t)((8 - 2) * FONT_SCALE_16X16);
	uint16_t x;
	int16_t dirty_min = -1;
	int16_t dirty_max = -1;

	if (wpm < 10) {
		w = 0;
	} else {
		w = wpm;
	}
	if (sw == 0) {
		mode = "US";
	} else {
		mode = "JP";
	}
	if (info_sep_drawn &&
		w == info_last_wpm &&
		sw == info_last_sw &&
		speed == info_last_speed) {
		return;
	}
	if (info_sep_drawn && speed != info_last_speed) {
		force_full = 1;
	}
	if (info_sep_drawn && sw != info_last_sw) {
		force_full = 1;
	}
#if !defined(TFT_ST7739)
	// ST7735 info line uses overlapping character advance; partial redraws can erase
	// pixels from following characters, so redraw the full line.
	force_full = 1;
#endif
#if defined(TFT_ST7739)
#if (TFT_WIDTH < 240)
	mini_snprintf(buf, sizeof(buf), "%2dW %s %dHz", w, mode, tone_hz[speed]);
#else
	mini_snprintf(buf, sizeof(buf), "%2dWPM %s Mode %4dHz", w, mode, tone_hz[speed]);
#endif
#else
	mini_snprintf(buf, sizeof(buf), "%2dWPM %s%s", w, mode, tone[speed]);
#endif
	buf_len = (uint8_t)strlen(buf);
	last_len = (uint8_t)strlen(info_last_buf);
	max_len = (buf_len > last_len) ? buf_len : last_len;
	if (max_len == 0) return;
#if defined(TFT_ST7739)
	if (!info_sep_drawn) {
		uint16_t info_h = (uint16_t)(8 * FONT_SCALE_16X16);
		uint16_t sep_y = (uint16_t)(info_h + 1);
		tft_draw_line(0, sep_y, TFT_WIDTH - 1, sep_y, GREEN);
		info_sep_drawn = 1;
	}
#else
	if (!info_sep_drawn) {
		tft_draw_line(0, 17, TFT_WIDTH - 1, 17, GREEN);
		info_sep_drawn = 1;
	}
#endif
	tft_set_color(BLUE);
	if (force_full) {
		dirty_min = 0;
		dirty_max = (int16_t)max_len - 1;
	} else {
		for (uint8_t i = 0; i < max_len; i++) {
			char now = (i < buf_len) ? buf[i] : ' ';
			char old = (i < last_len) ? info_last_buf[i] : ' ';
			if (now != old) {
				if (dirty_min < 0) dirty_min = (int16_t)i;
				dirty_max = (int16_t)i;
			}
		}
		if (dirty_min < 0) {
			tft_set_color(WHITE);
			return;
		}
		if (dirty_min > 0) dirty_min--;
		if (dirty_max + 1 < (int16_t)max_len) dirty_max++;
	}
	for (int16_t i = dirty_min; i <= dirty_max; i++) {
		char now = (i < buf_len) ? buf[i] : ' ';
		x = (uint16_t)(i * info_advance);
		tft_set_cursor(x, 0);
		tft_print_char(now, FONT_SCALE_16X16);
	}
	tft_set_color(WHITE);
	memcpy(info_last_buf, buf, buf_len + 1);
	info_last_wpm = w;
	info_last_sw = (uint8_t)sw;
	info_last_speed = speed;
}

static void cw_display_print_ascii(int16_t asciinumber)
{
#if defined(TFT_ST7739)
	uint16_t line_y[4];
	{
		for (uint8_t i = 0; i < 4; i++) {
			line_y[i] = (uint16_t)(text_top + (LINE_HEIGHT * i));
		}
	}
	{
		uint8_t* line = linebufs[current_line];
		line[lcdindex] = (uint8_t)asciinumber;
		tft_set_cursor(lcdindex * TEXT_ADVANCE, line_y[current_line]);
		tft_print_char((char)asciinumber, TEXT_SCALE);
	}
	lcdindex += 1;
#else
	const uint16_t base_y = (uint16_t)(8 * FONT_SCALE_16X16 + 3);
	const uint16_t line_y[] = { base_y, (uint16_t)(base_y + LINE_HEIGHT), (uint16_t)(base_y + LINE_HEIGHT * 2) };
	{
		uint8_t* line = linebufs[current_line];
		line[lcdindex] = (uint8_t)asciinumber;
		tft_set_cursor(lcdindex * TEXT_ADVANCE, line_y[current_line]);
		tft_print_char((char)asciinumber, TEXT_SCALE);
	}
	lcdindex += 1;
#endif
}

void cw_display_enqueue_char(int16_t asciinumber)
{
	if (asciinumber == 0) return;
	display_queue_push(asciinumber);
}

void cw_display_tick(void)
{
	if (scroll_in_progress) {
		display_draw_scroll_step();
		return;
	}
	if (pending_valid) {
		if (lcdindex > colums - 1) {
			display_start_scroll();
			if (scroll_in_progress) {
				return;
			}
		}
		cw_display_print_ascii(pending_char);
		pending_valid = 0;
		return;
	}
	{
		int16_t ch = 0;
		if (!display_queue_pop(&ch)) {
			return;
		}
		if (lcdindex > colums - 1) {
			pending_char = ch;
			pending_valid = 1;
			display_start_scroll();
			if (scroll_in_progress) {
				return;
			}
			pending_valid = 0;
		}
		cw_display_print_ascii(ch);
	}
}

void cw_display_draw_magnitude(int32_t magnitude)
{
	if (++draw_div >= 8) {
		draw_div = 0;
		tft_draw_line(TFT_WIDTH - 1, TFT_HEIGHT - 1, TFT_WIDTH - 1, 0, BLACK);
		{
			int16_t w = (int16_t)(TFT_HEIGHT - 1) - (magnitude / 8);
			if (w < 0) w = 0;
			if (w > (int16_t)(TFT_HEIGHT - 1)) w = (int16_t)(TFT_HEIGHT - 1);
			tft_draw_line(TFT_WIDTH - 1, TFT_HEIGHT - 1, TFT_WIDTH - 1, w, YELLOW);
		}
	}
}

//
//    CW Decoder header
//
#pragma once
#include <stdint.h>

int16_t decode(char *code, char *mode);
const char* encode_us_char(char c);
const char* encode_jp_char(uint8_t c);
const char* encode_us_data(uint8_t data);
const char* encode_jp_data(uint8_t data);

#define  MODE_US    0
#define  MODE_JP    1

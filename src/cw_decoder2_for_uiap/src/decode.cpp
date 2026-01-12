#include <stdint.h>
#include <string.h>
#include "decode.h"

typedef struct _code_data {
    const char *code;
    uint8_t  data;
} code_t;

static const code_t us_code[] = {
    {".-", 65},      // A
    {"-...", 66},    // B
    {"-.-.", 67},    // C
    {"-..", 68},     // D
    {".", 69},       // E
    {"..-.", 70},    // F
    {"--.", 71},     // G
    {"....", 72},    // H
    {"..", 73},      // I
    {".---", 74},    // J
    {"-.-", 75},     // K
    {".-..", 76},    // L
    {"--", 77},      // M
    {"-.", 78},      // N
    {"---", 79},     // O
    {".--.", 80},    // P
    {"--.-", 81},    // Q
    {".-.", 82},     // R
    {"...", 83},     // S
    {"-", 84},       // T
    {"..-", 85},     // U
    {"...-", 86},    // V
    {".--", 87},     // W
    {"-..-", 88},    // X
    {"-.--", 89},    // Y
    {"--..", 90},    // Z
    {".----", 49},   // 1
    {"..---", 50},   // 2
    {"...--", 51},   // 3
    {"....-", 52},   // 4
    {".....", 53},   // 5
    {"-....", 54},   // 6
    {"--...", 55},   // 7
    {"---..", 56},   // 8
    {"----.", 57},   // 9
    {"-----", 48},   // 0
    {"..--..", 63},  // ?
    {".-.-.-", 46},  // .
    {"--..--", 44},  // ,
    {"-.-.--", 33},  // !
    {".--.-.", 64},  // @
    {"---...", 58},  // :
    {"-....-", 45},  // -
    {"-..-.", 47},   // /
    {"-.--.-", 41},  // )
    {".-...", 95},   // _
    {"...-..-", 36}, // $
    {"...-.", 126},  // ~

    // The specials
    {".-.-.", 1},    // AR
    {"-.--.", 2},    // KN
    {"-...-", 3},    // BT
    {"...-.-", 4},   // VA
    {"-..---", 5},   // ホレ
    {"........", 7}, // HH (correction)
};

static const code_t jp_code[] = {
    {"--.--", 0xB1}, // ア
    {".-", 0xB2},    // イ
    {"..-", 0xB3},   // ウ
    {"-.---", 0xB4}, // エ
    {".-...", 0xB5}, // オ
    {".-..", 0xB6},  // カ
    {"-.-..", 0xB7}, // キ
    {"...-", 0xB8},  // ク
    {"-.--", 0xB9},  // ケ
    {"----", 0xBA},  // コ
    {"-.-.-", 0xBB}, // サ
    {"--.-.", 0xBC}, // シ
    {"---.-", 0xBD}, // ス
    {".---.", 0xBE}, // セ
    {"---.", 0xBF},  // ソ
    {"-.", 0xC0},    // タ
    {"..-.", 0xC1},  // チ
    {".--.", 0xC2},  // ツ
    {".-.--", 0xC3}, // テ
    {"..-..", 0xC4}, // ト
    {".-.", 0xC5},   // ナ
    {"-.-.", 0xC6},  // ニ
    {"....", 0xC7},  // ヌ
    {"--.-", 0xC8},  // ネ
    {"..--", 0xC9},  // ノ
    {"-...", 0xCA},  // ハ
    {"--..-", 0xCB}, // ヒ
    {"--..", 0xCC},  // フ
    {".", 0xCD},     // ヘ
    {"-..", 0xCE},   // ホ
    {"-..-", 0xCF},  // マ
    {"..-.-", 0xD0}, // ミ
    {"-", 0xD1},     // ム
    {"-...-", 0xD2}, // メ
    {"-..-.", 0xD3}, // モ
    {".--", 0xD4},   // ヤ
    {"-..--", 0xD5}, // ユ
    {"--", 0xD6},    // ヨ
    {"...", 0xD7},   // ラ
    {"--.", 0xD8},   // リ
    {"-.--.", 0xD9}, // ル
    {"---", 0xDA},   // レ
    {".-.-", 0xDB},  // ロ
    {"-.-", 0xDC},   // ワ
    {".-.-.", 0xDD}, // ン
    {"..", 0xDE},    // ゛
    {"..--.", 0xDF}, // ゜
    {".---", 0xA6},  // ヲ

    {".----", 49},   // 1
    {"..---", 50},   // 2
    {"...--", 51},   // 3
    {"....-", 52},   // 4
    {".....", 53},   // 5
    {"-....", 54},   // 6
    {"--...", 55},   // 7
    {"---..", 56},   // 8
    {"----.", 57},   // 9
    {"-----", 48},   // 0
    {"..--..", 63},  // ?
    {"--..--", 44},  // ,
    {"-.-.--", 33},  // !
    {".--.-.", 64},  // @
    {"---...", 58},  // :
    {"-....-", 45},  // -
    {"...-.-", 42},  // *
    {".--.-", 45},   // -

    // The specials
    {".-.-..", 0xA3}, // 」
    {".-.-.-", 0xA4}, // 、
    {"...-.",  6},    // ラタ
    {"........", 7}, // HH (correction)
};

static int16_t lookup_code(const char *code, const code_t *table, int count)
{
    for (int i = 0; i < count; i++) {
        if (strcmp(code, table[i].code) == 0) {
            return table[i].data;
        }
    }
    return 0;
}

static const char* lookup_char(uint8_t data, const code_t *table, int count)
{
    for (int i = 0; i < count; i++) {
        if (table[i].data == data) {
            return table[i].code;
        }
    }
    return nullptr;
}

const char* encode_us_data(uint8_t data)
{
    return lookup_char(data, us_code, (int)(sizeof(us_code) / sizeof(us_code[0])));
}

const char* encode_jp_data(uint8_t data)
{
    return lookup_char(data, jp_code, (int)(sizeof(jp_code) / sizeof(jp_code[0])));
}

const char* encode_us_char(char c)
{
    uint8_t data = 0;
    if (c == 'b') data = 3;        // BT
    else if (c == 'a') data = 1;   // AR
    else if (c == 'k') data = 2;   // KN
    else if (c == 'v') data = 4;   // VA
    else {
        if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
        data = (uint8_t)c;
    }

    return encode_us_data(data);
}

const char* encode_jp_char(uint8_t c)
{
    return encode_jp_data(c);
}

////////////////////////////////
// translate cw code to ascii //
////////////////////////////////
int16_t decode(char *code, char *sw)
{
    int16_t result = 0;

    if (*sw == MODE_US) {
        result = lookup_code(code, us_code, (int)(sizeof(us_code) / sizeof(us_code[0])));
        if (result == 5) {
            *sw = MODE_JP; //wabun start            
            return result;
        }
    }
    if (*sw == MODE_JP) {
        result = lookup_code(code, jp_code, (int)(sizeof(jp_code) / sizeof(jp_code[0])));
        if (result == 6) {
            *sw = MODE_US; //wabun start            
            return result;
        }        
    }
    return result;
}

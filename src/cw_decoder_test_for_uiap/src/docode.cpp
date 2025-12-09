///////////////////////////////////////////////////////////////////////
// CW Decoder made by Hjalmar Skovholm Hansen OZ1JHM  VER 1.01       //
// Feel free to change, copy or what ever you like but respect       //
// that license is <a href="http://www.gnu.org/copyleft/gpl.html" title="http://www.gnu.org/copyleft/gpl.html" rel="nofollow">http://www.gnu.org/copyleft/gpl.html</a>              //
// Discuss and give great ideas on                                   //
// <a href="https://groups.yahoo.com/neo/groups/oz1jhm/conversations/messages" title="https://groups.yahoo.com/neo/groups/oz1jhm/conversations/messages" rel="nofollow">https://groups.yahoo.com/neo/groups/oz1jhm/conversations/messages</a> //
//                                                                   //
// Modifications by KC2UEZ. Bumped to VER 1.2:                       //
// Changed to work with the Arduino NANO.                            //
// Added selection of "Target Frequency" and "Bandwith" at power up. //
///////////////////////////////////////////////////////////////////////
 
///////////////////////////////////////////////////////////////////////////
// Read more here <a href="http://en.wikipedia.org/wiki/Goertzel_algorithm" title="http://en.wikipedia.org/wiki/Goertzel_algorithm" rel="nofollow">http://en.wikipedia.org/wiki/Goertzel_algorithm</a>        //
// if you want to know about FFT the <a href="http://www.dspguide.com/pdfbook.htm" title="http://www.dspguide.com/pdfbook.htm" rel="nofollow">http://www.dspguide.com/pdfbook.htm</a> //
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Modified version by Kimio Ohe
// Modifications:
//  - port to UIAPduino Pro Micro(CH32V003)
//  - add freq detector function
//  - Refactoring of all source files
// Date: 2025-11-07
//
// このソフトウェアは GNU General Public License (GPL) に基づき配布されています。
// 改変版も同じ GPL ライセンスで再配布してください。
///////////////////////////////////////////////////////////////////////////
//
//	CW Decoder
//
#include <stdint.h>
#include <string.h>
#include "docode.h"

typedef struct _code_data {
    const char *code;
    uint8_t data;
} code_t;

//#define CODE_COUNT 36  // 定義したコードの数

// 符号とそれに対応する文字コードのマッピング
static const code_t code_map[] = {
    {".-", 65},        // A
    {"-...", 66},      // B
    {"-.-.", 67},      // C
    {"-..", 68},       // D
    {".", 69},         // E
    {"..-.", 70},      // F
    {"--.", 71},       // G
    {"....", 72},      // H
    {"..", 73},        // I
    {".---", 74},      // J
    {"-.-", 75},       // K
    {".-..", 76},      // L
    {"--", 77},        // M
    {"-.", 78},        // N
    {"---", 79},       // O
    {".--.", 80},      // P
    {"--.-", 81},      // Q
    {".-.", 82},       // R
    {"...", 83},       // S
    {"-", 84},         // T
    {"..-", 85},       // U
    {"...-", 86},      // V
    {".--", 87},       // W
    {"-..-", 88},      // X
    {"-.--", 89},      // Y
    {"--..", 90},      // Z
    {".----", 49},     // 1
    {"..---", 50},     // 2
    {"...--", 51},     // 3
    {"....-", 52},     // 4
    {".....", 53},     // 5
    {"-....", 54},     // 6
    {"--...", 55},     // 7
    {"---..", 56},     // 8
    {"----.", 57},     // 9
    {"-----", 48},     // 0
    {"..--..", 63},    // ?
    {".-.-.-", 46},    // .
    {"--..--", 44},    // ,
    {"-.-.--", 33},    // !
    {".--.-.", 64},    // @
    {"---...", 58},    // :
    {"-....-", 45},    // -
    {"-..-.", 47},     // /
};
/*
static const code_t jp_code_map[] = {
    {"--.--", 0xB1},   // ｱ
    {".-", 0xB2},       // ｲ
    {"..-", 0xB3},      // ｳ
    {"-.---", 0xB4},    // ｴ
    {".-...", 0xB5},    // ｵ
    {".-..", 0xB6},     // ｶ
    {"-.-..", 0xB7},    // ｷ
    {"...-", 0xB8},     // ｸ
    {"-.--", 0xB9},     // ｹ
    {"----", 0xBA},     // ｺ
    {"-.-.-", 0xBB},    // ｻ
    {"--.-.", 0xBC},    // ｼ
    {"---.-", 0xBD},    // ｽ
    {".---.", 0xBE},    // ｾ
    {"---.", 0xBF},     // ｿ
    {"-.", 0xC0},       // ﾀ
    {"..-.", 0xC1},     // ﾁ
    {".--.", 0xC2},     // ﾂ
    {".-.--", 0xC3},    // ﾃ
    {"..-..", 0xC4},    // ﾄ
    {".-.", 0xC5},      // ﾅ
    {"-.-.", 0xC6},     // ﾆ
    {"....", 0xC7},     // ﾇ
    {"--.-", 0xC8},     // ﾈ
    {"..--", 0xC9},     // ﾉ
    {"-...", 0xCA},     // ﾊ
    {"--..-", 0xCB},    // ﾋ
    {"--..", 0xCC},     // ﾌ
    {".", 0xCD},        // ﾍ
    {"-..", 0xCE},      // ﾎ
    {"-..-", 0xCF},     // ﾏ
    {"..-.-", 0xD0},    // ﾐ
    {"-", 0xD1},        // ﾑ
    {"-...-", 0xD2},    // ﾒ
    {"-..-.", 0xD3},    // ﾓ
    {".--", 0xD4},      // ﾔ
    {"-..--", 0xD5},    // ﾕ
    {"--", 0xD6},       // ﾖ
    {"...", 0xD7},      // ﾗ
    {"--.", 0xD8},      // ﾘ
    {"-.--.", 0xD9},    // ﾙ
    {"---", 0xDA},      // ﾚ
    {".-.-", 0xDB},     // ﾛ
    {"-.-", 0xDC},      // ﾜ
    {".-.-.", 0xDD},    // ﾝ
    {"..", 0xDE},       // ﾀﾞｸﾃﾝ
    {"..--.", 0xDF},    // ﾊﾝﾀﾞｸﾃﾝ
    {".---", 0xA6},     // ｦ
    {".----", 49},      // 1
    {"..---", 50},      // 2
    {"...--", 51},      // 3
    {"....-", 52},      // 4
    {".....", 53},      // 5
    {"-....", 54},      // 6
    {"--...", 55},      // 7
    {"---..", 56},      // 8
    {"----.", 57},      // 9
    {"-----", 48},      // 0
    {"..--..", 63},     // ?
    {"--..--", 44},     // ,
    {"-.-.--", 33},     // !
    {".--.-.", 64},     // @
    {"---...", 58},     // :
    {"-....-", 45},     // -
    {"...-.-", 42},     // *
};
*/
////////////////////////////////
// translate cw code to ascii //
////////////////////////////////
// モード判定
int16_t docode(char *code, char *sw)
{
/*
    if (strcmp(code, "-..---") == 0) {
        *sw = MODE_JP;  // wabun start
        return 0;
    }
    if (strcmp(code, "...-.") == 0) {
        *sw = MODE_US;  // wabun end
        return 0;
    }
*/    
    // USモードの処理
//    if (*sw == MODE_US) {
        for (int i = 0; i < (int)(sizeof(code_map) / sizeof(code_map[0])); i++) {
            if (strcmp(code, code_map[i].code) == 0) {
                return code_map[i].data;  // 一致するコードが見つかれば返す
            }
        }
//    }
/*    
    // JPモードの処理
    if (*sw == 1) {  // wabun モード
        for (int i = 0; i < (int)(sizeof(jp_code_map) / sizeof(jp_code_map[0])); i++) {
            if (strcmp(code, jp_code_map[i].code) == 0) {
                return jp_code_map[i].data;
            }
        }
    }
*/
    return 0;  // 一致しなければ0を返す
}



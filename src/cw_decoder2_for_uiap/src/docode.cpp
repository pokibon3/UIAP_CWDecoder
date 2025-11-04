//
//	CW Decoder
//
#include <stdint.h>
#include <string.h>
#include "docode.h"

typedef struct _code_data {
    char    *code;
    uint8_t  data;
} code_t;

#if 0
code_t us_code[] = {
    {".-", 	    65},
	{"-...",    66},
	{"-.-.",    67},
	{"-..", 	68},
	{".", 	    69},
	{"..-.",    70},
	{"--.", 	71},
	{"....",    72},
	{"..", 	    73},
	{".---",    74},
	{"-.-", 	75},
	{".-..",    76},
	{"--", 	    77},
	{"-.", 	    78},
	{"---", 	79},
	{".--.",    80},
	{"--.-",    81},
	{".-.", 	82},
	{"...", 	83},
	{"-", 	    84},
	{"..-", 	85},
	{"...-",    86},
	{".--", 	87},
	{"-..-",    88},
	{"-.--",    89},
	{"--..",    90},

	{".----",   49},
	{"..---",   50},
	{"...--",   51},
	{"....-",   52},
	{".....",   53},
	{"-....",   54},
	{"--...",   55},
	{"---..",   56},
	{"----.",   57},
	{"-----",   48},

	{"..--..",  63},
	{".-.-.-",  46},
	{"--..--",  44},
	{"-.-.--",  33},
	{".--.-.",  64},
	{"---...",  58},
	{"-....-",  45},
	{"-..-.",   47},

	{"-.--.", 	40},
	{"-.--.-", 	41},
	{".-...", 	95},
	{"...-..-", 36},
	{"...-.-", 	62},
	{".-.-.", 	60},
	{"...-.", 	126},
	//////////////////
	// The specials //
	//////////////////
//	{".-.-", 	3},
//	{"---.", 	4},
//	{".--.-", 	6},
};


////////////////////////////////
// translate cw code to ascii //
////////////////////////////////
int16_t docode(char *code, char *sw) 
{
    for (int i = 0; i < sizeof(us_code) / sizeof(code_t); i++) {
        if (strcmp(code, us_code[i].code) == 0) {
            return us_code[i].data;
        };
    }
    return 0;
}
#else
////////////////////////////////
// translate cw code to ascii //
////////////////////////////////
int16_t docode(char *code, char *sw)
{
  if (strcmp(code,"-..---") == 0) *sw = MODE_JP; //wabun start 
  if (strcmp(code,"...-.")  == 0) *sw = MODE_US;  //wabunn end
  if (*sw == MODE_US) {
    if (strcmp(code,".-..-.") == 0) {
		*sw = MODE_JP;
      	return(0x29); 
    }
    //Alphabets
    if (strcmp(code,".-")     == 0) return(65);   // A
    if (strcmp(code,"-...")   == 0) return(66);   // B
    if (strcmp(code,"-.-.")   == 0) return(67);   // C
    if (strcmp(code,"-..")    == 0) return(68);   // D
    if (strcmp(code,".")      == 0) return(69);   // E
    if (strcmp(code,"..-.")   == 0) return(70);   // F
    if (strcmp(code,"--.")    == 0) return(71);   // G
    if (strcmp(code,"....")   == 0) return(72);   // H
    if (strcmp(code,"..")     == 0) return(73);   // I
    if (strcmp(code,".---")   == 0) return(74);   // J
    if (strcmp(code,"-.-")    == 0) return(75);   // K
    if (strcmp(code,".-..")   == 0) return(76);   // L
    if (strcmp(code,"--")     == 0) return(77);   // M
    if (strcmp(code,"-.")     == 0) return(78);   // N
    if (strcmp(code,"---")    == 0) return(79);   // O
    if (strcmp(code,".--.")   == 0) return(80);   // P
    if (strcmp(code,"--.-")   == 0) return(81);   // Q
    if (strcmp(code,".-.")    == 0) return(82);   // R
    if (strcmp(code,"...")    == 0) return(83);   // S
    if (strcmp(code,"-")      == 0) return(84);   // T
    if (strcmp(code,"..-")    == 0) return(85);   // U
    if (strcmp(code,"...-")   == 0) return(86);   // V
    if (strcmp(code,".--")    == 0) return(87);   // W
    if (strcmp(code,"-..-")   == 0) return(88);   // X
    if (strcmp(code,"-.--")   == 0) return(89);   // Y
    if (strcmp(code,"--..")   == 0) return(90);   // Z
    if (strcmp(code,".----")  == 0) return(49);   // 1
    if (strcmp(code,"..---")  == 0) return(50);   // 2
    if (strcmp(code,"...--")  == 0) return(51);   // 3
    if (strcmp(code,"....-")  == 0) return(52);   // 4
    if (strcmp(code,".....")  == 0) return(53);   // 5
    if (strcmp(code,"-....")  == 0) return(54);   // 6
    if (strcmp(code,"--...")  == 0) return(55);   // 7
    if (strcmp(code,"---..")  == 0) return(56);   // 8
    if (strcmp(code,"----.")  == 0) return(57);   // 9
    if (strcmp(code,"-----")  == 0) return(48);   // 0

    if (strcmp(code,"..--..") == 0) return(63);   // ?
    if (strcmp(code,".-.-.-") == 0) return(46);   // .
    if (strcmp(code,"--..--") == 0) return(44);   // ,
    if (strcmp(code,"-.-.--") == 0) return(33);   // !
    if (strcmp(code,".--.-.") == 0) return(64);   // @
    if (strcmp(code,"---...") == 0) return(58);   // :
    if (strcmp(code,"-....-") == 0) return(45);   // -
    if (strcmp(code,"-..-.")  == 0) return(47);   // /
    if (strcmp(code,"-.--.")  == 0) return(40);   // (
    if (strcmp(code,"-.--.-") == 0) return(41);   // )
    if (strcmp(code,".-...")  == 0) return(95);   // _
    if (strcmp(code,"...-..-")== 0) return(36);   // $ 
    if (strcmp(code,"...-.-") == 0) return(42);   // *
    if (strcmp(code,".-.-.")  == 0) return(60);   // <
    if (strcmp(code,"...-.")  == 0) return(126);  // ~
  ////////////////// 
  // The specials // 
  ////////////////// 
//    if (strcmp(code,".-.-")   == 0) return(3); 
//    if (strcmp(code,"---.")   == 0) return(4);
//    if (strcmp(code,".--.-")  == 0) return(6);
  } 
  if (*sw == 1) { //wabun 
    if (strcmp(code,"-.--.-") == 0) {
      *sw = 0;
      return(0x28); 
    } 
    //wabun code
    if (strcmp(code,"--.--")  == 0) return(0xB1);   // ｱ
    if (strcmp(code,".-")     == 0) return(0xB2);   // ｲ
    if (strcmp(code,"..-")    == 0) return(0xB3);   // ｳ
    if (strcmp(code,"-.---")  == 0) return(0xB4);   // ｴ
    if (strcmp(code,".-...")  == 0) return(0xB5);   // ｵ
    if (strcmp(code,".-..")   == 0) return(0xB6);   // ｶ
    if (strcmp(code,"-.-..")  == 0) return(0xB7);   // ｷ
    if (strcmp(code,"...-")   == 0) return(0xB8);   // ｸ
    if (strcmp(code,"-.--")   == 0) return(0xB9);   // ｹ
    if (strcmp(code,"----")   == 0) return(0xBA);   // ｺ
    if (strcmp(code,"-.-.-")  == 0) return(0xBB);   // ｻ
    if (strcmp(code,"--.-.")  == 0) return(0xBC);   // ｼ
    if (strcmp(code,"---.-")  == 0) return(0xBD);   // ｽ
    if (strcmp(code,".---.")  == 0) return(0xBE);   // ｾ
    if (strcmp(code,"---.")   == 0) return(0xBF);   // ｿ
    if (strcmp(code,"-.")     == 0) return(0xC0);   // ﾀ
    if (strcmp(code,"..-.")   == 0) return(0xC1);   // ﾁ
    if (strcmp(code,".--.")   == 0) return(0xC2);   // ﾂ
    if (strcmp(code,".-.--")  == 0) return(0xC3);   // ﾃ
    if (strcmp(code,"..-..")  == 0) return(0xC4);   // ﾄ
    if (strcmp(code,".-.")    == 0) return(0xC5);   // ﾅ
    if (strcmp(code,"-.-.")   == 0) return(0xC6);   // ﾆ
    if (strcmp(code,"....")   == 0) return(0xC7);   // ﾇ
    if (strcmp(code,"--.-")   == 0) return(0xC8);   // ﾈ
    if (strcmp(code,"..--")   == 0) return(0xC9);   // ﾉ
    if (strcmp(code,"-...")   == 0) return(0xCA);   // ﾊ
    if (strcmp(code,"--..-")  == 0) return(0xCB);   // ﾋ
    if (strcmp(code,"--..")   == 0) return(0xCC);   // ﾌ
    if (strcmp(code,".")      == 0) return(0xCD);   // ﾍ
    if (strcmp(code,"-..")    == 0) return(0xCE);   // ﾎ
    if (strcmp(code,"-..-")   == 0) return(0xCF);   // ﾏ
    if (strcmp(code,"..-.-")  == 0) return(0xD0);   // ﾐ
    if (strcmp(code,"-")      == 0) return(0xD1);   // ﾑ
    if (strcmp(code,"-...-")  == 0) return(0xD2);   // ﾒ
    if (strcmp(code,"-..-.")  == 0) return(0xD3);   // ﾓ
    if (strcmp(code,".--")    == 0) return(0xD4);   // ﾔ
    if (strcmp(code,"-..--")  == 0) return(0xD5);   // ﾕ
    if (strcmp(code,"--")     == 0) return(0xD6);   // ﾖ
    if (strcmp(code,"...")    == 0) return(0xD7);   // ﾗ
    if (strcmp(code,"--.")    == 0) return(0xD8);   // ﾘ
    if (strcmp(code,"-.--.")  == 0) return(0xD9);   // ﾙ
    if (strcmp(code,"---")    == 0) return(0xDA);   // ﾚ
    if (strcmp(code,".-.-")   == 0) return(0xDB);   // ﾛ
    if (strcmp(code,"-.-")    == 0) return(0xDC);   // ﾜ
    if (strcmp(code,".-.-.")  == 0) return(0xDD);   // ﾝ
    if (strcmp(code,"..")     == 0) return(0xDE);   // ﾀﾞｸﾃﾝ
    if (strcmp(code,"..--.")  == 0) return(0xDF);   // ﾊﾝﾀﾞｸﾃﾝ
    if (strcmp(code,".---")   == 0) return(0xA6);   // ｦ
    if (strcmp(code,".----")  == 0) return(49);     // 1
    if (strcmp(code,"..---")  == 0) return(50);     // 2
    if (strcmp(code,"...--")  == 0) return(51);     // 3
    if (strcmp(code,"....-")  == 0) return(52);     // 4
    if (strcmp(code,".....")  == 0) return(53);     // 5
    if (strcmp(code,"-....")  == 0) return(54);     // 6
    if (strcmp(code,"--...")  == 0) return(55);     // 7
    if (strcmp(code,"---..")  == 0) return(56);     // 8
    if (strcmp(code,"----.")  == 0) return(57);     // 9
    if (strcmp(code,"-----")  == 0) return(48);     // 0
    if (strcmp(code,"..--..") == 0) return(63);     // ?
    //if (strcmp(code,".-.-.-") == 0) return(46);   // .
    if (strcmp(code,"--..--") == 0) return(44);     // ,
    if (strcmp(code,"-.-.--") == 0) return(33);     // !
    if (strcmp(code,".--.-.") == 0) return(64);     // @
    if (strcmp(code,"---...") == 0) return(58);     // :
    if (strcmp(code,"-....-") == 0) return(45);     // -
    //if (strcmp(code,"-..-.") == 0) return(47);    // /
    //if (strcmp(code,"-.--.") == 0) return(40);    // (
    //if (strcmp(code,"-.--.-") == 0) return(41);   // )
    //if (strcmp(code,".-...") == 0) return(95);    // _
    //if (strcmp(code,"...-..-") == 0) return(36);  // #
    if (strcmp(code,"...-.-") == 0) return(42);     // *
    //if (strcmp(code,".-.-.") == 0) return(60);    // <
    //if (strcmp(code,"...-.") == 0) return(126);   // ~
    if (strcmp(code,".--.-")  == 0) return(45);     // -
    //////////////////
    // The specials //
    ////////////////// 
    if (strcmp(code,".-.-..") == 0) return(0xA3);   // ]
    if (strcmp(code,".-.-.-") == 0) return(0xA4);   // ,
    //if (strcmp(code,".-.-") == 0) return(3);
    //if (strcmp(code,"---.") == 0) return(4);
    //if (strcmp(code,".--.-")== 0) return(6);
  }

	return 0;
}
#endif

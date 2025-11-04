#include <string.h>

////////////////////////////////
// translate cw code to ascii //
////////////////////////////////
int docode(char *code)
{
    if (strcmp(code,".-")   == 0) return(65);
	if (strcmp(code,"-...") == 0) return(66);
	if (strcmp(code,"-.-.") == 0) return(67);
	if (strcmp(code,"-..")  == 0) return(68);
	if (strcmp(code,".")    == 0) return(69);
	if (strcmp(code,"..-.") == 0) return(70);
	if (strcmp(code,"--.")  == 0) return(71);
	if (strcmp(code,"....") == 0) return(72);
	if (strcmp(code,"..")   == 0) return(73);
	if (strcmp(code,".---") == 0) return(74);
	if (strcmp(code,"-.-")  == 0) return(75);
	if (strcmp(code,".-..") == 0) return(76);
	if (strcmp(code,"--")   == 0) return(77);
	if (strcmp(code,"-.")   == 0) return(78);
	if (strcmp(code,"---")  == 0) return(79);
	if (strcmp(code,".--.") == 0) return(80);
	if (strcmp(code,"--.-") == 0) return(81);
	if (strcmp(code,".-.")  == 0) return(82);
	if (strcmp(code,"...")  == 0) return(83);
	if (strcmp(code,"-")    == 0) return(84);
	if (strcmp(code,"..-")  == 0) return(85);
	if (strcmp(code,"...-") == 0) return(86);
	if (strcmp(code,".--")  == 0) return(87);
	if (strcmp(code,"-..-") == 0) return(88);
	if (strcmp(code,"-.--") == 0) return(89);
	if (strcmp(code,"--..") == 0) return(90);

	if (strcmp(code,".----") == 0) return(49);
	if (strcmp(code,"..---") == 0) return(50);
	if (strcmp(code,"...--") == 0) return(51);
	if (strcmp(code,"....-") == 0) return(52);
	if (strcmp(code,".....") == 0) return(53);
	if (strcmp(code,"-....") == 0) return(54);
	if (strcmp(code,"--...") == 0) return(55);
	if (strcmp(code,"---..") == 0) return(56);
	if (strcmp(code,"----.") == 0) return(57);
	if (strcmp(code,"-----") == 0) return(48);

	if (strcmp(code,"..--..") == 0) return(63);
	if (strcmp(code,".-.-.-") == 0) return(46);
	if (strcmp(code,"--..--") == 0) return(44);
	if (strcmp(code,"-.-.--") == 0) return(33);
	if (strcmp(code,".--.-.") == 0) return(64);
	if (strcmp(code,"---...") == 0) return(58);
	if (strcmp(code,"-....-") == 0) return(45);
	if (strcmp(code,"-..-.")  == 0) return(47);

	if (strcmp(code,"-.--.")   == 0) return(40);
	if (strcmp(code,"-.--.-")  == 0) return(41);
	if (strcmp(code,".-...")   == 0) return(95);
	if (strcmp(code,"...-..-") == 0) return(36);
	if (strcmp(code,"...-.-")  == 0) return(62);
	if (strcmp(code,".-.-.")   == 0) return(60);
	if (strcmp(code,"...-.")   == 0) return(126);
	//////////////////
	// The specials //
	//////////////////
	if (strcmp(code,".-.-")     == 0) return(3);
	if (strcmp(code,"---.")     == 0) return(4);
	if (strcmp(code,".--.-")    == 0) return(6);
}
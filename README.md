# CW Decoder for UIAPduino 

UIAP Pro Microで動作するサンプルプログラムです。  
ビルド環境はVSCode + ch32v003funが必要です。  
書き込は、WCH-LinkEまたは、UIAPのUSB端子から行います。

## cw_decoder2_for_uiap
モールス信号のデコードと、音声信号のFFTアナライザの2つのモードが使用できます。  
MODE SWを押下することで切り換え可能です。  
製作方法、使用方法等は[R16 Friendship Radio Scrapbox](https://scrapbox.io/r16fr/UIAPduino%E3%82%92%E4%BD%BF%E3%81%A3%E3%81%9FCW_Decoder%E3%81%AE%E8%A3%BD%E4%BD%9C)を参照してください。

■ モールス信号デコード  
モールス信号をMIC入力し、デコードした結果を表示します。    
    SW1 : 英文/和文切り換え  
    SW2 : 666Hz / 888Hz / 1000Hz切り換え

■ FFTアナライザ  
　約2.5kHzまでのオーディオ信号のスペクトルを表示します。  
　ピーク周波数を表示します。


■ 変更履歴
- V1.0  
・ 新規リリース
- V1.1  
・ オーディオ入力のダイナミックレンジ拡大  
・ 欧文/和文切換えのBug Fix
・ モールスデコーダーのデフォルト周波数を600Hzに変更  
・ FFTアナライザのスプラッシュ表示削除

■ ライセンス
※本ソフトはOZIJHM氏のArduino用CW DECODERをCH32V003+ch32v003funの環境に移植したものです。  
 http://oz1jhm.dk/content/very-simpel-cw-decoder-easy-build
 
///////////////////////////////////////////////////////////////////////  
// CW Decoder made by Hjalmar Skovholm Hansen OZ1JHM  VER 1.01       //  
// Feel free to change, copy or what ever you like but respect       //  
// that license is <a href="http://www.gnu.org/copyleft/gpl.html"   title="http://www.gnu.org/copyleft/gpl.html" rel="nofollow">http://www.gnu.org/copyleft/gpl.html</a>              //  
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
// このソフトウェアは GNU General Public License (GPL) に基づき配布されています。  
// 改変版も同じ GPL ライセンスで再配布してください。  
///////////////////////////////////////////////////////////////////////////  


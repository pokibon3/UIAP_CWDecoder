# UIAPduino Pro Micro Sample Programs

UIAP Pro Microで動作するサンプルプログラムです。  
ビルド環境はVSCode + ch32v003funが必要です。  
書き込は、WCH-LinkEまたは、UIAPのUSB端子から行います。

## freqcounter
MIC入力を6000Hzでサンプリングし、FFT解析結果をスペアナ風に表示します。  
最大表示周波数は3KHz、ピーク周波数表示付きです。

## cw_decoder_for_uiap
モールス信号のデコードと、音声信号のFFTアナライザの2つのモードが使用できます。  
MODE SWを押下することで切り換え可能です。

■ モールス信号デコード  
モールス信号をMIC入力し、デコードした結果を表示します。    
    SW1 : 英文/和文切り換え  
    SW2 : 666Hz / 888Hz / 1000Hz切り換え

■ FFTアナライザ  
　約2.5kHzまでのオーディオ信号のスペクトルを表示します。  
　ピーク周波数を表示します。

※本ソフトはOZIJHM氏のArduino用CW DECODERをCH32V003+ch32v003funの環境に移植したものです。  
 http://oz1jhm.dk/content/very-simpel-cw-decoder-easy-build
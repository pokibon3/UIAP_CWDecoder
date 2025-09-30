# UIAPduino Pro Micro Sample Programs

UIAP Pro Microで動作するサンプルプログラムです。  
ビルド環境はVSCode + ch32v003funが必要です。  
書き込みには、WCH-LinkEが必要です。

## freqcounter
MIC入力を8000Hzでサンプリングし、FFT解析結果をスペアナ風に表示します。  
最大表示周波数は約3KHz、ピーク周波数表示付きです。

## cw_decoder_ik
モールス信号をMIC入力し、デコードした結果を表示します。
    SW1 : 英文/和文切り換え
    SW2 : 666Hz / 888Hz / 1000Hz切り換え

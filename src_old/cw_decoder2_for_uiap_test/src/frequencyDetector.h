//
//	Frequency Detector Header
//
#pragma once

#define FD_SAMPLING_FREQUENCY 6000	// Hz	+10%

extern int fd_setup(void);
extern int freqDetector(int8_t *vReal, int8_t *vImag    );

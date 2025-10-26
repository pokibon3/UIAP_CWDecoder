#include <stdio.h>
#include <math.h>

#define N 100                 //  BW = sampling_freq / N
//static float sampling_freq  = 8928.0;
static float sampling_freq  = 8000.0;
//static float target_freq=     558.0;
//static float target_freq=     666.7;
static float target_freq=     888.8;
//short int Data[N];

static float omega, coeff, cosine;
static float  Q0, Q1, Q2, mag2;
//static int rc ,i, k;

void initGoertzel(float freq)
{
//    int16_t k = (int) (0.5 + ((N * target_freq) / sampling_freq));
    int16_t k = (int) (0.5 + ((N * freq) / sampling_freq));
    omega = (2.0 * M_PI * k) / N;
    cosine = cos(omega);
    coeff = 2.0 * cosine;  
}

float goertzel(int16_t *data, int16_t n)
{
    Q1 = 0.0; 
    Q2 = 0.0;
    for (int16_t i = 0; i < n; i++) {
        Q0 = coeff * Q1 - Q2 + (float)(data[i]);
        Q2 = Q1; 
        Q1 = Q0;
    }
    mag2 = (Q1 * Q1) + (Q2 * Q2) - Q1 * Q2 * coeff;
    return sqrt(mag2);
}


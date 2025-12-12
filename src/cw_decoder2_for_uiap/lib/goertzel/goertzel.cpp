#include <stdio.h>
#include <math.h>

#define N 100                 //  BW = sampling_freq / N
//static float sampling_freq  = 8928.0;
//static float sampling_freq  = 8000.0;
//static float target_freq=     558.0;
//static float target_freq=     666.7;
//static float target_freq=     888.8;
//short int Data[N];

//static float omega, coeff, cosine;
static int16_t coeff100;
static int32_t  Q0, Q1, Q2, mag2;
//static int rc ,i, k;

void setSpeed(int16_t speed)
{
    switch(speed) {
        case 0 :
            coeff100 = 173;         // 666.7Hz
            break;
        case 1 :
            coeff100 = 158;         // 833.3Hz
            break;
        case 2 :
            coeff100 = 141;         // 1000.0Hz
            break;
        default :
            coeff100 = 173;
    }
}

// floor(sqrt(x)) を返す
static uint32_t isqrt32(uint32_t x)
{
    uint32_t op  = x;
    uint32_t res = 0;
    uint32_t one = 1u << 30;   // 2^30（32bit の「2番目に高いビット」）

    // one を、x 以下の最大の 4 の冪に調整
    while (one > op) {
        one >>= 2;
    }

    while (one != 0) {
        if (op >= res + one) {
            op  -= res + one;
            res += one << 1;
        }
        res >>= 1;
        one >>= 2;
    }

    return res;   // これが floor(sqrt(x))
}
void initGoertzel(int16_t speed)
{   
//    int16_t k = (int) (0.5 + ((N * target_freq) / sampling_freq));
//    omega = (2.0 * M_PI * k) / N;
//    cosine = cos(omega);
//    coeff = 2.0 * cosine;
//    printf("coeff = %d\n", (int16_t)coeff * 100);
//    while(1);
//    coeff100 = 185;             // coeff = coeff100/100   target_freq = 666.7
//    coeff100 = 154;             // coeff = coeff100/100    target_frea = 888.8
//    coeff100 = 100;             // coeff = coeff100/100    target_frea = 1000.0
        setSpeed(speed);
}

int32_t goertzel(int16_t *data, int16_t n)
{
    Q1 = 0; 
    Q2 = 0;
    for (int16_t i = 0; i < n; i++) {
        Q0 = coeff100 * Q1 / 100 - Q2 + data[i];
        Q2 = Q1; 
        Q1 = Q0;
    }
    mag2 = (Q1 * Q1) + (Q2 * Q2) - Q1 * Q2 * coeff100 / 100;

    return isqrt32(mag2);
}
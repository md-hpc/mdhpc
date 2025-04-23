#ifndef COMMON_H
#define COMMON_H

#include <math.h>

using namespace std;

// The potential well is centered at (26/7)^(1/6)*SIGMA
#define R_MAX (SIGMA*powf(26/7,1/6))
#define LJ(r) 4*EPSILON*(6*powf(SIGMA,6)/powf(r,8)-12*powf(SIGMA,12)/powf(r,14))
#define LJ_P(r) 4*EPSILON*(powf(SIGMA/r,12)-powf(SIGMA/r,6))

class timer {
public:
    timer();

    void start();
    void stop();
    unsigned long get();

private:
    unsigned long time;
    unsigned long last;
    bool running;
};

float frand();
unsigned long rdtsc();

#endif

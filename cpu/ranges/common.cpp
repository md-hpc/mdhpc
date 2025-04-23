#include <cstdio>
#include <climits>
#include <cstdlib>

#include "common.h"

using namespace std;

timer::timer() : time(0), running(false) {}

void timer::start() {
    last = rdtsc();
}

void timer::stop() {
    time += rdtsc() - last;
}

unsigned long timer::get() {
    return time;
}

float frand() {
	return ((float) random()) / ((float) INT_MAX);
}

unsigned long rdtsc() {
  union {
    unsigned long long int64;
    struct {unsigned int lo, hi;} int32;
  } p;
  __asm__ __volatile__ (
    "rdtsc" :
    "=a" (p.int32.lo),
    "=d"(p.int32.hi)
  );
  return p.int64;
}

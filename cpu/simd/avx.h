#ifndef I_AVX
#define I_AVX 1

#include <immintrin.h>

typedef int i8 __attribute__ ((vector_size(32)));
typedef float f8 __attribute__((vector_size(32)));
#define VBYTES sizeof(f8)
#define VSIZE (sizeof(f8)/sizeof(float))

typedef union {
	f8 v;
	float d[VSIZE];
} pack;

typedef union {
	i8 v;
	int d[VSIZE];
} ipack;

#define VAI(v,i) (((pack*)v)->d[i])
#define VI(v,i) (((pack*)&(v))->d[i])
#define VK(k) {k,k,k,k,k,k,k,k}

f8 permute(f8 x);
int alleq(i8 a, int b);
f8 clipv(f8 r, float c);
f8 sqrtv(f8 r);
f8 n2c(f8 x, float c);
float sum(f8 x);

__m256 _mm256_abs_ps(__m256 a);


#endif

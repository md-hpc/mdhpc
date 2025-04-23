#include "avx.h"

f8 clipv(f8 v, float m) {
	__m256 min = _mm256_set1_ps(m);
	__m256 a = (__m256) v;
	__m256 mask = _mm256_cmp_ps(a, min, _CMP_GT_OQ);

	a = _mm256_blendv_ps(a, min, mask);
	return (f8) a;
}

f8 sqrtv(f8 va) {
	return (f8) _mm256_sqrt_ps((__m256) va);
}

f8 permute(f8 x) {
	pack a;
	pack b;

	a.v = x;
	for (int i = 0; i < VSIZE; i++) {
		b.d[(i+1)%VSIZE] = a.d[i];
	}
	return b.v;
}

int alleq(i8 va, int sb) {
	__m256i a, b, m;
	int mask;

	a = (__m256i) va;
	b = _mm256_set1_epi32(sb);
	m = _mm256_cmpeq_epi32(a,b);
	
	mask = _mm256_movemask_ps(_mm256_castsi256_ps(m));
	return mask == 0xff;
}

__m256 _mm256_abs_ps(__m256 a) {
	__m256 k = _mm256_set1_ps(-1);
	__m256 z = _mm256_set1_ps(0);
	__m256 n = _mm256_mul_ps(a,k);
	__m256 m = _mm256_cmp_ps(a,z,_CMP_LT_OQ);
	__m256 r = _mm256_blendv_ps(a,n,m);
	return r;
}

f8 n2c(f8 x, float c) {
	__m256 m = _mm256_cmp_ps(x,x,_CMP_UNORD_Q);
	__m256 k = _mm256_set1_ps(c);
	return _mm256_blendv_ps(x,k,m);
}

float sum(f8 x) {
	pack p = {.v = x};
	float s = 0;
	for (int i = 0; i < VSIZE; i++)
		s += p.d[i];
	return s;
}

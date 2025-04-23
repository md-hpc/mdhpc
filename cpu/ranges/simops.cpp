#include <math.h>
#include <stdio.h>

#include "simulation.h"

float simulation::apbcf(float x) {
	return x < 0 ? x + L : x > L ? x - L : x;
}

void simulation::apbcfv(vec8 &v) {
	v.x = apbcfv(v.x);
	v.y = apbcfv(v.y);
	v.z = apbcfv(v.z);
}

f8 simulation::apbcfv(f8 vx) {
	__m256 x, mlt, mgt, z, l, xu, xl;

	x = (__m256) vx;
	z = _mm256_set1_ps(0);
	l = _mm256_set1_ps(L);

	xu = (__m256) (vx + l);
	xl = (__m256) (vx - l);
	
	mlt = _mm256_cmp_ps(x,z,_CMP_LT_OQ);
	mgt = _mm256_cmp_ps(x,l,_CMP_GT_OQ);
	x = _mm256_blendv_ps(x, xu, mlt);
	x = _mm256_blendv_ps(x, xl, mgt);

	return (f8) x;
}

int simulation::apbci(int i) {
	return i < 0 ? i + UNIVERSE_SIZE : i >= UNIVERSE_SIZE ? i - UNIVERSE_SIZE : i;
}


i8 simulation::cellv(const vec8 &r) {
	i8 i, j, k;
	const int u = UNIVERSE_SIZE;	
	const float c = CUTOFF;
	
	i = (i8) _mm256_cvttps_epi32(r.x / c);
	j = (i8) _mm256_cvttps_epi32(r.y / c);

	return i + j * u;
}

int simulation::cell(const vec &v) {
	int i,j,k;

	i = (int) (v.x / CUTOFF);
	j = (int) (v.y / CUTOFF);

	return i + j * UNIVERSE_SIZE;
}

vec8 simulation::lj(const vec8 &rp, const vec8 &np) {
	const float ep4 = EP4;
	const float spss = SPSS;
	const float tpst = TPST;
	const float one = 1;
	const float dt = DT;
	
	vec8 rv = submv(rp, np);
	
	f8 r = sqrtv(rv.x * rv.x + rv.y * rv.y + rv.z * rv.z); 
	r = one / r;
	
	f8 r8 = VK(one), r14 = VK(one);
	for (int i = 0; i < 8; i++)
		r8 *= r;
	for (int i = 0; i < 14; i++)
		r14 *= r;
	
	__m256 f = (__m256) (dt * clipv(ep4 * (spss * r8 + tpst * r14), LJ_MIN));
	// some of the members of fm may be nan because nan is used to represent
	// an empty particle. We need to mask those out
	__m256 nanm = _mm256_cmp_ps(f, f, _CMP_UNORD_Q);
	__m256 zero = _mm256_set1_ps(0);
	__m256 x, y, z;

	x = _mm256_mul_ps((__m256) rv.x, f);
	y = _mm256_mul_ps((__m256) rv.y, f);
	z = _mm256_mul_ps((__m256) rv.z, f);
	
	x = _mm256_blendv_ps(x, zero, nanm);
	y = _mm256_blendv_ps(y, zero, nanm);
	z = _mm256_blendv_ps(z, zero, nanm);
		
	return vec8(
		(f8) x,
		(f8) y,
		(f8) z
	);	
}

vec8 simulation::submv(const vec8 &va, const vec8 &vb) {
	return vec8(
		submv(va.x,vb.x),
		submv(va.y,vb.y),
		submv(va.z,vb.z)
	);
}	

f8 simulation::submv(f8 va, f8 vb) {
	__m256 a, b;
	
	a = (__m256) va;
	b = (__m256) vb;

	__m256 opts[3], aopts[3];
	__m256 lv = _mm256_set1_ps(L);
	opts[0] = _mm256_sub_ps(b,a);
	opts[1] = _mm256_sub_ps(opts[0],lv);
	opts[2] = _mm256_add_ps(opts[0],lv);

	aopts[0] = _mm256_abs_ps(opts[0]);
	aopts[1] = _mm256_abs_ps(opts[1]);
	aopts[2] = _mm256_abs_ps(opts[2]);

	__m256 m01, m12, m20;
	__m256 m10, m21, m02;
	__m256 m0, m1, m2;
	m01 = _mm256_cmp_ps(aopts[0], aopts[1], _CMP_LT_OQ);
	m10 = _mm256_cmp_ps(aopts[1], aopts[0], _CMP_LT_OQ);
	
	m12 = _mm256_cmp_ps(aopts[1], aopts[2], _CMP_LT_OQ);
	m21 = _mm256_cmp_ps(aopts[2], aopts[1], _CMP_LT_OQ);

	m20 = _mm256_cmp_ps(aopts[2], aopts[0], _CMP_LT_OQ);
	m02 = _mm256_cmp_ps(aopts[0], aopts[2], _CMP_LT_OQ);
	
	m0 = _mm256_and_ps(m01, m02);
	m1 = _mm256_and_ps(m12, m10);
	m2 = _mm256_and_ps(m20, m21);	
	
	__m256 res = _mm256_set1_ps(NAN);
	res = _mm256_blendv_ps(res, opts[0], m0);
	res = _mm256_blendv_ps(res, opts[1], m1);
	res = _mm256_blendv_ps(res, opts[2], m2);

	return (f8) res;
}

Pixel simulation::pixelof(int idx) {
	int i,j;
	const int u = UNIVERSE_SIZE;	
	return Pixel(idx/u,idx%u);
}

int simulation::cell(int i, int j) {
	const int u = UNIVERSE_SIZE;
	i = apbci(i);
	j = apbci(j);

	return i + u * j;
}

Pixel::Pixel(int i, int j) : i(i), j(j) {}

static inline int find_lower(float bound, vec8_vector &arr) {
	// elements of arr are sorted by z-axis position
	// find index i of arr such that arr[j] for j >= i has some member > bound

	// because > is transitive. We know that arr[i] only needs to be included if arr[i][7] is > bound

	int n = arr.size8();

	// corner case 1: all vectors have at least one element > bound
	f8 v = arr[0].z;
	if (VI(v,7) > bound)
		return 0;

	// corner case 2: no vectors have any elements > bound. In this case, because
	// of periodic boundary conditions, the lower bound is actually also 0
	v = arr[n-1].z;
	if (VI(v,7) < bound)
		return 0;
	
	for (int i = 1; i < n; i++) {
		v = arr[i].z;
		if (VI(v,7) > bound)
			return i;
	}

	printf("search for lower bound failed. Something has gone wrong\n");
	return -1; // this might crash the code. Idc!
}

static inline int find_upper(float bound, vec8_vector &arr) {
	// same logic as find_lower
	int n = arr.size8();

	f8 v = arr[n-1].z;
	if (VI(v,0) < bound)
		return n;
	
	v = arr[0].z;
	if (VI(v,0) > bound)
		return n;

	for (int i = n - 2; i >= 0; i--) {
		v = arr[i].z;
		if (VI(v,0) < bound)
			return i+1;
	}

	printf("search for upper bound failed. Something has gone wrong\n");
	return -1;
}

Range simulation::rangeof(float z, float dx, float dy, vec8_vector &arr) {
	float rel_cutoff = sqrt(CUTOFF * CUTOFF - dx * dx - dy * dy);
	return Range(
		find_lower(apbcf(z - rel_cutoff), arr),
		find_upper(apbcf(z + rel_cutoff), arr)
	);	
}

Range::Range(int start, int stop) : start(start), stop(stop) {}
Range::Range(): start(-1), stop(-1) {}

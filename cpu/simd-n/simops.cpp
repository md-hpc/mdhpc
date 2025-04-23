#include <math.h>
#include <stdio.h>

#include "simulation.h"
#include "avx.h"

void Simulation::apb(vec &v) {
	v.x = pbf(v.x);
	v.y = pbf(v.y);
	v.z = pbf(v.z);
}

float Simulation::pbf(float x) {
	return x < 0 ? x + L : x > L ? x - L : x;
}

f8 Simulation::subpb(__m256 a, __m256 b) {
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

vec8 Simulation::rpb(const vec8 &r, const vec8 &n) {
	return vec8(
		subpb(r.x, n.x),
		subpb(r.y, n.y),
		subpb(r.z, n.z)
	);
}

vec8 Simulation::lj(const vec8 &rp, const vec8 &np) {
	const float epsilon_mul_24 = EPSILON_MUL_24;
	const float sigma_pow_6 = SIGMA_POW_6;
	const float sigma_pow_12_mul_2 = SIGMA_POW_12_MUL_2;
	const float one = 1;
	const float dt = DT;

	vec8 rv = rpb(rp, np);
	
	f8 r = sqrtv(rv.x * rv.x + rv.y * rv.y + rv.z * rv.z); 
	r = _mm256_max_ps(r, _mm256_set1_ps(0.5*SIGMA)); 

	f8 r2 = r * r;
	f8 r8 = r2 * r2 * r2 * r2;
	f8 r14 = r8 * r2 * r2 * r2;
	
	__m256 f = (__m256) (dt * epsilon_mul_24 * (sigma_pow_6 / r8 + sigma_pow_12_mul_2 / r14));
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

i8 Simulation::cell(const vec8 &r) {
	i8 i, j, k;
	const int u = UNIVERSE_SIZE;	
	const float c = CUTOFF;
	
	i = (i8) _mm256_cvttps_epi32(r.x / c);
	j = (i8) _mm256_cvttps_epi32(r.y / c);
	k = (i8) _mm256_cvttps_epi32(r.z / c);

	return i + j * u + k * u * u;
}

int Simulation::cell(const vec &r) {
	float u = UNIVERSE_SIZE, c = CUTOFF;

	int i = ((int)(r.x / c));
	int j = ((int)(r.y / c));
	int k = ((int)(r.z / c));
	
	return i + j * u + k * u * u;
}

Voxel Simulation::voxelof(int idx) {
	int i,j,k;
	const int u = UNIVERSE_SIZE;

	k = idx / (u * u);
    idx -= k * u * u;
    j = idx / u;
    idx -= j * u;
    i = idx;
	
	return Voxel(i,j,k);
}

int Simulation::pbi(int i) {
	return i < 0 ? i + UNIVERSE_SIZE : i >= UNIVERSE_SIZE ? i - UNIVERSE_SIZE : i;
}

int Simulation::cell(int i, int j, int k) {
	const int u = UNIVERSE_SIZE;
	i = pbi(i);
	j = pbi(j);
	k = pbi(k);

	return i + u * j + u * u * k;
}

void Simulation::warn_nan() {
	printf("Nan detected");
}

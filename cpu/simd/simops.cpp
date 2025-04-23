#include <math.h>

#include "simulation.h"

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

i8 simulation::cellv(const vec8 &r) {
	i8 i, j, k;
	const int u = UNIVERSE_SIZE;	
	const float c = CUTOFF;
	
	i = (i8) _mm256_cvttps_epi32(r.x / c);
	j = (i8) _mm256_cvttps_epi32(r.y / c);
	k = (i8) _mm256_cvttps_epi32(r.z / c);

	return i + j * u + k * u * u;
}

int simulation::cell(const vec &v) {
	int i,j,k;

	i = (int) (v.x / CUTOFF);
	j = (int) (v.y / CUTOFF);
	k = (int) (v.z / CUTOFF);

	return i + j * UNIVERSE_SIZE + k * UNIVERSE_SIZE * UNIVERSE_SIZE;
}

f8 simulation::LJ(const vec8 &rp, const vec8 &np) {
	const float epsilon_mul_4 = EPSILON_MUL_4;
	const float sigma_pow_12 = SIGMA_POW_12;
	const float sigma_pow_6 = SIGMA_POW_6;
	const float one = 1;

	vec8 rv = submv(rp, np);

	f8 r = sqrtv(rv.x * rv.x + rv.y * rv.y + rv.z * rv.z); 

	f8 r3 = (r * r * r);
	f8 r6 = r3 * r3;
	f8 r12 = r6 * r6;

	__m256 v = (__m256) (epsilon_mul_4 * ( sigma_pow_12 / r12 + sigma_pow_6 / r6));

	// mask out any NAN values
	__m256 nanm = _mm256_cmp_ps(v, v, _CMP_UNORD_Q);
	__m256 zero = _mm256_set1_ps(0);
	
	v = _mm256_blendv_ps(v, zero, nanm);
	
	return (f8) v;
}

vec8 simulation::lj(const vec8 &rp, const vec8 &np) {
	const float epsilon_mul_24 = EPSILON_MUL_24;
	const float sigma_pow_6 = SIGMA_POW_6;
	const float sigma_pow_12_mul_2 = SIGMA_POW_12_MUL_2;
	const float one = 1;
	const float dt = DT;
	
	vec8 rv = submv(rp, np);
	
	f8 r = sqrtv(rv.x * rv.x + rv.y * rv.y + rv.z * rv.z); 
	
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

voxel simulation::voxelof(int idx) {
	int i,j,k;
	const int u = UNIVERSE_SIZE;

	k = idx / (u * u);
    idx -= k * u * u;
    j = idx / u;
    idx -= j * u;
    i = idx;
	
	return voxel(i,j,k);
}

int simulation::apbci(int i) {
	return i < 0 ? i + UNIVERSE_SIZE : i >= UNIVERSE_SIZE ? i - UNIVERSE_SIZE : i;
}

int simulation::cell(int i, int j, int k) {
	const int u = UNIVERSE_SIZE;
	i = apbci(i);
	j = apbci(j);
	k = apbci(k);

	return i + u * j + u * u * k;
}

int simulation::pcount() {
	int np = 0;
	for (int i = 0; i < CELLS; i++)
		np += positions[i].size1();
	return np;
}

int simulation::vcount() {
	int np = 0;
	for (int i = 0; i < CELLS; i++)
		np += velocities[i].size1();
	return np;
}

int simulation::ocount() {
	int np = 0;
	for (int i = 0; i < THREADS; i++)
		np += outbound_particles[i].size();
	return np;
}

voxel::voxel(int i, int j, int k) : i(i), j(j), k(k) {}

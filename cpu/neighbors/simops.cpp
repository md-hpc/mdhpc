#include <math.h>
#include <stdio.h>

#include "simulation.h"

void Simulation::apb(vec &v) {
	v.x = pbf(v.x);
	v.y = pbf(v.y);
	v.z = pbf(v.z);
}

float Simulation::pbf(float x) {
	return x < 0 ? x + L : x > L ? x - L : x;
}

float Simulation::subpb(float a, float b) {
	float opts[3] = {
		a - (b - L),
		a - b,
		a - (b + L)
	};

	float aopts[3] = {
		abs(opts[0]),
		abs(opts[1]),
		abs(opts[2])
	};

	int i = 0;
	int min = aopts[0];

	i = aopts[1] < aopts[i] ? 1 : i;
	i = aopts[2] < aopts[i] ? 2 : i;

	return opts[i];
}

vec Simulation::rpb(const vec &r, const vec &n) {
	return vec(
		subpb(r.x, n.x),
		subpb(r.y, n.y),
		subpb(r.z, n.z)
	);
}

vec Simulation::lj(const vec &rp, const vec &np) {
	vec r_v = rpb(rp, np);
	float r = r_v.norm();

	r = r < 0.5 * SIGMA ? 0.5 * SIGMA : r;
	
	float r8 = 1;
	for (int i = 0; i < 8; i++)
		r8 *= r;
	
	float r14 = 1;
	for (int i = 0; i < 14; i++)
		r14 *= r;
	

	float f = EPSILON_MUL_24 * (SIGMA_POW_6 / r8 - SIGMA_POW_12_MUL_2 / r14);
	

	#ifdef DEBUG
	if (is_nan(f)) {
		warn_nan();
	}
	#endif

	return r_v * (f * DT);
}

int Simulation::cell(const vec &v) {
	int i,j,k;

	i = (int) (v.x / CUTOFF);
	j = (int) (v.y / CUTOFF);
	k = (int) (v.z / CUTOFF);

	return i + j * UNIVERSE_SIZE + k * UNIVERSE_SIZE * UNIVERSE_SIZE;
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

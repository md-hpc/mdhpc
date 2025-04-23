#include <cmath>
#include <cstdio>

#include "particle.h"

vec::vec() {
	x = NAN;
	y = NAN;
	z = NAN;
}

vec::vec(float x, float y, float z) : x(x), y(y), z(z) {}

particle::particle() {
	r = vec();
	v = vec();
	cell = -1;
}

particle::particle(vec r) : r(r) {
	v = vec();
	cell = -1;
}

particle::particle(vec r, vec v, int cell) : r(r), v(v), cell(cell) {}

#ifdef DEBUG
char *particle::str() {
	sprintf(dbstr,"(%f %f %f, %.1e %.1e %.1e)", r.x, r.y, r.z, v.x, v.y, v.z);
	return dbstr;
}
#endif

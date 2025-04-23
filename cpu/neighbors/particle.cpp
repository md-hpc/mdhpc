#include <cmath>
#include <cstdio>

#include "particle.h"

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

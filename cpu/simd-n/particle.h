#ifndef PARTICLE_H
#define PARTICLE_H

#include "vec.h"

class particle {
public:
	particle();
	particle(vec r);
	particle(vec r, vec v, int cell);

	vec r;
	vec v;
	int cell;
};

#endif

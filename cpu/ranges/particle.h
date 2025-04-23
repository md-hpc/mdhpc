#ifndef PARTICLE_H
#define PARTICLE_H

class vec {
public:
	vec();
	vec(float x, float y, float z);

	float x;
	float y;
	float z;
};

class particle {
public:
	particle();
	particle(vec r);
	particle(vec r, vec v, int cell);

	bool operator<(const particle& other);

	vec r;
	vec v;
	int cell;

#ifdef DEBUG
	char *str();
	char dbstr[64];
#endif

};

#endif

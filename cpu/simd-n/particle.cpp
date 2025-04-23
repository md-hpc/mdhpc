#include <cmath>
#include <cstdio>

#include "particle.h"

particle::particle(vec r, vec v, int cell) : r(r), v(v), cell(cell) {}
particle::particle() : r(vec()), v(vec()), cell(-1) {}

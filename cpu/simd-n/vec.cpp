#include <math.h>

#include "vec.h"
#include "simulation.h"

vec::vec(float x, float y, float z) : x(x), y(y), z(z) {};
vec::vec(float d) : x(d), y(d), z(d) {};
vec::vec() : x(0), y(0), z(0) {};

vec vec::operator+(const vec &other) {
    return vec(x + other.x, y + other.y, z + other.z);
}

vec vec::operator-(const vec &other) {
	return vec(x - other.x, y - other.y, z - other.z);
}

vec vec::operator*(float c) {
    return vec(c*x,c*y,c*z);
}

vec &vec::operator*=(float c) {
    x *= c;
    y *= c;
    z *= c;
    
    return *this;
}

vec &vec::operator+=(float c) {
    x += c;
    y += c;
    z += c;

    return *this;
}

vec &vec::operator+=(const vec &other) {
    x += other.x;
    y += other.y;
    z += other.z;

    return *this;
}

vec &vec::operator-=(const vec &other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;

    return *this;
}

vec vec::operator/(float c) {
    return vec(
        x / c,
        y / c,
        z / c
    );
}

float vec::norm() {
	return sqrt(x*x + y*y + z*z);
}

bool vec::isnan() {
	return is_nan(x) || is_nan(y) || is_nan(z);
}

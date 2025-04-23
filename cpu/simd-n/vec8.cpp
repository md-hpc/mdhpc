#include <math.h>
#include <stdio.h>

#include "vec8.h"

vec8::vec8(float d) {
	f8 vd = VK(d);
	x = vd;
	y = vd;
	z = vd;
}

vec8::vec8(f8 x, f8 y, f8 z) : x(x), y(y), z(z) {}

vec8::vec8(int ofst, vector<vec> arr, float d) {
	f8 vd = VK(d);
	x = vd;
	y = vd;
	z = vd;

	int n = arr.size() - ofst < 8 ? arr.size() - ofst : 8;
	
	for (int i = 0; i < n; i++) {
		vec v = arr[i + ofst];
		VI(x,i) = v.x;
		VI(y,i) = v.y;
		VI(z,i) = v.z;
	}
}

vec8::vec8(vec v) {
	for (int i = 0; i < 8; i++) {
		VI(x,i) = v.x;
		VI(y,i) = v.y;
		VI(z,i) = v.z;
	}
}

vec vec8::get(int i) {
	return vec(
		VI(x,i),
		VI(y,i),
		VI(z,i)
	);
}


void vec8::set(const vec &v, int i) {
	VI(x,i) = v.x;
	VI(y,i) = v.y;
	VI(z,i) = v.z;
}

vec8 vec8::operator+(const vec8 &other) {
    return vec8(x + other.x, y + other.y, z + other.z);
}

vec8 vec8::operator*(const f8 c) {
    return vec8(c*x, c*y, c*z);
}

vec8 vec8::operator*(const float c) {
	return vec8(c*x, c*y, c*z);
}

vec8 &vec8::operator*=(const f8 c) {
    x *= c;
    y *= c;
    z *= c;
   
    return *this;
}

vec8 &vec8::operator*=(const float c) {
	x *= c;
	y *= c;
	z *= c;

	return *this;
}

vec8 &vec8::operator+=(const f8 c) {
    x += c;
    y += c;
    z += c;

    return *this;
}

vec8 &vec8::operator+=(const vec8 &other) {
    x += other.x;
    y += other.y;
    z += other.z;

    return *this;
}

void vec8::permutev() {
	x = permute(x);
	y = permute(y);
	z = permute(z);
}

f8 vec8::normsq() {
	return x*x+y*y+z*z;
}

f8 vec8::norm() {
	return sqrtv(normsq());
}

vec vec8::sum() {
	vec v = vec(0);
	for (int i = 0; i < 8; i++) {
		v.x += VI(x,i);
		v.y += VI(y,i);
		v.z += VI(z,i);
	}
	return v;
}

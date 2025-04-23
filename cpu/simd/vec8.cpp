#include <math.h>
#include <stdio.h>

#include "vec8.h"

vec8::vec8() {
	f8 nan = VK(NAN);
	x = nan;
	y = nan;
	z = nan;
}

vec8::vec8(f8 x, f8 y, f8 z) : x(x), y(y), z(z) {}

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

v8buf::v8buf() : i(0) {}

int v8buf::append(const vec &sv) {
	v.set(sv,i++);
	return i == 8;
}

vec8 v8buf::get() {
	vec8 ret = v;
	i = 0;
	v = vec8();
	return ret;
}

void vec8_vector::append(const vec &v) {
	if (n % VSIZE == 0) {
		arr.push_back(vec8());
	}
	set(v,n);
	n++;
}

void vec8_vector::set(const vec &v, int i) {
	arr[i/VSIZE].set(v, i%VSIZE);
}

vec vec8_vector::get(int i) {
	return arr[i/VSIZE].get(i%VSIZE);
}

vec8 &vec8_vector::operator[](int i) {
	return arr[i];
}

void vec8_vector::resize(int sz) {
	vec v; // NAN initalized
	n = sz;
	if (n % 8) {
		for (int i = n; i < n + (8 - n % 8); i++) {
			set(v,i);
		}
	}
	arr.resize(sz / 8 + (sz % 8 != 0));
}

int vec8_vector::size1() {
	return n;
}

int vec8_vector::size8() {
	return arr.size();
}

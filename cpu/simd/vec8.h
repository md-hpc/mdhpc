#ifndef PARTICLE8_H
#define PARTICLE8_H

#include <vector>

#include "particle.h"
#include "avx.h"

using namespace std;

class vec8 {
public:
	vec8();
	vec8(f8 x, f8 y, f8 z);

	vec get(int i);
	void set(const vec &v, int i);

	vec8 operator+(const vec8 &other);
	
	vec8 operator*(const float c);
	vec8 operator*(const f8 c);
	
	vec8 &operator+=(const f8 c);
	vec8 &operator+=(const vec8 &other);

	vec8 &operator*=(const float c);
	vec8 &operator*=(const f8 c);
	
	void permutev();

	f8 normsq();
	f8 norm();

	f8 x;
	f8 y;
	f8 z;
};

class v8buf {
public:
	v8buf();
	int append(const vec &sv);
	vec8 get();

	int i;
	vec8 v;
};

class vec8_vector {
public:
	void append(const vec &p);

	void set(const vec &v, int i); 
	vec get(int i); 

	void resize(int sz);
	int size1();
	int size8(); 
	
	vec8 &operator[](int i); 

private:
	vector<vec8> arr;
	int n;
};

#endif

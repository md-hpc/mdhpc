#ifndef VEC_H
#define VEC_H

class vec {
public:   
    vec(float x, float y, float z);
    vec(float d);
	vec();

    vec operator+(const vec &other);	
	vec operator-(const vec &other);

    vec operator*(float c);
    vec operator/(float c);

    vec& operator+=(float c);
    vec& operator*=(float c);

    vec& operator+=(const vec &other);
    vec& operator-=(const vec &other);

    float norm();
    float normsq();

	bool isnan();

	float x;
    float y;
    float z;
};

#endif

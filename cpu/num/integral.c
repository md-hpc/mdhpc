#include <math.h>
#include <stdio.h>
#include <stdlib.h>

double r = 1;
double d = 1e-4;

double c2(double x, double y, double r) {
	if (sqrt(x*x+y*y) < r) {
		return sqrt(r*r-x*x-y*y);
	} else {
		return 0;
	}
}

double c1(double x, double r) {
	return sqrt(r*r-x*x);
}

double c_tot(double x, double y, double r) {
	return c1(x,r) + c1(y,r) + 
		c1(r-x,r) + c1(r-y,r) + 
		c2(x,y,r) + c2(x,r-y,r) + c2(r-x,y,r) + c2(r-x,r-y,r);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("usage: %s [r]\n",argv[0]);
		return 1;
	} else {
		r = atof(argv[1]);
	}

	d = d * r;

	double sum = 0;
	for (double x = 0; x < r; x += d) {
		for (double y = 0; y < r; y += d) {
			sum += c_tot(x,y,r) * d * d;
		}
	}
	double A = 2 * sum;
	printf("%f\n",A);
}

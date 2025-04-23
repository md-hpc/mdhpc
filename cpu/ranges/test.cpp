#include <stdio.h>

#include "avx.h"
#include "simulation.h"
#include "particle8.h"

#define pk(x) particle(vec(x,x,x))

int main() {
	simulation s = simulation(0,NULL);

	particle8 p1, p2;

	p1.set(pk(0),0);
	p2.set(pk(1),0);

	p1.set(pk(.2),1);
	p2.set(pk(2.7),1);

	p1.set(pk(2.9),2);
	p2.set(pk(.1),2);


	vec8 r1 = p1.r;
	vec8 r2 = p2.r;

	pack f1, f2;
	for (int i = 0; i < 8; i++) {
		f1.d[i] = ((float)i) * .375;
		f2.d[i] = 2;
	}
	f8 f = f1.v / f2.v;

	

	ipack ip;
	
	ip.v = s.cellv(r1);
	for (int i = 0; i < 8; i++) {
		printf("%d ",ip.d[i]);
	}
	printf("\n");


	pack c;
	c.v = clipv(r2.x, 1.5);
	for (int i = 0; i < 8; i++) {
		printf("%f ", c.d[i]);
	}
	printf("\n");

	vec8 f = s.lj(r1,r2);

	for (int i = 0; i < 8; i++) {
		vec v = f.get(i);
		printf("(%f %f %f) ", v.x, v.y, v.z);	
	}
	printf("\n");

	ipack l;
	for (int i = 0; i < 8; i++) 
		l.d[i] = 5;

	printf("%d\n",alleq(l.v,5));
	l.d[4] = 3;
	printf("%d\n",alleq(l.v,5));
	
	return 0;
}

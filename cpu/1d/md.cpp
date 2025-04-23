#include <stdio.h>
#include <math.h>

typedef float real;

real SIGMA = 1.5;
real EPSILON = 1;
real DT = 1e-7;
int TIMESTEPS = 1000000;
int N = 9;

real lj(real r) {
    return 4*EPSILON*(6*powf(SIGMA,6)/powf(r,7) - 12*powf(SIGMA,12)/powf(r,13));
}

real LJ(real r) {
    return 4*EPSILON*(powf(SIGMA/r,12) - powf(SIGMA/r,6));
}

int main() {
	real r[N];
	real v[N];

	for (int i = 0; i < N; i++) {
		r[i] = (float) 2*i;
		v[i] = (float) 0;
	}

    real E = 0;

    real ke, pe;
    for (int t = 0; t < TIMESTEPS; t++) {
        ke = 0;
        pe = 0;

        for (int i = 0; i < N; i++) {
            for (int j = i+1; j < N; j++) {
                real disp = r[j] - r[i];
                real f = lj(disp);
                real dv = f * DT;
				
				v[i] += dv;
				v[j] -= dv;

                pe += 2*LJ(disp);
            }
        }
        
        if (t == 0) {
            E = pe;
        }

        for (int i = 0; i < N; i++) {
            r[i] += v[i] * DT;
            ke += v[i]*v[i]/2;
        }
    }
    printf("KE: %e\n", ke);
    printf("PE: %e\n", pe);
    printf("Total: %e\n", pe + ke);
    printf("Drift: %e\n", pe + ke - E);
}

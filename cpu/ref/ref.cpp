#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "ref.h"

using namespace std;

const float SIGMA = 1.5;
const float EPSILON = 1;
float CUTOFF = 3;
float L = 15;

int N_PARTICLE = 2;
int TIMESTEPS = 2000;
float DT = 1e-3;
int SEED = 0;
int FRAMES = -1;
int RESOLUTION = -1;

char *PDB_PATH = NULL;
char *OUTPUT_PATH = NULL;

char default_output[] = "particles.csv";

vector<vec> r;
vector<vec> v;

#define R_MAXLJ (SIGMA*powf(26/7,1/6))

vec::vec(float x, float y, float z) : x(x), y(y), z(z) {};
vec::vec() : x(0), y(0), z(0) {};

vec vec::operator+(const vec &other) {
    return vec(x + other.x, y + other.y, z + other.z);
}

vec vec::operator-(const vec &other) {
	return vec(x - other.x, y - other.y, z - other.z);
}

vec vec::operator*(const float c) {
    return vec(c*x,c*y,c*z);
}

vec &vec::operator*=(const float c) {
    x *= c;
    y *= c;
    z *= c;
    
    return *this;
}

vec &vec::operator+=(const float c) {
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

vec &vec::operator=(const vec& other) {
    x = other.x;
    y = other.y;
    z = other.z;

    return *this;
}

vec modr(const vec &a, const vec &b) {
    return vec(
        subm(a.x, b.x),
        subm(a.y, b.y),
        subm(a.z, b.z)
    );
}

float pbc(const float &x, float l) {
    float a = fmodf(x,l);
    if (a < 0)
        return l + a;
    else 
        return a;
}

vec pbc(const vec &v) {
    return vec(
        pbc(v.x, L),
        pbc(v.y, L),
        pbc(v.z, L)
    );
}

float vec::normsq() {
    return x*x+y*y+z*z;
}

float vec::norm() {
    return sqrt(x*x+y*y+z*z);
}

int particle::counter = 0;

float subm(float a, float b) {
    // distance from a to b under periodic boundary condition of length L
    float opts[3] = {
        b - (a - L),
        b - a,
        b - (a + L)
    };
    float ds[3] = {
        abs(opts[0]),
        abs(opts[1]),
        abs(opts[2])
    };
    int mi = 0;
    mi = ds[mi] > ds[1] ? 1 : mi;
    mi = ds[mi] > ds[2] ? 2 : mi;
    return opts[mi];
}

float lj(float r) {
	return 4*EPSILON*(6*pow(SIGMA,6)/pow(r,7) - 12*pow(SIGMA,12)/pow(r,13));
}

float LJ(float r) {
    return 4*EPSILON*(pow(SIGMA/r,12) - pow(SIGMA/r,6));
}

float frand() {
    return ((float)rand()/(float)RAND_MAX);
}

int parse_cli(int argc, char **argv) {    
	OUTPUT_PATH = default_output;

    for (char **arg = &argv[1]; arg < &argv[argc]; arg+=2) {
        if (!strcmp(arg[0],"--cutoff")) {
            CUTOFF = atof(arg[1]);
        } else if (!strcmp(arg[0],"--l")) {
            L = atol(arg[1]);
        } else if (!strcmp(arg[0],"--particles")) {
            N_PARTICLE = atoi(arg[1]);
        } else if (!strcmp(arg[0],"--timesteps")) {
            float x = atof(arg[1]);
			TIMESTEPS = (int) x;
        } else if (!strcmp(arg[0],"--dt")) {
            DT = atof(arg[1]);
        } else if (!strcmp(arg[0],"--seed")) {
            SEED = atoi(arg[1]);
        } else if (!strcmp(arg[0],"--frames")) {
            FRAMES = atoi(arg[1]);
        } else if (!strcmp(arg[0],"--output")) {
            OUTPUT_PATH = arg[1];
        } else if (!strcmp(arg[0],"--pdb")) {
            PDB_PATH = arg[1];
        } else {
            dprintf(2,"Unrecognized option: %s\n", arg[0]);
            return 1;
        }
    }

    if (CUTOFF * 3 >= L) {
        printf("Cutoff too large. Must be < L/3\n");
        exit(1);
    }

	if (N_PARTICLE == -1) {
		N_PARTICLE = 80 * ((int)powf(L / CUTOFF, 3));
	}

	if (FRAMES == -1) {
		FRAMES = TIMESTEPS;
	}

	RESOLUTION = TIMESTEPS/FRAMES;

    return 0;
}

void init_particles() {
    if (PDB_PATH == NULL) {
        srandom(SEED);
        for (int i = 0; i < N_PARTICLE; i++) {
            r.push_back((vec(L*frand(),L*frand(),L*frand())));
            v.push_back(vec());
        }
    } else {
        FILE *f = fopen(PDB_PATH,"r");
        char *tok;
        char *line;
        size_t n;
		int i = 0;
        while (getline(&line,&n,f)) {
            if (!strncmp(line,"END",3)) {
                fclose(f);
                break;
            }
            tok = strtok(line," ");
            for (int i = 0; i < 5; i++)
                tok = strtok(NULL," ");
            float x, y, z;
            x = atof(strtok(NULL," "));
            y = atof(strtok(NULL," "));
            z = atof(strtok(NULL," "));
			if (x > L || y > L || z > L) {
				dprintf(2,"error: particle %d is outside of global box\n",i);
				exit(1);
			}
            r.push_back(vec(x,y,z));
            v.push_back(vec());
			i++;
        }
    }
}

void save(int fd) {
    int np = r.size();
    for (int i = 0; i < np; i++) {
        dprintf(fd,"%d, %f, %f, %f, %f, %f, %f\n", i, r[i].x, r[i].y, r[i].z, v[i].x, v[i].y, v[i].z);
    }
    dprintf(fd,"\n");
}

unsigned long rdtsc() {
  union {
    unsigned long long int64;
    struct {unsigned int lo, hi;} int32;
  } p;
  __asm__ __volatile__ (
    "rdtsc" :
    "=a" (p.int32.lo),
    "=d"(p.int32.hi)
  );
  return p.int64;
}

int main(int argc, char **argv) {

    parse_cli(argc,argv); 
    init_particles();

	float *pe, *ke, *E;
	pe = (float*) malloc(sizeof(float) * TIMESTEPS);
	ke = (float*) malloc(sizeof(float) * TIMESTEPS);
	E = (float*) malloc(sizeof(float) * TIMESTEPS);

    int fd = open(OUTPUT_PATH, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Could not open file");
        exit(1);
    }

    for (int t = 0; t < TIMESTEPS; t++) {
        pe[t] = 0;
        ke[t] = 0;

        for (int ri = 0; ri < N_PARTICLE; ri++) {
            for (int ni = ri+1; ni < N_PARTICLE; ni++) {
                vec disp_v = modr(r[ri],r[ni]); 
                float disp = disp_v.norm();
                float f = lj(disp);
                vec dv = (disp_v / disp) * f * DT;

                v[ri] += dv;
                v[ni] -= dv;

                pe[t] += 2*LJ(disp);
            }
        }

        // motion update
        for (int i = 0; i < N_PARTICLE; i++) {
            r[i] += v[i] * DT;
            r[i] = pbc(r[i]);

            ke[t] += v[i].normsq() / 2;
        }

        // save results
        if (t % RESOLUTION == 0) {
			printf("timestep %d\n",t);
            save(fd);
        }

		E[t] = ke[t] + pe[t] - pe[0];
    }
    close(fd);

	fd = open("energy.csv",O_CREAT|O_TRUNC|O_RDWR, S_IRUSR|S_IWUSR);
	dprintf(fd,"t, pe, ke, E\n");
	for (int t = 0; t < TIMESTEPS; t++) {
		char line[512];
		int n = sprintf(line,"%d, %e, %e, %e\n",t,pe[t],ke[t],E[t]);
		write(fd,line,n);
	}
	close(fd);
}

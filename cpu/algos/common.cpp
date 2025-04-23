#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "common.h"
#include <unistd.h>

using namespace std;

// Constants for liquid argon
float SIGMA = 0.34;
float EPSILON = 120;
float CUTOFF = .85;

int UNIVERSE_SIZE = 5;
int N_PARTICLE = -1;
int N_TIMESTEP = 10;
float DT = 1e-15;
int SEED = 0;
int RESOLUTION = 100;
int NEIGHBOR_REFRESH_RATE = 18;
int BR = -1;
int BN = -1;
int THREADS = 128;
int SAVE = 0;
mdalgo_t ALGO = ALGO_NONE;

vec::vec(float x, float y, float z) : x(x), y(y), z(z) {};
vec::vec() : x(0), y(0), z(0) {};

vec vec::operator+(const vec &other) {
    return vec(x + other.x, y + other.y, z + other.z);
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

vec &vec::operator=(const vec& other) {
    x = other.x;
    y = other.y;
    z = other.z;

    return *this;
}

vec vec::operator%(const vec &other) {
    return vec(
        subm(other.x, x),
        subm(other.y, y),
        subm(other.z, z)
    );
}

void vec::apbc() {
    x = fmodf(x,L);
    y = fmodf(y,L);
    z = fmodf(z,L);
}

int vec::cell() {
    return linear_idx(
        (int)x/CUTOFF,
        (int)y/CUTOFF,
        (int)z/CUTOFF
    );
}

float vec::normsq() {
    return x*x+y*y+z*z;
}

float vec::norm() {
    return sqrt(x*x+y*y+z*z);
}

void vec::read(float *buf) {
    buf[0] = x;
    buf[1] = y;
    buf[2] = z;
}

#ifdef DEBUG
void vec::print() {
    printf("%.3f %.3f %.3f\n",x,y,z);
}

char *vec::str() {
    sprintf(strbuf,"(%.3f %.3f %.3f)",x,y,z);
    return strbuf;
}
#endif

int particle::counter = 0;

particle::particle() : r(vec(0,0,0)), v(vec(0,0,0)) {}
particle::particle(vec r) : r(r), v(vec(0,0,0)) {
    id = counter++;
}


int particle::update_cell() {
#ifdef DEBUG
    old_cell = cell;
#endif
    cell = r.cell();
    return cell;
}

#ifdef DEBUG
char *particle::str() {
    sprintf(dbstr,"%d %d %d", id, cell, old_cell);
    return dbstr;
}
#endif

timer::timer() : time(0), running(false) {}

void timer::start() {
    last = rdtsc();
}

void timer::stop() {
    time += rdtsc() - last;
}

unsigned long timer::get() {
    return time;
}

static inline int apbci(int i) {
    i = i < 0 ? i + UNIVERSE_SIZE : i;
    i = i >= UNIVERSE_SIZE ? i - UNIVERSE_SIZE : i;
    return i;
}

int linear_idx(int i, int j, int k) {
    i = apbci(i);
    j = apbci(j);
    k = apbci(k);
    return i + j*UNIVERSE_SIZE + k*UNIVERSE_SIZE*UNIVERSE_SIZE;
}

void cubic_idx(int *res, int idx) {
    res[2] = idx / (UNIVERSE_SIZE * UNIVERSE_SIZE);
    idx -= res[2] * UNIVERSE_SIZE * UNIVERSE_SIZE;
    res[1] = idx / UNIVERSE_SIZE;
    idx -= res[1] * UNIVERSE_SIZE;
    res[0] = idx;
}

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
    float f = -4*EPSILON*(6*pow(SIGMA,6)/pow(r,7) - 12*pow(SIGMA,12)/pow(r,13));

    if (f < LJ_MIN) {
            return LJ_MIN;
    } else {
        return f;
    }
}

float frand() {
    return ((float)rand()/(float)RAND_MAX);
}


typedef struct {
    int core;
    int start;
    int stop;
    void (*kernel)(int);
} job_t;

void *run_kernel(void *spec) {
    job_t *job = (job_t*) spec;

    cpu_set_t cpuset;

    CPU_ZERO(&cpuset);
    CPU_SET(job->core, &cpuset);
    if (0 != pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset)) {
        perror("could not set affinity");
        return (void*) 1;
    }
    
    for (int i = job->start; i < job->stop; i++) {
        job->kernel(i);
    }
    
    return (void*) 0;
}

void thread(void (*kernel)(int), int n) {
    pthread_t tids[THREADS];
    job_t jobs[THREADS];
    void *ret;
    int t;
    int bsize = (n + THREADS - 1) / THREADS;
    
    for (t = 0; t < THREADS; t++) {
        jobs[t].core = t;
        jobs[t].start = t * bsize;
        jobs[t].stop = (t + 1) * bsize < n ? (t + 1) * bsize : n;
        jobs[t].kernel = kernel;
        
        pthread_create(&tids[t], NULL, run_kernel, &jobs[t]);
    }

    for (t = 0; t < THREADS; t++) {
        pthread_join(tids[t], &ret);
        if (ret) {
            exit(1);
        }
    }
}

int parse_cli(int argc, char **argv) {    
    if (ALGO == ALGO_NONE) {
		dprintf(2,"Must set ALGO before calling parse_cli");
		return 1;
	}

	for (char **arg = &argv[1]; arg < &argv[argc]; arg+=2) {
        if (!strcmp(arg[0],"--sigma")) {
            SIGMA = atof(arg[1]);
        } else if (!strcmp(arg[0],"--epsilon")) {
            EPSILON = atof(arg[1]);
        } else if (!strcmp(arg[0],"--cutoff")) {
            CUTOFF = atof(arg[1]);
        } else if (!strcmp(arg[0],"--universe-size")) {
            UNIVERSE_SIZE = atol(arg[1]);
        } else if (!strcmp(arg[0],"--particles")) {
            N_PARTICLE = atoi(arg[1]);
        } else if (!strcmp(arg[0],"--timesteps")) {
            N_TIMESTEP = atoi(arg[1]);
        } else if (!strcmp(arg[0],"--dt")) {
            DT = atof(arg[1]);
        } else if (!strcmp(arg[0],"--seed")) {
            SEED = atoi(arg[1]);
        } else if (!strcmp(arg[0],"--resolution")) {
            RESOLUTION = atoi(arg[1]);
        } else if (!strcmp(arg[0],"--br")) {
            BR = atoi(arg[1]);
        } else if (!strcmp(arg[0],"--bn")) {
            BN = atoi(arg[1]);
        } else if (!strcmp(arg[0],"--threads")) {
            THREADS = atoi(arg[1]);
        } else if (!strcmp(arg[0],"--log-path")) {
            LOG_PATH = arg[1];
		} else if (!strcmp(arg[0],"--save")) {
			SAVE = 1;
			arg--;
        } else {
            dprintf(2,"Unrecognized option: %s\n", arg[0]);
            return 1;
        }
    }

    if (THREADS > sysconf(_SC_NPROCESSORS_ONLN)) {
        THREADS = sysconf(_SC_NPROCESSORS_ONLN);
    }

	if (ALGO = ALGO_LISTS) {
		CUTOFF = 1.2 * CUTOFF;
	}

	if (N_PARTICLE == -1) {
		if (ALGO == ALGO_CELLS) {
			N_PARTICLE = 80 * N_CELL;
		}
		if (ALGO == ALGO_LISTS) {
			N_PARTICLE = 138 * N_CELL;
		}
	}

    return 0;
}

void init_particles(vector<particle> &particles) {
    srandom(SEED);
    for (int i = 0; i < N_PARTICLE; i++) {
        particles.push_back(particle(vec(L*frand(),L*frand(),L*frand())));
    }
}

void save(vector<vector<particle>> &cells, int fd) {
    if (!SAVE) 
		return;

	int ci;
    int pi;
    int np;

    float buf[3];

    vector<particle> *cell;
    for (ci = 0; ci < N_CELL; ci++) {
        cell = &cells[ci];
        np = cell->size();
        for (pi = 0; pi < np; pi++) {
            (*cell)[pi].r.read(buf);
            write(fd,buf,3*sizeof(float));
        }
    }
}

void save(vector<particle> &particles, int fd) {
    if (!SAVE)
		return;

	int pi;
    int np;
    float buf[3];

    np = particles.size();
    for (pi = 0; pi < np; pi++) {
        particles[pi].r.read(buf);
        write(fd,buf,3*sizeof(float));
    }
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

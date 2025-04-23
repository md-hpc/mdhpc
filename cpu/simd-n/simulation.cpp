#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <math.h>
#include <limits.h>

#include "simulation.h"

using namespace std;

static float frand() {
	return ((float)rand()) / ((float)INT_MAX);
}

bool is_nan(float x) {
	return x == INFINITY || x == -INFINITY || isnan(x);
}

Simulation::Simulation(int argc, char **argv) {    
	THREADS = sysconf(_SC_NPROCESSORS_ONLN);
	SIGMA = 1;
	EPSILON = 1;
	CUTOFF = 2.5;
	DT = 1e-7;
	PARTICLES = -1;
	UNIVERSE_SIZE = 3;
	SEED = 0;
	SAVE = 0;
	TIMESTEPS = 5;
	RESOLUTION = 100;
	NEIGHBOR_REFRESH_RATE = 20;
	DENSITY = 138;
	PATH = (char*) default_path;

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
            PARTICLES = atoi(arg[1]);
        } else if (!strcmp(arg[0],"--timesteps")) {
            TIMESTEPS = atoi(arg[1]);
        } else if (!strcmp(arg[0],"--dt")) {
            DT = atof(arg[1]);
        } else if (!strcmp(arg[0],"--seed")) {
            SEED = atoi(arg[1]);
        } else if (!strcmp(arg[0],"--resolution")) {
            RESOLUTION = atoi(arg[1]);
        } else if (!strcmp(arg[0],"--threads")) {
            THREADS = atoi(arg[1]);
        } else if (!strcmp(arg[0],"--path")) {
            PATH = arg[1];
		} else if (!strcmp(arg[0],"--save")) {
			SAVE = 1;
			arg--;
        } else {
            dprintf(2,"Unrecognized option: %s\n", arg[0]);
        }
    }

	L = CUTOFF * UNIVERSE_SIZE;
	CELLS = UNIVERSE_SIZE * UNIVERSE_SIZE * UNIVERSE_SIZE;
	
	EPSILON_MUL_4 = EPSILON * 4;
	EPSILON_MUL_24 = EPSILON * 24;
	SIGMA_POW_6 = powf(SIGMA, 6);
	SIGMA_POW_12 = powf(SIGMA, 12);
	SIGMA_POW_12_MUL_2 = 2 * powf(SIGMA, 12);

	FD = open(PATH, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (THREADS > sysconf(_SC_NPROCESSORS_ONLN)) {
        THREADS = sysconf(_SC_NPROCESSORS_ONLN);
    }

	pthread_barrier_init(&barrier, NULL, THREADS);
	pthread_barrier_init(&parent_barrier, NULL, THREADS+1);
	pthread_mutex_init(&mutex, NULL);

	tids = (pthread_t*) malloc(sizeof(pthread_t) * THREADS);
	specs = (worker_spec_t*) malloc(sizeof(worker_spec_t) * THREADS);

	if (PARTICLES == -1)
		PARTICLES = DENSITY * UNIVERSE_SIZE * UNIVERSE_SIZE * UNIVERSE_SIZE;

	positions.resize(CELLS);
	velocities.resize(CELLS);
	neighborhoods.resize(CELLS);

	export_fbufs.resize(THREADS);
	import_fbuf_cores.resize(THREADS);
	outbound_particles.resize(THREADS);

	srandom(SEED);
	for (int i = 0; i < PARTICLES; i++) {
		vec r = vec(L*frand(), L*frand(), L*frand());	
		vec v = vec(0,0,0);
		int hci = cell(r);
		positions[hci].push_back(r);
		velocities[hci].push_back(v);
	}
}

void Simulation::simulate() {
	create_workers();
	for (timestep = 0; timestep < TIMESTEPS; timestep++) {
		printf("timestep %d\n",timestep);
		pthread_barrier_wait(&parent_barrier);	
		
		if (SAVE && timestep % RESOLUTION == 0) {
			save();
		}
			
		#ifdef DEBUG
		printf("timestep %d\n",timestep);
		#endif
	}
	join_workers();
}

void Simulation::save() {
	int nc = positions.size();
	for (int ci = 0; ci < nc; ci++) {
		int np = positions[ci].size();
		for (int pi = 0; pi < np; pi++) {
			vec r = positions[ci][pi];
			dprintf(FD,"%d, %f, %f, %f\n", timestep, r.x, r.y, r.z);
		}	
	}
    dprintf(FD,"\n");
}

void *Simulation::run_worker(void *arg) {
    worker_spec_t *spec = (worker_spec_t*) arg;

    cpu_set_t cpuset;

    CPU_ZERO(&cpuset);
    CPU_SET(spec->core, &cpuset);
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset)) {
        perror("could not set affinity");
        return (void*) 1;
    }
	spec->s->do_work(spec);

    return (void*) 0;
}

void Simulation::create_workers() {
    int B = CELLS / THREADS;
	int i = 0;

	for (int t = 0; t < THREADS; t++) {
		
        specs[t].core = t;
        specs[t].start = i;

		if (B*t + (B+1)*(THREADS-t) > CELLS) {
			i += B;	
		} else {
			i += B+1;
		}

        specs[t].stop = i;
       	specs[t].s = this;

        if (pthread_create(&tids[t], NULL, run_worker, &specs[t])) {
			perror("Could not start thread");
			exit(1);
		}
    }

	printf("\n");
}

void Simulation::join_workers() {
    void *ret;
	for (int t = 0; t < THREADS; t++) {
        pthread_join(tids[t], &ret);
        if (ret) {
            exit(1);
        }
    }
}

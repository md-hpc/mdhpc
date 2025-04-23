#ifndef I_SIMULATION
#define I_SIMULATION

#include <vector>
#include <unordered_map>
#include <pthread.h>

#include "particle.h"
#include "voxel.h"
#include "vec8.h"

using namespace std;

typedef unordered_map<int, vector<vec>> fbufd_t;
typedef unordered_map<int, vector<int>> cored_t;

bool is_nan(float x);

class Simulation;
typedef struct {
	int core;
	int start;
	int stop;
	Simulation *s;
} worker_spec_t;

typedef struct {
	int cell;
	int idx;
} neighbor;

class Simulation {
public:
	Simulation(int argc, char **argv);

	void simulate();
	void save();

	// physics computations	
	vec8 lj(const vec8 &r, const vec8 &n);
	i8 cell(const vec8 &r);
	int cell(const vec &r);
	Voxel voxelof(int i);
	int cell(int i, int j, int k);

	vec8 rpb(const vec8 &a, const vec8 &b);
	f8 subpb(f8 a, f8 b);
	void apb(vec &r);
	float pbf(float x);
	int pbi(int i);

	// workers
	void export_particles_worker(int hci, worker_spec_t* spec);
	void import_particles_worker(worker_spec_t* spec);
	void create_neighbor_lists_worker(int hci, worker_spec_t* spec);
	
	void velocity_update_worker(int hci, worker_spec_t* spec);
	void reduce_fbufs_worker(int hci, worker_spec_t* spec);
	void position_update_worker(int hci, worker_spec_t* spec);
	
	static void *run_worker(void *arg);
	void do_work(worker_spec_t *spec);
	void create_workers();
	void join_workers();

	void warn_nan();

	vector<vector<vector<neighbor>>> neighborhoods;
	vector<vector<vec>> positions;
	vector<vector<vec>> velocities;	

	// {import_fbuf_tids, export_fbuf_cores}[core] is a map 
	vector<fbufd_t> export_fbufs; // maps have one key for each cell in the core's import regions that maps to an array of vecs where forces for those imported cells can be accumulated
	vector<cored_t> import_fbuf_cores; // maps have one key for each of this core's cells that are imported by another core that maps to an array of ints declaring which cores import that cell.
	
	// {outbound_partices, core_neighbors}[core] is an array
	vector<vector<particle>> outbound_particles; // particles that have left this core's cells and needs to be imported by another

	float SIGMA;
	float EPSILON;
	float DT;
	int UNIVERSE_SIZE;
	int PARTICLES;
	float CUTOFF;
	int DENSITY; // avg particle per angstrom^3 should not change as we change the cutoff radius. Increase density appropriately
	int NEIGHBOR_REFRESH_RATE;

	float L;
	int CELLS;

	int TIMESTEPS;
	int SEED;
	int THREADS;

	int FD;
	int RESOLUTION;
	int SAVE;
	char *PATH;

	int timestep;

	const char default_path[32] = "particles.csv";

	// "constants" for LJ computation
	float EPSILON_MUL_4;
	float EPSILON_MUL_24;
	float SIGMA_POW_12;
	float SIGMA_POW_12_MUL_2;
	float SIGMA_POW_6;

	pthread_t *tids;
	pthread_barrier_t barrier;
	pthread_barrier_t parent_barrier;
	pthread_mutex_t mutex;
	worker_spec_t *specs;
};

#endif

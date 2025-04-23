#ifndef I_SIMULATION
#define I_SIMULATION

#include <vector>
#include <unordered_map>
#include <pthread.h>


#include "vec8.h"
#include "particle.h"

using namespace std;

class simulation;

typedef unordered_map<int, vector<vec8>> fbufd_t;
typedef unordered_map<int, vector<int>> cored_t;

typedef struct {
	int core;
	int start;
	int stop;
	simulation *s;
	vector<int> neighbors;
} worker_spec_t;

class voxel {
public:
	voxel(int i, int j, int k);

	int i;
	int j;
	int k;
};

class simulation {
public:
	simulation(int argc, char **argv);

	void simulate();
	void save();
	void printpv();

	f8 LJ(const vec8 &r, const vec8 &n);
	vec8 lj(const vec8 &r, const vec8 &n);
	f8 lj_p(const vec8 &r, const vec8 &n);
	vec8 submv(const vec8 &va, const vec8 &vb);
	f8 submv(f8 va, f8 vb);
	i8 cellv(const vec8 &r);
	void apbcfv(vec8 &r);
	f8 apbcfv(f8 x);
	int apbci(int i);

	int pcount();
	int vcount();
	int ocount();

	void velocity_update_worker(int hci, worker_spec_t* spec);
	void position_update_worker(int hci, worker_spec_t* spec);
	void reduce_fbufs_worker(int hci, worker_spec_t* spec);
	void particle_migrate_worker(worker_spec_t* spec);
	
	voxel voxelof(int i);
	int cell(int i, int j, int k);
	int cell(const vec &v);
	int core(int i);

	static void *run_worker(void *arg);
	void do_work(worker_spec_t *spec);
	void create_workers();
	void join_workers();

	vector<vec8_vector> positions;
	vector<vec8_vector> velocities;	

	// {import_fbuf_tids, export_fbuf_cores}[core] is a map 
	vector<fbufd_t> export_fbufs; // maps have one key for each cell in the core's import regions that maps to an array of vec8's where forces for those imported cells can be accumulated
	vector<cored_t> import_fbuf_cores; // maps have one key for each of this core's cells that are imported by another core that maps to an array of ints declaring which cores import that cell.
	
	// {outbound_partices, core_neighbors}[core] is an array
	vector<vector<particle>> outbound_particles; // particles that have left this core's cells and needs to be imported by another
	vector<vector<int>> core_neighbors;	// all of the cores that neighbor this core. We'll check these outbound arrays for 

	vector<vector<float>> ke;
	vector<vector<float>> pe;
	vector<vector<float>> E;

	float SIGMA;
	float EPSILON;
	float DT;
	int UNIVERSE_SIZE;
	int PARTICLES;
	float CUTOFF;

	float LJ_MIN;
	float L;
	int CELLS;

	int TIMESTEPS;
	int SEED;
	int THREADS;

	int LOGFD;
	int FD;
	int RESOLUTION;
	int SAVE;

	int t;

	char default_log[32] = "validate/cells-vec";
	char default_path[32] = "particles.csv";

	// "constants" for LJ computation
	float EPSILON_MUL_4;
	float EPSILON_MUL_24;
	float SIGMA_POW_12;
	float SIGMA_POW_12_MUL_2;
	float SIGMA_POW_6;

	pthread_t *tids;
	pthread_barrier_t barrier;
	pthread_barrier_t parent_barrier;
	worker_spec_t *specs;
};

#endif

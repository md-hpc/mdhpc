#include <vector>
#include <cmath>
using namespace std;

typedef enum {
	ALGO_CELLS,
	ALGO_LISTS,
	ALGO_NONE
} mdalgo_t;

extern float SIGMA;
extern float EPSILON;
extern float CUTOFF;
extern int UNIVERSE_SIZE;
extern int N_PARTICLE;
extern int N_TIMESTEP;
extern float DT;
extern int SEED;
extern char *path;
extern int RESOLUTION;
extern int NEIGHBOR_REFRESH_RATE;
extern int BR;
extern int BN;
extern int THREADS;
extern char *LOG_PATH;
extern mdalgo_t ALGO;


#define N_CELL (UNIVERSE_SIZE*UNIVERSE_SIZE*UNIVERSE_SIZE)
#define L (CUTOFF * UNIVERSE_SIZE)

#define LJ_MIN (-4*24*EPSILON/SIGMA*(powf(7./26.,7./6.)-2*powf(7./26.,13./6.)))

#define DIR_MODE (S_IRWXU|S_IRWXG|S_IROTH)
#define FILE_MODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH)

class vec {
public:
    float x;
    float y;
    float z;
    
    vec(float x, float y, float z);
    vec();

    vec operator+(const vec &other);
    vec operator*(const float c);
    vec operator%(const vec &other);
    
    vec& operator*=(const float c);
    vec& operator+=(const float c);
    vec& operator+=(const vec &other);

    vec& operator=(const vec &other);

    void apbc();
    int cell();

    float norm();
    float normsq();

    void read(float *buf);

#ifdef DEBUG
    void sprint(char *buf);
    void print();
    char *str();
private:
    char strbuf[32];
#endif
};

class particle {
public:
    particle();
    particle(vec r);
    
    int update_cell();

    vec r;
    vec v;
    int id;
    int cell;
    static int counter;
#ifdef DEBUG
    char *str();

private:
    char dbstr[16]; 
    int old_cell;
#endif

}; 

class timer {
public:
    timer();

    void start();
    void stop();
    unsigned long get();

private:
    unsigned long time;
    unsigned long last;
    bool running;
};

int linear_idx(int, int, int);
void cubic_idx(int *, int);
float subm(float, float);
float lj(float);
float frand(void);
void thread(void (*)(int), int);
int parse_cli(int, char **);
void init_particles(vector<particle> &);
void save(vector<particle> &, int);
void save(vector<vector<particle>> &, int);

unsigned long rdtsc();

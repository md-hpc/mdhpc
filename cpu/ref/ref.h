#include <vector>
#include <cmath>

using namespace std;

class vec {
public:
    float x;
    float y;
    float z;
    
    vec(float x, float y, float z);
    vec();

    vec operator+(const vec &other);
	
	vec operator-(const vec &other);

    vec operator*(const float c);

    vec operator/(float c);
 
    vec& operator*=(const float c);
    
    vec& operator+=(const float c);
    vec& operator+=(const vec &other);
    
    vec& operator-=(const vec &other);

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
float LJ(float);
float frand(void);
void thread(void (*)(int), int);
int parse_cli(int, char **);
void init_particles(vector<particle> &);
void save(vector<particle> &, int);
void save(vector<vector<particle>> &, int);

unsigned long rdtsc();

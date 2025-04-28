/* Vectorized MD */

/*
gcc -pthread -O3 -march=native -std=gnu99 -mavx  MD_vec_pthread.c -lrt -lm -o MD_vec_pthread
*/

#include <xmmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define EPSILON 120
#define SIGMA 0.34f
#define CUTOFF 0.85
#define TIMESTEP (1*pow(10,-7))
#define SEED 246
#define DT 1e-15f
#define LJ_MIN (-4*24*EPSILON/SIGMA*(powf(7./26.,7./6.)-2*powf(7./26.,13./6.)))
#define NUM_TIMESTEP 1
#define NUM_THREADS 1
#define PPC 80
#define LOWER_BOUND 3
#define UPPER_BOUND 10
#define ITERS 1
#define MIN_R powf(2,1/6)*SIGMA

/*SIMD*/

typedef float data_t;

/*Number of bytes in a vector*/
#define VBYTES 32
typedef data_t vec_t __attribute__ ((vector_size(VBYTES)));
typedef int veci_t __attribute__ ((vector_size(VBYTES)));



/*Number of elements in a vector*/
#define VSIZE VBYTES/sizeof(data_t)

typedef union{
    vec_t v;
    data_t d[VSIZE];
}pack_t;

typedef struct{
    vec_t position;
    vec_t velocity;
    int particleId;

}Particle;

typedef struct{
    int start;
    int end;
    int thread_id;
    Particle *read;
    Particle *write;
    int num_particles;
    int universe;
}thread_data;

void initParticleList(Particle* particleList,int NUM_PARTICLES_UNIVERSE);
void velocityUpdate(Particle* particleListRead,Particle* particleListWrite, int UNIVERSE_SIZE ,int NUM_PARTICLES_UNIVERSE);
void positionUpdate(Particle* particleListRead,Particle * particleListWrite, int NUM_PARTICLES_UNIVERSE);
void modr(Particle *c, Particle *a,Particle *b, int UNIVERSE_SIZE);
void scalar_mul(Particle *v,vec_t c);
void taskerVelocity(Particle * particleListRead, Particle* particleListWrite,int universe, int num_particles);
void *workerVelocity(void * args);
void taskerPosition(Particle * particleListRead, Particle* particleListWrite,int num_particles);
void *workerPosition(void *args);

vec_t vec_add(Particle *a, Particle *b);
vec_t lj(vec_t r);
vec_t norm(Particle *r);
vec_t subm(vec_t a,vec_t b,int UNIVERSE_SIZE);
__m256 mm256_fmod_ps(__m256 x, __m256 y);






int main(){

    
    struct timespec start,end;
    double * times = malloc(sizeof(double)*40);
    int count = 0;

    /*Experiments*/

    for(int i = LOWER_BOUND; i <= UPPER_BOUND; i++){
        int num_particles = i*i*i*PPC;
        Particle * particleListA;
        Particle * particleListB;
        if (posix_memalign((void**)&particleListA, 32, sizeof(Particle) * num_particles) != 0) {
            exit(1);
        }
        if (posix_memalign((void**)&particleListB, 32, sizeof(Particle) * num_particles) != 0) {
            exit(1);
        }
        initParticleList(particleListA,i);
        initParticleList(particleListB,i);
        clock_gettime(CLOCK_MONOTONIC,&start);
        for(int i = 0; i<ITERS;i++){
            if(i%2==0){
                taskerVelocity(particleListA,particleListB,i,num_particles);
                taskerPosition(particleListA,particleListB,num_particles);
            }
            else{
                taskerVelocity(particleListB,particleListA,i,num_particles);
                taskerPosition(particleListB,particleListA,num_particles);
            }
        }
        clock_gettime(CLOCK_MONOTONIC,&end);
        /*
        pack_t print;
        print.v = particleListB[0].position;
        for(int i = 0 ; i < VSIZE;i++){
            printf("particle 0 in dim %d is %f position \n",i,print.d[i]);
        }
        */
        free(particleListA);
        free(particleListB);
        times[count]=(end.tv_sec-start.tv_sec) + (end.tv_nsec - start.tv_nsec)/1e9;
        count++;
    }

    printf("Printing times:\n");
    
    /*Printing Times*/

    count = 0;
    int first = 1;
    for(int i = LOWER_BOUND; i <= UPPER_BOUND; i++){
        if (first)
            {
                printf("Universe Size, Particle Count, Time(s)\n");
                first = 0;
            }

            printf("%d^3, %d, %f \n", i, i*i*i*PPC, times[count]);
            count++;
    }


    return 0;
}


/*Velocity Update*/
/*
void velocityUpdate(Particle *particleListRead,Particle * particleListWrite ,int UNIVERSE_SIZE, int NUM_PARTICLES_UNIVERSE)
{
    Particle vec;
    vec_t vec_hold;
    vec_t r, f,hold;
    for (int ref = 0; ref < NUM_PARTICLES_UNIVERSE; ref++)
    {
        for (int neighbor = 0; neighbor < NUM_PARTICLES_UNIVERSE; neighbor++)
        {
                modr(&vec, &particleListRead[ref], &particleListRead[neighbor],UNIVERSE_SIZE);
                r = norm(&vec);

                f = lj(r);
                __m256 dt_v = _mm256_set1_ps(DT);
                _mm256_storeu_ps((float*)&hold,dt_v);
                scalar_mul(&vec,hold * f/r);
                vec_hold = vec_add(&particleListRead[ref], &vec);
                particleListWrite[ref].velocity = vec_hold;
        }   
    }
}
*/
void modr(Particle *c, Particle *a,Particle *b, int UNIVERSE_SIZE){
    c->position = subm(a->position,b->position,UNIVERSE_SIZE);
}

vec_t norm(Particle *r){
    pack_t p;
    p.v = r->position;
    __m256i mask = _mm256_setr_epi32(-1,-1,-1,0,0,0,0,0);
    //__m256 pos_vec = _mm256_loadu_ps(((float*)&p.v));
    __m256 pos_vec = _mm256_maskload_ps((float*)&p.v,mask);
    __m256 x_s = _mm256_mul_ps(pos_vec,pos_vec);
    __m256 sum = _mm256_hadd_ps(x_s,x_s);
    sum = _mm256_hadd_ps(sum,sum);
    float val = ((float*)&sum)[0];
    val = fmaxf(val,0.0f);
    __m256 sqrt = _mm256_sqrt_ps(_mm256_set1_ps(val));
    vec_t norm;
    _mm256_storeu_ps((float*)&norm,sqrt);
    return norm;
}
vec_t subm(vec_t a,vec_t b,int UNIVERSE_SIZE){

    

    float L = (float)(UNIVERSE_SIZE * CUTOFF);

    __m256 a_v = _mm256_loadu_ps((float*)&a);
    __m256 b_v = _mm256_loadu_ps((float*)&b);

    __m256 c = _mm256_sub_ps(b_v,a_v);

    __m256 L_v = _mm256_set1_ps(L);
    __m256 d = _mm256_sub_ps(_mm256_sub_ps(b_v,L_v),a_v);

    __m256 c_less = _mm256_cmp_ps(c,_mm256_sub_ps(_mm256_setzero_ps(),d),_CMP_LT_OQ);

    __m256 result = _mm256_blendv_ps(d,c,c_less);

    vec_t out;
    _mm256_storeu_ps((float*)&out,result);
    return out;
}

static inline float subm_reg(float a,float b,int UNIVERSE_SIZE){
        float c,d;
        int L = UNIVERSE_SIZE * CUTOFF;
        c = b-a;
        if(c>0){
            d=b-L-a;
            if(c<-d){
                return c;
            }
            else{
                return d;
            }
    
        }
        else{
            d=b+L-a;
            if(-c<d){
                return c;
            }
            else{
                return d;
            }
        }
      
    
}

void taskerVelocity(Particle * particleListRead, Particle* particleListWrite,int universe, int num_particles){
    pthread_t threads[NUM_THREADS];
    thread_data thread_data_array[NUM_THREADS];
    int block = (int)(num_particles/NUM_THREADS);
    for(int i = 0;i<NUM_THREADS;i++){
        thread_data_array[i].start = i*block;
        thread_data_array[i].end = thread_data_array[i].end = (i == NUM_THREADS - 1) ? num_particles : (i+1) * block;
        thread_data_array[i].thread_id = i;
        thread_data_array[i].read = particleListRead;
        thread_data_array[i].write = particleListWrite;
        thread_data_array[i].universe = universe;
        thread_data_array[i].num_particles = num_particles;
        int curr = pthread_create(&threads[i],NULL,workerVelocity,(void*)&thread_data_array[i]);
    }
    for(int i=0;i<NUM_THREADS;i++){
        if(pthread_join(threads[i],NULL)){
            printf("ERROR ON JOIN\n");
            exit(19);
        }
    }
    
}
void *workerVelocity(void *args){
    thread_data *in = (thread_data*)args;
    int start = in->start;
    int end = in->end;
    Particle *particleListRead = in->read;
    Particle *particleListWrite = in->write;
    int NUM_PARTICLES_UNIVERSE = in->num_particles;
    int UNIVERSE_SIZE = in->universe;

    Particle vec;
    vec_t vec_hold;
    vec_t r, f,hold;
    for (int ref = start; ref < end; ref++)
    {
        for (int neighbor = 0; neighbor < NUM_PARTICLES_UNIVERSE; neighbor++)
        {
                if(ref == neighbor)continue;
                modr(&vec, &particleListRead[ref], &particleListRead[neighbor],UNIVERSE_SIZE);
                r = norm(&vec);

                f = lj(r);
                __m256 dt_v = _mm256_set1_ps(DT);
                _mm256_storeu_ps((float*)&hold,dt_v);
                scalar_mul(&vec,hold * f/r);
                particleListWrite[ref].velocity  = vec_add(&particleListRead[ref], &vec); 
        }   
    }
    pthread_exit(NULL);
}

void taskerPosition(Particle* particleListRead, Particle* particleListWrite,int num_particles){
    pthread_t threads[NUM_THREADS];
    thread_data thread_data_array[NUM_THREADS];
    int block = (int)(num_particles/NUM_THREADS);
    for(int i = 0; i<NUM_THREADS; i++){
        thread_data_array[i].start = i*block;
        thread_data_array[i].end = thread_data_array[i].end = (i == NUM_THREADS - 1) ? num_particles : (i+1) * block;
        thread_data_array[i].thread_id = i;
        thread_data_array[i].read = particleListRead;
        thread_data_array[i].write = particleListWrite;
        thread_data_array[i].num_particles = num_particles;
        int curr = pthread_create(&threads[i],NULL,workerPosition,(void*)&thread_data_array[i]);
    }
    for(int i=0;i<NUM_THREADS;i++){
        if(pthread_join(threads[i],NULL)){
            printf("ERROR ON JOIN\n");
            exit(19);
        }
    }

}
void  *workerPosition(void* args){
    thread_data *in = (thread_data*)args;
    int UNIVERSE_SIZE = in->universe;
    int start = in->start;
    int end = in->end;
    Particle * particleListRead = in->read;
    Particle * particleListWrite = in->write;

    int NUM_PARTICLES_UNIVERSE = UNIVERSE_SIZE * UNIVERSE_SIZE * UNIVERSE_SIZE * 80;
    float L = CUTOFF * UNIVERSE_SIZE;

    __m256 particles = _mm256_set1_ps(NUM_PARTICLES_UNIVERSE);
    __m256 L_vec = _mm256_set1_ps(L);
    __m256 DT_vec = _mm256_set1_ps(DT);

    
    for (int ref = start; ref < end; ref++)
    {
        Particle current = particleListRead[ref];

        __m256 curr_pos = _mm256_loadu_ps((float*)&current.position);
        __m256 curr_vel = _mm256_loadu_ps((float*)&current.velocity);
        
        __m256 inner = _mm256_add_ps(curr_pos,_mm256_mul_ps(curr_vel,DT_vec));
        __m256 mod = mm256_fmod_ps(inner,L_vec);

        curr_pos = _mm256_add_ps(curr_pos,mod);

        __m256 pos_less = _mm256_cmp_ps(curr_pos,_mm256_setzero_ps(),_CMP_LT_OQ);
        __m256 pos_greater = _mm256_cmp_ps(curr_pos,L_vec,_CMP_GT_OQ);

        curr_pos = _mm256_blendv_ps(curr_pos,_mm256_add_ps(curr_pos,L_vec),pos_less);
        curr_pos = _mm256_blendv_ps(curr_pos,_mm256_sub_ps(curr_pos,L_vec),pos_greater);
        if(ref == 0){
            
        }
        _mm256_storeu_ps((float*)&particleListWrite[ref].position,curr_pos);
    }
    pthread_exit(NULL);
}

vec_t lj(vec_t r){

    vec_t f;
    __m256 r_v = _mm256_loadu_ps((float*)&r);
    __m256 min_r = _mm256_set1_ps();
    r_v = _mm256_max_ps(r_v,min_r);
    __m256 r2 = _mm256_mul_ps(r_v, r_v);
    __m256 r6 = _mm256_mul_ps(r2, r2);  
    __m256 r12 = _mm256_mul_ps(r6, r6);  

    __m256 coeff = _mm256_set1_ps(4*EPSILON);
    __m256 sig = _mm256_set1_ps(SIGMA);  
    __m256 sig6 = _mm256_mul_ps(sig, _mm256_mul_ps(sig, _mm256_mul_ps(sig, _mm256_mul_ps(sig, _mm256_mul_ps(sig, sig))))); 
    __m256 sig12 = _mm256_mul_ps(sig6, sig6);

    __m256 t1 = _mm256_div_ps(_mm256_mul_ps(sig6, _mm256_set1_ps(6)), r6);  
    __m256 t2 = _mm256_div_ps(_mm256_mul_ps(sig12, _mm256_set1_ps(12)), r12);  

    __m256 result = _mm256_sub_ps(t1,t2);

    __m256 out = _mm256_mul_ps(coeff,result);

    __m256 lj_min_v = _mm256_set1_ps(LJ_MIN);
    out = _mm256_max_ps(out, lj_min_v);

    _mm256_storeu_ps((float*)&f,out);
    return f;
} 
void scalar_mul(Particle *v,vec_t c){
    pack_t check;
    check.v = c;
    for (int i = 0; i < VSIZE; i++) {
        if (isnan(check.d[i])) {
            printf("NaN detected in scalar_mul at index %d!\n", i);
        }
        
    }
    v->position *= c;
}

/*
void vec_add(Particle *a, Particle *b){
    vec_t hold_a = a->position;
    vec_t hold_b = b->position;

    hold_a = hold_a + hold_b;
}
*/
vec_t vec_add(Particle *a, Particle *b){
    pack_t check_a, check_b, check_res;
    check_a.v = a->position;
    check_b.v = b->position;
    check_res.v = check_a.v + check_b.v;

    for (int i = 0; i < VSIZE; i++) {
        if (isnan(check_a.d[i]) || isnan(check_b.d[i])) {
            printf("NaN detected in vec_add() input at index %d!\n", i);
        }
        if (isnan(check_res.d[i])) {
            printf("NaN detected in vec_add() output at index %d!\n", i);
        }
    }
    return a->position + b->position;
}
void positionUpdate(Particle *particleListRead,Particle* particleListWrite ,int UNIVERSE_SIZE)
{
    float NUM_PARTICLES_UNIVERSE = UNIVERSE_SIZE * UNIVERSE_SIZE * UNIVERSE_SIZE * 80;
    float L = CUTOFF * UNIVERSE_SIZE;

    __m256 particles = _mm256_set1_ps(NUM_PARTICLES_UNIVERSE);
    __m256 L_vec = _mm256_set1_ps(L);
    __m256 DT_vec = _mm256_set1_ps(DT);

    for (int ref = 0; ref < NUM_PARTICLES_UNIVERSE; ref++)
    {
        __m256 curr_pos = _mm256_loadu_ps((float*)&particleListRead[ref].position);
        __m256 curr_vel = _mm256_loadu_ps((float*)&particleListRead[ref].velocity);
        
        __m256 inner = _mm256_add_ps(curr_pos,_mm256_mul_ps(curr_vel,DT_vec));
        __m256 mod = mm256_fmod_ps(inner,L_vec);

        curr_pos = _mm256_add_ps(curr_pos,mod);

        __m256 pos_less = _mm256_cmp_ps(curr_pos,_mm256_setzero_ps(),_CMP_LE_OQ);
        __m256 pos_greater = _mm256_cmp_ps(curr_pos,L_vec,_CMP_GE_OQ);

        curr_pos = _mm256_blendv_ps(curr_pos,_mm256_add_ps(curr_pos,L_vec),pos_less);
        curr_pos = _mm256_blendv_ps(curr_pos,_mm256_sub_ps(curr_pos,L_vec),pos_greater);
        
        _mm256_storeu_ps((float*)&particleListWrite[ref].position,curr_pos);
    }
}
__m256 mm256_fmod_ps(__m256 x, __m256 y) {
    __m256 zero_vec = _mm256_setzero_ps();
    __m256 mask = _mm256_cmp_ps(y, zero_vec, _CMP_EQ_OQ); 
    __m256 safe_y = _mm256_blendv_ps(y, _mm256_set1_ps(1.0f), mask); 

    __m256 div = _mm256_div_ps(x, safe_y);        
    __m256 floor_div = _mm256_floor_ps(div); 
    __m256 prod = _mm256_mul_ps(floor_div, safe_y); 
    return _mm256_sub_ps(x, prod);             
}
void initParticleList(Particle *particleList, int UNIVERSE_SIZE)
{
    srand(42);

    int NUM_PARTICLES_UNIVERSE = UNIVERSE_SIZE * UNIVERSE_SIZE * UNIVERSE_SIZE * PPC;
    for (int i = 0; i < NUM_PARTICLES_UNIVERSE; i++)
    {
        pack_t curr;
        for(int j = 0;j<3;j++){
            curr.d[j] = ((float)rand() / (float)(RAND_MAX)) * UNIVERSE_SIZE;
        }
        for(int j = 3;j<VSIZE;j++){
            curr.d[j] = 0.0f;
        }
        
        particleList[i].position = curr.v;

        pack_t vel_curr;
        for(int j = 0;j<3;j++){
            vel_curr.d[j] = ((float)rand() / (float)(RAND_MAX)) * UNIVERSE_SIZE;
        }
        for(int j = 3;j<VSIZE;j++){
            vel_curr.d[j] = 0.0f;
        }
        

        particleList[i].velocity = vel_curr.v;
        
        particleList[i].particleId = i;
    }   
}

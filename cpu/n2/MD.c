

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#define NUM_TIMESTEP 1
#define NUM_THREADS 1
#define BLOCKED 0
#define PPC 80
#define EPSILON 120
#define SIGMA 0.34
#define CUTOFF 0.85
#define TIMESTEP (1*pow(10,-7))
#define SEED 246
#define DT 1e-15
#define LJ_MIN (-4*24*EPSILON/SIGMA*(powf(7./26.,7./6.)-2*powf(7./26.,13./6.)))

typedef struct{
    float x,y,z;
    float vX,vY,vZ;
    int particleId;

}Particle;
void velocityUpdateBlocked(Particle *particleList, int BSIZE, int NUM_PARTICLES_UNIVERSE);
void velocityUpdate(Particle *particleList, int UNIVERSE_SIZE);
void velocityUpdateN3L(Particle *particleList, int UNIVERSE_SIZE);
void positionUpdate(Particle *particleList, int UNIVERSE_SIZE);
void printData(Particle *particleList, int NUM_PARTICLES_UNIVERSE);
void plot_particles(Particle *particleList, FILE *fp, int NUM_PARTICLES_UNIVERSE);
void init_ParticleList(Particle *particleList, int NUM_PARTICLES_UNIVERSE);
void modr(Particle *c, Particle *a,Particle *b, int UNIVERSE_SIZE);
float norm(Particle *r);
float lj(float r);
void scalar_mul(Particle *v,float c);
void vec_add(Particle *a, Particle *b);
float subm(float a,float b,int UNIVERSE_SIZE);

int main()
{
    struct timespec start, end;

    double *times = malloc(sizeof(double) * 40);

    int count = 0;


    if (BLOCKED)
    {
            for (int j = 3; j <= 24; j ++)
            {
                Particle *particleList = malloc(sizeof(Particle)*j*j*j*PPC);
                init_ParticleList(particleList, j);
                clock_gettime(CLOCK_MONOTONIC,&start);
                velocityUpdateBlocked(particleList, 20, j);
                positionUpdate(particleList,j);
                clock_gettime(CLOCK_MONOTONIC,&end);
                free(particleList);
                times[count]=(end.tv_sec-start.tv_sec) + (end.tv_nsec - start.tv_nsec)/1e9;
                count++;
            }
        
    }
    else{
        for(int i = 3;i<=5;i++){
            Particle *particleList = malloc(sizeof(Particle)*i*i*i*PPC);
            init_ParticleList(particleList,i);
            clock_gettime(CLOCK_MONOTONIC,&start);
            velocityUpdate(particleList,i);
            positionUpdate(particleList,i);
            clock_gettime(CLOCK_MONOTONIC,&end);
            free(particleList);
            times[count]=(end.tv_sec-start.tv_sec) + (end.tv_nsec - start.tv_nsec)/1e9;
            count++;

        }
    }
   
    count = 0;
    int first = 1;
    for (int j = 3; j <=5 ;j ++)
    {
            if (first)
            {
                printf("Universe Size, Particle Count, Time(s)\n");
                first = 0;
            }

            printf("%d^3, %d, %f \n", j, j*j*j*PPC, times[count]);
            count++;
    }    
    return 0;
}
void init_ParticleList(Particle *particleList, int UNIVERSE_SIZE)
{
    srand(SEED);

    int NUM_PARTICLES_UNIVERSE = UNIVERSE_SIZE * UNIVERSE_SIZE * UNIVERSE_SIZE * 80;
    for (int i = 0; i < NUM_PARTICLES_UNIVERSE; i++)
    {
        particleList[i].x = ((float)rand() / (float)(RAND_MAX)) * UNIVERSE_SIZE;
        particleList[i].y = ((float)rand() / (float)(RAND_MAX)) * UNIVERSE_SIZE;
        particleList[i].z = ((float)rand() / (float)(RAND_MAX)) * UNIVERSE_SIZE;
        
        
        particleList[i].vX = 0;
        particleList[i].vY = 0;
        particleList[i].vZ = 0;

        
        particleList[i].particleId = i;
    }
}
void velocityUpdateBlocked(Particle *particleList, int BSIZE, int UNIVERSE_SIZE)
{
    Particle vec;
    float r, f;
    int NUM_PARTICLES_UNIVERSE = UNIVERSE_SIZE*UNIVERSE_SIZE*UNIVERSE_SIZE*PPC;
    #pragma omp parallel for num_threads(NUM_THREADS)
    for (int ii = 0; ii < NUM_PARTICLES_UNIVERSE; ii += BSIZE)
    {
        for (int jj = 0; jj < NUM_PARTICLES_UNIVERSE; jj += BSIZE)
        {
            for (int ref = ii; ref < ii + BSIZE; ref++)
            {
                for (int neighbor = jj; neighbor < jj + BSIZE; neighbor++)
                {
                    if (ref == neighbor)
                    {
                        continue;
                    }
                    else
                    {
                        modr(&vec, &particleList[ref], &particleList[neighbor],UNIVERSE_SIZE);
                        r = norm(&vec);
                        if (r > CUTOFF)
                        {
                            continue;
                        }
                        if (vec.x < 0)
                        {
                            continue;
                        }
                        f = lj(r);
                        scalar_mul(&vec, DT * f / r);
                        vec_add(&particleList[ref], &vec);
                    }
                }
            }
        }
    }
}
void velocityUpdate(Particle *particleList, int UNIVERSE_SIZE)
{
    Particle vec;
    float r, f;
    int NUM_PARTICLES_UNIVERSE = UNIVERSE_SIZE * UNIVERSE_SIZE * UNIVERSE_SIZE *PPC;
    for (int ref = 0; ref < NUM_PARTICLES_UNIVERSE; ref++)
    {
        for (int neighbor = 0; neighbor < NUM_PARTICLES_UNIVERSE; neighbor++)
        {

            if (ref == neighbor)
            {
                continue;
            }
            else
            {

                modr(&vec, &particleList[ref], &particleList[neighbor],UNIVERSE_SIZE);
                r = norm(&vec);

                f = lj(r);
                scalar_mul(&vec, DT * f / r);
                vec_add(&particleList[ref], &vec);
            }
        }
    }
}
void velocityUpdateN3L(Particle *particleList, int UNIVERSE_SIZE)
{
    Particle vec;
    float r, f;
    int NUM_PARTICLES_UNIVERSE = UNIVERSE_SIZE * UNIVERSE_SIZE * UNIVERSE_SIZE *PPC;
    #pragma omp parallel for num_threads(NUM_THREADS)
    for (int ref = 0; ref < NUM_PARTICLES_UNIVERSE; ref++)
    {
        for (int neighbor = 0; neighbor < NUM_PARTICLES_UNIVERSE-ref; neighbor++)
        {

            if (ref == neighbor)
            {
                continue;
            }
            else
            {

                modr(&vec, &particleList[ref], &particleList[neighbor],UNIVERSE_SIZE);
                r = norm(&vec);

                if (r > CUTOFF)
                {
                    continue;
                }

                if (vec.x < 0)
                {
                    continue;
                }

                f = lj(r);
                scalar_mul(&vec, DT * f / r);
                vec_add(&particleList[ref], &vec);
                
                scalar_mul(&vec, -1.);
                vec_add(&particleList[neighbor], &vec);
                
            }
        }
    }
}
void printData(Particle *particleList, int NUM_PARTICLES_UNIVERSE)
{
    printf("Printing Particle List!!\n --------------------\n");
    for (int i = 0; i < NUM_PARTICLES_UNIVERSE; i++)
    {
        printf("Particle:%d\n", particleList[i].particleId);
        printf("X: %f\n", particleList[i].x);
        printf("Y: %f\n", particleList[i].y);
        printf("Z: %f\n", particleList[i].z);
        printf("\n");
    }
    printf("Printing Particle Velocity!!\n --------------------\n");
    for (int i = 0; i < NUM_PARTICLES_UNIVERSE; i++)
    {
        printf("Particle:%d\n", particleList[i].particleId);
        printf("X: %f\n", particleList[i].vX);
        printf("Y: %f\n", particleList[i].vY);
        printf("Z: %f\n", particleList[i].vZ);
        printf("\n");
    }
}
void plot_particles(Particle *particleList, FILE *fp, int NUM_PARTICLES_UNIVERSE)
{

    for (int i = 0; i < NUM_PARTICLES_UNIVERSE; i++)
    {
        fprintf(fp, "%lf %lf %lf\n", particleList[i].x, particleList[i].y, particleList[i].z);
    }
    fclose(fp);
}
void positionUpdate(Particle *particleList, int UNIVERSE_SIZE)
{
    int NUM_PARTICLES_UNIVERSE = UNIVERSE_SIZE * UNIVERSE_SIZE * UNIVERSE_SIZE * 80;
    int L = CUTOFF * UNIVERSE_SIZE;

    for (int ref = 0; ref < NUM_PARTICLES_UNIVERSE; ref++)
    {
        Particle current = particleList[ref];

        current.x = fmod(current.x + current.vX * DT, L);
        current.y = fmod(current.y + current.vY * DT, L);
        current.z = fmod(current.z + current.vZ * DT, L);

        current.x = current.x < 0 ? current.x + L : current.x;
        current.y = current.y < 0 ? current.y + L : current.y;
        current.z = current.z < 0 ? current.z + L : current.z;

        current.x = current.x > L ? current.x - L : current.x;
        current.y = current.y > L ? current.y - L : current.y;
        current.z = current.z > L ? current.z - L : current.z;
    }
}
void modr(Particle *c, Particle *a,Particle *b, int UNIVERSE_SIZE){
    c->x=subm(a->x,b->x,UNIVERSE_SIZE);
    c->y=subm(a->y,b->y,UNIVERSE_SIZE);
    c->z=subm(a->z,b->z,UNIVERSE_SIZE);
}
float norm(Particle *r){
    return sqrt(powf(r->x,2)+powf(r->y,2)+powf(r->z,2));
}
float lj(float r){
    float f=4*EPSILON*(6*powf(SIGMA,6)/powf(r,7)-12*powf(SIGMA,12)/powf(r,13));

    if(f<LJ_MIN){
        return LJ_MIN;
    }
    else{
        return f;
    }

} 
void scalar_mul(Particle *v,float c){
    v->vX*=c;
    v->vY*=c;
    v->vY*=c;
}

void vec_add(Particle *a, Particle *b){
    a->x+=b->x;
    a->y+=b->y;
    a->z+=b->z;
}
float subm(float a,float b,int UNIVERSE_SIZE){
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
/* MPI-based Vectorized MD 
mpicc -O3 -mavx -std=gnu99 -lm -lrt MD_vec_MPI.c -o MD_vec_MPI
*/

#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <mpi.h>

#define EPSILON 120
#define SIGMA 0.34f
#define CUTOFF 0.85f
#define DT 1e-15f
#define PPC 80
#define LOWER_BOUND 4
#define UPPER_BOUND 4
#define ITERS 1
#define SCALAR 0
#define MIN_R powf(2,1/6)*SIGMA

typedef float data_t;

typedef struct {
    data_t *x, *y, *z;
} SoA;

typedef struct {
    SoA pos;
    SoA vel;
    int *id;
} ParticleArray;

void allocate_particle_array(ParticleArray *pa, int n) {
    posix_memalign((void**)&pa->pos.x, 32, sizeof(data_t) * n);
    posix_memalign((void**)&pa->pos.y, 32, sizeof(data_t) * n);
    posix_memalign((void**)&pa->pos.z, 32, sizeof(data_t) * n);
    posix_memalign((void**)&pa->vel.x, 32, sizeof(data_t) * n);
    posix_memalign((void**)&pa->vel.y, 32, sizeof(data_t) * n);
    posix_memalign((void**)&pa->vel.z, 32, sizeof(data_t) * n);
    posix_memalign((void**)&pa->id,    32, sizeof(int) * n);
}

void init_particles(ParticleArray *pa, int n, int universe_size) {
    float box_size = CUTOFF * universe_size;
    for (int i = 0; i < n; ++i) {
        pa->pos.x[i] = ((float)rand() / RAND_MAX) * box_size;
        pa->pos.y[i] = ((float)rand() / RAND_MAX) * box_size;
        pa->pos.z[i] = ((float)rand() / RAND_MAX) * box_size;
        pa->vel.x[i] = ((float)rand() / RAND_MAX) * box_size;
        pa->vel.y[i] = ((float)rand() / RAND_MAX) * box_size;
        pa->vel.z[i] = ((float)rand() / RAND_MAX) * box_size;
        pa->id[i] = i;
    }
}

__m128 lj_force_vec(__m128 r) {
    __m128 min_r = _mm_set1_ps(MIN_R);
    r = _mm_max_ps(r, min_r);
    __m128 r2 = _mm_mul_ps(r, r);
    __m128 r6 = _mm_mul_ps(_mm_mul_ps(r2, r2), r2);
    __m128 r12 = _mm_mul_ps(r6, r6);

    __m128 sig = _mm_set1_ps(SIGMA);
    __m128 sig2 = _mm_mul_ps(sig, sig);
    __m128 sig6 = _mm_mul_ps(_mm_mul_ps(sig2, sig2), sig2);
    __m128 sig12 = _mm_mul_ps(sig6, sig6);

    __m128 num1 = _mm_mul_ps(_mm_set1_ps(6.0f), sig6);
    __m128 num2 = _mm_mul_ps(_mm_set1_ps(12.0f), sig12);

    __m128 t1 = _mm_div_ps(num1, r6);
    __m128 t2 = _mm_div_ps(num2, r12);

    __m128 result = _mm_sub_ps(t1, t2);
    return _mm_mul_ps(_mm_set1_ps(4 * EPSILON * DT), _mm_div_ps(result, r));
}

void workerVelocity(ParticleArray *read, ParticleArray *write, int universe, int n, int start, int end) {
    float L = universe * CUTOFF;
    for (int i = start; i < end; i += 4) {
        __m128 xi = _mm_loadu_ps(&read->pos.x[i]);
        __m128 yi = _mm_loadu_ps(&read->pos.y[i]);
        __m128 zi = _mm_loadu_ps(&read->pos.z[i]);

        __m128 vxi = _mm_loadu_ps(&read->vel.x[i]);
        __m128 vyi = _mm_loadu_ps(&read->vel.y[i]);
        __m128 vzi = _mm_loadu_ps(&read->vel.z[i]);

        __m128 fx = _mm_setzero_ps();
        __m128 fy = _mm_setzero_ps();
        __m128 fz = _mm_setzero_ps();

        for (int j = 0; j < n; ++j) {
            __m128 xj = _mm_set1_ps(read->pos.x[j]);
            __m128 yj = _mm_set1_ps(read->pos.y[j]);
            __m128 zj = _mm_set1_ps(read->pos.z[j]);

            __m128 dx = _mm_sub_ps(xj, xi);
            __m128 dy = _mm_sub_ps(yj, yi);
            __m128 dz = _mm_sub_ps(zj, zi);

            __m128 Lvec = _mm_set1_ps(L);
            dx = _mm_sub_ps(dx, _mm_mul_ps(Lvec, _mm_round_ps(_mm_div_ps(dx, Lvec), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)));
            dy = _mm_sub_ps(dy, _mm_mul_ps(Lvec, _mm_round_ps(_mm_div_ps(dy, Lvec), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)));
            dz = _mm_sub_ps(dz, _mm_mul_ps(Lvec, _mm_round_ps(_mm_div_ps(dz, Lvec), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)));

            __m128 r2 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(dx, dx), _mm_mul_ps(dy, dy)), _mm_mul_ps(dz, dz));
            __m128 r = _mm_sqrt_ps(r2);
            __m128 f = lj_force_vec(r);

            fx = _mm_add_ps(fx, _mm_mul_ps(f, dx));
            fy = _mm_add_ps(fy, _mm_mul_ps(f, dy));
            fz = _mm_add_ps(fz, _mm_mul_ps(f, dz));
        }

        _mm_storeu_ps(&write->vel.x[i], _mm_add_ps(vxi, fx));
        _mm_storeu_ps(&write->vel.y[i], _mm_add_ps(vyi, fy));
        _mm_storeu_ps(&write->vel.z[i], _mm_add_ps(vzi, fz));
    }
}

void workerPosition(ParticleArray *read, ParticleArray *write, int universe, int n, int start, int end) {
    float L = universe * CUTOFF;
    __m128 Lvec = _mm_set1_ps(L);
    __m128 DTvec = _mm_set1_ps(DT);
    for (int i = start; i < end; i += 4) {
        __m128 px = _mm_loadu_ps(&read->pos.x[i]);
        __m128 py = _mm_loadu_ps(&read->pos.y[i]);
        __m128 pz = _mm_loadu_ps(&read->pos.z[i]);

        __m128 vx = _mm_loadu_ps(&read->vel.x[i]);
        __m128 vy = _mm_loadu_ps(&read->vel.y[i]);
        __m128 vz = _mm_loadu_ps(&read->vel.z[i]);

        __m128 new_x = _mm_add_ps(px, _mm_mul_ps(vx, DTvec));
        __m128 new_y = _mm_add_ps(py, _mm_mul_ps(vy, DTvec));
        __m128 new_z = _mm_add_ps(pz, _mm_mul_ps(vz, DTvec));

        __m128 qx = _mm_floor_ps(_mm_div_ps(new_x, Lvec));
        __m128 qy = _mm_floor_ps(_mm_div_ps(new_y, Lvec));
        __m128 qz = _mm_floor_ps(_mm_div_ps(new_z, Lvec));

        new_x = _mm_sub_ps(new_x, _mm_mul_ps(qx, Lvec));
        new_y = _mm_sub_ps(new_y, _mm_mul_ps(qy, Lvec));
        new_z = _mm_sub_ps(new_z, _mm_mul_ps(qz, Lvec));

        _mm_storeu_ps(&write->pos.x[i], new_x);
        _mm_storeu_ps(&write->pos.y[i], new_y);
        _mm_storeu_ps(&write->pos.z[i], new_z);
    }
}

void workerVelocity_scalar(ParticleArray *read, ParticleArray *write, int universe, int n, int start, int end) {
    float L = universe * CUTOFF;
    for (int i = start; i < end; ++i) {
        float fx = 0.0f, fy = 0.0f, fz = 0.0f;
        float xi = read->pos.x[i];
        float yi = read->pos.y[i];
        float zi = read->pos.z[i];
        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            float dx = read->pos.x[j] - xi;
            float dy = read->pos.y[j] - yi;
            float dz = read->pos.z[j] - zi;
            if (dx >  L/2) dx -= L; if (dx < -L/2) dx += L;
            if (dy >  L/2) dy -= L; if (dy < -L/2) dy += L;
            if (dz >  L/2) dz -= L; if (dz < -L/2) dz += L;
            float r2 = dx*dx + dy*dy + dz*dz;
            float r = sqrtf(r2);
            float r6 = r2 * r2 * r2;
            float r12 = r6 * r6;
            float sig6 = powf(SIGMA, 6);
            float sig12 = sig6 * sig6;
            float f_mag = 4 * EPSILON * ((6 * sig6 / r6) - (12 * sig12 / r12));
            f_mag = (f_mag * DT) / r;
            fx += f_mag * dx;
            fy += f_mag * dy;
            fz += f_mag * dz;
        }
        write->vel.x[i] = read->vel.x[i] + fx;
        write->vel.y[i] = read->vel.y[i] + fy;
        write->vel.z[i] = read->vel.z[i] + fz;
    }
}

void workerPosition_scalar(ParticleArray *read, ParticleArray *write, int universe, int n, int start, int end) {
    float L = universe * CUTOFF;
    for (int i = start; i < end; ++i) {
        float x = read->pos.x[i] + read->vel.x[i] * DT;
        float y = read->pos.y[i] + read->vel.y[i] * DT;
        float z = read->pos.z[i] + read->vel.z[i] * DT;
        x = fmodf(x, L); if (x < 0) x += L;
        y = fmodf(y, L); if (y < 0) y += L;
        z = fmodf(z, L); if (z < 0) z += L;
        write->pos.x[i] = x;
        write->pos.y[i] = y;
        write->pos.z[i] = z;
    }
}

float compare_arrays(const float *a, const float *b, int n) {
    float max_diff = 0.0f;
    for (int i = 0; i < n; ++i) {
        float diff = fabsf(a[i] - b[i]);
        if (diff > max_diff) max_diff = diff;
    }
    return max_diff;
}

void validate_outputs(ParticleArray *a, ParticleArray *b, int n) {
    float max_x = compare_arrays(a->pos.x, b->pos.x, n);
    float max_y = compare_arrays(a->pos.y, b->pos.y, n);
    float max_z = compare_arrays(a->pos.z, b->pos.z, n);
    printf("Max position error: x=%e, y=%e, z=%e\n", max_x, max_y, max_z);
    float max_vx = compare_arrays(a->vel.x, b->vel.x, n);
    float max_vy = compare_arrays(a->vel.y, b->vel.y, n);
    float max_vz = compare_arrays(a->vel.z, b->vel.z, n);
    printf("Max velocity error: x=%e, y=%e, z=%e\n", max_vx, max_vy, max_vz);
}
int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    srand(42);
    for (int U = LOWER_BOUND; U <= UPPER_BOUND; ++U) {
        if (world_rank == 0) printf("--- Universe Size: %d ---", U);
        int N = U * U * U * PPC;
        int base = N / world_size;
        int start = world_rank * base;
        int end = (world_rank == world_size - 1) ? N : start + base;
        printf("\n--- Universe Size: %d, Particles: %d ---\n", U, N);

        ParticleArray simdA, simdB, scalarA, scalarB;
        allocate_particle_array(&simdA, N);
        allocate_particle_array(&simdB, N);
        allocate_particle_array(&scalarA, N);
        allocate_particle_array(&scalarB, N);

        if (world_rank == 0) {
            init_particles(&simdA, N, U);
            memcpy(scalarA.pos.x, simdA.pos.x, sizeof(data_t)*N);
            memcpy(scalarA.pos.y, simdA.pos.y, sizeof(data_t)*N);
            memcpy(scalarA.pos.z, simdA.pos.z, sizeof(data_t)*N);
            memcpy(scalarA.vel.x, simdA.vel.x, sizeof(data_t)*N);
            memcpy(scalarA.vel.y, simdA.vel.y, sizeof(data_t)*N);
            memcpy(scalarA.vel.z, simdA.vel.z, sizeof(data_t)*N);
            memcpy(scalarA.id,    simdA.id,    sizeof(int)*N);
        }
        MPI_Bcast(simdA.pos.x, N, MPI_FLOAT, 0, MPI_COMM_WORLD);
        MPI_Bcast(simdA.pos.y, N, MPI_FLOAT, 0, MPI_COMM_WORLD);
        MPI_Bcast(simdA.pos.z, N, MPI_FLOAT, 0, MPI_COMM_WORLD);
        MPI_Bcast(simdA.vel.x, N, MPI_FLOAT, 0, MPI_COMM_WORLD);
        MPI_Bcast(simdA.vel.y, N, MPI_FLOAT, 0, MPI_COMM_WORLD);
        MPI_Bcast(simdA.vel.z, N, MPI_FLOAT, 0, MPI_COMM_WORLD);

        struct timespec starting, ending;

        // SIMD path
        double simd_start = MPI_Wtime();
        for (int iter = 0; iter < ITERS; ++iter) {
            workerVelocity(&simdA, &simdB, U, N, start, end);
            
            MPI_Allgather(&simdB.vel.x[start], end - start, MPI_FLOAT, simdA.vel.x, end - start, MPI_FLOAT, MPI_COMM_WORLD);
            MPI_Allgather(&simdB.vel.y[start], end - start, MPI_FLOAT, simdA.vel.y, end - start, MPI_FLOAT, MPI_COMM_WORLD);
            MPI_Allgather(&simdB.vel.z[start], end - start, MPI_FLOAT, simdA.vel.z, end - start, MPI_FLOAT, MPI_COMM_WORLD);
            MPI_Barrier(MPI_COMM_WORLD);

            workerPosition(&simdB, &simdA, U, N, start, end);
            
            MPI_Allgather(&simdA.pos.x[start], end - start, MPI_FLOAT, simdA.pos.x, end - start, MPI_FLOAT, MPI_COMM_WORLD);
            MPI_Allgather(&simdA.pos.y[start], end - start, MPI_FLOAT, simdA.pos.y, end - start, MPI_FLOAT, MPI_COMM_WORLD);
            MPI_Allgather(&simdA.pos.z[start], end - start, MPI_FLOAT, simdA.pos.z, end - start, MPI_FLOAT, MPI_COMM_WORLD);
            MPI_Barrier(MPI_COMM_WORLD);

        }
        double simd_end = MPI_Wtime();
        double simd_time = simd_end - simd_start;


        if(SCALAR){
        // Scalar path
        double scalar_start = MPI_Wtime();
        for (int iter = 0; iter < ITERS; ++iter) {
            workerVelocity_scalar(&scalarA, &scalarB, U, N, 0, N);
            workerPosition_scalar(&scalarB, &scalarA, U, N, 0, N);
        }
        double scalar_end = MPI_Wtime();
        double scalar_time = scalar_end - scalar_start;

        
        if(world_rank==0){
        printf("SIMD Time:   %f s\n", simd_time);
        printf("Scalar Time: %f s\n", scalar_time);

        validate_outputs(&simdA, &scalarA, N);
        }

        free(scalarA.pos.x); free(scalarA.pos.y); free(scalarA.pos.z);
        free(scalarA.vel.x); free(scalarA.vel.y); free(scalarA.vel.z);
        free(scalarA.id);
        free(scalarB.pos.x); free(scalarB.pos.y); free(scalarB.pos.z);
        free(scalarB.vel.x); free(scalarB.vel.y); free(scalarB.vel.z);
        free(scalarB.id);
        
        }
        else{
                printf("SIMD Time: %f s\n", simd_time);
        }
        free(simdA.pos.x); free(simdA.pos.y); free(simdA.pos.z);
        free(simdA.vel.x); free(simdA.vel.y); free(simdA.vel.z);
        free(simdA.id);
        free(simdB.pos.x); free(simdB.pos.y); free(simdB.pos.z);
        free(simdB.vel.x); free(simdB.vel.y); free(simdB.vel.z);
        free(simdB.id);
        
    }
    MPI_Finalize();
    return 0;
}

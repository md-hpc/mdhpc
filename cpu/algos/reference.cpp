#include "common.h"

#include <cstdio>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <vector>

using namespace std;

char default_path[] = "particles";
char default_log[] = "verify/particles.txt";
char *path = default_path;
char *LOG_PATH = default_log;

// macros for determining import regions

vector<particle> particles;


int main(int argc, char **argv) {
    char **arg;
    particle *p, p_;
    vector<particle> *hc, *nc, *cell;
    vector<particle> *outbound;

    int i, j, k, n;
    int di, dj, dk;
    int cidx, pidx, nidx;
    int ccidx[3];

    int dirfd, fd;

    float r, f, buf[3];

    vec v;

    parse_cli(argc,argv);
    
    init_particles(particles);

    fd = open(path,O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Could not open file");
        exit(1);
    }
    int hdr[2] = {N_PARTICLE, RESOLUTION};
    write(fd, hdr, sizeof(int) * 2);

    for (int t = 0; t < N_TIMESTEP; t++) {
        dprintf(2,"Timestep %d\n",t);
        // velocity update
        particle *pr, *pn;
        int ri, ni;
        for (ri = 0; ri < N_PARTICLE; ri++) {
            pr = &particles[ri];
            for (ni = 0; ni < N_PARTICLE; ni++) {
                pn = &particles[ni];

                if (ri == ni)
                    continue;

#ifdef DEBUG
                if (pr->id == BR && pn->id == BN) {
                    printf("%d %d\n", BR, BN);    
                }
#endif


                if (pr->id == BR && pn->id == BN) {
                    printf("%d %d\n", BR, BN);    
                }


                
                v = pn->r % pr->r;

                r = v.norm();
                if (r > CUTOFF)
                    continue;
#ifdef DEBUG
                printf("%d %d %d\n",t,pr->id,pn->id);
#endif



                r = v.norm();
                f = lj(r);
                v *= f * DT / r;
                
                pr->v += v;
            }
        }

        // motion update
        for (int pi = 0; pi < N_PARTICLE; pi++) {
            p = &particles[pi];
            p->r += p->v * DT;
            p->r.apbc();
        }

        // save results
        if (t % RESOLUTION == 0) {
            save(particles,fd);
        }
    }

    close(fd);
}

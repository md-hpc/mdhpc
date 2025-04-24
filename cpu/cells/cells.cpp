// cell lists. Do not apply N3L

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
char *path = default_path;
char default_log[] = "verify/cells.cpp";
char *LOG_PATH = default_log;


// holds particles that have exceeded the bounds of their cells, but have yet to be migrated to their new cell
// we'll need this for thread-safety as there cannot be multiple writers to an std::vector
vector<vector<particle>> outbounds;
vector<vector<particle>> cells;
vector<particle> particles;


int main(int argc, char **argv) {
    char **arg;
    particle *p, p_;
    vector<particle> *hc, *nc, *cell;
    vector<particle> *outbound;

    int i, j, k, n;
    int di, dj, dk;
    int ci, pi, nci;
    int cci[3];
    
    int dirfd, fd, logfd, nulfd;

    float r, f, buf[3];

    vec v, v_r;

    int cur;

    long interactions = 0;

    if (parse_cli(argc, argv)) {
        exit(1);
    }

    cells.resize(N_CELL);
    outbounds.resize(N_CELL); 

    init_particles(particles);
    for (int i = 0; i < N_PARTICLE; i++) {
        p = &particles[i];
        cells[p->update_cell()].push_back(*p);
    }
    particles.resize(0);

    fd = open(path,O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Could not open file");
        exit(1);
    }
    int hdr[2] = {N_PARTICLE, RESOLUTION};
    write(fd, hdr, sizeof(int) * 2);
    
    logfd = open(LOG_PATH, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    nulfd = open("/dev/null", O_RDWR);

    for (int t = 0; t < N_TIMESTEP; t++) {
        dprintf(2,"Timestep %d\n",t);        
        for (int hci = 0; hci < N_CELL; hci++) {
            hc = &cells[hci];

            cubic_idx(cci, hci);
            i = cci[0];
            j = cci[1];
            k = cci[2];

            for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                    for (int dk = -1; dk <= 1; dk++) {
                        nci = linear_idx(i+di, j+dj, k+dk);
                        nc = &cells[nci];
                        
                        particle *pr, *pn;
                        int ri, ni;
                        int nr = hc->size(), nn = nc->size(); 
                        for (ri = 0; ri < nr; ri++) {
                            pr = &(*hc)[ri];
                            for (ni = 0; ni < nn; ni++) {
                                pn = &(*nc)[ni];
                               
#ifdef DEBUG
                                if (pr->id == BR && pn->id == BN) {
                                    dprintf(nulfd,"%d %d\n");
                                }
#endif

                                if (hci == nci && ri == ni)
                                    continue;
                                
                                v = pn->r % pr->r;            
                                r = v.norm();

                                if (r > CUTOFF) {
                                   continue;
                                }
#ifdef DEBUG
                                dprintf(logfd, "%d %d %d\n",t, pr->id, pn->id);
#endif
                                f = lj(r);
                                v *= f / r * DT;
                                
                                pr->v += v;
                            }
                        }
                    }
                }
            }
        }

       
        // position update
        for (int i = 0; i < outbounds.size(); i++) {
            outbounds[i].resize(0);
        }
        
        int hci, nc, np;
        vector<particle> *hc;
        particle *p;
        for (ci = 0; ci < N_CELL; ci++) {
            cell = &cells[ci];
            cur = 0;
            np = cell->size();
            for (pi = 0; pi < np; pi++) {
                p = &(*cell)[pi];
                p->r += p->v * DT;
                p->r.apbc();
                hci = p->update_cell();

                if (hci == ci) {
                    (*cell)[cur++] = *p;
                } else {
                    #ifdef DEBUG                
                    printf("s %d %s\n",t,p->str());
                    #endif

                    outbounds[ci].push_back(*p);
                }
            }
            cell->resize(cur);
        }
        
        // particle migration
        for (int hci = 0; hci < N_CELL; hci++) {
            hc = &cells[hci];
            cubic_idx(cci, hci);
            i = cci[0];
            j = cci[1];
            k = cci[2];

            for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                    for (int dk = -1; dk <= 1; dk++) {
                        nci = linear_idx(i+di, j+dj, k+dk);
                        if (hci == 72)
                            printf("break\n");
                        outbound = &outbounds[nci];
                        
                        int pi;
                        particle *p;
                        int np = outbound->size();
                        for (pi = 0; pi < np; pi++) {
                            p = &(*outbound)[pi];
                            if (p->cell == hci) {
                                hc->push_back(*p);
                                
                                #ifdef DEBUG
                                printf("r %d %s\n",t,p->str());
                                #endif
                            }
                        }
                    }
                }
            }
        }
	
        // save results
        if (t % RESOLUTION == 0) {
            save(cells, fd);
        }
    }

    close(fd);
}

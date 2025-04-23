#include <cstdio>
#include <cmath>
#include <set>
#include <algorithm>
#include <array>

#include "simulation.h"

void simulation::do_work(worker_spec_t *spec) {
	int start = spec->start;
	int stop = spec->stop;

	set<int> ns;

	// initialize export_fbufs[spec->core]
	for (int i = start; i < stop; i++) {
		Pixel hcp = pixelof(i);

		for (int di = -1; di <= 1; di++) {
			for (int dj = -1; dj <= 1; dj++) {
				if (di < 0 || di == 0 && dj < 0)
					continue;
				int nci = cell(hcp.i + di, hcp.j + dj);
				if (nci < start || nci >= stop)
					ns.insert(nci);
			}
		}
	}


	for (set<int>::iterator it = ns.begin(); it != ns.end(); ++it) {
		export_fbufs[spec->core].emplace(*it, vector<vec8>());
	}
	ns.clear();

	pthread_barrier_wait(&barrier);
	
	
	// initialize import_fbufs_cores[spec->core]
	for (int core = 0; core < THREADS; core++) {
		if (core == spec->core)
			continue;

		for (fbufd_t::iterator it = export_fbufs[core].begin(); it != export_fbufs[core].end(); ++it) {
			int cell = it->first;
			if (start <= cell && cell < stop) {
				if (!import_fbuf_cores[spec->core].count(cell)) {
					import_fbuf_cores[spec->core].emplace(cell, vector<int>());
				}
				import_fbuf_cores[spec->core][cell].push_back(core);
			}
		}
	}

	for (int t = 0; t < TIMESTEPS; t++) {
		pthread_barrier_wait(&parent_barrier);
		// let the parent do its per-timestep processing
		pthread_barrier_wait(&parent_barrier);

		// reset this core's shared memory objects
		for (fbufd_t::iterator it = export_fbufs[spec->core].begin(); it != export_fbufs[spec->core].end(); ++it) {
			f8 fz = VK(0);
			vec8 z = vec8(fz, fz, fz);
			vector<vec8> *fbuf = &export_fbufs[spec->core][it->first];
			fbuf->resize(velocities[it->first].size8());
			fill(fbuf->begin(), fbuf->end(), z);
		}
		outbound_particles[spec->core].resize(0);
    
        if (t % NEIGHBOR_REFRESH_RATE == 0) {
            for (int i = start; i < stop; i++) {
                export_particles_worker(i,spec);
            }
            pthread_barrier_wait(&barrier);
            
            import_particles_worker(spec);

			// no barrier needed

			for (int i = start; i < stop; i++) {
            	sort_particles_worker(i,spec);
			}

			pthread_barrier_wait(&barrier);

            for (int i = start; i < stop; i++) {
                compute_ranges_worker(i,spec);
            }

			// no barrier needed
        }

		for (int i = start; i < stop; i++) {
			velocity_update_worker(i,spec);
		}
		pthread_barrier_wait(&barrier);
		
		#ifdef DEBUG
		pthread_barrier_wait(&parent_barrier);
		pthread_barrier_wait(&parent_barrier);
		#endif

		for (int i = start; i < stop; i++) {
			reduce_fbufs_worker(i,spec);
		}
		pthread_barrier_wait(&barrier);

		#ifdef DEBUG
		pthread_barrier_wait(&parent_barrier);
		pthread_barrier_wait(&parent_barrier);
		#endif

		for (int i = start; i < stop; i++) {
			position_update_worker(i,spec);
		}
		pthread_barrier_wait(&barrier);

		#ifdef DEBUG
		pthread_barrier_wait(&parent_barrier);
		pthread_barrier_wait(&parent_barrier);
		#endif
	}
}

void simulation::export_particles_worker(int hci, worker_spec_t* spec) {
 	int np = positions[hci].size8();
	int cur = 0;
	const int hciv = hci;

	v8buf rbuf, vbuf;

	for (int pi = 0; pi < np; pi++) {
		vec8 r = positions[hci][pi];
		vec8 v = velocities[hci][pi];

		ipack cells = { .v=cellv(r) };

		if (alleq(cells.v, hciv)) {
			// if no particles have left this cell, perform aligned move
			positions[hci][cur] = r;
			velocities[hci][cur] = v;
			cur++;

		} else {
			// we must pick particle-by-particle who needs to be moved to the outbound buffer
			for (int i = 0; i < VSIZE; i++) {
				if (cells.d[i] == hci) {
					// append to consolidation buffer
					
					if (rbuf.append(r.get(i)), vbuf.append(v.get(i))) {
						// if buffer is full, perform aligned store to the cell list
						positions[hci][cur] = rbuf.get();
						velocities[hci][cur] = vbuf.get();
						cur++;
					}
				} else if (spec->start <= cells.d[i] && cells.d[i] < spec->stop) {
					// the particle has moved cells, but the cell is in our thread group, so 
					// no need to worry about synchronization
					positions[cells.d[i]].append(r.get(i));
					velocities[cells.d[i]].append(v.get(i));

				} else if (cells.d[i] >= 0) {
					// append to outbound buffer
					// 
					// if this is a nonexistent particle (NAN) its cell will be a large negative number, so
					// we filter out those with the conditional clause
			
					particle p = particle(
						r.get(i),
						v.get(i),
						cells.d[i]
					);
					outbound_particles[spec->core].push_back(p);
				}
			}
		}
	}
	

	int sz = cur * VSIZE;
	if (vbuf.i > 0) {
		// flush remaining buffer to the cell list
		sz += vbuf.i;
		positions[hci][cur] = rbuf.get();
		velocities[hci][cur] = vbuf.get();
	}
	positions[hci].resize(sz);
	velocities[hci].resize(sz);
 
}

void simulation::import_particles_worker(worker_spec_t *spec) {
	for (int i = 0; i < THREADS; i++) {
		if (i == spec->core)
			continue;

		int np = outbound_particles[i].size();

		for (int j = 0; j < np; j++) {
			particle p = outbound_particles[i][j];
			if (spec->start <= p.cell && p.cell < spec->stop) {
				positions[p.cell].append(p.r);
				velocities[p.cell].append(p.v);
			}
		}
	}

}

void simulation::sort_particles_worker(int hci, worker_spec_t *spec) {
	int np = positions[hci].size1();
	vector<particle> particles;
	particles.resize(np);

	for (int i = 0; i < np; i++) {
		particles[i].r = positions[hci].get(i);
		particles[i].v = velocities[hci].get(i);
	}

	sort(particles.begin(), particles.end());
	
	for (int i = 0; i < np; i++) {
		positions[hci].set(particles[i].r, i);
		velocities[hci].set(particles[i].v, i);
	}
}

// boundary distance
static inline float bd(float dx, float cutoff, int di) {
	return di == -1 ? dx : di == 0 ? 0 : cutoff - dx;
}

void simulation::compute_ranges_worker(int hci, worker_spec_t *spec) {
	int np = positions[hci].size1();
	Pixel hpx = pixelof(hci);

	neighborhoods[hci].resize(np);

	for (int pi = 0; pi < np; pi++) {
		vec v = positions[hci].get(pi);
		float z, dx, dy;
		z = v.z;
		dx = v.x - CUTOFF * hpx.i;
		dy = v.y - CUTOFF * hpx.j;

		for (int di = -1; di <= 1; di++) {
			for (int dj = -1; dj <= 1; dj++) {
				if (di < 0 || di == 0 && dj < 0)
					continue;

				neighborhoods[hci][pi][di+1][dj+1] = rangeof(
					z,
					bd(dx,CUTOFF,di),
					bd(dy,CUTOFF,dj),
					positions[cell(hpx.i+di,hpx.j+dj)]
				);
			}	
		}
		// n3l, so start >= reference_idx when neighbor_cell == home_cell
		neighborhoods[hci][pi][1][1].start = pi + 1;
	}
}

void simulation::velocity_update_worker(int hci, worker_spec_t* spec) {
	Pixel hcp = pixelof(hci);
	int nr = positions[hci].size1();
	
	for (int di = -1; di <= 1; di++) {
		for (int dj = -1; dj <= 1; dj++) {
			int nci = cell(hcp.i + di, hcp.j + dj);				
			int nn = positions[nci].size8();
			
			vec8 *nrs, *nvs;
			
			nrs = &positions[nci][0];
			
			if (nci < spec->start || spec->stop <= nci) {
				nvs = &export_fbufs[spec->core][nci][0];
			} else {
				nvs = &velocities[nci][0];
			}

			for (int ri = 0; ri < nr; ri++) {
				vec8 rr = vec8(positions[hci].get(ri));
				vec8 rv = vec8(velocities[hci].get(ri));

				Range range = neighborhoods[hci][ri][di+1][dj+1];

				if (range.start == -1 || range.stop == -1) {
					printf("Range is (%d, %d). Something has gone wrong\n", range.start, range.stop);
					exit(1);
				}
				for (int ni = range.start; ni < range.stop; ni++) {
					vec8 nr = nrs[ni];

					vec8 v = lj(rr, nr);
					rv += v;

					v *= -1;
					nvs[ni] += v;
				}
				velocities[hci][ri].set(
					vec(sum(rv.x), sum(rv.y), sum(rv.z)), ri
				);
			}

		}
	}

	// handle ri == ni && hci == nci for this cell. Do not apply n3l
	for (int i = 0; i < nr; i++) {
		vec8 rr, nr, rv, v;
		rr = positions[hci][i];
		rv = velocities[hci][i];
		nr = positions[hci][i];

		for (int p = 0; p < 7; p++) {
			nr.permutev();
			rv += lj(rr, nr);
		}
		velocities[hci][i] = rv;
	}
}

void simulation::reduce_fbufs_worker(int hci, worker_spec_t *spec) {
	int nc = import_fbuf_cores[spec->core][hci].size();
	for (int i = 0; i < nc; i++) {
		int core = import_fbuf_cores[spec->core][hci][i];

		int nv = export_fbufs[core][hci].size();
		for (int j = 0; j < nv; j++) {
			velocities[hci][j] += export_fbufs[core][hci][j];
		}
	}
}

void simulation::position_update_worker(int hci, worker_spec_t *spec) {
	// consolidation buffer
	v8buf vbuf, rbuf;
	int np = positions[hci].size8();
 	for (int i = 0; i < np; i++) {
		vec8 r, v;
		r = positions[hci][i];
		v = velocities[hci][i];
		r += v * DT;
		apbcfv(r); // apply periodic boundary condition (floating-point, vector)
		positions[hci][i] = r;
	}
}

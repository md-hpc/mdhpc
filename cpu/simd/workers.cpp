#include <cstdio>
#include <cmath>
#include <set>
#include <algorithm>

#include "simulation.h"

void simulation::do_work(worker_spec_t *spec) {
	int start = spec->start;
	int stop = spec->stop;

	set<int> ns;

	// initialize export_fbufs[spec->core]
	for (int i = start; i < stop; i++) {
		voxel hcv = voxelof(i);

		for (int di = -1; di <= 1; di++) {
			for (int dj = -1; dj <= 1; dj++) {
				for (int dk = -1; dk <= 1; dk++) {
					if (di < 0 || di == 0 && dj < 0 || di == 0 && dj == 0 && dk < 0)
						continue;
					int nci = cell(hcv.i + di, hcv.j + dj, hcv.k + dk);
					if (nci < start || nci >= stop)
						ns.insert(nci);
				}
			}
		}
	}


	for (set<int>::iterator it = ns.begin(); it != ns.end(); ++it) {
		export_fbufs[spec->core].emplace(*it, vector<vec8>());
	}

	pthread_barrier_wait(&barrier);
	
	ns.clear();
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
				ns.insert(core);
			}
		}
	}

	for (set<int>::iterator it = ns.begin(); it != ns.end(); ++it) {
		core_neighbors[spec->core].push_back(*it);	
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

		particle_migrate_worker(spec);
	}
}

void simulation::velocity_update_worker(int hci, worker_spec_t* spec) {
	voxel hcv = voxelof(hci);
	int nr = positions[hci].size8();
	
	f8 potential = VK(0);
	const float two = 2;

	for (int di = -1; di <= 1; di++) {
		for (int dj = -1; dj <= 1; dj++) {
			for (int dk = -1; dk <= 1; dk++) {
				if (di < 0 || di == 0 && dj < 0 || di == 0 && dj == 0 && dk < 0)
					continue;

				int nci = cell(hcv.i + di, hcv.j + dj, hcv.k + dk);				
				int nn = positions[nci].size8();
				
				
				vec8 *nrs, *nvs;
				
				nrs = &positions[nci][0];
				
				if (nci < spec->start || spec->stop <= nci) {
					nvs = &export_fbufs[spec->core][nci][0];
				} else {
					nvs = &velocities[nci][0];
				}

				for (int ri = 0; ri < nr; ri++) {
					vec8 rr = positions[hci][ri];
					vec8 rv = velocities[hci][ri];

					for (int p = 0; p < VSIZE; p++) {
						for (int ni = 0; ni < nn; ni++) {
							// nci >= hci from conditional before
							//
							// ni > ri (&& nci == hci) should be omitted to leverage n3l
							// ni == ri (&& nci == hci) requires special handling (no n3l)
							if (nci == hci && ni >= ri) 
								continue;

							vec8 nr = nrs[ni];

							vec8 v = lj(rr, nr);
							rv += v;

							v *= -1;
							nvs[ni] += v;

							potential += 2*LJ(rr, nr);
						}
				
						rr.permutev();
						rv.permutev();
					}
					velocities[hci][ri] = rv;
				}
			}
		}
	}

	// handle ri == ni && hci == nci for this cell. Do not apply n3l
	for (int i = 0; i < nr; i++) {
		vec8 rp, np, rv, v;
		rp = positions[hci][i];
		rv = velocities[hci][i];
		np = positions[hci][i];

		for (int p = 0; p < 7; p++) {
			np.permutev();
			rv += lj(rp, np);
			potential += LJ(rp,np);
		}
		velocities[hci][i] = rv;
	}

	// potential energy
	pe[spec->core].push_back(sum(potential));
}

void simulation::reduce_fbufs_worker(int hci, worker_spec_t *spec) {
	if (!import_fbuf_cores[spec->core].count(hci)) {
		return;
	}

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
	// hci === home cell index

	int np = positions[hci].size8();
	int cur = 0;
	const int hciv = hci;

	// consolidation buffer
	v8buf vbuf, rbuf;

	// kinetic energy
	f8 kinetic = VK(0);

	for (int pi = 0; pi < np; pi++) {
		vec8 r, v;
		r = positions[hci][pi];
		v = velocities[hci][pi];
		r += v * DT;
		apbcfv(r); // apply periodic boundary condition (floating-point, vector)
		
		kinetic += v.normsq() / 2;

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

	ke[spec->core].push_back(sum(kinetic));
}

void simulation::particle_migrate_worker(worker_spec_t *spec) {	
	
	for (int i = 0; i < THREADS; i++) {
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

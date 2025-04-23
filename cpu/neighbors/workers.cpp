#include <cstdio>
#include <cmath>
#include <set>
#include <algorithm>

#include "simulation.h"
#include "offset.h"

void Simulation::do_work(worker_spec_t *spec) {
	int start = spec->start;
	int stop = spec->stop;
	int core = spec->core;

	set<int> ns;

	// initialize export_fbufs[core]
	for (int i = start; i < stop; i++) {
		Voxel hcv = voxelof(i);
		for (Offset d = Offset(); !d.done(); d.inc()) {
			if (d.i < 0 || d.i == 0 && d.j < 0 || d.i == 0 && d.j == 0 && d.k < 0) {
				continue;
			}

			int nci = cell(hcv.i + d.i, hcv.j + d.j, hcv.k + d.k);
			if (nci < start || stop <= nci) {
				ns.insert(nci);
			}
		}	
	}


	for (set<int>::iterator it = ns.begin(); it != ns.end(); ++it) {
		export_fbufs[core].emplace(*it, vector<vec>());
	}

	pthread_barrier_wait(&barrier);
	
	ns.clear();
	// initialize import_fbufs_cores[core]
	for (int core = 0; core < THREADS; core++) {
		if (core == spec->core)
			continue;

		for (fbufd_t::iterator it = export_fbufs[core].begin(); it != export_fbufs[core].end(); ++it) {
			int cell = it->first;
			if (start <= cell && cell < stop) {
				if (!import_fbuf_cores[core].count(cell)) {
					import_fbuf_cores[core].emplace(cell, vector<int>());
				}
				import_fbuf_cores[core][cell].push_back(core);
			}
		}
	}
	

	for (int t = 0; t < TIMESTEPS; t++) {		
		for (fbufd_t::iterator it = export_fbufs[core].begin(); it != export_fbufs[core].end(); ++it) {
			vec z = vec();
			vector<vec> *fbuf = &export_fbufs[core][it->first];
			fbuf->resize(velocities[it->first].size());
			fill(fbuf->begin(), fbuf->end(), z);
		}

		if (t % NEIGHBOR_REFRESH_RATE == 0) {
			outbound_particles[core].resize(0);

			for (int i = start; i < stop; i++) {
				export_particles_worker(i, spec);
			}
			pthread_barrier_wait(&barrier);

			import_particles_worker(spec);
			pthread_barrier_wait(&barrier);

			for (int i = start; i < stop; i++) {
				create_neighbor_lists_worker(i, spec);
			}

			#ifdef DEBUG
			int n = 0;
			for (int ci = start; ci < stop; ci++) {
				int np = neighborhoods[ci].size();
				for (int pi = 0; pi < np; pi++) {
					n += neighborhoods[ci][pi].size();
				}
			}
			pthread_mutex_lock(&mutex);
			printf("%d %d: %d\n", timestep, core, n);
			pthread_mutex_unlock(&mutex);
			#endif

		}


		for (int i = start; i < stop; i++) {
			velocity_update_worker(i,spec);
		}
		pthread_barrier_wait(&barrier);
		
		for (int i = start; i < stop; i++) {
			reduce_fbufs_worker(i,spec);
		}
		pthread_barrier_wait(&barrier);

		for (int i = start; i < stop; i++) {
			position_update_worker(i,spec);
		}
		pthread_barrier_wait(&barrier);

		pthread_barrier_wait(&parent_barrier);
		// let the parent do its per-timestep processing
	}
}

void Simulation::export_particles_worker(int hci, worker_spec_t* spec) {
	int np = positions[hci].size();
	int cur = 0;

	for (int pi = 0; pi < np; pi++) {
		int ci = cell(positions[hci][pi]);
		if (ci == hci) {
			positions[hci][cur] = positions[hci][pi];
			velocities[hci][cur] = velocities[hci][pi];
			cur++;

		} else if (spec->start <= ci && ci < spec->stop) {
			positions[ci].push_back(positions[hci][pi]);
			velocities[ci].push_back(velocities[hci][pi]);
		} else {
			outbound_particles[spec->core].push_back(particle(
				positions[hci][pi],
				velocities[hci][pi],
				ci
			));
		}
	}
}

void Simulation::import_particles_worker(worker_spec_t* spec) {
	for (int t = 0; t < THREADS; t++) {
		int np = outbound_particles[t].size();
		for (int pi = 0; pi < np; pi++) {
			particle p = outbound_particles[t][pi];
			if (spec->start <= p.cell && p.cell < spec->stop) {
				positions[p.cell].push_back(p.r);
				velocities[p.cell].push_back(p.v);
			}
		}
	}
}

void Simulation::create_neighbor_lists_worker(int hci, worker_spec_t* spec) {
	int nr = positions[hci].size();
	Voxel hvx = voxelof(hci);

	neighborhoods[hci].resize(nr);

	for (int ri = 0; ri < nr; ri++) {
		neighborhoods[hci][ri].resize(0);
		vec rr = positions[hci][ri];

		for (Offset d = Offset(); !d.done(); d.inc()) {
			if (d.i < 0 || d.i == 0 && d.j < 0 || d.i == 0 && d.j == 0 && d.k < 0) {
				continue;
			}

			int nci = cell(hvx.i + d.i, hvx.j + d.j, hvx.k + d.k);
			int nn = positions[nci].size();


			for (int ni = 0; ni < nn; ni++) {
				vec nr = positions[nci][ni];

				if (!(hci == nci && ri == ni) && rr.x < nr.x && rpb(rr,nr).norm() < CUTOFF) {
					neighborhoods[hci][ri].push_back({ .cell = nci, .idx = ni});
				}
			}
		}
	}
}

void Simulation::velocity_update_worker(int hci, worker_spec_t* spec) {
	Voxel hcv = voxelof(hci);
	int nr = positions[hci].size();

	for (int ri = 0; ri < nr; ri++) {
		vec rp = positions[hci][ri];
		int nn = neighborhoods[hci][ri].size();
		vec rdv = vec(0,0,0);

		for (int ni = 0; ni < nn; ni++) {
			neighbor n = neighborhoods[hci][ri][ni];
			vec np = positions[n.cell][n.idx];

			vec dv = lj(rp, np);
			
			#ifdef DEBUG
			if (dv.isnan()) {
				warn_nan();
				exit(1);
			}
			#endif

			rdv += dv;

			if (n.cell < spec->start || spec->stop <= n.cell) {
				#ifdef DEBUG
				if (export_fbufs[spec->core].count(n.cell) == 0 || export_fbufs[spec->core][n.cell].size() <= n.idx) {
					Voxel ncv = voxelof(n.cell);
					printf("Faulty neighbor: %d, %s -> %s, %d\n", spec->core, hcv.str(), ncv.str(), n.idx);
					exit(1);
				}
				#endif
				export_fbufs[spec->core][n.cell][n.idx] -= dv;
			} else {
				velocities[n.cell][n.idx] -= dv;
			}
		}

		velocities[hci][ri] += rdv;
	}
}

void Simulation::reduce_fbufs_worker(int hci, worker_spec_t *spec) {
	// first check if this cell is imported by any other cores
	if (!import_fbuf_cores[spec->core].count(hci)) {
		return;
	}

	int nc = import_fbuf_cores[spec->core][hci].size();
	for (int i = 0; i < nc; i++) {
		int neighbor_core = import_fbuf_cores[spec->core][hci][i];

		int nv = export_fbufs[neighbor_core][hci].size();
		for (int j = 0; j < nv; j++) {
			#ifdef DEBUG
			if (export_fbufs[neighbor_core][hci][j].isnan()) {
				warn_nan();
				exit(1);
			}
			#endif
			velocities[hci][j] += export_fbufs[neighbor_core][hci][j];
		}
	}
}

void Simulation::position_update_worker(int hci, worker_spec_t *spec) {
	int np = positions[hci].size();
	int cur = 0;
	const int hciv = hci;

	for (int pi = 0; pi < np; pi++) {
		#ifdef DEBUG
		if (velocities[hci][pi].isnan()) {
			warn_nan();
			exit(1);
		}
		#endif

		positions[hci][pi] += velocities[hci][pi]*DT;
		apb(positions[hci][pi]);
	}
}

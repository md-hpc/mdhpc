gxx := g++ -g -march=native
src := main.cpp offset.cpp particle.cpp simulation.cpp simops.cpp workers.cpp vec.cpp voxel.cpp avx.cpp vec8.cpp

all:
	$(gxx) -O3 $(src) -o sim
debug:
	$(gxx) -O0 -DDEBUG $(src) -o sim


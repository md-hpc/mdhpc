gxx := g++ -g
src := main.cpp offset.cpp particle.cpp simulation.cpp simops.cpp workers.cpp vec.cpp voxel.cpp

all:
	$(gxx) -O3 $(src) -o sim
debug:
	$(gxx) -O0 $(src) -o sim


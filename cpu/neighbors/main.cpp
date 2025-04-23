#include "simulation.h"

int main(int argc, char **argv) {
	Simulation s = Simulation(argc, argv);
	s.simulate();
	return 0;
}

#include "simulation.h"

int main(int argc, char **argv) {
	// maybe object oriented a bit too hard here lol
	simulation s = simulation(argc, argv);
	s.simulate();
	return 0;
}

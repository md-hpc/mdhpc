
src = common.cpp particle.cpp vec8.cpp workers.cpp simulation.cpp simops.cpp avx.cpp

gxx = g++ -march=native -lm -lpthread -g

all:
	$(gxx) -o sim -O3 md.cpp $(src)
debug: 
	$(gxx) -DDEBUG -O0 -o sim md.cpp $(src)
test:
	$(gxx) -o tst test.cpp $(src)


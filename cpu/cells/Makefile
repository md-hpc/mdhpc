gxx := g++ -g -lm -lpthread
bins= cells-thread.cpp common.cpp

all:
	$(gxx) -O3 $(bins) -o sim
debug:
	$(gxx) -O0 -DDEBUG $(bins) -o sim

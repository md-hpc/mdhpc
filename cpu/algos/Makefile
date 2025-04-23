
ROOT_DIR := /home/vraiti/mdhpc/cpu/cpp
gxx := g++ -O3 -lm -lpthread common.cpp 
bin = cells-thread list-thread

all: stamp-all build

debug: gxx += -g -DDEBUG 
debug: stamp-debug build

stamp-debug:
	-rm stamp-all $(bin)
	touch stamp-debug
stamp-all:
	-rm stamp-debug $(bin)
	touch stamp-all

clean:
	-rm $(bin) test

build: $(bin)

%: %.cpp $(common)
	$(gxx) $@.cpp -o $@

.PHONY: neighbor-filters
neighbor-filters: 
	$(MAKE) -C $@ 

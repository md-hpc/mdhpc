import time
import os

bins = [
    "cells-thread",
    "hemisphere-thread",
    "list-thread",
    "hemilist-thread",
]

for N in range(3,24):
    fp = open("benchmark.out","a")
    print(N, end=' ', file=fp)
    
    argv = f"--timesteps 17 --universe-size {N}"
    for f in bins:
        start = time.time()
        os.system(f"./{f} {argv}")
        stop = time.time()
        print(stop - start, end=' ', file=fp)
    
    print(file=fp)

    fp.close()

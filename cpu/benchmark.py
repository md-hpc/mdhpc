import time
import os
from sys import argv


fp = open('benchmark.csv','w')
for u in range(3,15):
    start = time.time()
    os.system(f"{argv[1]} --universe-size {u} --timesteps 5")
    stop = time.time()
    print(u,(stop-start)/(5*80*u**3))

import subprocess
import time

fp = open("benchmark.csv","w")
print("universe size, time", file=fp)
fp.close()

for u in range(3,121):
    print(u,"cells")
    cmd = f"sim --universe-size {u} --timesteps 5"
    start = time.time()
    subprocess.run(cmd.split(' '))
    stop = time.time()

    fp = open("benchmark.csv","a")
    print(f"{u} {stop - start}", file=fp)
    fp.close()

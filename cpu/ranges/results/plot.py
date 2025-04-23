import csvp
import matplotlib.pyplot as plt

for u in [3, 4, 5]:
    x, y1 = csvp.parse(f"energy-t1-u{u}.csv",dtype=float,hdr=False)
    x, y2 = csvp.parse(f"energy-t16-u{u}.csv",dtype=float,hdr=False)
    y = (y2 / y1) - 1

    plt.scatter(
        x,
        y,
        s = 4,
        label=f"{u**3} cells"
    )

plt.title("Ostrich Algorithm 16 Threads % Error")
plt.xlabel("timestep")
plt.ylabel("% error")
plt.legend()
plt.savefig("ostrich")

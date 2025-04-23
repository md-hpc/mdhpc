import matplotlib.pyplot as plt
import csvp

hdr, x, y = csvp.parse("benchmark.csv")

y *= 1e6

x = (18 * x**3)[:,0]

x_cell = 80 * x
x_neighbor = 138 * x

t_cell = y[:,0]
t_neighbor = y[:,2]

plt.scatter(
    x_cell,
    t_cell / x_cell,
    label="Cell lists"
)

plt.scatter(
    x_neighbor,
    t_neighbor / x_neighbor,
    label="Neighbor lists",
)

plt.title("SIMD Methods for RLNB MD")
plt.xlabel("Particles")
plt.ylabel("usec per Particle")

plt.savefig("simd-methods")

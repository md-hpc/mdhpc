from mpl_toolkits.mplot3d import Axes3D
from matplotlib import animation
import matplotlib.pyplot as plt
import numpy as np

from sys import argv

if len(argv) != 3:
    print(f"usage: {argv[0]} [path] [L]")
    exit(1)

L = float(argv[2])


def will_it_float(tok):
    try:
        x = float(tok)
        return True
    except:
        return False

timesteps = [[]]

for i, line in enumerate(open(argv[1])):
    if i == 0 and any([not will_it_float(tok) for tok in line.split(',')]):
        continue
    if (len(line) == 1):
        timesteps.append([])
        continue
    line = [d.strip() for d in line.split(',')]
    timesteps[-1].append([float(tok) for tok in line[1:4]])

if (len(timesteps[-1]) == 0):
    timesteps = timesteps[:-1]

timesteps = [np.stack(t) for t in timesteps]


fig = plt.figure()
ax = fig.add_subplot(projection='3d')

def update(t, ax):
    print(f"Animating {t}")
    ax.clear()
    xs, ys, zs = np.split(timesteps[t],3,1)

    ax.set_xlim3d([0.0, L])
    ax.set_xlabel('X')

    ax.set_ylim3d([0.0, L])
    ax.set_ylabel('Y')

    ax.set_zlim3d([0.0, L])
    ax.set_zlabel('Z')
    ax.scatter(xs, ys, zs, c="r")

# Setting the axes properties
ani = animation.FuncAnimation(fig, update, len(timesteps), fargs=(ax,), interval=10, repeat_delay=5000, blit=False)
ani.save('md.gif', writer='imagemagick')
plt.show()

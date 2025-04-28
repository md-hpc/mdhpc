# Software Report

## CPU

### Prerequisites

* gcc 12.0
* Fedora 40
* A CPU with AVX2 support

### Installation

Each subdirectory contains its own Makefile that will manage compilation of the specific binaries. Simply invoke `make` in the target directory, and it will produce a `sim` simulation binary accepting all the standard command-line arguments (as detailed in the User's Manual).

### Cell Lists (cells)

The code is split arbitrarily between `common.cpp` and `cells-thread.cpp` (this is an artifact from when we had tried to have a "common" code that was reused between all of the CPU implementations. It turned out to be more trouble than it was worth). This whole implementation is wrought with poor design choices, but it is also algorithmically the worst out of all the O(n) algorithms we implement, so no futher optimizations were made after the working implementation was completed

### Neighbor Lists (neighbors)

The main objects in the code are given in the header `simulation.h`, which declares the `Simulation` class that contains all simulation constants and data and any methods that operate on those. Three source files, `simulation.cpp`, `simops.cpp`, and `workers.cpp` together implement `Simulation`. They're organized as follows:
* `simulation.cpp`: Functions that involve simulation setup and management such as command line parsing, thread supervision, and simulation state logging
* `simops.cpp`: Computational functions that use simulation constants, such as intermolecular forces and distance metrics
* `workers.cpp`: High-level algorithmic functions invoked by the worker threads such as force computation, position update, and particle-core migration

Several other files implement object and functions that are used by `Simulation`, but that are not strictly dependent upon it:
* `vec.cpp`: the `vec` class, which allows (x,y,z) tuples to be operated on with standard arithmetic operations (+,-,\*,/)
* `offset.cpp`: a simple object that allows voxel neighborhoods to be iterated over without a triple-nested for-loop.

`main.cpp` is a short file that instantiates a `Simulation` and invokes its main `simulate` method.

### SIMD Cell Lists (simd)

This implementation has the same overall structure as Neighbor Lists, but with the addition of two files implementing SIMD-accelearted computation:

* `avx.cpp`: defines vector data types and a few simple vector functions (such as vector square root and a maximum).
* `vec8.cpp`: defines the `vec8` class, which operates similarly to the `vec` class above, but each coordinate is instead a SIMD vector element, allowing this class to leverage the SIMD capabilities of the target CPU. It also implements the (somewhat) peculiar classes v8buf and vec8\_vector.

### SIMD Neighbor Lists (simd-n)

The main objects in the code are given in the header `simulation.h`, which declares the `Simulation` class that contains all simulation constants and data and any methods that operate on those. Three source files, `simulation.cpp`, `simops.cpp`, and `workers.cpp` together implement `Simulation`. They're organized as follows:
* `simulation.cpp`: Functions that involve simulation setup and management such as command line parsing, thread supervision, and simulation state logging
* `simops.cpp`: Computational functions that use simulation constants, such as intermolecular forces and distance metrics
* `workers.cpp`: High-level algorithmic functions invoked by the worker threads such as force computation, position update, and particle-core migration

Several other files implement object and functions that are used by `Simulation`, but that are not strictly dependent upon it:
* `vec.cpp`: the `vec` class, which allows (x,y,z) tuples to be operated on with standard arithmetic operations (+,-,\*,/)
* `offset.cpp`: a simple object that allows voxel neighborhoods to be iterated over without a triple-nested for-loop.
* `avx.cpp`: defines vector data types and a few simple vector functions (such as vector square root and a maximum).
* `vec8.cpp`: defines the `vec8` class, which operates similarly to the `vec` class above, but each coordinate is instead a SIMD vector element, allowing this class to leverage the SIMD capabilities of the target CPU.

`main.cpp` is a short file that instantiates a `Simulation` and invokes its main `simulate` method.


## GPU


### Prerequisites
* CUDA 12.5
* NVIDIA GPU
* python3
    * matplotlib
    * numpy

### Flowchart
<img width="80%" alt="Screenshot_2025-04-27_at_9 36 56_PM" src="https://github.com/user-attachments/assets/431cb8a9-2234-4d71-b6f5-1dd183b1a5e7" />

### Installation

#### Generating testing PDB files
In the `src` directory, there is a python program called `generate_pdb.py`. Run this file with the given arguments to generate a pdb file of random particle positions: `-o output file name`, `-n number of particles`, `-x number of cells in the x direction`, same for `-y` and `-z`, `-c cell cutoff radius`
#### Generating scripts for running batch jobs
`script_generator.sh` generates scripts used to submit non-interactive batch jobs through qsub on the Boston University Shared Computing Cluster (SCC). The parameters `PROJ_DIR`, `SCRIPTS_DIR`, `TIMESTEPS`, `TIMESTEP_DURATION`, `TYPE` can be changed easily depending on what the user wants to be simulated. Additionally, when the user wants to generate new scripts, the previous ones are replaced.   
#### Generating executable for interactive use
For each implementation (nsquared, nsquared shared, nsquared n3l, cell list, cell list n3l), simply invoke the Makefile within the corresponding implementation to build the binary. For example to generate the executable for the naive implementation use the following command: make nsquared. Depending on attributes of the PDB file, the user may need to modify the variables in the Makefile named `NSQUARED_FLAGS` and `CELL_LIST_FLAGS`.
Running the executable for each implementation
The executables can be run by running `batch_job.sh` or through an interactive GPU job. `batch_job.sh` is also generated by `script_generator.sh`. If you are running executables on an interactive GPU, invoke it like so: `<implementation> <input pdb file> <output csv file>`.
#### Generating visualizations of the simulations
 To generate a gif of the simulation, run `src/viz.py <axis length> <input csv file> <output gif file>`.




## FPGA

Molecular dynamics (MD) simulates particle-level interactions in physical systems using Newtonian mechanics over discrete timesteps, offering deep insight into biological structures and behaviors. While MD maps well to parallel architectures, the dense short-range interactions, such as the Lennard-Jones potential, make efficient hardware implementation challenging. In this project, we optimize MD for FPGA by designing a parameterizable, ring-connected architecture that balances performance, resource usage, and routing simplicity.


### Prerequisites

* Vitis 2023.1
* Network Layer build file
* Memory Mapped build files
* Source files


### Installation




To build the simulator, run ```make```. Please give up to 24 hours as the build time is quite significant to match timing and minimize wire congestion and resource usage.

### Runtime

To run the simulator, run ```python run.py```.


### Visualization

To visualize the data, run ```viz.py```.

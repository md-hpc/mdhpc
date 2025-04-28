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

All files are available within the ```FPGA``` directory.


### Implementation

To implement the simulator on the FPGA, numerous challenges were faced and tackled by implementing unique solutions. The main challenges are summarized within the following Verilog modules:

* `PositionRingNode.v`: This module is responsible for several key tasks. First, it iterates through all the particles in the BRAM it is connected to consecutively, such that at the next iteration a different particle is chosen as the neighbor particle and is transmitted to all other nodes while the remainin particles are considered as reference particles for that iteration. The neighbor particle along with 13 neighbor particles from other ring nodes (a total of 14 particles) are sent along the pipeline along with the reference particles. To minimize BRAM usage and wiring, only 1 of the 14 pair of neighbor/reference particles are processed by the pipeline at a time. While this results in a performance cost, it results in flexibility when implementing on FPGAs with very limited resources.
* `simulator.v`: This module serves as the connection system of the simulator. It processes the control signals out of all the modules and selects the correct wires corresponding to the BRAM inputs depending on which phase is being processed. It also handles BRAM intialization from the host PC.
* `CellIndex.v`: This module resulted in one of the more elusive bugs in the system. From the tests we ran, it seemed clear that the floating point to integer conversion IP provided by Xilinx would be analogous to a floor operation (for example, 2.3 or 2.84 would both be considered 2). However, the IP actually rounded at the midpoint (for example, 2.3 and 2.84 would be considered 2 and 3, respectivley). After discovering this issue, we implemented a floating point floor operation to remove any possibility of rounding up before proceessing a number and converting it to an integer.
* `LJ.v`: This module was tricky to implement efficiently, but some optimizations made managed to greatly reduce resources utilization. For example, all constants were precomputed instead of being unecessarily computed at runtime. Even constants such as dT is multiplied to the constants, reducing three multiplier modules for each pipeline.


Other key optimizations include minimizing complex operations. For example, a division module results in about 3 times the amount of Look-Up Tables (LUTs) used per module when compared to multiplication. Therefore, we replaced constant division with precalculated reciprocal multiplication. For example, instead of performing x/5, we perform x*0.2, resulting in huge resource savings. This is shown in `fp32_mod_const.v` were a/b is implemented as a * b_rec, where b_rec is provided.

For the current implementation, which is optimized for a universe size of 3x3x3, we utiizes about 25% of the BRAMs, 40% of the DSPs and 80% of LUTs.

### Prerequisites

* Vitis 2023.1
* Network Layer build file
* Memory Mapped build files
* Source files


### Installation

Please note that initiale position and velocity values for the particles must be set in the BRAM_INIT.txt file in hexadecimal format, containing the position followed by velocity (each being a set of 3 8-character long strings), and then 2 characters for BRAM selection, and finally 3 characters for the BRAM address.

To build the simulator, run ```make```. Please give up to 24 hours as the build time is quite significant to match timing and minimize wire congestion and resource usage.

### Runtime

To run the simulator, run ```python run.py```.


### Visualization

To visualize the data, run ```viz.py```.

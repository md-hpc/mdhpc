# MDHPC

MDHPC is a project that explores high-performance methods for computing the range-limited non-bonded forces for Molecular Dynamics (MD) simulations. Our project creates implementations for three high-performance hardware platforms: CPU, GPU, and FPGA. Each was developed independently (as high performance code must often be), with only some high-level theoretical ideas shared between the three, so each has its own structure and setup.

## CPU

Having the most flexiblitiy out of the three platforms, this subproject contains several implementations using a number of different approaches to test efficacy. There are three algorithms: naive (no cutoff radius, O(n^2) complexity), cell lists, and neighbor lists. For each of these we implemented both a scalar and SIMD code. The following subdirectories correspond to the following implementations:
* `cells`: Cell lists, no SIMD
* `neighbors`: Neighbor lists, no SIMD
* `simd`: Cell lists, SIMD
* `simd-n`: Neighbor lists, SIMD

Some important things to look out for when programming CPU:

__SIMD Vector consolidating operations__: This happens when the data your SIMD operations are using are not stored contiguously in memory (e.g., it's a linked list), and it is horrible for performance. CPUs have the capability to load an entire SIMD vector's data in a single instruction if the data is aligned and contiguous, and not letting it do that will cause your memory performance to take an ~8x hit.

__Cache coherence misses__: this happens when multiple threads are writing to the same address or same cache block. Doing so will immediately cause that block to be invalidated in all other L2 and L1 caches, causing those secitons to only achieve L3-levels of memory performance.

__Thread affinity__: Linux threads are suitable for high-performance environments, but only if you use proper thread affinity. Otherwise, a thread may be migrated to another core, invalidating all of its L1 and L2 cache. Using pthread_setaffinity_np, you can avoid this problem.

## FPGA

This subproject implements a molecular dynamics simulator specifically optimized for FPGA acceleration, focusing on efficient resource usage and scalability. Particle positions and velocities are distributed across Block RAMs, which are interconnected using a ring topology to minimize wiring congestion. Instead of a complex all-to-all network, each node connects only to its immediate neighbors, greatly simplifying the design. The simulator is fully parameterizable, allowing users to adjust the number of compute pipelines to best fit the available resources on a given FPGA.

Some important things to look out for when programming FPGA:

__Routing congestion__: An all-to-all connection between memory banks quickly becomes unsustainable as the design scales. The ring architecture reduces fanout and keeps the interconnect simple, which is critical for large numbers of pipelines.

__Timing closure__: As pipeline count increases, maintaining high clock frequencies becomes more challenging. Deep pipelining and careful placement of interconnects are necessary to meet timing.

__Resource balancing__: FPGAs have limited and fixed ratios of logic cells, BRAMs, and DSP slices. Over-parameterizing the design may cause one type of resource to become a bottleneck, so tuning the number of pipelines is important for efficient utilization.


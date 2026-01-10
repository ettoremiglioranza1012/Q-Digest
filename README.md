# Q-Digest Parallel Implementation

High-performance C implementations of the Q-Digest quantile summary used for approximate range queries. The project provides:

- A reference serial core, plus OpenMP and MPI variants for shared-memory and distributed-memory execution.
- Supporting utilities for dataset generation, benchmarking scripts, and reproducibility artifacts.
- Doxygen documentation.

## What you can do here

- Build the core library and run unit/regression tests for correctness.
- Compare serial vs OpenMP vs MPI performance by generating your own datasets and benchmark scripts.
- Regenerate datasets and documentation to extend or validate the original experiments.

## Build & test quickstart

Run `make help` to see all available targets. Common ones:

1. `make clean` – remove `lib`, `bin`, and `build`.
2. `make library` – build the serial/static library.
3. `make mpi` – build the MPI parallel program.
4. `make docs` – generate Doxygen HTML in `docs/doxygen`.
5. `make serial-test-core` – build serial core regression suite (`bin/serial-test_core`).
6. `make serial-test-all` – build the comprehensive serial test driver (`bin/serial-test_all`).
7. `make serial-test-queue` – build the standalone queue test (`bin/serial-test_queue`).
8. `make serial-test-serialization` – build the serialization-focused test binary.
9. `make serial-run-local-test` – run the serial core test with `mpirun -n 1` (needs target 5).

## Documentation

Documentation is available online through GH pages and is compiled automatically through GH actions whenever a
a new push or merge on the `main` branch is carried out.

## Directory guide

```text
.
├─ README.md                  # Project overview (this file)
├─ Makefile                   # Top-level build targets
├─ include/                   # Public headers (core data structures and utils)
├─ src/                       # Serial implementation and dataset generator
│  └─ dataset-generator/      # Scripts to create synthetic datasets
├─ mpi-implementation/        # MPI version (sources, headers, build artifacts)
├─ omp-implementation/        # OpenMP version (sources, headers, build rules)
├─ serial-implementation/     # Serial tests, binaries, and submission scripts
├─ results/                   # Benchmark outputs grouped by run and CPU count
├─ docs/                      # Doxygen config and generated artifacts
├─ run_benchmark.sh           # Helper script to execute performance suites and submit jobs on cluster
├─ submit_cluster.sh          # Example batch submission script
├─ Doxyfile                   # Documentation configuration
└─ LICENSE.md                 # License info
```

## Notes

- MPI and OpenMP builds expect appropriate toolchains and `mpirun` available on your system.
- Benchmark directories contain captured runs; can be regenerate via `run_benchmark.sh` when implementation is changed.

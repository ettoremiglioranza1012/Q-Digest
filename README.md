# Q-Digest Parallel Implementation

This is a test `README` for the HPC project.

## Building the project

Run `make help` to check all the options available to build the
project.

**DISCLAIMER**: not all make functionalities have been tested.
Those known to be working are:

1. `make help`
2. `make clean` -> removes the `lib`, `bin`, and `build` directories
3. `make library` -> builds the library
4. `make mpi` -> builds the MPI parallel program
5. `make docs` -> builds the documentation in docs/doxygen.
6. `make serial-test-core` -> builds the serial qcore regression suite in `bin/serial-test_core`
7. `make serial-test-all` -> builds the comprehensive serial test driver in `bin/serial-test_all`
8. `make serial-test-queue` -> builds the standalone queue test in `bin/serial-test_queue`
9. `make serial-test-serialization` -> builds the serialization-focused test binary
10. `make serial-run-local-test` -> runs the serial core test with `mpirun -n 1` (depends on target 6)

## Docs

The quickest way to build the docs and explore them is to run
`make docs` and then open the `docs/doxygen/html/index.html` file
in a web browser to explore the documentation interactively.

![WARNING]

> Make sure to include the directories that should be included for documentation
> purposes as arguments to the `INCLUDE` field inside the `Doxyfile`.

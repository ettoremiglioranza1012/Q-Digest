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

## Docs

The quickest way to build the docs and explore them is to run
`make docs` and then open the `docs/doxygen/html/index.html` file 
in a web browser to explore the documentation interactively.

#!/bin/bash

#PBS -l select=10:ncpus=10:mem=10gb
#PBS -l walltime=00:10:00
#PBS -q short_cpuQ

cd $PBS_O_WORKDIR

module load mpich-3.2

mpirun.actual -n 1 ./bin/main

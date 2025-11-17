#!/bin/bash
#PBS -l select=1:ncpus=1:mem=8gb
#PBS -l walltime=00:01:00
#PBS -q short_cpuQ

cd $PBS_O_WORKDIR

module load mpich-3.2
mpirun.actual -n 1 ./bin/test_core

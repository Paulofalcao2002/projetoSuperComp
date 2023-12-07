#!/bin/bash
#SBATCH --ntasks=4
#SBATCH --cpus-per-task=2
#SBATCH --partition=normal
#SBATCH --job-name=distribuido-50-vertices

# Executa o código MPI
mpirun -np 4 ./forca-bruta-memoizado-distribuido
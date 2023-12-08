#!/bin/bash
#SBATCH --ntasks=2
#SBATCH --cpus-per-task=4
#SBATCH --partition=normal
#SBATCH --job-name=distribuido-2-tasks-50-vertices

# Executa o código MPI
mpirun -np 2 ./forca-bruta-recursivo-distribuido
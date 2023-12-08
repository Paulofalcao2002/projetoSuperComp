#!/bin/bash
#SBATCH --ntasks=8
#SBATCH --cpus-per-task=1
#SBATCH --partition=normal
#SBATCH --job-name=distribuido-2-tasks-50-vertices

# Executa o código MPI
mpirun -np 8 ./forca-bruta-recursivo-distribuido
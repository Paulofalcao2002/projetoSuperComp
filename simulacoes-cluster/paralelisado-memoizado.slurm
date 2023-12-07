#!/bin/bash
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=3
#SBATCH --partition=normal
#SBATCH --job-name=paralelizado-memoizado-50-vertices

# Executa o código MPI
./forca-bruta-memoizado-paralelisado
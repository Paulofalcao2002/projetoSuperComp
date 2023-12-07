#!/bin/bash
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --partition=normal
#SBATCH --job-name=paralelizado-50-vertices

# Executa o código MPI
./forca-bruta-recursivo-paralelisado
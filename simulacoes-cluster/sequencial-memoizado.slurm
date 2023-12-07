#!/bin/bash
#SBATCH --ntasks=1
#SBATCH --partition=normal
#SBATCH --job-name=sequencial-50-vertices

# Executa o código MPI
./forca-bruta-memoizado
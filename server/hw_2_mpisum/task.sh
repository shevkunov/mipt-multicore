#!/bin/sh

# taskname (optional)
#SBATCH --job-name="mpisum"

# maxtime
#SBATCH --time=00:10:00

# ask 1 node
#SBATCH --nodes=1

# ask for the approve for using cores on nodes
#SBATCH --ntasks-per-node=2

# run for your live!

mpiexec -np 2 ./a.out


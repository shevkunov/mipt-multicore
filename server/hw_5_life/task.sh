#!/bin/sh

# taskname (optional)
#SBATCH --job-name="mpisum"

# maxtime
#SBATCH --time=00:10:00

# ask 1 node
#SBATCH --nodes=1

# ask for the approve for using cores on nodes
#SBATCH --ntasks-per-node=12

# run for your live!

# mpicc main.c -std="c99" -O2

make clean
make 
rm times.txt

for ((i = 1; i <= 12; ++i))
do
    for ((j = 0; j < 3; ++j))
    do		
        ./a.out 1000 $i
    done
done

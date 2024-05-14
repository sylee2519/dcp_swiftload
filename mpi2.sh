#!/bin/sh
#PBS -V
#PBS -N prism
#PBS -q exclusive
#PBS -A etc
#PBS -l select=2:ncpus=68:mpiprocs=24:ompthreads=1 
#PBS -l walltime=04:00:00
#PBS -m abe
#PBS -M sylee2519@naver.com
#PBS -W sandbox=PRIVATE

cd $PBS_O_WORKDIR
module purge
module load craype-x86-skylake gcc/8.3.0 openmpi/3.1.0
module load forge/18.1.2
module load cmake/3.17.4

mpirun -np 24 dcp /home01/sample_data/nurion_stripe/500mb /home01/sample_data/nurion_stripe/500mb2 1>stdout 2>stderr

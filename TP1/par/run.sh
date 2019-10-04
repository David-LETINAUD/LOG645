make all

echo ''
echo '>>> 1 8 12'
mpirun -np 64 ./lab1 1 8 12
echo '<<<'

echo '>>> 1 4 5'
mpirun -np 64 ./lab1 1 4 5
echo '<<<'

echo '>>> 1 13 22'
mpirun -np 64 ./lab1 1 13 22
echo '<<<'

echo '>>> 2 0 5'
mpirun -np 64 ./lab1 2 0 5
echo '<<<'

echo '>>> 2 2 4'
mpirun -np 64 ./lab1 2 2 4
echo '<<<'

echo '>>> 2 7 3'
mpirun -np 64 ./lab1 2 7 3
echo '<<<'

make clean

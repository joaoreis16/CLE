### How to compile and run

```bash
mpicc -Wall -o prog2 main.c sortInt.c

mpiexec -n 4 ./prog2 dataset/datSeq32.bin
```
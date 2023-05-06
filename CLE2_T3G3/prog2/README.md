### How to compile and run

```bash
mpicc -Wall -o prog2 main.c sortInt.c

# running with 4 workers
mpiexec -n 5 ./prog2 dataset/datSeq32.bin
```
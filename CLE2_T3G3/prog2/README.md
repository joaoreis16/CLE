### How to compile and run

```bash
mpicc -Wall -o prog2 countWords.c main.c

mpiexec -n 4 ./prog2 -f dataset/datSeq1M.bin
```
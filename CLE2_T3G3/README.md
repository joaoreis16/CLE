### How to compile and run

```bash
mpicc -Wall -o prog1 countWords.c main.c

mpiexec -n 4 ./prog1 -f dataset/text0.txt
```
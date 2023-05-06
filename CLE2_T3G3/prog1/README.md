### How to compile and run

```bash
mpicc -Wall -o prog1 countWords.c main.c

# running with 4 workers
mpiexec -n 5 ./prog1 -f dataset/text0.txt -f dataset/text1.txt -f dataset/text2.txt -f dataset/text3.txt -f dataset/text4.txt
```
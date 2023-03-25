### How to compile and run

```bash
gcc -o prog1 main.c shared.c countWords.c

# with 4 workers (default) and 4k per chunk (default) 
./prog1 -f dataset/text0.txt -f dataset/text1.txt -f dataset/text2.txt -f dataset/text3.txt -f dataset/text4.txt

# with 8 workers and 4k per chunk (default)
./prog1 -f dataset/text0.txt -f dataset/text1.txt -f dataset/text2.txt -f dataset/text3.txt -f dataset/text4.txt -n 8

# with 4 workers (default) and 8k per chunk
./prog1 -f dataset/text0.txt -f dataset/text1.txt -f dataset/text2.txt -f dataset/text3.txt -f dataset/text4.txt -m 8
```
all: sodoku-solve

sodoku-solve: sodoku.o main.o
	gcc -o sodoku-solve sodoku.o main.o

sodoku.o: sodoku/sodoku.c
	gcc -c -o sodoku.o sodoku/sodoku.c

main.o: sodoku/main.c
	gcc -c -o main.o sodoku/main.c

clean:
	rm -f *.o

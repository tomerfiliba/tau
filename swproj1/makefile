all: authentication

clean:
	-rm main.o md5.o authentication

authentication: main.o md5.o 
	gcc -ansi -pedantic-errors -g -lm -o authentication main.o md5.o 

main.o: main.c md5.h 
	gcc -ansi -pedantic-errors -c -Wall -g main.c

md5.o: md5.c md5.h
	gcc -ansi -pedantic-errors -c -Wall -g md5.c


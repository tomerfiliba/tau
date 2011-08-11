all: create_rainbow_table crack_using_rainbow_table 

clean:
	-rm *.o create_rainbow_table crack_using_rainbow_table

md5.o: md5.c md5.h
	gcc -std=c99 -pedantic-errors -c -Wall -Werror -g md5.c

sha1.o: sha1.c sha1.h
	gcc -std=c99 -pedantic-errors -c -Wall -Werror -g sha1.c

misc.o: misc.c md5.o sha1.o
	gcc -std=c99 -pedantic-errors -c -Wall -Werror -g misc.c

deht.o: misc.o deht.c
	gcc -std=c99 -pedantic-errors -c -Wall -Werror -g deht.c

iniloader.o: iniloader.c
	gcc -std=c99 -pedantic-errors -c -Wall -Werror -g iniloader.c

rules.o: misc.o rules.c iniloader.o
	gcc -std=c99 -pedantic-errors -c -Wall -Werror -g rules.c

create_rainbow_table.o: deht.o misc.o create_rainbow_table.c iniloader.o rules.o
	gcc -std=c99 -pedantic-errors -c -Wall -Werror -g create_rainbow_table.c

crack_using_rainbow_table.o: deht.o misc.o crack_using_rainbow_table.c iniloader.o
	gcc -std=c99 -pedantic-errors -c -Wall -Werror -g crack_using_rainbow_table.c

create_rainbow_table: create_rainbow_table.o 
	gcc -pedantic-errors -Wall -Werror -g -lm -o create_rainbow_table \
	deht.o iniloader.o misc.o md5.o sha1.o create_rainbow_table.o rules.o

crack_using_rainbow_table: crack_using_rainbow_table.o
	gcc -pedantic-errors -Wall -Werror -g -lm -o crack_using_rainbow_table \
	deht.o iniloader.o misc.o md5.o sha1.o crack_using_rainbow_table.o

test-rules: rules.o
	gcc -pedantic-errors -Wall -Werror -g -lm -o test-rules \
	iniloader.o misc.o md5.o sha1.o rules.o

test: clean all
	-rm *.key *.data


all: create_rainbow_table crack_using_rainbow_table 

clean:
	-rm *.o create_rainbow_table crack_using_rainbow_table create_rainbow_table-debug

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

rainbow.o: misc.o rainbow.c rules.o deht.o
	gcc -std=c99 -pedantic-errors -c -Wall -Werror -g rainbow.c

create_rainbow_table.o: deht.o misc.o create_rainbow_table.c iniloader.o rules.o rainbow.o
	gcc -std=c99 -pedantic-errors -c -Wall -Werror -g create_rainbow_table.c

create_rainbow_table-debug.o: deht.o misc.o create_rainbow_table.c iniloader.o rules.o rainbow.o
	gcc -std=c99 -pedantic-errors -c -Wall -g -o create_rainbow_table-debug.o create_rainbow_table.c -DSHOW_GENERATED_CHAINS

crack_using_rainbow_table.o: deht.o misc.o crack_using_rainbow_table.c iniloader.o rainbow.o rules.o
	gcc -std=c99 -pedantic-errors -c -Wall -Werror -g crack_using_rainbow_table.c

create_rainbow_table: create_rainbow_table.o 
	gcc -pedantic-errors -Wall -Werror -g -lm -o create_rainbow_table \
	deht.o iniloader.o misc.o md5.o sha1.o create_rainbow_table.o rules.o rainbow.o

create_rainbow_table-debug: create_rainbow_table-debug.o 
	gcc -pedantic-errors -Wall -Werror -g -lm -o create_rainbow_table-debug \
	deht.o iniloader.o misc.o md5.o sha1.o create_rainbow_table-debug.o rules.o rainbow.o

crack_using_rainbow_table: crack_using_rainbow_table.o
	gcc -pedantic-errors -Wall -Werror -g -lm -o crack_using_rainbow_table \
	deht.o iniloader.o misc.o md5.o sha1.o crack_using_rainbow_table.o rainbow.o rules.o

debug: clean create_rainbow_table-debug crack_using_rainbow_table
	-rm *.key *.data *.seed


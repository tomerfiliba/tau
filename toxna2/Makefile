all: exhaustive_table_generator exhaustive_query 

clean:
	-rm deht.o md5.o sha1.o misc.o \
	exhaustive_table_generator.o exhaustive_table_generator exhaustive_query \
	exhaustive_query.o rules.o

md5.o: md5.c md5.h
	gcc -ansi -pedantic-errors -c -Wall -Werror -g md5.c

sha1.o: sha1.c sha1.h
	gcc -ansi -pedantic-errors -c -Wall -Werror -g sha1.c

misc.o: misc.c md5.o sha1.o
	gcc -ansi -pedantic-errors -c -Wall -Werror -g misc.c

deht.o: misc.o deht.c
	gcc -ansi -pedantic-errors -c -Wall -Werror -g deht.c

rules.o: misc.o rules.c
	gcc -ansi -pedantic-errors -c -Wall -Werror -g rules.c

exhaustive_table_generator.o: deht.o misc.o rules.o exhaustive_table_generator.c
	gcc -ansi -pedantic-errors -c -Wall -Werror -g exhaustive_table_generator.c

exhaustive_query.o: deht.o misc.o exhaustive_query.c
	gcc -ansi -pedantic-errors -c -Wall -Werror -g exhaustive_query.c

exhaustive_table_generator: exhaustive_table_generator.o 
	gcc -pedantic-errors -Wall -Werror -g -lm -o exhaustive_table_generator exhaustive_table_generator.o deht.o misc.o md5.o sha1.o rules.o

exhaustive_query: exhaustive_query.o 
	gcc -pedantic-errors -Wall -Werror -g -lm -o exhaustive_query exhaustive_query.o deht.o misc.o md5.o sha1.o rules.o


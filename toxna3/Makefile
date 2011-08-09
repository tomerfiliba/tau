all: create_rainbow_table crack_using_rainbow_table 

clean:
	-rm *.o create_rainbow_table crack_using_rainbow_table

md5.o: md5.c md5.h
	gcc -ansi -pedantic-errors -c -Wall -Werror -g md5.c

sha1.o: sha1.c sha1.h
	gcc -ansi -pedantic-errors -c -Wall -Werror -g sha1.c

#misc.o: misc.c md5.o sha1.o
#	gcc -ansi -pedantic-errors -c -Wall -Werror -g misc.c

#deht.o: misc.o deht.c
#	gcc -ansi -pedantic-errors -c -Wall -Werror -g deht.c

#exhaustive_table_generator.o: deht.o misc.o cracking_pass.o exhaustive_table_generator.c
#	gcc -ansi -pedantic-errors -c -Wall -Werror -g exhaustive_table_generator.c

#exhaustive_query.o: deht.o misc.o cracking_pass.o exhaustive_query.c
#	gcc -ansi -pedantic-errors -c -Wall -Werror -g exhaustive_query.c

create_rainbow_table: 
	gcc -ansi -pedantic-errors -Wall -Werror -g -lm -o exhaustive_table_generator exhaustive_table_generator.o deht.o cracking_pass.o misc.o md5.o sha1.o

crack_using_rainbow_table: 
	gcc -ansi -pedantic-errors -Wall -Werror -g -lm -o create_rainbow_table exhaustive_query.o deht.o cracking_pass.o misc.o md5.o sha1.o

CC=gcc

mdu: mdu.o stacks.o
	$(CC) -lm -pthread -o mdu stacks.o mdu.o

mdu.o: mdu.c mdu.h
	$(CC) -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition -c mdu.c
	
stacks.o: stacks.c stacks.h
	$(CC) -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition -c stacks.c

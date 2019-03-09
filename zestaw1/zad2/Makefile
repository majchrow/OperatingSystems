CC = gcc
CFLAGS = -Wall -std=c99 -ggdb
CNAME = library
LIBNAME = searchlib

all: static shared clear

static:
	$(CC) $(CFLAGS) main.c -o test_static -L./Libs ./Libs/libsearchlib.a

shared:
	$(CC) $(CFLAGS) main.c -o test_shared -L./Libs -lsearchlib -Wl,-rpath=./Libs

clear:
	rm -f *.o
	#static
	./test_static
	#shared
	./test_shared
	rm test_*

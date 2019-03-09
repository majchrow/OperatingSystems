CC = gcc
CFLAGS = -Wall -std=c99 -ggdb
CNAME = library
LIBNAME = searchlib

all: static shared clean

static: 
	$(CC) $(CFLAGS) -c $(CNAME).c
	ar crs lib$(LIBNAME).a $(CNAME).o

shared: 
	$(CC) $(CFLAGS) -c -fPIC library.c 
	$(CC) $(CFLAGS) $(CNAME).o -shared -fPIC -o lib$(LIBNAME).so 

clean:
	rm -f *.o

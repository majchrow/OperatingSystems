CC = gcc
CFLAGS = -Wall -std=c99 -ggdb
CNAME = library
LIBNAME = searchlib
FOLDERNAME = Libs

all: compile tests clean

compile:
	$(CC) $(CFLAGS) main.c -o test -L./$(FOLDERNAME) -l$(LIBNAME) -Wl,-rpath=./$(FOLDERNAME)

tests:
	./test --create 10 --dir /bin --searchf ss --temporaryf tmp.txt -f >> require2.txt
clean:
	rm -f *.o


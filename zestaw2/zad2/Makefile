CC = gcc
CFLAGS = -Wall -std=c11
MAIN = main.c
OUT = main.out

all: compile tests clean

compile:
	$(CC) $(CFLAGS) $(MAIN) -o $(OUT)

tests:
	echo 'Test using dirent.h\n\n' >> komentarz.txt
	./$(OUT) '-dir' './test' '>' '2019-01-01 00:00:00' >> komentarz.txt
	./$(OUT) '-dir' `pwd`/test '<' '2019-04-01 00:00:00' >> komentarz.txt
	echo '\nTest using ftw.h\n\n' >> komentarz.txt
	./$(OUT) '-nftw' './test' '>' '2019-01-01 00:00:00' >> komentarz.txt
	./$(OUT) '-nftw' `pwd`/test '<' '2019-04-01 00:00:00' >> komentarz.txt
	echo '\nKomenatrz: Obie metody daja te same rezultaty\n' >> komentarz.txt
	
clean:
	rm $(OUT)

	


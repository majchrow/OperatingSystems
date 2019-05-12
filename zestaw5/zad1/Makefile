#Plik makefile prawdopodobnie nie zadziała, coś nie działa do końca w mainie, ale jeszcze nie wiem co dokładnie

CC = gcc
CFLAGS = -Wall
MAIN = main.c
MAIN_OUT = main.out
FILE = file.txt

all: compile tests clean

compile:
	$(CC) $(CFLAGS) $(MAIN) -o $(MAIN_OUT)

tests:	
	./$(MAIN_OUT) $(FILE)
	
clean:
	rm  *.out

	

	


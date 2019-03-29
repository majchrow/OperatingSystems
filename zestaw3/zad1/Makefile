CC = gcc
CFLAGS = -Wall -std=c11
MAIN = main.c
OUT = main.out

all: compile tests clean

compile:
	$(CC) $(CFLAGS) $(MAIN) -o $(OUT)

tests:
	./$(OUT) './test'
clean:
	rm $(OUT)

	


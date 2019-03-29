CC = gcc
CFLAGS = -Wall -std=c11
MAIN = main.c
OUT = main.out
TEST = tester.c
OUT_TEST = tester.out

all: compile tests clean

compile:
	$(CC) $(CFLAGS) $(MAIN) -o $(OUT)
	$(CC) $(CFLAGS) $(TEST) -o $(OUT_TEST)

tests:
	./$(OUT) lista 60 disc
	./$(OUT) lista 60 memory
	./$(OUT_TEST) test 5 10 15

clean:
	rm $(OUT) test archiwum/* 
	

	


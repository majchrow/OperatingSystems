CC = gcc
CFLAGS = -Wall
MAIN = main.c
OUT = main.out
TEST = tester.c
OUT_TEST = tester.out

all: compile tests clean

compile:
	$(CC) $(CFLAGS) $(MAIN) -o $(OUT)
	$(CC) $(CFLAGS) $(TEST) -o $(OUT_TEST)

tests:
	./$(OUT_TEST) test 2 5 15 &
	./$(OUT_TEST) test1 2 5 15 &
	./$(OUT_TEST) test2 2 5 15 &
	./$(OUT_TEST) test3 2 5 15 &
	./$(OUT) lista < file >> results.txt 
clean:
	killall $(OUT_TEST)
	rm $(OUT) $(OUT_TEST)

	

	


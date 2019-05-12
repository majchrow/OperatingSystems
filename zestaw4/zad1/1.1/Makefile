CC = gcc
CFLAGS = -Wall
MAIN = main.c
OUT = main.out

all: compile tests clean

compile:
	$(CC) $(CFLAGS) $(MAIN) -o $(OUT)

tests:
	./$(OUT) &
	sleep 5
	killall -SIGTSTP $(OUT)
	sleep 2
	killall -SIGTSTP $(OUT)
	sleep 5
	killall -SIGTSTP $(OUT)
	sleep 2
	killall -SIGINT $(OUT)

clean:
	rm $(OUT)

	

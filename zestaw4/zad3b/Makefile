CC = gcc
CFLAGS = -Wall
SENDER = sender.c
CATCHER = catcher.c
SENDER_OUT = sender.out
CATCHER_OUT = catcher.out

all: compile test clean

compile:
	$(CC) $(CFLAGS) $(SENDER) -o $(SENDER_OUT)
	$(CC) $(CFLAGS) $(CATCHER) -o $(CATCHER_OUT)

tests:
	# ./$(CATCHER_OUT) &
	# We don't know pid, so we have to input it by hand

clean:
	rm $(SENDER_OUT) $(CATCHER_OUT)

	

	


CC = gcc
CFLAGS = -Wall
MASTER = master.c
SLAVE = slave.c
MASTER_OUT = master.out
SLAVE_OUT = slave.out
FIFO_NAME = fifo

all: compile tests clean

compile:
	$(CC) $(CFLAGS) $(MASTER) -o $(MASTER_OUT)
	$(CC) $(CFLAGS) $(SLAVE) -o $(SLAVE_OUT)

tests:	
	./$(MASTER_OUT) $(FIFO_NAME) &
	./$(SLAVE_OUT) $(FIFO_NAME) 5 &
	./$(SLAVE_OUT) $(FIFO_NAME) 4 &
	./$(SLAVE_OUT) $(FIFO_NAME) 3 &
	./$(SLAVE_OUT) $(FIFO_NAME) 3 &
	sleep 30

clean:
	killall $(MASTER_OUT)
	rm $(FIFO_NAME) *.out

	

	


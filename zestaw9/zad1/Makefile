CC = gcc 
CFLAGS = -Wall -lpthread
MAIN = main.c
MAIN_OUT = main.out

all: compile tests clean

compile:
	$(CC) $(MAIN) $(CFLAGS) -o $(MAIN_OUT) 
	
tests:	
	./$(MAIN_OUT) 50 5 5 10 # (Number of passengers) (Number of carts) (Cart capacity) (Number of rides) | ! Cond: (Number of passengers) >=  (Number of carts) * (Cart capacity)
	
clean:
	rm *.out 
	
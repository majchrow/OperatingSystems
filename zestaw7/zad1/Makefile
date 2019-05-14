CC = gcc
CFLAGS = -Wall
TRUCKER = trucker.c
LOADER = loader.c
WORKER = worker.c
TRUCKER_OUT = trucker.out
LOADER_OUT = loader.out
WORKER_OUT = worker.out

all: compile tests clean

compile:
	$(CC) $(CFLAGS) $(TRUCKER) -o $(TRUCKER_OUT)
	$(CC) $(CFLAGS) $(LOADER) -o $(LOADER_OUT)
	$(CC) $(CFLAGS) $(WORKER) -o $(WORKER_OUT)

tests:	# there is no sleep in any synchronized process, so the output may be confusing due to the fast printing, may be good idea to run make > file.txt and read the output history
	./$(TRUCKER_OUT) 50 5 15 &  # truck_size belt_size belt_cap | all > 0 | belt_size < 1000
	./$(LOADER_OUT) 10 & # num_of_workers
	sleep 3 # it is better to wait 3 seconds and let "make" kill the processes 

clean:
	killall -SIGINT $(TRUCKER_OUT)  # clean up the trucker process(signal + atexit), because there is infinite loop for trucker arrival
	sleep 1 # don't kill workers before trucker cleanup is done
	killall $(WORKER_OUT) # number of cycles for workers was set up to infinite, so we need to kill it by hand
	rm *.out 

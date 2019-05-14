CC = gcc 
CFLAGS = -Wall -lpthread -lrt 
TRUCKER = trucker.c
LOADER = loader.c
WORKER = worker.c
TRUCKER_OUT = trucker.out
LOADER_OUT = loader.out
WORKER_OUT = worker.out

all: compile tests clean

compile:
	$(CC) $(TRUCKER) $(CFLAGS) -o $(TRUCKER_OUT)
	$(CC) $(LOADER)  $(CFLAGS) -o $(LOADER_OUT)
	$(CC) $(WORKER)  $(CFLAGS) -o $(WORKER_OUT)

tests:	# may be good idea to run make > file.txt and read the output history
	./$(TRUCKER_OUT) 50 5 15 &  # truck_size belt_size belt_cap | all > 0 | belt_size < 1000
	./$(LOADER_OUT) 10 & # num_of_workers
	sleep 5 # it is better to wait 5 seconds and let "make" kill the processes 

clean:
	killall -SIGINT $(TRUCKER_OUT)  # clean up the trucker process(signal + atexit) (rest will be cleaned by itself)
	rm *.out 
	rm /dev/shm/sem.* # remove semaphores from memory in case of shm_unlink semaphores fails

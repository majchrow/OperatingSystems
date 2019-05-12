CC = gcc
CFLAGS = -Wall
SERVER = server.c
CLIENT = client.c
SERVER_OUT = server.out
CLIENT_OUT = client.out
TEST_FILE = file

all: compile tests clean

compile:
	$(CC) $(CFLAGS) $(SERVER) -o $(SERVER_OUT) -lrt
	$(CC) $(CFLAGS) $(CLIENT) -o $(CLIENT_OUT) -lrt

tests:	
	./$(SERVER_OUT) & #IF YOU WANT TO READ FROM FILE JUST PASS < FILE TO THE CLIENT PROGRAM
	./$(CLIENT_OUT) #< $(TEST_FILE) # IT IS BETTER NOT TO PASS FILE, BUT WRITE DOWN COMMENDS FROM file MANUALLY

clean:
	killall -SIGINT $(SERVER_OUT) 
	rm *.out # if you pass the STOP request to the client clean will work properly, but if you end it with the CTRL-C it won't handle the rm *.out command

CC = gcc 
CFLAGS = -Wall -lpthread
SERVER = server.c
CLIENT = client.c
SERVER_OUT = server.out
CLIENT_OUT = client.out

all: compile tests clean

server:
	$(CC) $(SERVER) $(CFLAGS) -o $(SERVER_OUT) 
	./$(SERVER_OUT) 8080 local_path  			 		# (TCP port) (UNIX path)

compile:
	$(CC) $(CLIENT) $(CFLAGS) -o $(CLIENT_OUT) 

tests:	
	./$(CLIENT_OUT) WebSlave Web 127.0.0.1 8080 & 		# (Client Name) (Web) (IPv4) (Port) 
	./$(CLIENT_OUT) LocalSlave Local local_path &		# (Client Name) (Local) (UNIX Path)
	./$(CLIENT_OUT) WebSlave2 Web 127.0.0.1 8080 & 		# (Client Name) (Web) (IPv4) (Port) 
	./$(CLIENT_OUT) Local2 Local local_path &		# (Client Name) (Local) (UNIX Path)
	sleep 20

clean:
	killall -SIGINT $(CLIENT_OUT)  
	sleep 2
	killall -SIGINT $(SERVER_OUT) 
	rm *.out 

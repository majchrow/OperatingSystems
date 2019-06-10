#include "shared.h"

int web_fd = -1; // web descriptor
int local_fd = -1; // local descriptor
int epoll_fd = -1; // epoll
char local_path[MAX_PATH]; // UNIX path(second parameter) for cleanup
pthread_t ping_thread = 0; // thread for pinging
pthread_t terminal_thread = 0; // thread for terminal usage
struct Client clients[MAX_CLIENTS];
int curr_clients = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int global_id = 0;


void init(int argc, char *argv[]);
void clean_server();
void exit_handler();
void *ping_routine(void *arg);
void *terminal_routine(void *arg);
int read_file(char* filename, char *result);
void send_request(int type, int len, struct request req, int i);
void handle_connection(int fd);
void handle_response(int fd);
void handle_register(int fd, char* client_name);
void handle_unregister(int fd, char* client_name);
void clear_poll(int fd);
int check_exist(char* client_name);

int main(int argc, char *argv[])
{
    srand(time(NULL)); // only for all client being busy at the same time
    printf("Server: Starting...\n");
    init(argc, argv);
    struct epoll_event events[MAX_EVENTS];
    int event_count;
    while (TRUE) {
        printf("Server: Polling for requests...\n");
        if (( event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1)) == -1){
            fprintf(stderr, "Server: Polling wait failed\n");
            exit(EXIT_FAILURE);
        }
        printf("Server: %d ready events\n", event_count);
        for(int cur_event = 0; cur_event < event_count; ++cur_event){
            printf("Server: Processing event nr %d\n", cur_event + 1);
            if (events[cur_event].data.fd < 0){
                handle_connection(-events[cur_event].data.fd);
            }
            else{
                handle_response(events[cur_event].data.fd);
            }
        }
    }
}

void init(int argc, char *argv[]){

    printf("Server: Initializing server...\n");

    char usage[MAX_PATH];
    sprintf(usage, "Usage: %s (TCP port) (UNIX path)\n", argv[0]);

    if (argc != 3) {
        fprintf(stderr, "Server: Wrong number of parameters expected 2\n%s", usage);
        exit(EXIT_FAILURE);
    }

    int port;
    if (sscanf(argv[1], "%d", &port) != 1) {
        fprintf(stderr, "Server: First parameter should be integer\n%s", usage);
        exit(EXIT_FAILURE);
    }
    assert(port >= 1024 && port <= 65535);

    strcpy(local_path, argv[2]);
    // web address
    struct sockaddr_in web_serv_addr;
    memset(&web_serv_addr, '0', sizeof(web_serv_addr));
    web_serv_addr.sin_family = AF_INET;
    web_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    web_serv_addr.sin_port = htons(port);

    // local address
    struct sockaddr_un local_serv_addr;
    memset(&local_serv_addr, '0', sizeof(local_serv_addr));
    local_serv_addr.sun_family = AF_UNIX;
    strcpy(local_serv_addr.sun_path, local_path);

    // atexit before setting sockets
    if (atexit(clean_server) == -1){
        fprintf(stderr, "Server: Failed to set clean method\n%s", usage);
        exit(EXIT_FAILURE);
    }

    // Ctrl + C handler right after atexit
    if (signal(SIGINT, exit_handler) == SIG_ERR) {
        fprintf(stderr, "Server: Couldn't set CTRL-C handler\n");
        exit(EXIT_FAILURE);
    }

    // web + local sockets initialization
    if((web_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){ // IPv4 + Stream
        fprintf(stderr, "Server: Failed to create web socket\n");
        exit(EXIT_FAILURE);
    }

    if((local_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){ // Local + Stream
        fprintf(stderr, "Server: Failed to create local socket\n");
        exit(EXIT_FAILURE);
    }

    if(bind(web_fd, (struct sockaddr*)&web_serv_addr, sizeof(web_serv_addr))){
        fprintf(stderr, "Server: Failed to bind web socket\n");
        exit(EXIT_FAILURE);
    }

    if(bind(local_fd, (struct sockaddr*)&local_serv_addr, sizeof(local_serv_addr))){
        fprintf(stderr, "Server: Failed to bind local socket\n");
        exit(EXIT_FAILURE);
    }

    if((listen(web_fd, MAX_CLIENTS)) < 0 ){
        fprintf(stderr, "Server: Failed to listen on web socket\n");
        exit(EXIT_FAILURE);
    }

    if((listen(local_fd, MAX_CLIENTS)) < 0 ){
        fprintf(stderr, "Server: Failed to listen on local socket\n");
        exit(EXIT_FAILURE);
    }

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLPRI;

    if ((epoll_fd = epoll_create1(0)) == -1){
        fprintf(stderr, "Server: Creating epoll failed\n");
        exit(EXIT_FAILURE);
    }

    // descriptors < 0 means that clients are not initialized yet!

    event.data.fd = -web_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, web_fd, &event) == -1){
        fprintf(stderr, "Server: Failed to add web descriptor to the poll\n");
        exit(EXIT_FAILURE);
    }


    event.data.fd = -local_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, local_fd, &event) == -1){
        fprintf(stderr, "Server: Failed to add local descriptor\n");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&ping_thread, NULL, ping_routine, NULL) != 0){
        fprintf(stderr, "Server: Failed to create pinging routine\n");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&terminal_thread, NULL, terminal_routine, NULL) != 0){
        fprintf(stderr, "Server: Failed to create terminal service routine\n");
        exit(EXIT_FAILURE);
    }

    printf("Server: Server initialized successfully...\n");
}

void clean_server(){
    printf("Server: Cleaning before exiting...\n");

    if(ping_thread){
        pthread_cancel(ping_thread);
    }

    if(terminal_thread){
        pthread_cancel(terminal_thread);
    }

    if(shutdown(web_fd, SHUT_RDWR) == -1){
        fprintf(stderr, "Server: Failed to shutdown web socket\n");
    }

    if(close(web_fd) == -1){
        fprintf(stderr, "Server: Failed to close web socket\n");
    }

    if(shutdown(local_fd, SHUT_RDWR)  == -1){
        fprintf(stderr, "Server: Failed to shutdown local socket\n");
    }

    if(close(local_fd) == -1){
        fprintf(stderr, "Server: Failed to close local socket\n");
    }

    if(close(epoll_fd) == -1){
        fprintf(stderr, "Server: Failed to close epoll\n");
    }

    if(unlink(local_path) == -1){
        fprintf(stderr, "Server: Failed to unlink path %s\n", local_path);
    }

    printf("Server: Cleanup done, exiting...\n");
}

void exit_handler(){
    printf("\nServer: CTRL-C received\n");
    exit(0); // atexit cleanup
}

void *ping_routine(void *arg){
    uint8_t tag = PING;
    while(TRUE){
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < curr_clients; ++i) {
            if (clients[i].ping != 0) {
                printf("Server-ping: Client is not responding. Removing... %s\n", clients[i].client_name);
                clear_poll(clients[i].client_fd);
                curr_clients--;
                for (int j = i; j < curr_clients; ++j)
                    clients[j] = clients[j + 1];
                --i;
            } else {
                printf("Server-ping: Pinging client %s...\n", clients[i].client_name);
                if (write(clients[i].client_fd, &tag, TAG) != TAG){
                    fprintf(stderr, "Server-ping: Failed to ping client %s\n", clients[i].client_name);
                    continue;
                }
                clients[i].ping++;
            }
        }
        pthread_mutex_unlock(&mutex);
        sleep(CRON);
    }
}

void *terminal_routine(void *arg){
    char buffer[MAX_PATH];

    while(TRUE){
        printf("Server-terminal: Provide file name: ");
        fgets(buffer, MAX_PATH, stdin);
        if(curr_clients == 0){
            fprintf(stderr, "Server-terminal: No clients are registered at the moment, please wait for connections\n");
            continue;
        }
        char* filename = strtok(buffer, "\n");
        if (access(filename, F_OK) == -1) {
            fprintf(stderr, "Server-terminal: Input file name %s doesn't exists\n", filename);
            continue;
        }
        struct request req;
        if(read_file(filename, req.buffer)){
            fprintf(stderr, "Server-terminal: Failed to read content from file %s\n", filename);
            continue;
        }
        req.id = ++global_id;
        int i = 0;
        int busy = 1;
        for (i = 0; i < curr_clients; ++i) {
            if (clients[i].reserved == 0) {
                busy = 0;
                break;
            }
        }
        printf("Server: Request has been sent to %s\n", clients[i].client_name);
        if(busy){ // all clients are working choose the random one
            i = rand() % curr_clients;
        }
        clients[i].reserved++;
        send_request(REQUEST, sizeof(struct request), req, i);
    }
}

void send_request(int type, int len, struct request req, int i) {
    if (write(clients[i].client_fd, &type, TAG) != TAG) {
        fprintf(stderr, "Server: Failed to send request TAG\n");
        exit(EXIT_FAILURE);
    }
    if (write(clients[i].client_fd, &len, LENGTH) != LENGTH) {
        fprintf(stderr, "Server: Failed to send request length\n");
        exit(EXIT_FAILURE);
    }
    if (write(clients[i].client_fd, &req, len) != len) {
        fprintf(stderr, "Server: Failed to send request content\n");
        exit(EXIT_FAILURE);
    }
}

int read_file(char* filename, char* result){ // read content from filename and copy it into result (result memory must be allocated)
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Server-terminal: Couldn't read file %s\n", filename);
        return 1;
    }

    fseek(fp, 0L, SEEK_END);
    long lSize = ftell(fp);
    if (lSize == 0) {
        fprintf(stderr, "Server-terminal: Given file is empty %s\n", filename);
        return 1;
    }
    rewind(fp);


    if (!result) {
        fclose(fp);
        fprintf(stderr, "Server-terminal: Memory allocating failed\n");
        return 1;
    }

    long size = (lSize < MAX_SIZE) ? lSize : MAX_SIZE; // do not read more than 2^16 bytes

    if (fread(result, size, 1, fp) != 1) {
        fclose(fp);
        fprintf(stderr, "Server-terminal: Coping file content %s into memory failed\n", filename);
        return 1;
    }
    fclose(fp);
    return 0;
}

void handle_connection(int fd){
    printf("Server: Handling connection from client...\n");
    int client = accept(fd, NULL, NULL);
    if (client == -1){
        fprintf(stderr, "Server: Failed to accept new client\n");
        exit(EXIT_FAILURE);
    }

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLPRI;
    event.data.fd = client;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client, &event) == -1){
        fprintf(stderr, "Server: Failed to add new client to the epoll\n");
        exit(EXIT_FAILURE);
    }
}

void handle_response(int fd){
    printf("Server: Handling response from client...\n");

    uint8_t tag;
    uint16_t length;

    if (read(fd, &tag, TAG) != TAG){
        fprintf(stderr, "Server: Failed to read tag from message\n");
        exit(EXIT_FAILURE);
    }
    if (read(fd, &length, LENGTH) != LENGTH){
        fprintf(stderr, "Server: Failed to read length from message\n");
        exit(EXIT_FAILURE);
    }
    char client_name[10];

    switch (tag){
        case REGISTER:
            if (read(fd, client_name, length) != length){
                fprintf(stderr, "Server: Failed to read client name from register message\n");
                exit(EXIT_FAILURE);
            }
            pthread_mutex_lock(&mutex);
            handle_register(fd, client_name);
            pthread_mutex_unlock(&mutex);
            break;
        case UNREGISTER: // it's only because when client is shutting down without unregistering, it sends wrong message and crashes server
            if (read(fd, client_name, length) != length){
                fprintf(stderr, "Server: Failed to read client name from register message\n");
                exit(EXIT_FAILURE);
            }
            pthread_mutex_lock(&mutex);
            handle_unregister(fd, client_name);
            pthread_mutex_unlock(&mutex);
            break;
        case RESULT:
            pthread_mutex_lock(&mutex);
            if (read(fd, client_name, length) != length){
                fprintf(stderr, "Server: Failed to read client name from register message\n");
                exit(EXIT_FAILURE);
            }
            int i = check_exist(client_name);
            clients[i].reserved --;
            clients[i].ping = 0;
            char buffer[MAX_SIZE];
            int res_len;
            if (read(fd, &res_len, sizeof(int)) != sizeof(int)) {
                fprintf(stderr, "Server: Failed to read result size\n");
                exit(EXIT_FAILURE);
            }

            if (read(fd, buffer, res_len) < 0) {
                fprintf(stderr, "Server: Failed to read result message\n");
                exit(EXIT_FAILURE);
            }
            printf("Server: Received from `%s`\n%s", client_name, buffer);

            pthread_mutex_unlock(&mutex);
            break;
        case PONG:
            pthread_mutex_lock(&mutex);
            if (read(fd, client_name, length) != length){
                fprintf(stderr, "Server: Failed to read client name from register message\n");
                exit(EXIT_FAILURE);
            }
            int index = check_exist(client_name);
            clients[index].ping = 0;
            pthread_mutex_unlock(&mutex);
        default:
            break;
    }
}

void handle_register(int fd, char* client_name){
    uint8_t message_type;
    if (curr_clients == MAX_CLIENTS) { // max clients exceeded
        message_type = CLIENTSEXCEED;
        if (write(fd, &message_type, 1) != 1){
            fprintf(stderr, "Server: Failed to send `CLIENTSEXCEED` message to client\n");
            exit(EXIT_FAILURE);
        }
        clear_poll(fd);
    } else if (check_exist(client_name) >= 0) { // client_name already exists
            message_type = NAMETAKEN;
            if (write(fd, &message_type, 1) != 1){
                fprintf(stderr, "Server: Failed to send `NAMETAKEN` message to client\n");
                exit(EXIT_FAILURE);
            }
            clear_poll(fd);
        } else { // all ok
            printf("Server: registering client %s\n", client_name);
            clients[curr_clients].client_fd = fd;
            clients[curr_clients].reserved = 0;
            clients[curr_clients].ping = 0;
            strcpy(clients[curr_clients++].client_name, client_name);
            message_type = OK;
            if (write(fd, &message_type, 1) != 1){
                fprintf(stderr, "Server: Failed to send `OK` message to client\n");
                exit(EXIT_FAILURE);
        }
    }
}

void handle_unregister(int fd, char* client_name){
    int i = check_exist(client_name);
    if (i >= 0) {
        clear_poll(fd);
        curr_clients --;
        for (int j = i; j < curr_clients; ++j) //reshuffle
            clients[j] = clients[j + 1];
        printf("Server: Client `%s` unregistered\n", client_name);
    }
}

void clear_poll(int fd){
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1){
        fprintf(stderr, "Server: Failed to remove descriptor from the pool\n");
        exit(EXIT_FAILURE);
    }

    if (shutdown(fd, SHUT_RDWR) == -1){
        fprintf(stderr, "Server: Failed to shutdown client's socket\n");
        exit(EXIT_FAILURE);
    }

    if (close(fd) == -1){
        fprintf(stderr, "Server: Failed to close client's socket\n");
        exit(EXIT_FAILURE);
    }
}

int check_exist(char* client_name){
    for(int i = 0; i < curr_clients; ++i){
        if(!strcmp(clients[i].client_name, client_name)){
            return i;
        }
    }
    return -1;
}

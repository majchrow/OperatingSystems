#include "shared.h"

char client_name[MAX_PATH] = "?";
int client_fd = -1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void init(int argc, char *argv[]);
void clean_client();
void exit_handler();
void try_connect();
void tag_length_value(uint8_t tag);
void *dummy_handler(void *args);

int main(int argc, char *argv[])
{

    printf("Client %-10s: Starting...\n", client_name);
    init(argc, argv);
    try_connect();

    while(1){
        uint8_t tag;
        pthread_t thread;
        int req_len;
        struct request req;
        while (TRUE) {
            if (read(client_fd, &tag, TAG) != TAG){
                fprintf(stderr, "Client %-10s: Couldn't read TAG from server request\n", client_name);
                exit(EXIT_FAILURE);
            }
            switch (tag) {
                case REQUEST:
                    if (read(client_fd, &req_len, LENGTH) <= 0) {
                        fprintf(stderr, "Client %-10s: Couldn't get request length\n", client_name);
                        exit(EXIT_FAILURE);
                    }

                    if (read(client_fd, &req, req_len) < 0) {
                        fprintf(stderr, "Client %-10s: Couldn't get request content\n", client_name);
                        exit(EXIT_FAILURE);
                    }

                    printf("Client %-10s: Starting to process id %d\n", client_name, req.id);
                    pthread_create(&thread, NULL, dummy_handler, &req);
                    pthread_detach(thread); // Creating new thread for request! Otherwise we can't response for server pings in time!
                    break;
                case PING:
                    printf("Client %-10s: Sending response for ping to the server\n", client_name);
                    tag_length_value(PONG);
                    break;
                default:
                    fprintf(stderr, "Client %-10s: Got wrong TAG from server request\n", client_name);
                    exit(EXIT_FAILURE);
            }
        }
    }
}

void init(int argc, char *argv[]){

    printf("Client %-10s: Initializing client...\n", client_name);

    char usage[MAX_PATH];
    sprintf(usage, "Usage Web Connection:   %s (Client Name) (Web) (IPv4) (TCP Port)\n"
                   "Usage Local Connection: %s (Client Name) (Local) (UNIX Path)\n",
                   argv[0], argv[0]);

    if (argc < 4 || argc > 5) {
        fprintf(stderr, "Client %-10s: Wrong number of parameters expected 3 or 4\n%s", client_name,  usage);
        exit(EXIT_FAILURE);
    }

    if(strlen(argv[1]) > 10){ // Max client name = 10, but it's not macro for purpose of printf formatting
        fprintf(stderr, "Client %-10s: Wrong client name %s, max char length for client name is 10\n%s", client_name, argv[1], usage);
        exit(EXIT_FAILURE);
    }
    strcpy(client_name, argv[1]);

    if (atexit(clean_client) == -1){
        fprintf(stderr, "Client %-10s: Failed to set clean method\n%s", client_name, usage);
        exit(EXIT_FAILURE);
    }

    if (signal(SIGINT, exit_handler) == SIG_ERR) {
        fprintf(stderr, "Client %-10s: Failed to set clean method\n", client_name);
        exit(EXIT_FAILURE);
    }

    if(strcmp(argv[2], "Web") == 0){ // Web connection initializing

        if (argc != 5) {
            fprintf(stderr, "Client %-10s: Wrong number of parameters expected 4 for Web connection\n%s", client_name,  usage);
            exit(EXIT_FAILURE);
        }

        int port;

        if (sscanf(argv[4], "%d", &port) != 1) {
            fprintf(stderr, "Client %-10s: Fourth parameter should be integer\n%s", client_name, usage);
            exit(EXIT_FAILURE);
        }
        assert(port >= 1024 && port <= 65535);

        struct sockaddr_in web_serv_addr;
        memset(&web_serv_addr, '0', sizeof(web_serv_addr));
        web_serv_addr.sin_family = AF_INET;
        web_serv_addr.sin_port = htons(port);
        if(inet_pton(AF_INET, argv[3], &web_serv_addr.sin_addr) <= 0)
        {
            fprintf(stderr, "Client %-10s: Failed to set ip_address %s\n%s",client_name, argv[3], usage);
            exit(EXIT_FAILURE);
        }

        if((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){ // IPv4 + Stream
            fprintf(stderr, "Client %-10s: Failed to create web socket\n", client_name);
            exit(EXIT_FAILURE);
        }

        if(connect(client_fd, (struct sockaddr *)&web_serv_addr, sizeof(web_serv_addr)) < 0)
        {
            fprintf(stderr, "Client %-10s: Failed to connect to web socket\n", client_name);
            exit(EXIT_FAILURE);
        }

    }else if(strcmp(argv[2], "Local") == 0){ // Local connection initializing
        if (argc != 4) {
            fprintf(stderr, "Client %-10s: Wrong number of parameters expected 3 for Local connection\n%s", client_name,  usage);
            exit(EXIT_FAILURE);
        }

        struct sockaddr_un local_serv_addr;
        memset(&local_serv_addr, '0', sizeof(local_serv_addr));
        local_serv_addr.sun_family = AF_UNIX;
        strcpy(local_serv_addr.sun_path, argv[3]);

        if((client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){ // Unix path + Stream
            fprintf(stderr, "Client %-10s: Failed to create local socket\n", client_name);
            exit(EXIT_FAILURE);
        }

        if(connect(client_fd, (struct sockaddr *)&local_serv_addr, sizeof(local_serv_addr)) < 0)
        {
            fprintf(stderr, "Client %-10s: Failed to connect to local socket\n", client_name);
            exit(EXIT_FAILURE);
        }

    } else{ // Strong connection passed
        fprintf(stderr, "Client %-10s: wrong connection type, expected `Web` or `Local` got `%s`\n", client_name, argv[2]);
        exit(EXIT_FAILURE);
    }

    printf("Client %-10s: Client initialized successfully...\n", client_name);
}

void clean_client(){
    printf("Client %-10s: Cleaning before exiting...\n", client_name);
    tag_length_value(UNREGISTER);

    if(shutdown(client_fd, SHUT_RDWR) == -1){
        fprintf(stderr, "Client %-10s: Failed to shutdown client socket\n", client_name);
    }

    if(close(client_fd) == -1){
        fprintf(stderr, "Client %-10s: Failed to close client socket\n", client_name);
    }

    printf("Client %-10s: Cleanup done, exiting...\n", client_name);
}

void exit_handler(){
    printf("\nClient %-10s: CTRL-C received\n", client_name);
    exit(0);
}

void try_connect(){
    printf("Client %-10s: Sending registration message...\n", client_name);
    tag_length_value(REGISTER);
    uint8_t response;

    printf("Client %-10s: Waiting for response...\n", client_name);

    if( read(client_fd, &response, 1) != 1){
        fprintf(stderr, "Client %-10s: Failed to receive response message for registration\n", client_name);
        exit(EXIT_FAILURE);
    }

    switch (response) {
        case NAMETAKEN:
            fprintf(stderr, "Client %-10s: Failed to register, my name is already taken\n", client_name);
            exit(EXIT_FAILURE);
        case CLIENTSEXCEED:
            fprintf(stderr, "Client %-10s: Failed to register, to many clients are connected\n", client_name);
            exit(EXIT_FAILURE);
        case OK:
            printf("Client %-10s: Successfully logged into the server\n", client_name);
            break;
        default:
            fprintf(stderr, "Client %-10s: Received wrong tag in registration response\n", client_name);
            exit(EXIT_FAILURE);
    }
}

void tag_length_value(uint8_t tag){
    pthread_mutex_lock(&mutex);
    if( write(client_fd, &tag, TAG) != TAG){
        fprintf(stderr, "Client %-10s: Failed to write tag\n", client_name);
        exit(EXIT_FAILURE);
    }
    uint16_t msg_len = (int16_t) (strlen(client_name) + 1);

    if( write(client_fd, &msg_len, LENGTH) != LENGTH){
        fprintf(stderr, "Client %-10s: Failed to write length\n", client_name);
        exit(EXIT_FAILURE);
    }

    if( write(client_fd, client_name, msg_len) != msg_len){
        fprintf(stderr, "Client %-10s: Failed to write value\n", client_name);
        exit(EXIT_FAILURE);
    }
    pthread_mutex_unlock(&mutex);
}

void *dummy_handler(void *args){ // TODO this should be done by hashmap, but w/e

    struct request req = *(struct request*) args;

    char buffer_unique[MAX_SIZE];
    char buffer_counter[MAX_SIZE];
    char res[MAX_SIZE];
    char cmd_unique[MAX_SIZE*2];
    char cmd_counter[MAX_SIZE*2];

    sprintf(cmd_unique, "echo '%s' | awk '{for(x=1;$x;++x)print $x}' | sort | uniq -c", (char *) req.buffer);
    sprintf(cmd_counter, "echo '%s' | awk '{for(x=1;$x;++x)print $x}' | wc -w", (char *) req.buffer);

    FILE *fp;
    fp = popen(cmd_unique, "r");
    int n = fread(buffer_unique, 1, MAX_SIZE, fp);
    pclose(fp);
    buffer_unique[n] = '\0';
    fp = popen(cmd_counter, "r");
    fread(buffer_counter, 1, MAX_SIZE, fp);
    pclose(fp);

    sprintf(res, "Request ID: %d\nAll word sum: %sWords count: \n%s",req.id, buffer_counter, buffer_unique);
    tag_length_value(RESULT);
    int len = strlen(res);
    if (write(client_fd, &len, sizeof(int)) != sizeof(int)){
        fprintf(stderr, "Client %-10s: Failed to send result\n", client_name);
        exit(EXIT_FAILURE);
    }

    if (write(client_fd, res, len) != len){
        fprintf(stderr, "Client %-10s: Failed to send result\n", client_name);
        exit(EXIT_FAILURE);
    }

    printf("Client %-10s: Request %d done, and result was sent to the server\n", client_name, req.id);

    pthread_exit(NULL);
}

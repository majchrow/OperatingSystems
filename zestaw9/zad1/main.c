#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#define MAX_PATH 4096
#define RIDE_TIME 2000 // 2 milliseconds for a single ride

// input arguments
int passengers_count; // number of passengers threads to create
int carts_count; // number of carts threads to create
int cart_capacity; // capacity of each cart
int rides_count; // number of rides that will be done for each cart

// start structure to get relative time in printing messages
struct timeval start; // get realtime before starting threads to measure relative time in threads

// values for sync
int available_carts; // current cart count in the amusement park (0 means that there is no more carts, and passengers can leave)
int current_cart; // current cart that is allowed to arrive (works as a simple queue)
int start_button; // start button possible values are (0 - don't press , 1 - can be pressed, 2 - already pressed) to sync
int incomers_count; // count of the passengers that enter the cart
int leavers_count; // count of the passengers that leave the cart

// sync for entire "cart in the station" time
pthread_mutex_t arrival_mutex;
pthread_cond_t arrival_cond;

// sync for entering passengers (+ trick for notifying waiting passengers that last cart ended their job)
pthread_mutex_t enter_mutex;
pthread_cond_t enter_cond;

// sync for the starting button
pthread_mutex_t button_mutex;
pthread_cond_t button_cond;

// sync for leaving passengers
pthread_mutex_t leave_mutex;
pthread_cond_t leave_cond;

void parse_input(int argc, char **argv); // parse the input arguments
void init(); // initialize sync/mutex/cond values
void *passenger(void *arg); // passenger thread
void *cart(void *arg); // cart thread

struct timeval get_relative_time() { // get current timestamp (start must be initialized before)
    struct timeval result;
    struct timeval end;
    gettimeofday(&end, NULL);
    timersub(&end, &start, &result);
    return result;
}

int main(int argc, char *argv[]) { // Main amusement park thread

    printf("Main thread: Starting roller coaster...\n");

    parse_input(argc, argv);

    init();

    printf("Main thread: Creating the threads...\n");

    pthread_t passengers[passengers_count];
    pthread_t carts[cart_capacity];

    for (int i = 0; i < carts_count; ++i) { // Create cart threads
        int *cart_id = calloc(1, sizeof(int));
        *cart_id = i; // do not pass the local values(which may be changed) to the threads!
        if (pthread_create(&carts[i], NULL, cart, cart_id) == -1) {
            fprintf(stderr, "Main thread: Failed to create cart thread nr %d\n", i + 1);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < passengers_count; ++i) { // Create passengers threads
        int *passenger_id = calloc(1, sizeof(int));
        *passenger_id = i; // do not pass the local values(which may be changed) to the threads!
        if (pthread_create(&passengers[i], NULL, passenger, passenger_id) == -1) {
            fprintf(stderr, "Main thread: Failed to create passenger thread nr %d\n", i + 1);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < carts_count; ++i) { // Join carts threads
        if (pthread_join(carts[i], NULL) != 0) {
            fprintf(stderr, "Main thread: Failed to join cart thread nr %d\n", i + 1);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < passengers_count; ++i) { // Join passengers threads
        if (pthread_join(passengers[i], NULL) != 0) {
            fprintf(stderr, "Main thread: Failed to join passenger thread nr %d\n", i + 1);
            exit(EXIT_FAILURE);
        }
    }

    printf("Main thread: Roller coaster finished working, closing...\n");

    return 0;
}

void parse_input(int argc, char **argv) {


    printf("Main thread: Parsing the input params...\n");

    char usage[MAX_PATH];

    sprintf(usage, "Usage: %s (Number of passengers) (Number of carts) (Cart capacity) (Number of rides) | ! (Number of passengers) >= (Number of carts) * (Cart capacity) !\n", argv[0]);

    if (argc != 5) {
        fprintf(stderr, "Main thread: Wrong number of parameters expected 4\n%s", usage);
        exit(EXIT_FAILURE);
    }

    if (sscanf(argv[1], "%d", &passengers_count) != 1) {
        fprintf(stderr, "Main thread: First parameter should be integer\n%s", usage);
        exit(EXIT_FAILURE);
    }

    if (sscanf(argv[2], "%d", &carts_count) != 1) {
        fprintf(stderr, "Main thread: Second parameter should be integer\n%s", usage);
        exit(EXIT_FAILURE);
    }

    if (sscanf(argv[3], "%d", &cart_capacity) != 1) {
        fprintf(stderr, "Main thread: Third parameter should be integer\n%s", usage);
        exit(EXIT_FAILURE);
    }

    if (sscanf(argv[4], "%d", &rides_count) != 1) {
        fprintf(stderr, "Main thread: Fourth parameter should be integer\n%s", usage);
        exit(EXIT_FAILURE);
    }

    assert(passengers_count > 0 && carts_count > 0 && cart_capacity > 0 && rides_count > 0); // none of the input params can be negative

    assert(passengers_count >= cart_capacity * carts_count ); // ! there must be at least (cart_capacity * carts_count) passengers to be able to sync program

    printf("Main thread: Input is valid...\n");

}

void init() {
    printf("Main thread: Initializing sync mechanism...\n");

    // init mutexes
    pthread_mutex_init (&arrival_mutex, NULL);
    pthread_mutex_init (&enter_mutex, NULL);
    pthread_mutex_init (&button_mutex, NULL);
    pthread_mutex_init (&leave_mutex, NULL);

    // init conditions variables
    pthread_cond_init(&arrival_cond, NULL);
    pthread_cond_init(&enter_cond, NULL);
    pthread_cond_init(&button_cond, NULL);
    pthread_cond_init(&leave_cond, NULL);

    // set all the variables for synchronization
    available_carts = carts_count; // when it's reach 0, notify the passengers to leave
    current_cart = 0;  // set the first cart to start arrival
    start_button = 0; // block start button for start
    incomers_count = cart_capacity; // block incomers for start
    leavers_count = 0; // block leavers for start

    printf("Main thread: Sync mechanism has been initialized successfully...\n");
    gettimeofday(&start, NULL);

}

void *cart(void *args) {
    int cart_id = *((int *) args); // given cart_id
    struct timeval actual_time;  // struct for relative_time measure
    int current_rides_count = 0; // keep track of rides_count to know when exit the thread
    int first_ride = 1; // flag for the first to signal there is no passengers inside yet (only for first ride)

    while(1) {

        pthread_mutex_lock(&arrival_mutex); // entire station time

        while (cart_id != current_cart) { // wait for your turn to arrive
            pthread_cond_wait(&arrival_cond, &arrival_mutex);
        }

        actual_time = get_relative_time();
        printf("Cart %-12dTime: %ld.%03ld seconds: Arriving in the station, opening the door\n",
               cart_id + 1, actual_time.tv_sec, actual_time.tv_usec / 1000);

        start_button = 0; // block the start button for now

        if(first_ride){ // first ride, don't release passengers
            first_ride = 0;
        }else{ // not first ride, release passengers
            pthread_mutex_lock(&leave_mutex);
            leavers_count = cart_capacity; // let the passengers leave
            pthread_cond_broadcast(&leave_cond); // wait for passengers to leave before others starts to enter
            while(leavers_count != 0){
                pthread_cond_wait(&leave_cond, &leave_mutex); // wait for passengers to get out from the cart
            }
            pthread_mutex_unlock(&leave_mutex);
        }

        if(current_rides_count == rides_count){ // ride_counts were made, and passengers were released, end the cart thread
            break;
        }

        pthread_mutex_lock(&enter_mutex);
        incomers_count = 0; // let the passengers enter
        pthread_cond_broadcast(&enter_cond); // signal passengers that they can enter the cart for next ride
        while (incomers_count != cart_capacity) { // wait for passengers to get into the cart
            pthread_cond_wait(&enter_cond, &enter_mutex);
        }

        actual_time = get_relative_time();
        printf("Cart %-12dTime: %ld.%03ld seconds: All passenger are in, closing the door\n",
               cart_id + 1, actual_time.tv_sec, actual_time.tv_usec / 1000);

        pthread_mutex_unlock(&enter_mutex);

        pthread_mutex_lock(&button_mutex);
        start_button = 1;
        pthread_cond_broadcast(&button_cond); // signal passengers that they can press the button
        while (start_button != 2) { // wait for passenger to press start button
            pthread_cond_wait(&button_cond, &button_mutex);
        }
        pthread_mutex_unlock(&button_mutex);


        actual_time = get_relative_time();
        printf("Cart %-12dTime: %ld.%03ld seconds: Start button has been pressed, starting the ride nr %d\n",
               cart_id + 1, actual_time.tv_sec, actual_time.tv_usec / 1000, current_rides_count + 1);

        current_cart = (current_cart + 1) % carts_count;
        pthread_cond_broadcast(&arrival_cond); // signal next cart in the "queue" that it can arrive
        pthread_mutex_unlock(&arrival_mutex);

        usleep(RIDE_TIME); // time for a ride

        current_rides_count ++;
        actual_time = get_relative_time();
        printf("Cart %-12dTime: %ld.%03ld seconds: Ride nr %d has been ended, waiting for the place in station\n",
               cart_id + 1, actual_time.tv_sec, actual_time.tv_usec / 1000, current_rides_count);
    }

    // last ride for given cart were ended

    current_cart = (current_cart + 1) % carts_count;
    pthread_cond_broadcast(&arrival_cond); // signal next cart in the "queue" that it can arrive (if there is any)

    actual_time = get_relative_time();
    printf("Cart %-12dTime: %ld.%03ld seconds: Ride nr %d has been ended, exiting thread\n",
           cart_id + 1, actual_time.tv_sec, actual_time.tv_usec / 1000, current_rides_count);

    pthread_mutex_lock(&enter_mutex); // just for the exit passengers trick
    available_carts--; // decrease the count of available carts when it reaches 0, it means the passengers can go home
    pthread_cond_broadcast(&enter_cond); // trick to signal the "waiting to enter" passengers that there is no more carts in amusement park by using enter_cond
    pthread_mutex_unlock(&enter_mutex);

    pthread_mutex_unlock(&arrival_mutex); // don't forget to unlock the arrival

    pthread_exit(0);
}

void *passenger(void *args) {
    struct timeval actual_time;
    int passenger_id = *((int *) args);
    int cart_id; // variable that keeps track in which cart we are (to sync entering and leaving from the same cart)


    while(1) {
        // enter to the cart
        pthread_mutex_lock(&enter_mutex);
        while (incomers_count == cart_capacity && available_carts > 0) { // wait for door to be open (to enter cart) OR get signal that there is no more carts
            pthread_cond_wait(&enter_cond, &enter_mutex);
        }

        if (available_carts == 0) { // check if there is no more carts
            break;
        }

        // otherwise we get in to the cart
        incomers_count++;
        cart_id = current_cart;

        actual_time = get_relative_time();
        printf("Passenger %-7dTime: %ld.%03ld seconds: Entering to the cart, current number of passengers in the cart=%d\n",
               passenger_id + 1, actual_time.tv_sec, actual_time.tv_usec / 1000, incomers_count);

        pthread_cond_broadcast(&enter_cond);
        pthread_mutex_unlock(&enter_mutex);

        // start button sync
        pthread_mutex_lock(&button_mutex);
        while (start_button == 0) { // wait for door to be closed (button state in [1, 2])
            pthread_cond_wait(&button_cond, &button_mutex);
        }

        if (start_button == 1) { // only one passenger will press the start_button
            actual_time = get_relative_time();
            printf("Passenger %-7dTime: %ld.%03ld seconds: Pressing the start button\n",
                   passenger_id + 1, actual_time.tv_sec, actual_time.tv_usec / 1000);
            start_button = 2; // set state = already pressed
            pthread_cond_broadcast(&button_cond); // signal the cart to start ride
        }
        pthread_mutex_unlock(&button_mutex);

        // get out of the cart
        pthread_mutex_lock(&leave_mutex);
        while (leavers_count == 0 || cart_id != current_cart) { // wait for door to be open (to leave the cart which we enter)
            pthread_cond_wait(&leave_cond, &leave_mutex);
        }

        leavers_count--;

        actual_time = get_relative_time();
        printf("Passenger %-7dTime: %ld.%03ld seconds: Leaving from cart, current number of passengers in the cart=%d\n",
               passenger_id + 1, actual_time.tv_sec, actual_time.tv_usec / 1000, leavers_count);

        pthread_cond_broadcast(&leave_cond);
        pthread_mutex_unlock(&leave_mutex);
    }

    actual_time = get_relative_time();
    printf("Passenger %-7dTime: %ld.%03ld seconds: No more carts left, exiting the thread\n",
           passenger_id + 1, actual_time.tv_sec, actual_time.tv_usec / 1000);

    pthread_mutex_unlock(&enter_mutex); // don't forget to unlock the enter mutex for rest passengers

    pthread_exit(0);
}

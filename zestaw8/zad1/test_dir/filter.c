#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) { // generate random filter 
    
    srand(time(NULL));

    int c;
    if (sscanf(argv[2], "%d", &c) != 1) {
        fprintf(stderr, "Second parameter should be integer\n");
        exit(EXIT_FAILURE);
    }
    double arr[c*c];
    int sum = 0;
    for(int i = 0; i < c*c ; ++i){
        arr[i] = rand() % 100;
        sum += arr[i];
    }
    for(int i = 0; i < c*c ; ++i){
        arr[i] /= sum;
    }


    FILE *fp;
    if (!(fp = fopen(argv[1], "w"))) {
        fprintf(stderr, "Failed to open file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    };

    fprintf(fp, "%d", c);
    for(int i = 0; i < c*c ; ++i){
        if(i % c == 0) fprintf(fp, "\n");
        fprintf(fp, "%lf ", arr[i]);
    }
  
    fclose(fp);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "sobel.h"
#include "rtclock.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <stdbool.h>

// thread argument struct declaration
typedef struct arg_t {
    int startIndex;
    int endIndex;
} arg_t;

// Sobel kernels
int Kx[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
};

int Ky[3][3] = {
    {-1, -2, -1},
    { 0,  0,  0},
    { 1,  2,  1}
};

// Globals: Image and threading data
unsigned char **input_image;
unsigned char **output_image;
unsigned char threshold = 127;
int width, height;
int num_threads;
bool mode;

/**
 * Main method
 */
int main(int argc, char *argv[]) {

    // check if we have enough args
    if (argc < 4) {
        printf("Usage: sobel <path/to/image> <numThreads (>= 1)> <threshold (0-255)>\n");
        return -1;
    }

    // move args into appropriate variables.
    char *filename = argv[1];
    num_threads = atoi(argv[2]);
    threshold = atoi(argv[3]);
    if (num_threads == 1) {
        mode = false;
    } else {
        mode = true;
    }

    loadFile(filename); // loads file into **input_image (including allocation)
    allocateOutput(); // allocates 2D array for output

    // Start clocking!
    double startTime, endTime;
    startTime = rtclock();

    // prepare and create threads
    int step = height / num_threads;
    arg_t **params = (arg_t**) malloc(sizeof(arg_t)*num_threads);
    pthread_t *threads = (pthread_t*) malloc(sizeof(pthread_t)*num_threads);
    for (int i = 0; i < num_threads; i++) {
        params[i] = (arg_t*) malloc(sizeof(arg_t));
        params[i]->startIndex = step*i;
        params[i]->endIndex = step*(i+1);
        if (mode) {
            pthread_create(&threads[i], NULL, threadWork, params[i]);
        } else {
            threadWork(params[0]); // just do it manually if we're single threaded
        }
    }

    // join back if we're in MULTI-THREADING MODE (explosion sound)
    if (mode) {
        for (int i = 0; i < num_threads; i++) {
            pthread_join(threads[i], NULL);
        }
    }
    
    // End clocking!
    endTime = rtclock();
    printf("Time taken (thread count = %d): %.6f sec\n", num_threads, (endTime - startTime));

    // prepare to write file
    unsigned char* array1D = convertTo1D(output_image); // moves 2D array contents to a new 1D array
    char *newFilename = (char*) malloc(strlen(filename) + 7);
    strcpy(newFilename, filename);
    newFilename[(strlen(newFilename))-4] = '\0'; // remove old ".jpg"
    strcat(newFilename, "-sobel.jpg");

    // write file
    stbi_write_jpg(newFilename, width, height, 1, array1D, 80);

    // free memory
    free(input_image[0]); // 1D array of input (from stbi_load)
    free(input_image); // 2D array of input
    // all inner 2D array pointers
    for (int i = 0; i < height; i++) {
        free(output_image[i]);
    }
    free(output_image); // 2D array of output
    free(array1D); // 1D array of output
    free(threads); //thread id array

    // each threads' parameters
    for (int i = 0; i < num_threads; i++) {
        free(params[i]);
    }
    free(params); // parameter array
    free(newFilename); // filename string

    return 0;
}

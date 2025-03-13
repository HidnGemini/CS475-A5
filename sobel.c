#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "sobel.h"
#include "stb_image.h"
#include "stb_image_write.h"

// thread argument struct declaration
typedef struct arg_t {
    int startIndex;
    int endIndex;
} arg_t;

/**
 * loads the given file and converts it to a 2D array, storing that in
 * input_image global.
 */
int loadFile(char* filename) {
    unsigned char *data = stbi_load(filename, &width, &height, NULL, 1);
    if (data == NULL) {
        printf("File \"%s\" does not exist!\n", filename);
        return -1;
    }

    input_image = (unsigned char**) malloc(sizeof(unsigned char*) * height);

    for (int i = 0; i < height; i++) {
        // assign each row the proper pixel offset
        input_image[i] = &data[i * width];
    }

    printf("Loaded %s. Height=%d, Width=%d\n", filename, height, width);
    return 0;
}

/**
 * threadWork defines the work that each thread will be doing. It applies
 * the sobel filter on all rows from startIndex (inclusive) to endIndex
 * (exclusive) given in the arg_t input.
 */
void threadWork(void *arg) {
    // unpack void* to startIndex and endIndex
    arg_t* input = (arg_t*) arg;
    int startIndex = input->startIndex;
    int endIndex = input->endIndex;

    // loop through each pixel and apply sobel filter
    for (int i = startIndex; i < endIndex; i++) {
        for (int j = 0; j < width; j++) {
            if (i == 0 || j == 0 || i == height-1 || j == width-1) {
                // border pixels black
                output_image[i][j] = (unsigned char) 0;
            } else {
                // gross g_x and g_y defintions :(
                int gx = 
                    (input_image[i-1][j-1] * Kx[0][0]) +
                    (input_image[i-1][j] * Kx[0][1]) +
                    (input_image[i-1][j+1] * Kx[0][2]) +
                    (input_image[i][j-1] * Kx[1][0]) +
                    (input_image[i][j] * Kx[1][1]) +
                    (input_image[i][j+1] * Kx[1][2]) +
                    (input_image[i+1][j-1] * Kx[2][0]) +
                    (input_image[i+1][j] * Kx[2][1]) +
                    (input_image[i+1][j+1] * Kx[2][2]);
                int gy = 
                    (input_image[i-1][j-1] * Ky[0][0]) +
                    (input_image[i-1][j] * Ky[0][1]) +
                    (input_image[i-1][j+1] * Ky[0][2]) +
                    (input_image[i][j-1] * Ky[1][0]) +
                    (input_image[i][j] * Ky[1][1]) +
                    (input_image[i][j+1] * Ky[1][2]) +
                    (input_image[i+1][j-1] * Ky[2][0]) +
                    (input_image[i+1][j] * Ky[2][1]) +
                    (input_image[i+1][j+1] * Ky[2][2]);
                output_image[i][j] = sqrt(pow(gx,2) + pow(gy,2));

                // clamp to 255
                if (sqrt(pow(gx,2) + pow(gy,2)) >= 255) {
                    output_image[i][j] = 255;
                }

                // apply threshold
                if ((int) output_image[i][j] < threshold) {
                    output_image[i][j] = 0;
                }
            } 
        }
    }
}

/**
 * allocates a 2D array for output (100% copy-pasted from the assignment
 * outline)
 */
void allocateOutput() {
   // malloc a size 'height' array of pointers (these are the rows)
   output_image = (unsigned char**) malloc(sizeof(unsigned char*) * height);

   // iterate through each row and malloc an array of size 'width'
   for (int i = 0; i < height; i++) {
      output_image[i] = (unsigned char*) malloc(sizeof(int) * width);
   }
   // Now we have a 2D array, accessible via output_image[x][y]!!
}

/**
 * converts given 2D array into a 1D array representation
 */
unsigned char* convertTo1D(unsigned char** array2D) {
    unsigned char* array1D = (unsigned char*) malloc(sizeof(unsigned char*) * height * width);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            array1D[i*width+j] = array2D[i][j];
        }
    }
    
    return array1D;
}
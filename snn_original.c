/*	
 *	@author: Crow Crossman
 * 	@description: Beginning framework for a homebrewed spiking neural network.
 *
 *	TODO:
 *		Add ability to connect layers.
 *		Add values calculated based on network interconnectivity.
 *		Add time steps
 *		Add random backwards propagtaion
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include <string.h>
#define NUM_ROWS 10000
#define NUM_COLS 785

/* Synaptic connection matrix structure. */
typedef struct {
	int rows;
	int cols;
	float **mat;
} syn;

/* Prototype functions. */
syn* create_syn(int rows, int cols);
syn* random_syn(syn* layer);
void print_syn(syn* layer);
void get_MNIST_data(const char *filename, int data[NUM_ROWS][NUM_COLS]);
 
/* Mainline. */
int main(void) {
	srand(time(NULL));
//	syn* S1 = create_syn(10,10);
	
    	int (*mnist_data)[NUM_COLS] = malloc(sizeof(int[NUM_ROWS][NUM_COLS]));
    	get_MNIST_data("mnist_train.csv", mnist_data);

//    	printf("Solution label of the first image: %d\n", mnist_data[0][0]);
//   	printf("First pixel of the first image: %d\n", mnist_data[0][1]);




    	free(mnist_data);


	return 0;
}

/* Function definitions. */
/* Create a new layer. */
syn* create_syn(int rows, int cols) {
	syn* new_layer = malloc(sizeof(syn));
	new_layer->rows = rows;
	new_layer->cols = cols;
	new_layer->mat = malloc(rows*sizeof(float*));

	for (int i = 0; i < rows; i++) {
		new_layer->mat[i] = calloc(cols, sizeof(float)); // zero out the matrix
	}

	random_syn(new_layer);
	print_syn(new_layer);
	return new_layer;
}

/* Populate with random numbers between 0 and 1. */
syn* random_syn(syn* layer) {
	for (int i = 0; i < layer->rows; i++) {
		for (int j = 0; j < layer->cols; j++) {
			float rand_num = (float)rand()/RAND_MAX;
			layer->mat[i][j] = rand_num;
		}
	}
	return layer;
}

/* Print matrix to terminal. */
void print_syn(syn* layer) {
	for (int i = 0; i < layer->rows; i++) {
		for (int j = 0; j < layer->cols; j++) {
			printf("[%.3f]", layer->mat[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

void get_MNIST_data(const char *filename, int data[NUM_ROWS][NUM_COLS]) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    char line[4096];
    int row = 0;
    while (fgets(line, sizeof(line), file) && row < NUM_ROWS) {
        char *token;
        // Get the first token
        token = strtok(line, ",");
        int col = 0;
        while (token != NULL && col < NUM_COLS) {
            data[row][col] = atoi(token);
            token = strtok(NULL, ",");
            col++;
        }
        row++;
    }

    fclose(file);
}

/*	
 *	@author: Crow Crossman
 * 	@description: Beginning framework for a homebrewed spiking neural network.
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
	int rows;
	int cols;
	float **mat;
} syn;

syn* create_syn(int rows, int cols);
syn* random_syn(syn* layer);
void print_syn(syn* layer);

int main(void) {
	srand(time(NULL));
	syn* S1 = create_syn(10,10);
	S1 = random_syn(S1);
	print_syn(S1);
	return 0;
}

syn* create_syn(int rows, int cols) {
	syn* new_layer = malloc(sizeof(syn));
	new_layer->rows = rows;
	new_layer->cols = cols;
	new_layer->mat = malloc(rows*sizeof(float*));

	for (int i = 0; i < rows; i++) {
		new_layer->mat[i] = calloc(cols, sizeof(float)); // zero out the matrix
	}
	return new_layer;
}

syn* random_syn(syn* layer) {
	for (int i = 0; i < layer->rows; i++) {
		for (int j = 0; j < layer->cols; j++) {
			float rand_num = (float)rand()/RAND_MAX;
			layer->mat[i][j] = rand_num;
		}
	}
	return layer;
}

void print_syn(syn* layer) {
	for (int i = 0; i < layer->rows; i++) {
		for (int j = 0; j < layer->cols; j++) {
			printf("[%.3f]", layer->mat[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

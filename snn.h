#ifndef SNN_H
#define SNN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

/* Hyperparameters */
#define EPOCHS 20
#define NUM_INPUTS 784 // Based on the MNIST pixels
#define NUM_HIDDEN 100 // Number of neurons in the hidden layer
#define NUM_OUTPUTS 10 // 10 digits (0-9)
#define SIM_TIME_MS 100 // Simulation time for one image in ms
#define LEARNING_RATE 0.0005	
#define LEARNING_DECAY 0.95
#define TARGET_SPIKE_COUNT 20.0 // Target spikes for the correct neuron

/* MNIST specific parameters */
#define MNIST_TRAIN_ROWS 1000
#define MNIST_COLS 785 // 1 label + 784 pixels

/* Generic matrix struct (synaptic, etc...) */
typedef struct {
    int rows;
    int cols;
    float** data;
} matrix;

/* Izhikevich model specific parameters */
typedef struct {
    int num_neurons;
    float* v; // Membrane potential
    float* u; // Recovery variable
    float* a;
    float* b;
    float* c;
    float* d;
    int* spikes; // Spike state for current time step (0 or 1)
    float* spike_trace; // Running average of spike activity (for learning)
} ratbrain;

/* Network structure */
typedef struct {
    ratbrain* hidden_layer; // ie, input to hidden
    ratbrain* output_layer; // ie, hidden to output
    matrix* syn_S1; // Synaptic weight matrix from input to hidden layer
    matrix* syn_H1; // Synaptic weight matrix from hidden to output layer
    matrix* random_backprop_matrix; // Fixed values for random BP learning method
} network;

/* Function prototypes */
matrix* create_matrix(int rows, int cols);
void free_matrix(matrix* m);
void randomize_matrix(matrix* m, float scale);

ratbrain* create_ratbrain(int num_neurons);
void free_ratbrain(ratbrain* layer);
network* create_network(int num_inputs, int num_hidden, int num_outputs);
void free_network(network* net);

int** load_mnist_data(const char* filename, int num_rows);
void encode(int* image_pixels, float** encoded_spikes, int num_inputs, int time_ms);

void run_simulation(network* net, float** input_spikes, int train_mode);
void random_backprop_update(network* net, float* error, float** input_spikes, float learning_rate);
int decode(network* net);

void save_weights(network* net, const char* w1_filename, const char* w2_filename);

#endif

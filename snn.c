#include "snn.h"

/* Mainline */
int main(void) {
    srand(time(NULL));

    /* Load data */
    printf("Creating ratbrain network with the following stats:\n\n");
    printf("Input Layer Neurons: %d | Hidden Layer Neurons: %d | Output Layer Neurons: %d\n\n", NUM_INPUTS, NUM_HIDDEN, NUM_OUTPUTS);
    network* net = create_network(NUM_INPUTS, NUM_HIDDEN, NUM_OUTPUTS);

    printf("Loading MNIST training data...\t");
    int** mnist_data = load_mnist_data("mnist_train.csv", MNIST_TRAIN_ROWS);
    if (!mnist_data) {perror("Failure to load MNIST training data!"); return 1;}
    printf("Successfully loaded %d MNIST images!\n\n", MNIST_TRAIN_ROWS);

    /* Memory allocation */
    float** encoded_spikes = malloc(SIM_TIME_MS * sizeof(float*));
    for (int i = 0; i < SIM_TIME_MS; i++) {
        encoded_spikes[i] = malloc(NUM_INPUTS * sizeof(float));
    }

    
    printf("Starting training for %d epochs, relax and enjoy the show.\n\n", EPOCHS);

    float best_accuracy = 0.0f;
    float learning_rate = LEARNING_RATE;

    for (int i = 0; i < EPOCHS; i++) {
        int correct_predictions = 0;
        for (int j = 0; j < MNIST_TRAIN_ROWS; j++) {
            int label = mnist_data[j][0];
            int* pixels = &mnist_data[j][1];

            encode(pixels, encoded_spikes, NUM_INPUTS, SIM_TIME_MS);
            run_simulation(net, encoded_spikes, 1); // train_mode = 1

            int prediction = decode(net);
            if (prediction == label) correct_predictions++;

            float error[NUM_OUTPUTS] = {0};
            for (int k = 0; k < NUM_OUTPUTS; k++) {
                float target = (k == label) ? TARGET_SPIKE_COUNT : 0.0;
                error[k] = target - net->output_layer->spike_trace[k];
            }

            random_backprop_update(net, error, encoded_spikes, learning_rate);
	    
            //printf("Epoch %d, Image %d | Accuracy so far: %.4f%%\n", i + 1, j + 1, (float)correct_predictions / (j + 1) * 100.0); // Can be modified to only display every N steps
        }

    	float overall_accuracy = (float)correct_predictions / MNIST_TRAIN_ROWS * 100.0;
        printf("End of Epoch %d | Overall Accuracy: %.4f%%\n", i + 1, overall_accuracy);

        // --- Check for best accuracy and save weights ---
        if (overall_accuracy > best_accuracy) {
            best_accuracy = overall_accuracy;
            printf("---------------------------------------------------\n");
            printf("✨ New best accuracy! Saving weights to files...\n");
            printf("---------------------------------------------------\n\n");
            save_weights(net, "best_weights_input_hidden.csv", "best_weights_hidden_output.csv");
        }

	learning_rate *= LEARNING_DECAY;
	printf("New learning rate after decay = %f\n", learning_rate);

    }

    printf("\nTraining complete.\n");
    for (int i = 0; i < MNIST_TRAIN_ROWS; i++) free(mnist_data[i]);
    free(mnist_data);
    for (int i = 0; i < SIM_TIME_MS; i++) free(encoded_spikes[i]);
    free(encoded_spikes);
    free_network(net);

    return 0;
}

matrix* create_matrix(int rows, int cols) {
    matrix* m = malloc(sizeof(matrix));
    m->rows = rows;
    m->cols = cols;
    m->data = malloc(rows * sizeof(float*));
    for (int i = 0; i < rows; i++) {
        m->data[i] = calloc(cols, sizeof(float)); // Set all values to zero
    }
    return m;
}

void free_matrix(matrix* m) {
    if (!m) perror("No matrix memory to free!");
    for (int i = 0; i < m->rows; i++) {
        free(m->data[i]);
    }
    free(m->data);
    free(m);
}

void randomize_matrix(matrix* m, float scale) {
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            m->data[i][j] = scale * ((float)rand() / RAND_MAX - 0.5);
        }
    }
}

/* Regular Spiking Izhikevich Neurons */
ratbrain* create_ratbrain(int num_neurons) {
    ratbrain* layer = malloc(sizeof(ratbrain));
    layer->num_neurons = num_neurons;
    layer->v = malloc(num_neurons * sizeof(float));
    layer->u = malloc(num_neurons * sizeof(float));
    layer->a = malloc(num_neurons * sizeof(float));
    layer->b = malloc(num_neurons * sizeof(float));
    layer->c = malloc(num_neurons * sizeof(float));
    layer->d = malloc(num_neurons * sizeof(float));
    layer->spikes = calloc(num_neurons, sizeof(int));
    layer->spike_trace = calloc(num_neurons, sizeof(float));

    // TODO: Add NE and NI random allocation in a 4:1 ratio
    for (int i = 0; i < num_neurons; i++) {
        layer->a[i] = 0.02f;
        layer->b[i] = 0.2f;
        layer->c[i] = -65.0f;
        layer->d[i] = 8.0f;
        layer->v[i] = -65.0f; // Initial potential
        layer->u[i] = layer->b[i] * layer->v[i];
    }
    return layer;
}

void free_ratbrain(ratbrain* layer) {
    if (!layer) {perror("No layer memory to free!"); return;}
    free(layer->v);
    free(layer->u);
    free(layer->a);
    free(layer->b);
    free(layer->c);
    free(layer->d);
    free(layer->spikes);
    free(layer->spike_trace);
    free(layer);
}

network* create_network(int num_inputs, int num_hidden, int num_outputs) {
    network* net = malloc(sizeof(network));
    net->hidden_layer = create_ratbrain(num_hidden);
    net->output_layer = create_ratbrain(num_outputs);

    net->syn_S1 = create_matrix(num_inputs, num_hidden);
    net->syn_H1 = create_matrix(num_hidden, num_outputs);
    net->random_backprop_matrix = create_matrix(num_outputs, num_hidden); // Dimensions are transposed so that backpropagration works appropriately 

    randomize_matrix(net->syn_S1, 0.1);
    randomize_matrix(net->syn_H1, 0.1);
    randomize_matrix(net->random_backprop_matrix, 1.0);

    return net;
}

void free_network(network* net) {
    if (!net) {perror("No network memory to free!"); return;}
    free_ratbrain(net->hidden_layer);
    free_ratbrain(net->output_layer);
    free_matrix(net->syn_S1);
    free_matrix(net->syn_H1);
    free_matrix(net->random_backprop_matrix);
    free(net);
}

/* Boilerplate */
int** load_mnist_data(const char* filename, int num_rows) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening MNIST file");
        return NULL;
    }

    int** data = malloc(num_rows * sizeof(int*));
    char line[MNIST_TRAIN_ROWS];
    int row = 0;
    fgets(line, sizeof(line), file);

    while (fgets(line, sizeof(line), file) && row < num_rows) {
        data[row] = malloc(MNIST_COLS * sizeof(int));
        char* token = strtok(line, ",");
        int col = 0;
        while (token) {
            data[row][col++] = atoi(token);
            token = strtok(NULL, ",");
        }
        row++;
    }
    fclose(file);
    return data;
}

void encode(int* image_pixels, float** encoded_spikes, int num_inputs, int time_ms) {
    float normalized_pixels[num_inputs];
    for (int i = 0; i < num_inputs; i++) {
        normalized_pixels[i] = image_pixels[i] / 255.0f;
    }

    for (int i = 0; i < time_ms; i++) {
        for (int j = 0; j < num_inputs; j++) {
            encoded_spikes[i][j] = ((float)rand() / RAND_MAX < normalized_pixels[j]) ? 1.0f : 0.0f;
        }
    }
}


void run_simulation(network* net, float** input_spikes, int train_mode) {
    for (int i = 0; i < net->hidden_layer->num_neurons; i++) {
        net->hidden_layer->v[i] = -65.0f;
        net->hidden_layer->u[i] = net->hidden_layer->b[i] * net->hidden_layer->v[i];
        net->hidden_layer->spike_trace[i] = 0.0f;
    }
    for (int i = 0; i < net->output_layer->num_neurons; i++) {
        net->output_layer->v[i] = -65.0f;
        net->output_layer->u[i] = net->output_layer->b[i] * net->output_layer->v[i];
        net->output_layer->spike_trace[i] = 0.0f;
    }

    // Simulation
    for (int t = 0; t < SIM_TIME_MS; t++) {
        float I_hidden[NUM_HIDDEN] = {0};
        for (int j = 0; j < NUM_HIDDEN; j++) {
            for (int i = 0; i < NUM_INPUTS; i++) {
                I_hidden[j] += input_spikes[t][i] * net->syn_S1->data[i][j];
            }
        }

        for (int i = 0; i < NUM_HIDDEN; i++) {
            net->hidden_layer->v[i] += 0.5 * (0.04 * powf(net->hidden_layer->v[i], 2) + 5 * net->hidden_layer->v[i] + 140 - net->hidden_layer->u[i] + I_hidden[i]);
            net->hidden_layer->v[i] += 0.5 * (0.04 * powf(net->hidden_layer->v[i], 2) + 5 * net->hidden_layer->v[i] + 140 - net->hidden_layer->u[i] + I_hidden[i]);
            net->hidden_layer->u[i] += net->hidden_layer->a[i] * (net->hidden_layer->b[i] * net->hidden_layer->v[i] - net->hidden_layer->u[i]);
            
            if (net->hidden_layer->v[i] >= 30.0f) {
                net->hidden_layer->spikes[i] = 1;
                net->hidden_layer->v[i] = net->hidden_layer->c[i];
                net->hidden_layer->u[i] += net->hidden_layer->d[i];
            } else {
                net->hidden_layer->spikes[i] = 0;
            }
            if(train_mode) net->hidden_layer->spike_trace[i] += net->hidden_layer->spikes[i];
        }

        float I_output[NUM_OUTPUTS] = {0};
        for (int j = 0; j < NUM_OUTPUTS; j++) {
            for (int i = 0; i < NUM_HIDDEN; i++) {
                I_output[j] += net->hidden_layer->spikes[i] * net->syn_H1->data[i][j];
            }
        }

        for (int i = 0; i < NUM_OUTPUTS; i++) {
            net->output_layer->v[i] += 0.5 * (0.04 * powf(net->output_layer->v[i], 2) + 5 * net->output_layer->v[i] + 140 - net->output_layer->u[i] + I_output[i]);
            net->output_layer->v[i] += 0.5 * (0.04 * powf(net->output_layer->v[i], 2) + 5 * net->output_layer->v[i] + 140 - net->output_layer->u[i] + I_output[i]);
            net->output_layer->u[i] += net->output_layer->a[i] * (net->output_layer->b[i] * net->output_layer->v[i] - net->output_layer->u[i]);

            if (net->output_layer->v[i] >= 30.0f) {
                net->output_layer->spikes[i] = 1;
                net->output_layer->v[i] = net->output_layer->c[i];
                net->output_layer->u[i] += net->output_layer->d[i];
            } else {
                net->output_layer->spikes[i] = 0;
            }
             if(train_mode) net->output_layer->spike_trace[i] += net->output_layer->spikes[i];
        }
    }
}

int decode(network* net) {
    int max_spikes = -1;
    int predicted_digit = -1;
    for (int i = 0; i < NUM_OUTPUTS; i++) {
        if (net->output_layer->spike_trace[i] > max_spikes) {
            max_spikes = net->output_layer->spike_trace[i];
            predicted_digit = i;
        }
    }
    return predicted_digit;
}

void random_backprop_update(network* net, float* error, float** input_spikes, float learning_rate) {
    for (int i = 0; i < NUM_HIDDEN; i++) {
        for (int j = 0; j < NUM_OUTPUTS; j++) {
            float delta_W = learning_rate * error[j] * net->hidden_layer->spike_trace[i];
            net->syn_H1->data[i][j] += delta_W;
        }
    }

    float hidden_error[NUM_HIDDEN] = {0};
    for(int i = 0; i < NUM_HIDDEN; i++) {
        for(int j = 0; j < NUM_OUTPUTS; j++) {
            hidden_error[i] += error[j] * net->random_backprop_matrix->data[j][i];
        }
    }

    float input_trace[NUM_INPUTS] = {0};
    for(int i=0; i < NUM_INPUTS; i++){
        for(int t=0; t < SIM_TIME_MS; t++){
            input_trace[i] += input_spikes[t][i];
        }
    }

    for (int i = 0; i < NUM_INPUTS; i++) {
        for (int j = 0; j < NUM_HIDDEN; j++) {
            float delta_W = learning_rate * hidden_error[j] * input_trace[i];
            net->syn_S1->data[i][j] += delta_W;
        }
    }
}


// In snn.c, add this new function at the end

/**
 * @brief Saves the network's weight matrices to specified files.
 * @param net The network containing the weights.
 * @param w1_filename Filename for the input-to-hidden weights.
 * @param w2_filename Filename for the hidden-to-output weights.
 */
void save_weights(network* net, const char* w1_filename, const char* w2_filename) {
    // --- Save Input -> Hidden Weights (syn_S1) ---
    FILE* fp1 = fopen(w1_filename, "w");
    if (fp1 == NULL) {
        perror("ERROR: Could not open file for saving input weights");
        return;
    }
    for (int i = 0; i < net->syn_S1->rows; i++) {
        for (int j = 0; j < net->syn_S1->cols; j++) {
            fprintf(fp1, "%f", net->syn_S1->data[i][j]);
            if (j < net->syn_S1->cols - 1) {
                fprintf(fp1, ",");
            }
        }
        fprintf(fp1, "\n");
    }
    fclose(fp1);

    // --- Save Hidden -> Output Weights (syn_H1) ---
    FILE* fp2 = fopen(w2_filename, "w");
    if (fp2 == NULL) {
        perror("ERROR: Could not open file for saving hidden weights");
        return;
    }
    for (int i = 0; i < net->syn_H1->rows; i++) {
        for (int j = 0; j < net->syn_H1->cols; j++) {
            fprintf(fp2, "%f", net->syn_H1->data[i][j]);
            if (j < net->syn_H1->cols - 1) {
                fprintf(fp2, ",");
            }
        }
        fprintf(fp2, "\n");
    }
    fclose(fp2);
}

/*
 *	@author:	Crow Crossman (Updated by Google Gemini)
 *	@description:	This is a basic model of the "Izhikevich Model" to be used as a basis of neuron activity in SNNs.
 * This version includes dynamic memory allocation to handle larger neuron counts
 * and removes unnecessary print statements.
 *
 *	references:
 *		"Simple Model of Spiking Neurons" by Eugene M. Izhikevich
 */

#include <stdio.h>
#include <stdlib.h> // Required for malloc and free
#include <time.h>   // Required for time() and srand()
#include <string.h> // Required for memset (for initialization)

#define TIME_MS 1000 // Number of milliseconds to simulate
#define NE 8000      // Number of excitatory neurons (example for total 10000)
#define NI 2000      // Number of inhibitory neurons (example for total 10000)

// Function prototypes
// Removed stats() as it was commented out in main and not essential for core logic.
// print_data now takes float** for dynamically allocated 2D array.
int print_data(int rows, int cols, float** data);
float deltav(float v, float u, float I);
float deltau(float a, float b, float u, float v);
float resetv(float v, float c);
float resetu(float u, float d);

int main(void) {
    srand(time(NULL));

    // Calculate total number of neurons
    long num_neurons = NE + NI;

    // Declare pointers for dynamic memory allocation
    float *r, *a, *b, *c, *d, *v, *u, *I;
    int *fired;
    float **S;    // Synaptic weight matrix
    float **data; // Data recording for output.csv

    // --- Dynamic Memory Allocation ---
    // Allocate 1D arrays
    r = (float *)malloc(num_neurons * sizeof(float));
    a = (float *)malloc(num_neurons * sizeof(float));
    b = (float *)malloc(num_neurons * sizeof(float));
    c = (float *)malloc(num_neurons * sizeof(float));
    d = (float *)malloc(num_neurons * sizeof(float));
    v = (float *)malloc(num_neurons * sizeof(float));
    u = (float *)malloc(num_neurons * sizeof(float));
    I = (float *)malloc(num_neurons * sizeof(float));
    fired = (int *)malloc(num_neurons * sizeof(int));

    // Basic error checking for 1D array allocations
    if (!r || !a || !b || !c || !d || !v || !u || !I || !fired) {
        perror("Failed to allocate 1D arrays");
        // Attempt to free any memory that might have been allocated
        free(r); free(a); free(b); free(c); free(d); free(v); free(u); free(I); free(fired);
        return 1; // Indicate error
    }

    // Allocate 2D array S (synaptic weight matrix)
    S = (float **)malloc(num_neurons * sizeof(float *));
    if (S == NULL) {
        perror("Failed to allocate rows for S matrix");
        // Cleanup 1D arrays before exit
        free(r); free(a); free(b); free(c); free(d); free(v); free(u); free(I); free(fired);
        return 1;
    }
    for (long i = 0; i < num_neurons; i++) {
        S[i] = (float *)malloc(num_neurons * sizeof(float));
        if (S[i] == NULL) {
            perror("Failed to allocate columns for S matrix");
            // Cleanup: free previously allocated rows and 1D arrays
            for (long j = 0; j < i; j++) free(S[j]);
            free(S);
            free(r); free(a); free(b); free(c); free(d); free(v); free(u); free(I); free(fired);
            return 1;
        }
    }

    // Allocate 2D array data (for output recording)
    data = (float **)malloc(TIME_MS * sizeof(float *));
    if (data == NULL) {
        perror("Failed to allocate rows for data matrix");
        // Cleanup S matrix and 1D arrays
        for (long i = 0; i < num_neurons; i++) free(S[i]); free(S);
        free(r); free(a); free(b); free(c); free(d); free(v); free(u); free(I); free(fired);
        return 1;
    }
    for (int i = 0; i < TIME_MS; i++) {
        data[i] = (float *)malloc(num_neurons * sizeof(float));
        if (data[i] == NULL) {
            perror("Failed to allocate columns for data matrix");
            // Cleanup: free previously allocated data rows, S matrix, and 1D arrays
            for (int j = 0; j < i; j++) free(data[j]);
            free(data);
            for (long k = 0; k < num_neurons; k++) free(S[k]); free(S);
            free(r); free(a); free(b); free(c); free(d); free(v); free(u); free(I); free(fired);
            return 1;
        }
    }
    // --- End Dynamic Memory Allocation ---

    // Initialize dynamically allocated arrays. malloc does not zero memory.
    memset(fired, 0, num_neurons * sizeof(int)); // Efficiently zero out fired array
    for (int t = 0; t < TIME_MS; t++) {
        memset(data[t], 0, num_neurons * sizeof(float)); // Zero out each row of data
    }

    // Populate synapse weight matrix
    for (long x = 0; x < num_neurons; x++) {
        for (long y = 0; y < NE; y++) {
            S[x][y] = 0.5 * ((float)rand() / RAND_MAX);
        }
        for (long z = NE; z < num_neurons; z++) {
            S[x][z] = (-1) * ((float)rand() / RAND_MAX);
        }
    }

    // Populate excitatory neurons
    for (long i = 0; i < NE; i++) {
        r[i] = (float)rand() / RAND_MAX;
        a[i] = 0.02f;
        b[i] = 0.2f;
        c[i] = -65.0f + 15.0f * (r[i] * r[i]);
        d[i] = 8.0f + (-6.0f * (r[i] * r[i]));
        v[i] = -65.0f;
        u[i] = (b[i]) * (v[i]);
    }

    // Populate inhibitory neurons
    for (long i = NE; i < num_neurons; i++) {
        r[i] = (float)rand() / RAND_MAX;
        a[i] = 0.02f + 0.08f * r[i];
        b[i] = 0.25f - 0.05f * r[i];
        c[i] = -65.0f;
        d[i] = 2.0f;
        v[i] = -65.0f;
        u[i] = (b[i]) * (v[i]);
    }

    // Run time steps in ms
    for (int t = 0; t < TIME_MS; t++) {
        for (long i = 0; i < num_neurons; i++) {
            float R = (float)rand() / RAND_MAX;
            I[i] = (i < NE) ? 5.0f * R : 2.0f * R;

            if (v[i] >= 30.0f) {
                fired[i] = 1;
                data[t][i] = v[i]; // Record voltage at time of firing
                v[i] = resetv(v[i], c[i]); // Reset v
                u[i] = resetu(u[i], d[i]); // Reset u
            }
        }

        for (long i = 0; i < num_neurons; i++) {
            float sum_s = 0.0f;
            for (long j = 0; j < num_neurons; j++) {
                if (fired[j] == 1) {
                    sum_s += S[i][j];
                }
            }
            I[i] = I[i] + sum_s;

            // Update voltage and recovery variable
            // Using a single step as per the provided C code structure,
            // though the paper's MATLAB implies a two-half-step integration for v
            v[i] = v[i] + deltav(v[i], u[i], I[i]);
            u[i] = u[i] + deltau(a[i], b[i], u[i], v[i]);
        }

        // Reset fired status for next time step
        for (long i = 0; i < num_neurons; i++) {
            fired[i] = 0;
        }
    }

    // Print final v and u values (optional, uncomment if needed for debugging)
    /*
    printf("v values: ");
    for (long i = 0; i < num_neurons; i++) printf("%f ", v[i]);
    printf("\n--------------------\n");

    printf("u values: ");
    for (long i = 0; i < num_neurons; i++) printf("%f ", u[i]);
    printf("\n--------------------\n");
    */

    // Print recorded data to CSV file
    print_data(TIME_MS, num_neurons, data);

    // --- Free dynamically allocated memory ---
    free(r);
    free(a);
    free(b);
    free(c);
    free(d);
    free(v);
    free(u);
    free(I);
    free(fired);

    for (long i = 0; i < num_neurons; i++) {
        free(S[i]);
    }
    free(S);

    for (int i = 0; i < TIME_MS; i++) {
        free(data[i]);
    }
    free(data);
    // --- End Freeing Memory ---

    return 0;
}

// Function to print data to output.csv
// Now takes float** for dynamically allocated 2D array
int print_data(int rows, int cols, float** data) {
    FILE *fp = fopen("output.csv", "w");
    if (fp == NULL) {
        perror("Unable to open output.csv for writing");
        return 1;
    }

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            fprintf(fp, "%.2f", data[i][j]);
            if (j < cols - 1) {
                fprintf(fp, ",");
            }
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
    return 0;
}

// Differential equation for membrane potential v'
float deltav(float v, float u, float I) {
    return 0.04f * (v * v) + 5.0f * v + 140.0f - u + I;
}

// Differential equation for recovery variable u'
float deltau(float a, float b, float u, float v) {
    return a * ((b * v) - u);
}

// Reset function for membrane potential v after a spike
float resetv(float v_current_unused, float c) { // v_current_unused is kept for signature consistency
    return c;
}

// Reset function for recovery variable u after a spike
float resetu(float u, float d) {
    return u + d;
}


/*
 * @author: Crow Crossman
 * @description: This is a basic model of the "Izhikevich Model" to be used as a basis of neuron activity in SNNs.
 *	
 * references:
 * "Simple Model of Spiking Neurons" by Eugene M. Izhikevich
 *
 * version history:
 * 1.0 (27 Jun 2025) Initial model based on referenced paper above.
 * 1.1 (28 Jun 2025) Implementation of random distribution of Ne and Ni neurons and dynamic memory allocation for larger models.
 * 1.2 (01 Jul 2025) Added CLI support to change number of neurons and time frame.
 * 1.3 (11 Jul 2025) Conversion to header file for API support.
 */

#include <time.h>
#include <stdlib.h>
#include "ratbrain.h"

#define BRAIN_VARS(X) X(r) X(a) X(b) X(c) X(d) X(v) X(u) X(I)


brain_t* create_brain(int ex, int in) {
    srand(time(NULL));
    brain_t* new_brain = (brain_t*)malloc(sizeof(brain_t));
    new_brain->ne = ex;
    new_brain->ni = in;
    new_brain->total_neurons = ex + in;
    new_brain->Vm = THRESHOLD;
    /* Memory allocation for variables in the brain. */
    #define X(var) new_brain->var = (float*)malloc((ex + in)*sizeof(float));
    BRAIN_VARS(X)
    #undef X

    /* Initiate spike train to zeroes. This is the encoded value of the input to the brain. */
    new_brain->spike_train = (int*)calloc(ex + in, sizeof(int));

    /* Memory allocation for fully connected synapse matrix as a 1D array. */
    new_brain->S = (float*)malloc(((ex + in)*(ex + in))*sizeof(float));

    new_brain->rastor = (float*)malloc((ex + in)*TIME*sizeof(float));

    for (int i = 0; i < (ex + in); i++) {
        if (i < ex) { // populate excitatory neuron values
            new_brain->r[i] = (float)rand()/RAND_MAX;
            new_brain->a[i] = A;
            new_brain->b[i] = B;
            new_brain->c[i] = C + 15*(new_brain->r[i]*new_brain->r[i]);
            new_brain->d[i] = D + (-6*(new_brain->r[i]*new_brain->r[i]));
            new_brain->v[i] = V;
            new_brain->u[i] = new_brain->b[i]*new_brain->v[i];
        } else { // populate inhibitory neuron values
            new_brain->r[i] = (float)rand()/RAND_MAX;
            new_brain->a[i] = A + 0.08*new_brain->r[i];
            new_brain->b[i] = 0.25 - 0.05*new_brain->r[i];
            new_brain->c[i] = C;
            new_brain->d[i] = 2;
            new_brain->v[i] = V;
            new_brain->u[i] = new_brain->b[i]*new_brain->v[i];
        }
    }

    for (int i = 0; i < (ex + in)*(ex + in); i++) {
        if (i % (ex + in) < ex) {
            new_brain->S[i] = (0.5)*((float)rand()/RAND_MAX);
        } else {
            new_brain->S[i] = (-1)*((float)rand()/RAND_MAX);
        }
    }

    return new_brain;
}

brain_t* run_sim(brain_t* model, int time) {
    for (int t = 0; t < time; t++) {
        for (int i = 0; i < model->total_neurons; i++) {
            model->I[i] = (i < model->ne) ? 5*randnorm() : 2*randnorm();
            if (model->v[i] >= model->Vm) {
                model->spike_train[i] = 1;
                model->rastor[(t*model->total_neurons)+i] = 1;
                model->v[i] = model->c[i];
                model->u[i] += model->d[i];
            }
        }
        for (int i = 0; i < model->total_neurons; i++) {
            float sum = 0;
            if (model->spike_train[i] == 1) {
                for (int j = 0; j < model->total_neurons; j++) {
                    sum += model->S[(i*model->total_neurons)+j];
                }
            }
            model->I[i] += sum;
            model->v[i] += 0.5*(deltav(model->v[i], model->u[i], model->I[i]));
            model->v[i] += 0.5*(deltav(model->v[i], model->u[i], model->I[i]));
            model->u[i] += deltau(model->a[i], model->b[i], model->u[i], model->v[i]);
            model->spike_train[i] = 0;
        }        
    }

    return model;
}

void free_brain(brain_t* model) {
    #define X(var) free(model->var);
    BRAIN_VARS(X)
    #undef X
    free(model->spike_train);
    free(model->S);
    free(model->rastor);
    free(model);
}

/* Based on MATLAB randn() which picks a random number from a normal distribution. This is just a simple function that leverages central limit theorem. */ 
float randnorm(void) {
	float sum = 0.0;
	for (int i = 0; i < 20; i++) {
		sum = sum + (float)rand()/(float)RAND_MAX;
	}
	return sum - 10.0;
}

/* Incremental change in v returning delta v */
float deltav(float v, float u, float I) {
	v = 0.04*(v*v) + 5*v + 140 - u + I;
	return v;
};

/* Incremental change in u returning delta u */
float deltau(float a, float b, float u, float v){
	u = a*((b*v) - u);
	return u;
};

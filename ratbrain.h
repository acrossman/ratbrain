#ifndef RATBRAIN_H
#define RATBRAIN_H

#define NE 80
#define NI 20
#define A 0.02
#define B 0.2
#define C -65
#define D 8
#define V -65
#define THRESHOLD 30
#define TIME 1000

typedef struct {
    int ne;
    int ni;
    int total_neurons;
    float Vm;
    float *r, *a, *b, *c, *d, *v, *u, *I, *S, *rastor;
    int *spike_train;
} brain_t;

/* Prototype functions */
brain_t* create_brain(int excitatory_neurons, int inhibitory_neurons);
brain_t* run_sim(brain_t* model, int time);
void free_brain(brain_t* model);
float randnorm(void);
float deltav(float v, float u, float I);
float deltau(float a, float b, float u, float v);
float resetv(float v, float c);
float resetu(float u, float d);

#endif

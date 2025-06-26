/*
 *	@author:	Crow Crossman
 *	@description:	This is a basic model of the "Izhikevich Model" to be used as a basis of neuron activity in SNNs.
 *	
 *	references:
 *		"Simple Model of Spiking Neurons" by Eugene M. Izhikevich
 *
 *	version history:
 *	1.0	Initial model based on referenced paper above.
 *	1.1	Implementation of random distribution of Ne and Ni neurons and dynamic memory allocation for larger models.
 *	1.2	Added CLI support to change number of neurons and time frame.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define A 0.02
#define B 0.2
#define C -65
#define D 8
#define V -65

#define THRESHOLD 30

/* Prototype functions */
float randn();
float deltav(float v, float u, float I);
float deltau(float a, float b, float u, float v);
float resetv(float v, float c);
float resetu(float u, float d);
int print_data(int rows, int cols, float** data);

/* Mainline */
int main(int argc, char** argv) {
	srand(time(NULL));

	int NE = 800;	// NE:NI is 4:1 based on model.
	int NI = 200;
	int n_cnt = NE+NI;
	int time = 1000;	// Time in ms

	/* CLI arg handler */
	switch(argc) {
		case 2: NE = atol(argv[1]); break;
		case 3: NE = atol(argv[1]); NI = atol(argv[2]); break;
		case 4: NE = atol(argv[1]); NI = atol(argv[2]); time = atol(argv[3]); break;
		default: break;
	}
	
	/* mem allocation */
	float *r, *a, *b, *c, *d, *v, *u, *I;
	int *fired;

	float **S = (float**)malloc((n_cnt)*sizeof(float*));
	for (int i = 0; i < n_cnt; i++) {
		S[i] = (float*)malloc((n_cnt)*sizeof(float));
	}
	
	float **data = (float**)malloc(time*sizeof(float*));
	for (int i = 0; i < time; i++) {
		data[i] = (float*)malloc(n_cnt*sizeof(float));
	}
	
	r = (float*)malloc((n_cnt)*sizeof(float));
    	a = (float*)malloc((n_cnt)*sizeof(float));
    	b = (float*)malloc((n_cnt)*sizeof(float));
    	c = (float*)malloc((n_cnt)*sizeof(float));
    	d = (float*)malloc((n_cnt)*sizeof(float));
    	v = (float*)malloc((n_cnt)*sizeof(float));
    	u = (float*)malloc((n_cnt)*sizeof(float));
    	I = (float*)malloc((n_cnt)*sizeof(float));
    	fired = (int*)malloc((n_cnt)*sizeof(int));

	for (int i = 0; i < n_cnt; i++) {
		fired[i] = 0;
	}

	for (int i = 0; i < time; i++) {
		for (int j = 0; j < n_cnt; j++) {
			data[i][j] = 0.0;
		}
	}
	
	/* Populate synapse weight matrix */
	for (int x = 0; x < n_cnt; x++) {
		for (int y = 0; y < NE; y++) {
			S[x][y] = 0.5 * ((float)rand()/RAND_MAX);
		}
		for (int z = 0 + NE; z < n_cnt; z++) {
			S[x][z] = (-1)*((float)rand()/RAND_MAX);
		} 
	}

	/* Populate excitatory neurons */
	for (int i = 0; i < NE; i++) {
		r[i] = (float)rand()/RAND_MAX;
		a[i] = A;
		b[i] = B;
		c[i] = C + 15*(r[i]*r[i]);
		d[i] = D + (-6*(r[i]*r[i]));
		v[i] = V;
		u[i] = (b[i])*(v[i]);
	}
	
	/* Populate inhibitory neurons */
	for (int i = 0 + NE; i < n_cnt; i++) {
		r[i] = (float)rand()/RAND_MAX;
		a[i] = A + 0.08*r[i];
		b[i] = 0.25 - 0.05*r[i];
		c[i] = C;
		d[i] = 2;
		v[i] = V;
		u[i] = (b[i])*(v[i]);
	}


	/* Run time sim */
	for (int t = 0; t < time; t++) {

		for (int i = 0; i < n_cnt; i++) {
			I[i] = (i < NE) ? 5*randn() : 2*randn();
			if (v[i] >= THRESHOLD) {	// Check threshold voltage
				fired[i] = 1;
				data[t][i] = v[i];
				v[i] = c[i];
				u[i] = u[i]+d[i];	
			}
		}
		
		for (int i = 0; i < n_cnt; i++) {
			float sum = 0;
			for (int j = 0; j < n_cnt; j++) {
				if (fired[j] == 1) sum = sum + S[i][j];
			}
			I[i] = I[i] + sum;
			v[i] = v[i] + 0.5*(deltav(v[i], u[i], I[i]));
			v[i] = v[i] + 0.5*(deltav(v[i], u[i], I[i]));
			u[i] = u[i] + deltau(a[i], b[i], u[i], v[i]);
		}

		for (int i = 0; i < n_cnt; i++) {fired[i] = 0;}
	}

	print_data(time, n_cnt, data);

	for (int i = 0; i < n_cnt; i++) {
		free(S[i]);
	}
	for (int i = 0; i < time; i++) {
		free(data[i]);
	}
	free(S);
	free(data);
	free(r);
	free(a);
	free(b);
	free(c);
	free(d);
	free(v);
	free(u);
	free(I);
	free(fired);

	return 0;
}

/* Based on MATLAB randn() which picks a random number from a normal distribution. This is just a simple function that leverages central limith theorem. */ 
float randn() {
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

/* Boiler plate print to csv funciton */
int print_data(int rows, int cols, float** data) {
	FILE *fp = fopen("output.csv", "w");
    	if (fp == NULL) {
        	perror("Unable to open file");
        	return 1;
    	}

    	for (int i = 0; i < rows; ++i) {
        	for (int j = 0; j < cols; ++j) {
	            	fprintf(fp, "%.2f", data[i][j]);
        	    	if (j < cols - 1) fprintf(fp, ",");
        	}
        	fprintf(fp, "\n");
    	}

    	fclose(fp);
	return 0;
}


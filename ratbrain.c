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
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TIME_MS 1000
#define NE 800
#define NI 200

float deltav(float v, float u, float I);
float deltau(float a, float b, float u, float v);
float resetv(float v, float c);
float resetu(float u, float d);
int print_data(int rows, int cols, float** data);

int main(void) {
	srand(time(NULL));
	int n_cnt = NE+NI;
	
	float *r, *a, *b, *c, *d, *v, *u, *I;
	int *fired;

	float **S = (float**)malloc((n_cnt)*sizeof(float*));
	for (int i = 0; i < n_cnt; i++) {
		S[i] = (float*)malloc((n_cnt)*sizeof(float));
	}
	
	float **data = (float**)malloc(TIME_MS*sizeof(float*));
	for (int i = 0; i < TIME_MS; i++) {
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

	for (int i = 0; i < TIME_MS; i++) {
		for (int j = 0; j < n_cnt; j++) {
			data[i][j] = 0.0;
		}
	}
	
	// Populate synapse weight matrix
	for (int x = 0; x < n_cnt; x++) {
		for (int y = 0; y < NE; y++) {
			S[x][y] = 0.5 * ((float)rand()/RAND_MAX);
		}
		for (int z = 0 + NE; z < n_cnt; z++) {
			S[x][z] = (-1)*((float)rand()/RAND_MAX);
		} 
	}

	// Populate excitatory neurons
	for (int i = 0; i < NE; i++) {
		r[i] = (float)rand()/RAND_MAX;
		a[i] = 0.02;
		b[i] = 0.2;
		c[i] = -65 + 15*(r[i]*r[i]);
		d[i] = 8 + (-6*(r[i]*r[i]));
		v[i] = -65;
		u[i] = (b[i])*(v[i]);
	}
	
	// Populate inhibitory neurons
	for (int i = 0 + NE; i < n_cnt; i++) {
		r[i] = (float)rand()/RAND_MAX;
		a[i] = 0.02 + 0.08*r[i];
		b[i] = 0.25 - 0.05*r[i];
		c[i] = -65;
		d[i] = 2;
		v[i] = -65;
		u[i] = (b[i])*(v[i]);
	}


	// Run time steps in ms
	for (int t = 0; t < TIME_MS; t++) {

		for (int i = 0; i < n_cnt; i++) {
			float R = (float)rand()/RAND_MAX;
			I[i] = (i < NE) ? 5*R : 2*R;

			if (v[i] >= 30) {
				fired[i] = 1;
				data[t][i] = v[i];
				v[i] = resetv(v[i],c[i]);
				u[i] = resetu(u[i],d[i]);	
			}
		}
		
		for (int i = 0; i < n_cnt; i++) {
			int sum = 0;
			for (int j = 0; j < n_cnt; j++) {
				if (fired[j] == 1) sum = sum + S[i][j];
			}
			I[i] = I[i] + sum;
			v[i] = v[i] + deltav(v[i], u[i], I[i]);
			u[i] = u[i] + deltau(a[i], b[i], u[i], v[i]);
		}

		for (int i = 0; i < n_cnt; i++) {fired[i] = 0;}
	}

	print_data(TIME_MS, n_cnt, data);

	for (int i = 0; i < n_cnt; i++) {
		free(S[i]);
	}
	for (int i = 0; i < TIME_MS; i++) {
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

float deltav(float v, float u, float I) {
	v = 0.04*(v*v) + 5*v + 140 - u + I;
	return v;
};

float deltau(float a, float b, float u, float v){
	u = a*((b*v) - u);
	return u;
};

float resetv(float v, float c) {
	v = c;
	return v;
};

float resetu(float u, float d){
	u = u + d;
	return u;
};

int print_data(int rows, int cols, float** data) {
	FILE *fp = fopen("output.csv", "w");
    	if (fp == NULL) {
        	perror("Unable to open file");
        	return 1;
    	}

    	for (int i = 0; i < TIME_MS; ++i) {
        	for (int j = 0; j < NE+NI; ++j) {
	            	fprintf(fp, "%.2f", data[i][j]);
        	    	if (j < NE+NI - 1) fprintf(fp, ",");
        	}
        	fprintf(fp, "\n");
    	}

    	fclose(fp);
	return 0;
}


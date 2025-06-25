/*
 *	@author:	Crow Crossman
 *	@description:	This is a basic model of the "Izhikevich Model" to be used as a basis of neuron activity in SNNs.
 *
 *	references:
 *		"Simple Model of Spiking Neurons" by Eugene M. Izhikevich
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TIME_MS 1000
#define NE 800
#define NI 200

void stats(float r, float a, float b, float c, float d, float v, float u);
int print_data(int rows, int cols, float data[rows][cols]);
float deltav(float v, float u, float I);
float deltau(float a, float b, float u, float v);
float resetv(float v, float c);
float resetu(float u, float d);

int main(void) {
	srand(time(NULL));

	float r[NE+NI], a[NE+NI], b[NE+NI], c[NE+NI], d[NE+NI], v[NE+NI], u[NE+NI], I[NE+NI], S[NE+NI][NE+NI];
	int fired[NE+NI] = {0};
	float data[TIME_MS][NE+NI] = {0};
	
	// Populate synapse weight matrix
	for (int x = 0; x < NE+NI; x++) {
		for (int y = 0; y < NE; y++) {
			S[x][y] = 0.5 * ((float)rand()/RAND_MAX);
//			printf("%f|", S[x][y]);
		}
		for (int z = 0 + NE; z < NE+NI; z++) {
			S[x][z] = (-1)*((float)rand()/RAND_MAX);
//			printf("%f|", S[x][z]);
		} 
//		printf("\n\n");
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
	for (int i = 0 + NE; i < NE+NI; i++) {
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

//		printf("t = %d\t", t);

		for (int i = 0; i < NE+NI; i++) {
			float R = (float)rand()/RAND_MAX;
			I[i] = (i < NE) ? 5*R : 2*R;

			if (v[i] >= 30) {
				fired[i] = 1;
				data[t][i] = v[i];
				v[i] = resetv(v[i],c[i]);
//				printf("New v[%d] value: %f\n", i, v[i]);
				u[i] = resetu(u[i],d[i]);	
//				printf("New u[%d] value: %f\n", i, u[i]);
			}
/*
			if (fired[i] == 1) {
				printf("|");
			} else {
				printf(" ");
			}
*/
		}
		
//		printf("\n");
		for (int i = 0; i < NE+NI; i++) {
			int sum = 0;
			for (int j = 0; j < NE+NI; j++) {
				if (fired[j] == 1) sum = sum + S[i][j];
			}
			I[i] = I[i] + sum;
//			printf("New I[%d] value: %f\n", i, I[i]);
//			printf("Delta V = %f\n", deltav(v[i], u[i], I[i]));
			v[i] = v[i] + deltav(v[i], u[i], I[i]);
//			printf("New v[%d] value: %f\n", i, v[i]);
//			printf("Delta U = %f\n", deltau(a[i], b[i], u[i], v[i]));
			u[i] = u[i] + deltau(a[i], b[i], u[i], v[i]);
//			printf("New u[%d] value: %f\n", i, u[i]);
		}

		for (int i = 0; i < NE+NI; i++) {fired[i] = 0;}

//		printf("\n");
	}

	//stats(r, a, b, c, d, v, u);
	out:
/*
	printf("r values: ");
	for (int i = 0; i < NE+NI; i++) printf("%f ", r[i]);
	printf("\n--------------------\n");

	printf("a values: ");
	for (int i = 0; i < NE+NI; i++) printf("%f ", a[i]);
	printf("\n--------------------\n");

	printf("b values: ");
	for (int i = 0; i < NE+NI; i++) printf("%f ", b[i]);
	printf("\n--------------------\n");

	printf("c values: ");
	for (int i = 0; i < NE+NI; i++) printf("%f ", c[i]);
	printf("\n--------------------\n");

	printf("d values: ");
	for (int i = 0; i < NE+NI; i++) printf("%f ", d[i]);
	printf("\n--------------------\n");

	printf("v values: ");
	for (int i = 0; i < NE+NI; i++) printf("%f ", v[i]);
	printf("\n--------------------\n");

	printf("u values: ");
	for (int i = 0; i < NE+NI; i++) printf("%f ", u[i]);
	printf("\n--------------------\n");
*/
	print_data(TIME_MS, NE+NI, data);
	return 0;
}

void stats(float r, float a, float b, float c, float d, float v, float u) {
	printf("r = %f\n", r);
	printf("a = %f\n", a);
	printf("b = %f\n", b);
	printf("c = %f\n", c);
	printf("d = %f\n", d);
	printf("v = %f\n", v);
	printf("u = %f\n", u);
}

int print_data(int rows, int cols, float data[rows][cols]) {
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

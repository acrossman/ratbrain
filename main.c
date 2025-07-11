#include <stdio.h>
#include <stdlib.h>
#include "ratbrain.h"

void thinking(void) {
    brain_t* my_rat = create_brain(NE,NI);
    run_sim(my_rat, TIME);

    
    for(int i = 0; i < TIME * my_rat->total_neurons; i++) {
        if(my_rat->rastor[i] == 1) {printf("|");}
        else {printf(" ");}
	if(i > 0 && i % my_rat->total_neurons == 0) printf("\n");
    }
    printf("\n");

    free_brain(my_rat);
}

int main(void) {
    while(1) {thinking();}
    return 0;
}

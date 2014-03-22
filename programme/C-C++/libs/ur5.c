#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "ur5.h"


void print_ur5_data(struct ur5_data * ur5_d){
	puts("------------------ UR 5 Data ------------------");
    printf("positions:\t1 => %.2f,\n\t\t2 => %.2f,\n\t\t3 => %.2f,\n\t\t4 => %.2f,\n\t\t5 => %.2f,\n\t\t6 => %.2f\n\n",
			ur5_d->position[0],
			ur5_d->position[1],
			ur5_d->position[2],
			ur5_d->position[3],
			ur5_d->position[4],
			ur5_d->position[5]);
	puts("-----------------------------------------------");
}

struct ur5_data * create_ur5_data(void){
	struct ur5_data * ur5_d = (struct ur5_data *) calloc(1, sizeof(struct ur5_data));
	int i= 0;

    for(i = 0; i < ANZAHL_GELENKE; i++)  ur5_d->position[i] = 0.0;
    for(i = 0; i < ANZAHL_GELENKE; i++)  ur5_d->read_position[i] = 0.0;
    for(i = 0; i < ANZAHL_GELENKE; i++)  ur5_d->zero_vector[i] = 0.0;
    for(i = 0; i < ANZAHL_GELENKE; i++)  ur5_d->position_vector[i] = 0.0;
    for(i = 0; i < ANZAHL_GELENKE; i++)  ur5_d->speed_vector[i] = 0.0;
    for(i = 0; i < ANZAHL_GELENKE; i++)  ur5_d->current_vector[i] = 0.0;
    ur5_d->connect = 0;
	return ur5_d;
}



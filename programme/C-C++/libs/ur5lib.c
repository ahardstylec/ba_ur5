#include "ur5lib.h"
#include "../libs/ur_kin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <endian.h>
#include <time.h>
#include "../libs/base_utils.h"

int max_t_ipo;
int move_iteration=0;
bool debug=false;
char *debug_filename_host= "/usr/src/ur5-server/ptp_test.csv";
char *debug_filename_local="./ptp_test.csv";

double rad_to_deg(double rad){ return rad*(180.0/M_PI);}
double deg_to_rad(double deg){ return deg*(M_PI/180.0);}

void move_linear(const double * q, double * pz, MoveLinearPacket *move_linear_packet){
    move_iteration++;
    int profil = SINOIDENPROFIL;
    double distance_pz = fabs(sqrt(POW(pz[X])+POW(pz[Y])+POW(pz[Z])));
    // if distanze of pz to zero vector is higher than the max reach length of the roboter the pz distance,
    // must be limitet to the max length possible

    // q are joint positions. so we must forward them
    forward(q, move_linear_packet->T);
    double pst[3];
    pst[X]= move_linear_packet->T[TX];
    pst[Y]= move_linear_packet->T[TY];
    pst[Z]= move_linear_packet->T[TZ];
    memcpy((void *) &move_linear_packet->qst, q, sizeof(q));

    if(distance_pz > MAX_SE){
        double faktor = MAX_SE / (distance_pz);
        distance_pz=MAX_SE;
        int i=0;
        for(i=0; i< 3; i++){
            pz[i] = pz[i]*faktor;
        }
        distance_pz = fabs(sqrt(POW(pz[X])+POW(pz[Y])+POW(pz[Z])));
    }

    move_linear_packet->se = sqrt( (POW(pst[X]) - (2*pz[X]*pst[X]) + POW(pz[X])) +
                                   (POW(pst[Y]) - (2*pz[Y]*pst[Y]) + POW(pz[Y])) +
                                   (POW(pst[Z]) - (2*pz[Z]*pst[Z]) + POW(pz[Z])));


    double t[4];
    bzero(&t, sizeof(t));
    move_linear_packet->a_max = ACCELERATION_MAX;
    move_linear_packet->v_max = VELOCITY_MAX;

    double factor= (profil == RAMPENPROFIL) ?  1.0 : 2.0;

    // adapt tb and tv to the Iterpolationphase
    adapt_t_ipo(move_linear_packet, factor);
    move_linear_packet->interpolations= (int) round(move_linear_packet->te / T_IPO);


    if(debug) print_linear_debug(move_linear_packet, profil);

}






void interpolation_sin_linear(MoveLinearPacket * move_linear_packet){
    if((move_linear_packet->point_in_time < 0) || (move_linear_packet->se < deg_to_rad(EPS))){
        move_linear_packet->acceleration = 0.0;
        move_linear_packet->velocity = 0.0;
        move_linear_packet->position = 0.0;
    }else if(move_linear_packet->point_in_time <= move_linear_packet->tb){ // acceleration phase
        move_linear_packet->acceleration = move_linear_packet->a_max * POW(sin((M_PI / move_linear_packet->tb) * move_linear_packet->point_in_time));
        move_linear_packet->velocity = move_linear_packet->a_max * ( (move_linear_packet->point_in_time*.5) - ( (move_linear_packet->tb / (4*M_PI)) * sin( ( (2*M_PI)/ move_linear_packet->tb )* move_linear_packet->point_in_time) ) );
        move_linear_packet->position = move_linear_packet->a_max*( (POW(move_linear_packet->point_in_time)/4) + (( POW(move_linear_packet->tb) / (8*POW(M_PI) ))* (cos(((2*M_PI)/move_linear_packet->tb)*move_linear_packet->point_in_time)-1))  );

    }else if(move_linear_packet->point_in_time< move_linear_packet->tv){ // constant velocity phase
        move_linear_packet->acceleration = 0.0;
        move_linear_packet->velocity = move_linear_packet->v_max;
        move_linear_packet->position = move_linear_packet->v_max * (move_linear_packet->point_in_time -( 0.5 * move_linear_packet->tb ) );
    }else if (move_linear_packet->point_in_time <= move_linear_packet->te){ //deceleration phase
        move_linear_packet->acceleration = ((-move_linear_packet->a_max) * POW(sin((M_PI / move_linear_packet->tb) *(move_linear_packet->point_in_time-move_linear_packet->tv))));
        move_linear_packet->velocity = move_linear_packet->v_max -move_linear_packet->a_max*(((move_linear_packet->point_in_time-move_linear_packet->tv)/2) - (move_linear_packet->tb/(4*M_PI) * sin( ((2*M_PI)/move_linear_packet->tb)*(move_linear_packet->point_in_time-move_linear_packet->tv) )));
        move_linear_packet->position = move_linear_packet->a_max/2*((move_linear_packet->te * (move_linear_packet->point_in_time+move_linear_packet->tb)) -
                                                          (( (POW(move_linear_packet->point_in_time)) + POW(move_linear_packet->te) + (2*POW(move_linear_packet->tb)) ) /2) +
                                                          (POW(move_linear_packet->tb)/(4*POW(M_PI))) * (1 -  cos( (((2*M_PI) / move_linear_packet->tb)*(move_linear_packet->point_in_time-move_linear_packet->tv))) ));
    }else{
        move_linear_packet->acceleration = 0.0;
        move_linear_packet->velocity = 0.0;
        move_linear_packet->position = move_linear_packet->se;
    }

    move_linear_packet->T[TX] =(double) (move_linear_packet->qst[X] + move_linear_packet->position * ( (move_linear_packet->qz[X] - move_linear_packet->qst[X]) / move_linear_packet->se ));
    move_linear_packet->T[TY] =(double) (move_linear_packet->qst[Y] + move_linear_packet->position * ( (move_linear_packet->qz[Y] - move_linear_packet->qst[Y]) / move_linear_packet->se ));
    move_linear_packet->T[TZ] =(double) (move_linear_packet->qst[Z] + move_linear_packet->position * ( (move_linear_packet->qz[Z] - move_linear_packet->qst[Z]) / move_linear_packet->se ));
    return;
}

void interpolation_ramp_ptp(MovePTPPacket * move_ptp_packet){
    int k;
    for(k=0; k<6; k++){
        if((move_ptp_packet->point_in_time < 0) || (move_ptp_packet->se[k] < deg_to_rad(EPS))){
            move_ptp_packet->pva.acceleration[k] = 0.0;
            move_ptp_packet->pva.velocity[k] = 0.0;
            move_ptp_packet->pva.position[k] = 0.0;
        }else if(move_ptp_packet->point_in_time <= move_ptp_packet->tb[k]){
            move_ptp_packet->pva.acceleration[k] = move_ptp_packet->a_max[k];
            move_ptp_packet->pva.velocity[k] = (move_ptp_packet->a_max[k] * move_ptp_packet->point_in_time);
            move_ptp_packet->pva.position[k] = ( 0.5 * move_ptp_packet->a_max[k] * POW(move_ptp_packet->point_in_time));
        }else if(move_ptp_packet->point_in_time < move_ptp_packet->tv[k]){
            move_ptp_packet->pva.acceleration[k]=  0.0;
            move_ptp_packet->pva.velocity[k] = move_ptp_packet->v_max[k];
            move_ptp_packet->pva.position[k] = ((move_ptp_packet->v_max[k] * move_ptp_packet->point_in_time) - (0.5 *( POW(move_ptp_packet->v_max[k]) / move_ptp_packet->a_max[k] )));
        }else if (move_ptp_packet->point_in_time <= move_ptp_packet->te[k]){
            move_ptp_packet->pva.acceleration[k]= -move_ptp_packet->a_max[k];
            move_ptp_packet->pva.velocity[k] = (move_ptp_packet->v_max[k] -move_ptp_packet->a_max[k]* (move_ptp_packet->point_in_time - move_ptp_packet->tv[k]));
            move_ptp_packet->pva.position[k] = (move_ptp_packet->v_max[k] * move_ptp_packet->tv[k] - ( move_ptp_packet->a_max[k]*0.5*((move_ptp_packet->te[k] - move_ptp_packet->point_in_time)*(move_ptp_packet->te[k] - move_ptp_packet->point_in_time)) ));
        }else{
            move_ptp_packet->pva.acceleration[k] = 0.0;
            move_ptp_packet->pva.velocity[k] = 0.0;
            move_ptp_packet->pva.position[k] =  move_ptp_packet->se[k];
        }

        move_ptp_packet->pva.position[k]= move_ptp_packet->qst[k]+(move_ptp_packet->signse[k]*move_ptp_packet->pva.position[k]);
        move_ptp_packet->pva.acceleration[k] *= move_ptp_packet->signse[k];
        move_ptp_packet->pva.velocity[k] *= move_ptp_packet->signse[k];
    }
    return;
}

void interpolation_sin_ptp(MovePTPPacket * move_pva){
    int k;
    for(k=0; k<6; k++){
        if((move_pva->point_in_time < 0) || (move_pva->se[k] < deg_to_rad(EPS))){
            move_pva->pva.acceleration[k] = 0.0;
            move_pva->pva.velocity[k] = 0.0;
            move_pva->pva.position[k] = 0.0;
        }else if(move_pva->point_in_time <= move_pva->tb[k]){ // acceleration phase
            move_pva->pva.acceleration[k] = move_pva->a_max[k] * POW(sin((M_PI / move_pva->tb[k]) * move_pva->point_in_time));
            move_pva->pva.velocity[k] = move_pva->a_max[k] * ( (move_pva->point_in_time*.5) - ( (move_pva->tb[k] / (4*M_PI)) * sin( ( (2*M_PI)/ move_pva->tb[k] )* move_pva->point_in_time) ) );
            move_pva->pva.position[k] = move_pva->a_max[k]*( (POW(move_pva->point_in_time)/4) + (( POW(move_pva->tb[k]) / (8*POW(M_PI) ))* (cos(((2*M_PI)/move_pva->tb[k])*move_pva->point_in_time)-1))  );

        }else if(move_pva->point_in_time < move_pva->tv[k]){ // constant velocity phase
            move_pva->pva.acceleration[k] = 0.0;
            move_pva->pva.velocity[k] = move_pva->v_max[k];
            move_pva->pva.position[k] = move_pva->v_max[k] * (move_pva->point_in_time -( 0.5 * move_pva->tb[k] ) );
        }else if (move_pva->point_in_time <= move_pva->te[k]){ //deceleration phase
            move_pva->pva.acceleration[k] = ((-move_pva->a_max[k]) * POW(sin((M_PI / move_pva->tb[k]) *(move_pva->point_in_time-move_pva->tv[k]))));
            move_pva->pva.velocity[k] = move_pva->v_max[k] -move_pva->a_max[k]*(((move_pva->point_in_time-move_pva->tv[k])/2) - (move_pva->tb[k]/(4*M_PI) * sin( ((2*M_PI)/move_pva->tb[k])*(move_pva->point_in_time-move_pva->tv[k]) )));
            move_pva->pva.position[k] = move_pva->a_max[k]/2*((move_pva->te[k] * (move_pva->point_in_time+move_pva->tb[k])) -
                                                              (( (POW(move_pva->point_in_time)) + POW(move_pva->te[k]) + (2*POW(move_pva->tb[k])) ) /2) +
                                                              (POW(move_pva->tb[k])/(4*POW(M_PI))) * (1 -  cos( (((2*M_PI) / move_pva->tb[k])*(move_pva->point_in_time-move_pva->tv[k]))) ));
        }else{
            move_pva->pva.acceleration[k] = 0.0;
            move_pva->pva.velocity[k] = 0.0;
            move_pva->pva.position[k] = move_pva->se[k];
        }
        move_pva->pva.position[k]= move_pva->qst[k]+(move_pva->signse[k]*move_pva->pva.position[k]);
        move_pva->pva.acceleration[k] *= move_pva->signse[k];
        move_pva->pva.velocity[k] *= move_pva->signse[k];
    }
    return;
}

void clear_rad_360(double *position){
    int i;
    for(i=0;i<6; i++) position[i]= position[i]<0.0 ?  (180 - fmod(fabs(position[i]), 180.0)) : (position[i]>180.0 ? -180 + fmod(fabs(position[i]), 180.0) : position[i]);
}

void position_to_deg(double* position){
    int i;
    for(i=0; i<6;i++) position[i]= rad_to_deg(position[i]);
    return;
}

void position_to_rad(double* position){
    int i;
    for(i=0; i<6;i++) position[i]= deg_to_rad(position[i]);
    return;
}


void calc_ptp_profile(const double * q, double * pz, MovePTPPacket *move_pva, int profil){
    //    double T[16];
    double factor= (profil == RAMPENPROFIL) ?  1.0 : 2.0;
    if(debug)
        printf("factor %f, profil %s\n", factor, profil == RAMPENPROFIL ? "Rampenprofil" : "Sinoidenprofil");
    double se= 0.0;
    int i, l=0;
    for(i=0; i < 6; i++){

        move_pva->qst[i]=q[i];
        move_pva->qz[i] =pz[i];
        move_pva->v_max[i]= deg_to_rad(VELOCITY_RAD_MAX);
        move_pva->a_max[i]= deg_to_rad(ACCELERATION_RAD_MAX);
        se = pz[i] - q[i];
        move_pva->signse[i]= se < 0.0 ? -1.0 : (se > 0  ? 1.0 : 0.0);
        move_pva->se[i] = fabs(se);

        if(move_pva->se[i] < deg_to_rad(EPS)){
            move_pva->tb[i]=0.0;
            move_pva->te[i]=0.0;
            move_pva->tv[i]=0.0;
            move_pva->v_max[i]=0.0;
            move_pva->a_max[i]=0.0;
        }else{
            if(move_pva->v_max[i] > sqrt((move_pva->se[i]*move_pva->a_max[i])/factor))
                move_pva->v_max[i] = sqrt((move_pva->se[i]*move_pva->a_max[i])/factor);

            move_pva->tb[i] = (factor*move_pva->v_max[i])/move_pva->a_max[i];
            move_pva->te[i] = (move_pva->se[i] / move_pva->v_max[i]) + move_pva->tb[i];
            move_pva->tv[i] = move_pva->te[i] - move_pva->tb[i];
        }
        if(move_pva->te[i] > move_pva->te[l]){
            l=i;
        }
    }
    if(debug){
        puts("------- before t ipo -----------");
        printf("acceleration: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n", move_pva->a_max[0], move_pva->a_max[1], move_pva->a_max[2], move_pva->a_max[3], move_pva->a_max[4], move_pva->a_max[5]);
        printf("velocity: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n", move_pva->v_max[0], move_pva->v_max[1], move_pva->v_max[2], move_pva->v_max[3], move_pva->v_max[4], move_pva->v_max[5]);
        printf("se: %.5f, %.5f, %.5f, %.5f, %.5f, %.5f\n", move_pva->se[0], move_pva->se[1], move_pva->se[2], move_pva->se[3], move_pva->se[4], move_pva->se[5]);
        printf("time: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n", move_pva->te[0], move_pva->te[1], move_pva->te[2], move_pva->te[3], move_pva->te[4], move_pva->te[5]);
        printf("time accelerate: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n", move_pva->tb[0], move_pva->tb[1], move_pva->tb[2], move_pva->tb[3], move_pva->tb[4], move_pva->tb[5]);
        printf("time decelerate: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n", move_pva->tv[0], move_pva->tv[1], move_pva->tv[2], move_pva->tv[3], move_pva->tv[4], move_pva->tv[5]);
        printf("longest join is: %d\n",l);
        puts("------- after t ipo -----------");
    }
    if(move_pva->se[l] < deg_to_rad(EPS)){
        // adapt longest joint to interpolationphase
        move_pva->tb[l] = (floor((factor*move_pva->v_max[l])/(move_pva->a_max[l]*T_IPO))+1)*T_IPO;
        move_pva->tv[l] = (floor(move_pva->se[l]/(move_pva->v_max[l]*T_IPO))+1)*T_IPO;
        move_pva->te[l] = move_pva->tv[l]+move_pva->tb[l];
        move_pva->v_max[l] = move_pva->se[l]/move_pva->tv[l];
        move_pva->a_max[l] = (factor*move_pva->se[l])/(move_pva->tv[l]*move_pva->tb[l]);
    }

//     adapt te of all joints to te of longest joint
    for(i=0;i<6;i++){
        if(l != i && move_pva->se[i] > deg_to_rad(EPS)){
            move_pva->te[i] = move_pva->te[l];
            if(profil == RAMPENPROFIL){
                move_pva->v_max[i] = (move_pva->a_max[i]*move_pva->te[l])/2 - sqrt(((POW(move_pva->a_max[i])*POW(move_pva->te[l]))/4)-(move_pva->se[i]*move_pva->a_max[i]));
            }else{
                move_pva->v_max[i] = (move_pva->a_max[i]*move_pva->te[l])/4 - sqrt(((POW(move_pva->a_max[i])*POW(move_pva->te[l]))-(8*move_pva->se[i]*move_pva->a_max[i]))/16);
            }

            if(move_pva->v_max[i] > sqrt((move_pva->se[i]*move_pva->a_max[i])/factor))
                move_pva->v_max[i] = sqrt((move_pva->se[i]*move_pva->a_max[i])/factor);

            move_pva->tv[i] = floor(move_pva->se[i]/(move_pva->v_max[i]*T_IPO))*T_IPO;
            move_pva->tb[i] = move_pva->te[i] - move_pva->tv[i];
            move_pva->v_max[i] = move_pva->se[i] / move_pva->tv[i];
            move_pva->a_max[i] = (factor*move_pva->v_max[i]) /  move_pva->tb[i];
        }
    }

    move_pva->interpolations= move_pva->se[l] < deg_to_rad(EPS) ? 0 :
                                                                 (int) round(move_pva->te[l] / T_IPO);


    if(debug) print_ptp_debug(move_pva, profil);

    return;
}

void adapt_t_ipo(MoveLinearPacket *move_linear_packet, double factor){
    double v_max_corr = sqrt((move_linear_packet->se *move_linear_packet->a_max)/factor);
    if(move_linear_packet->v_max > v_max_corr){
        move_linear_packet->v_max = v_max_corr;
        move_linear_packet->tb = floor((factor * move_linear_packet->v_max)/(move_linear_packet->a_max*T_IPO))*T_IPO;
        move_linear_packet->tv = floor(move_linear_packet->se/(move_linear_packet->v_max*T_IPO))*T_IPO;
    }else{
        move_linear_packet->tb = round((factor*move_linear_packet->v_max)/(move_linear_packet->a_max*T_IPO))*T_IPO;
        move_linear_packet->tv = round(move_linear_packet->se/(move_linear_packet->v_max*T_IPO))*T_IPO;
    }
    // ----- round up tb and tv so we have a synchron T_IPO

    move_linear_packet->te = move_linear_packet->tv + move_linear_packet->tb;
    move_linear_packet->v_max = (move_linear_packet->se/move_linear_packet->tv);
    move_linear_packet->a_max = (factor*move_linear_packet->se) / (move_linear_packet->tv * move_linear_packet->tb);
    return;
}

void print_ptp_debug(MovePTPPacket * move_pva, int profil){

    printf("interpolations %d\n", move_pva->interpolations);
    printf("acceleration: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n", move_pva->a_max[0], move_pva->a_max[1], move_pva->a_max[2], move_pva->a_max[3], move_pva->a_max[4], move_pva->a_max[5]);
    printf("velocity: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n", move_pva->v_max[0], move_pva->v_max[1], move_pva->v_max[2], move_pva->v_max[3], move_pva->v_max[4], move_pva->v_max[5]);
    printf("se: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n", move_pva->se[0], move_pva->se[1], move_pva->se[2], move_pva->se[3], move_pva->se[4], move_pva->se[5]);
    printf("time: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n", move_pva->te[0], move_pva->te[1], move_pva->te[2], move_pva->te[3], move_pva->te[4], move_pva->te[5]);
    printf("time accelerate: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n", move_pva->tb[0], move_pva->tb[1], move_pva->tb[2], move_pva->tb[3], move_pva->tb[4], move_pva->tb[5]);
    printf("time decelerate: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n", move_pva->tv[0], move_pva->tv[1], move_pva->tv[2], move_pva->tv[3], move_pva->tv[4], move_pva->tv[5]);
    FILE *fd;
    fd = fopen(debug_filename_local, "w");
    fprintf(fd,"time;acceleration0;velocity0;distance0;acceleration1;velocity1;distance1;");
    fprintf(fd,"acceleration2;velocity2;distance2;acceleration3;velocity3;distance3;");
    fprintf(fd,"acceleration4;velocity4;distance4;acceleration5;velocity5;distance5;\n");
    int i;
    for(i=-1; i <= move_pva->interpolations+1; i++){
        move_pva->point_in_time= (double) i * T_IPO;
        if(profil == RAMPENPROFIL){
            interpolation_ramp_ptp(move_pva);
        }else{
            interpolation_sin_ptp(move_pva);
        }
        fprintf(fd, "%.3f;%f;%f;%f;", move_pva->point_in_time, move_pva->pva.acceleration[0], move_pva->pva.velocity[0], move_pva->pva.position[0]);
        fprintf(fd, "%f;%f;%f;", move_pva->pva.acceleration[1], move_pva->pva.velocity[1], move_pva->pva.position[1]);
        fprintf(fd, "%f;%f;%f;", move_pva->pva.acceleration[2], move_pva->pva.velocity[2], move_pva->pva.position[2]);
        fprintf(fd, "%f;%f;%f;", move_pva->pva.acceleration[3], move_pva->pva.velocity[3], move_pva->pva.position[3]);
        fprintf(fd, "%f;%f;%f;", move_pva->pva.acceleration[4], move_pva->pva.velocity[4], move_pva->pva.position[4]);
        fprintf(fd, "%f;%f;%f;\n",  move_pva->pva.acceleration[5], move_pva->pva.velocity[5], move_pva->pva.position[5]);
    }
    fclose(fd);
}

void print_linear_debug(MoveLinearPacket * move_linear_packet, int profil){
    FILE *fd = fopen("linear_test.csv", "w");
    fprintf(fd,"time;x;y;z;acceleration;velocity;distance\n");
    fprintf(fd, "0.000;%.2f;%.2f;%.2f;0.000;0.000;0.000\n", move_linear_packet->qst[X], move_linear_packet->qst[Y], move_linear_packet->qst[Z]);

    printf("acceleration_end_time: %f\n", move_linear_packet->tb);
    printf("deceleration_time_start: %f\n",move_linear_packet->tv);
    printf("\nfull_time: %f\n", move_linear_packet->te);
    printf("se: %f\n", move_linear_packet->se);
    printf("iterations: %d\n", move_linear_packet->interpolations);

    int i;
    double q_sols[48];

    for(i=0; i <= max_t_ipo; i++){
        move_linear_packet->point_in_time= (double) i * T_IPO;
        if(profil == RAMPENPROFIL){
         //   interpolation_ramp_linear(move_linear_packet);
        }else{
            interpolation_sin_linear(move_linear_packet);
        }
        inverse(move_linear_packet->T, q_sols, 1);
        fprintf(fd, "%.3f;%f;%f;%f;%f;%f;\n", move_linear_packet->point_in_time, move_linear_packet->T[TX], move_linear_packet->T[TX], move_linear_packet->T[TX], move_linear_packet->acceleration, move_linear_packet->velocity, move_linear_packet->position);
    }
    fclose(fd);
}


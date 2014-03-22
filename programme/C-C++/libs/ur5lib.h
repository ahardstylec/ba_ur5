#ifndef _UR5LIB_H
#define _UR5LIB_H

#include "ur_kin.h"
#include <time.h>
#include <endian.h>
#include <sys/types.h>
#include "../libs/base_utils.h"
#include "../libs/robotinterface.h"
#include <math.h>

#define POW(v) ((v)*(v))

#define VELOCITY_MAX 1.0 // m/s
#define ACCELERATION_MAX 1.2 // m/s²
#define VELOCITY_RAD_MAX 60.0   // rad/s
#define ACCELERATION_RAD_MAX 80.0 // rad/s²

#define EPS 0.2 //

#define RAMPENPROFIL 0
#define SINOIDENPROFIL 1
#define T_IPO 0.008
#define TB 0
#define TV 1
#define TE 2
#define T_NOW 3
#define POSITION 0
#define VELOCITY 1
#define ACCELERATION 2
#define X 0
#define Y 1
#define Z 2
#define ACCELERATION_PHASE 1
#define VELOCITY_PHASE 2
#define DECELERATION_PHASE 3
#define MAX_SE 1.5
#define TX 3
#define TY 7

#define TZ 11

extern bool debug;
extern char *debug_filename;

double rad_to_deg(double rad);
double deg_to_rad(double deg);

// Move_Linear arguments:
// pst is start vector [X, Y, Z, RX, RY, RZ]
// pz is end vector [X, Y, Z, RX, RY, RZ]
// pva is position, velocity, acceleration vector for ur5 interface
// profil is Rampenprofil(0) or Sinuidenprofil(!=0 || 1)
void move_linear(const double * q, double * pz, MoveLinearPacket *move_pva);

// Move_Ptp arguments:
// q is start vector of joints 1-6
// pz is end vector of joints 1-6
// profil is Rampenprofil(0) or Sinuidenprofil(!=0 || 1)
void calc_ptp_profile(const double *q, double *pz, MovePTPPacket *, int profil);

void adapt_t_ipo(MoveLinearPacket *move_linear_packet, double factor);

void interpolation_ramp_ptp(MovePTPPacket*);

void interpolation_sin_ptp(MovePTPPacket*);
void interpolation_sin_linear(MoveLinearPacket*);

void clear_rad_360(double*);

void print_ptp_debug(MovePTPPacket *, int );
void print_linear_debug(MoveLinearPacket * move_linear_packet, int);

void position_to_deg(double*);
void position_to_rad(double*);


#endif

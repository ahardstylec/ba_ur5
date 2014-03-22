#ifndef UR5_H
#define UR5_H

#define ANZAHL_GELENKE 6
#define TCP_X 0
#define TCP_Y 1
#define TCP_Z 2

struct ur5_data{
    double read_position[ANZAHL_GELENKE];
    double position[ANZAHL_GELENKE];
    double zero_vector[ANZAHL_GELENKE];
    double position_vector[ANZAHL_GELENKE];
    double speed_vector[ANZAHL_GELENKE];
    double current_vector[ANZAHL_GELENKE];
    short int connect;
};


void print_ur5_data(struct ur5_data *);

struct ur5_data * create_ur5_data(void);

#endif

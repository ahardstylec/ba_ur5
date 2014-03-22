#ifndef _UR5_REALTIME_CI_H
#define _UR5_REALTIME_CI_H

#include <endian.h>
#include <sys/types.h>

double rad_to_deg(double rad);
double deg_to_rad(double deg);

#define UR5_REALTIME_CI_SIZE 812

#pragma pack(1)



// Struct for data send by Polyscope Program Server over TCP/IP on port 30003
// Data is send in 125hz interval
struct ur5_data_rci
{
    double time;                            //  8           1             	Time elapsed since the controller was started
    double q_target[6];                     //  48          2-7             Target joint positions
    double qd_target[6];                    //  48          8-13            Target joint velocities
    double qdd_target[6];                   //  48          14-19           Target joint accelerations
    double i_target[6];                     //  48          20-25           Target joint currents
    double m_target[6];                     //  48          26-31           Target joint moments (torques)
    double q_actual[6];                     //  48          32-37           Actual joint positions
    double qd_actual[6];                    //  48          38-43           Actual joint velocities
    double i_actual[6];                     //  48          44-49           Actual joint currents
    double tool_accelerometer_values[3];    //  24          50-53           Tool x,y and z accelerometer values (software version 1.7)
    double unused[15];                      //  120         54-67           Unused
    double tcp_force[6];                    //  48          68-73           Generalised forces in the TCP
    double tool_vector[6];                  //  48          74-79           Cartesian coordinates of the tool: (x,y,z,rx,ry,rz), where rx, ry and rz is a rotation vector representation of the tool orientation
    double tcp_speed[6];                    //  48          80-85           Speed of the tool given in cartesian coordinates
    double digital_in;                      //  48          86              Current state of the digital inputs.NOTE: these are bits encoded as int64_t, e.g. a value of 5 corresponds to bit 0 and bit 2 set high
    double motor_temperatures[6];           //  48          87-92           Temperature of each joint in degrees celcius
    double controller_timer;                //  8           93              Controller realtime thread execution time
    double test_value;                      //  8           94              A value used by Universal Robots software only
    double robot_mode;                      //  8           95              Robot control mode (see PolyScopeProgramServer)
    double joint_modes[6];                  //  48          96-101          Joint control modes (see PolyScopeProgramServer) (only from software version 1.8 and on)
};

struct ur5_realtime_ci
{
    int length;
    union{
        struct ur5_data_rci data;
        u_int64_t data_packed[sizeof(struct ur5_data_rci)/sizeof(u_int64_t)];
    } data_union;
};


#pragma pack()

struct ur5_data_rci * parse_ur5_realtime_ci(struct ur5_realtime_ci *, char *);

void write_data_to_file(const char* filename, struct ur5_data_rci * ur5_d);

#endif

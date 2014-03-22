/*
This proram provides a TCP network interface to some functions
of the UR_COMM_API. It has to run on the Universal Robots
controller box.

This program is very experimental and the use of it can lead to 
damage of personal and hardware. DO NOT USE IT UNDER ANY 
CIRCUMSTANCES!!! It is just for demonstration purposes to show
how to access the UR_COMM_API.

The code, the protocol, the logic, etc., may be subject to change 
without any further notice.

Copyright: NTNU, SINTEF 2012
Authors: Johannes Schrimpf and Morten Lind
*/

/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "../libs/robotinterface.h"
#include "../libs/microprocessor_commands.h"
#include "../libs/microprocessor_definitions.h"
#include "../libs/startup_utils.h"
#include "../libs/base_utils.h"
#include "../libs/ur5lib.h"

bool initialized = false;
// initialize_direction is set 1 or -1 for the direction in initializing mode
double initialize_direction=-1.0;

void open_interface(){
    int retryCounter;
    puts("Opening robotinterface (real robot)");
    printf("Mode: %d\n",robotinterface_get_robot_mode());
    /* open robotinterface */
    if (robotinterface_open(0) == FALSE) {
        puts("robotinterface_open() failed");
        exit(EXIT_FAILURE);
    }
    
    retryCounter = 100;
    while (retryCounter>0 && !robotinterface_is_robot_connected()) {
        robotinterface_read_state_blocking();
        robotinterface_command_velocity(zero_vector);
        robotinterface_send();
        retryCounter--;
    }

    if (!robotinterface_is_robot_connected()) {
        robotinterface_close();
        puts("Real robot not connected");
        exit(EXIT_FAILURE);
    }
    puts("Enabling robot after securtity stop");
    while (robotinterface_get_robot_mode() == 4){
        robotinterface_read_state_blocking();
        robotinterface_unlock_security_stop();
        robotinterface_command_velocity(zero_vector);
        robotinterface_send();
//        printf("Mode: %d\n",robotinterface_get_robot_mode());
    }

    // special wait time
    for(retryCounter=0;retryCounter<200;retryCounter++){
        robotinterface_read_state_blocking();
        robotinterface_command_empty_command();
        robotinterface_send();
    }
    puts("Interface opened");
//    printf("Mode: %d\n",robotinterface_get_robot_mode());
}

void power_up(){
    if(robotinterface_is_power_on_robot()) return;

    puts("Power up robot");
//    printf("Mode: %d\n",robotinterface_get_robot_mode());
    int maxAttemptsToPowerup = 5;
    int retryCounter,i;
    for (i = 0; i < maxAttemptsToPowerup; ++i) {
        retryCounter=250;
        robotinterface_read_state_blocking();
        robotinterface_command_empty_command();
        robotinterface_power_on_robot();
        robotinterface_send();
        while (retryCounter > 0) {
            robotinterface_read_state_blocking();
            robotinterface_command_velocity(zero_vector);
            robotinterface_send();
            if (robotinterface_is_power_on_robot()) {
                break;
            }
            retryCounter--;
        }
    }

    if (!robotinterface_is_power_on_robot()) {
        robotinterface_close();
        puts("Unable to power up robot");
        exit(EXIT_FAILURE);
    }

    // special wait time
    for(i=0;i<200;i++){
        robotinterface_read_state_blocking();
        robotinterface_command_empty_command();
        robotinterface_send();
    }

    puts("Powered Up");
//    printf("Mode: %d\n",robotinterface_get_robot_mode());
}

void unlock_security_stop_mode(){
    if (robotinterface_get_robot_mode() == ROBOT_SECURITY_STOPPED_MODE) {
//        puts("Robot in security stop mode, unlocking...");

        while (robotinterface_get_robot_mode() == 4){
            robotinterface_read_state_blocking();
            robotinterface_unlock_security_stop();
            robotinterface_command_velocity(zero_vector);
            robotinterface_send();
//            printf("Mode: %d\n",robotinterface_get_robot_mode());
        }
    }
}

void unlock_emergency_stop_mode(){
    if (robotinterface_get_robot_mode() == ROBOT_EMERGENCY_STOPPED_MODE) {
        puts("Robot in emergency stop mode, unlocking...");
        power_up();
        int j;
        for(j=0; j<200; j++){
            robotinterface_read_state_blocking();
            robotinterface_unlock_security_stop();
            robotinterface_command_velocity(zero_vector);
            robotinterface_send();
            if(robotinterface_get_robot_mode() != ROBOT_EMERGENCY_STOPPED_MODE) break;
        }
    }
}


bool initialize(void){
	int i = 0;
    if(robotinterface_get_robot_mode() != ROBOT_INITIALIZING_MODE){
        power_up();
        unlock_security_stop_mode();
        unlock_emergency_stop_mode();
    }
    puts("Initializing robot");
    /// Set zero velocity and acceleration as guard
    int j;
    for (j=0; j<6; ++j) {
		pva_packet.velocity[j] = 0.0;
		pva_packet.acceleration[j] = 0.0;
    }
	do {
        ++i;
        robotinterface_read_state_blocking();
        int j;
        for (j=0; j<6; ++j) {
            pva_packet.velocity[j] = ((robotinterface_get_joint_mode(j) == JOINT_INITIALISATION_MODE)) ? (initialize_direction)* 0.1 : 0.0;
        }
        robotinterface_command_velocity(pva_packet.velocity);
        robotinterface_send();
    } while (robotinterface_get_robot_mode() == ROBOT_INITIALIZING_MODE && exit_flag == false);
    puts(" Done!");
    if (robotinterface_get_robot_mode() != ROBOT_RUNNING_MODE){
//            unlock_emergency_stop_mode();
//            unlock_security_stop_mode();
//        printf("Mode: %d\n", robotinterface_get_robot_mode());
//        return initialize(retry_counter+1);
        robotinterface_close();
        exit(EXIT_FAILURE);
    }
    // Save original posotion 
    puts("Sampling initial position");

    robotinterface_read_state_blocking();
    robotinterface_get_actual_position(pva_packet.position);

    JointVector currents_actual;
    JointVector currents_target;
    robotinterface_get_actual_current(currents_actual);
    robotinterface_get_target_current(currents_target);
    printf("actual current: %.7f, %.7f, %.7f, %.7f, %.7f, %.7f\n", currents_actual[0], currents_actual[1], currents_actual[2], currents_actual[3], currents_actual[4], currents_actual[5]);
    printf("target current: %.7f, %.7f, %.7f, %.7f, %.7f, %.7f\n", currents_target[0], currents_target[1], currents_target[2], currents_target[3], currents_target[4], currents_target[5]);

    robotinterface_command_velocity(zero_vector);
    robotinterface_send();

    /// Initialize also the last pva status
    memcpy((void *) &last_pva_packet, (void *) &pva_packet, sizeof(PVAPacket));

    initialized = true;
    return true;
}


void settle(){

    printf("Waiting for settling\n");
    /// Warning: It is assumed that the real position has been sampled in pva_packet.position.
    int i;

    for(i=0 ; i<125*5 ; i++){
        if(i%125 == 0) 
//            printf("Waiting for settling, %d\n",(int) i/125);
        robotinterface_read_state_blocking();
        if (initialized){
            robotinterface_command_position_velocity_acceleration(pva_packet.position, zero_vector, zero_vector);
        } else{
            robotinterface_command_velocity(zero_vector);
        }
        robotinterface_send();
    }
}


void set_wrench(){
    /// Warning: It is assumed that the real position has been sampled in pva_packet.position.
    robotinterface_read_state_blocking();    
    robotinterface_set_tcp_wrench(tcp_wrench_vector, 1);
    robotinterface_command_velocity(zero_vector);
    robotinterface_send();
    int i;
    for(i=0; i< 125; i++){
        robotinterface_read_state_blocking();
        robotinterface_command_velocity(zero_vector);
        robotinterface_send();
    }
}

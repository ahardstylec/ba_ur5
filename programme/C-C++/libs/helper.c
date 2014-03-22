#include "helper.h"
#include "tcphelper.h"
#include "../libs/robotinterface.h"
#include "../libs/base_utils.h"
#include "../libs/startup_utils.h"
#include "../libs/ur5lib.h"


void error(char *msg) {
    perror(msg);
    exit(0);
}

void read_args(int argc, char ** argv,  struct connection_data *server){
    int c;
    char * garbage;
    while ((c = getopt (argc, argv, "p:i:j:")) != -1)
        switch (c)
        {
        case 'p':
            server->port = (int) strtol(optarg, &garbage, 10);
            if(server->port >65535){
                puts("port %d port is not valid it must be below 65535");
                exit(1);
            }else if(server->port < 1024 && geteuid() != 0){
                puts("ports below 1024 are priviliged! Run programm as root\n");
                exit(1);
            }
            break;
        case 'i':
            server->iflag=1;
            server->interface = optarg;
            break;
        case 'j':
            server->initialize_direction=atof(optarg);
            break;
        case '?':
            if (optopt == 'p' || optopt == 'i')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
            exit(1);
        default:
            abort ();
        }


    if(server->iflag == 0){
        if(optind < argc){
            server->ip_addr_s = argv[optind];
        }else{
            server->interface = "lo";
            server->ip_addr_s = "127.0.0.1";
        }
        if(inet_aton(server->ip_addr_s, &server->serv_addr.sin_addr) == 0){
            printf("ip address not valid");
            exit(1);
        }
    }else{
        server->ip_addr_s = get_interface_addr(server->interface, &server->serv_addr.sin_addr);
    }

    bzero((char *) &server->serv_addr, sizeof(server->serv_addr));
    server->serv_addr.sin_family = AF_INET;
    server->serv_addr.sin_port = htons(server->port);
}

void timeval_diff(struct timeval *difference, struct timeval* start_time, struct timeval* end_time){
    struct timeval temp_diff;

    if(difference==NULL){
        difference=&temp_diff;
    }

    difference->tv_sec = end_time->tv_sec -start_time->tv_sec;
    difference->tv_usec= end_time->tv_usec-start_time->tv_usec;

    /* Using while instead of if below makes the code slightly more robust. */

    while(difference->tv_usec<0)
    {
        difference->tv_usec+=1000000;
        difference->tv_sec -=1;
    }

} /* timeval_diff() */


void move_to_position(double *qz){
    PVAPacket pva;
    MovePTPPacket move_pva_packet;
    bzero(&move_pva_packet,sizeof(move_pva_packet));
    int profil = RAMPENPROFIL;

    // get the actual position so we can be sure where to start calculating the ptp profile
    robotinterface_read_state_blocking();
    robotinterface_get_actual_position(pva.position);
    memcpy(&last_pva_packet, &pva_packet, sizeof(pva_packet));
    robotinterface_command_velocity(zero_vector);
    robotinterface_send();

    // calculate ptp profile
    calc_ptp_profile(pva.position, qz, &move_pva_packet, profil);

    printf("interpolations: %d\n", move_pva_packet.interpolations);

    int i;
    JointVector currents_actual;
    JointVector currents_target;
    JointVector average_current;
    JointVector currents_actual_average;
    JointVector currents_target_average;
    int j;
    for(j=0;j<6;j++){
        currents_actual_average[j]=0.0;
        currents_target_average[j]=0.0;
        average_current[j]=0.0;
    }

    for(i=0; i < move_pva_packet.interpolations+1; i++){
        robotinterface_read_state_blocking();
        robotinterface_get_actual(pva_packet.position, pva_packet.velocity);
        robotinterface_get_actual_current(currents_actual);
        robotinterface_get_target_current(currents_target);


        if(robotinterface_is_security_stopped()) {
            robotinterface_get_actual_current(currents_actual);
            robotinterface_get_target_current(currents_target);
            printf("actual current: %.7f, %.7f, %.7f, %.7f, %.7f, %.7f\n", currents_actual[0], currents_actual[1], currents_actual[2], currents_actual[3], currents_actual[4], currents_actual[5]);
            printf("target current: %.7f, %.7f, %.7f, %.7f, %.7f, %.7f\n", currents_target[0], currents_target[1], currents_target[2], currents_target[3], currents_target[4], currents_target[5]);
            printf("abweichung durchschnitt von target und actual current %.7f, %.7f, %.7f, %.7f, %.7f, %.7f\n", average_current[0]/i, average_current[1]/i, average_current[2]/i, average_current[3]/i, average_current[4]/i, average_current[5]/i);
            printf("acceleration takt end %f,\n%f,\n%f,\n%f,\n%f,\n%f", move_pva_packet.tb[0]/T_IPO, move_pva_packet.tb[1]/T_IPO, move_pva_packet.tb[2]/T_IPO, move_pva_packet.tb[3]/T_IPO, move_pva_packet.tb[4]/T_IPO, move_pva_packet.tb[5]/T_IPO);
            printf("deceleration takt start %f,\n%f,\n%f,\n%f,\n%f,\n%f", move_pva_packet.tv[0]/T_IPO, move_pva_packet.tv[1]/T_IPO, move_pva_packet.tv[2]/T_IPO, move_pva_packet.tv[3]/T_IPO, move_pva_packet.tv[4]/T_IPO, move_pva_packet.tv[5]/T_IPO);

            printf("security stopped at interpolation: %d\n", i);
            robotinterface_command_empty_command();

            robotinterface_send();
            break;
        }
        for(j=0;j<6;j++){
            average_current[j]+= fabs(currents_actual[j]-currents_target[j]);
        }
//
        move_pva_packet.point_in_time= (double) i * T_IPO;
        if(profil == RAMPENPROFIL)
            interpolation_ramp_ptp(&move_pva_packet);
        else
            interpolation_sin_ptp(&move_pva_packet);
        move_pva_packet.pva.position[5] = pva_packet.position[5];
        move_pva_packet.pva.velocity[5] = pva_packet.velocity[5];
        move_pva_packet.pva.position[4] = pva_packet.position[4];
        move_pva_packet.pva.velocity[4] = pva_packet.velocity[4];
        move_pva_packet.pva.position[3] = pva_packet.position[3];
        move_pva_packet.pva.velocity[3] = pva_packet.velocity[3];
        *((char *)move_pva_packet.pva.position) = 7;
        robotinterface_command_position_velocity_acceleration(move_pva_packet.pva.position,
                                                              move_pva_packet.pva.velocity,
                                                              zero_vector);

        memcpy(&last_pva_packet, &pva_packet, sizeof(pva_packet));
        robotinterface_send();
    }

    robotinterface_read_state_blocking();
    robotinterface_get_target_position(pva.position);
    robotinterface_command_velocity(zero_vector);
    robotinterface_send();


    // slow down
    for(i=0;i<10;i++){
        robotinterface_read_state_blocking();
        robotinterface_command_position_velocity_acceleration(pva.position, zero_vector, zero_vector);
        robotinterface_send();
    }
}

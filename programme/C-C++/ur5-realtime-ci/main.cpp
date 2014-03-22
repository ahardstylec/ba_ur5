#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <sched.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <endian.h>
#include <fstream>
#include <pthread.h>

extern "C"{
    #include "../libs/ur5_realtime_ci.h"
    #include "../libs/base_utils.h"
    #include "../libs/interrupt_utils.h"
}
#include <vector>

using namespace std;
FILE *ur5_data_fd;

pthread_mutex_t quit_programm_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv){
    int sockfd, portno;
    struct sockaddr_in serveraddr;
    struct ur5_realtime_ci ur5_rci_old;

    bzero(&ur5_rci_old, sizeof(ur5_rci_old));
    struct ur5_realtime_ci *ur5_rci;
    struct ur5_data_rci *ur5_d;


    char * ip_string= "";
    char *garbage= NULL;

    bzero(&serveraddr, sizeof(serveraddr));

    if (argc != 3) {
        fprintf(stderr,"usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    ip_string = argv[1];
    if(inet_aton(ip_string, &serveraddr.sin_addr) == 0){
        printf("ip address not valid");
        exit(1);
    }

    portno = (int) strtol(argv[2], &garbage, 10);
    printf("Start UR5 Client\n");

    setup_sigint();
    setup_sigpipe();

    /* build the server's Internet address */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(portno);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        printf("failed\n");
        exit(1);
    }
    if (connect(sockfd, (struct sockaddr*) &serveraddr, sizeof(serveraddr)) < 0){
        printf("failed\n");
//            error("ERROR connecting");
        printf("error: cant connect to server!\n");
        fflush(stdout);
        close(sockfd);
        return 1;
    }
    printf("suceess\n");

    char buf[UR5_REALTIME_CI_SIZE];
    bzero(buf, sizeof(buf));

    int i=0;
    unsigned int n=0;
    vector<ur5_data_rci> data_recv;

    while(quit_program == false){
        n = recv(sockfd, &buf, UR5_REALTIME_CI_SIZE, MSG_WAITALL);
        if(n < 1){
            puts("connection to server lost");
            break;
            return 0;
        }
        ur5_d = parse_ur5_realtime_ci(ur5_rci, buf);
        printf("\r current_joint1 %f          ", ur5_d->i_target[1]);
//        if(((int64_t) ur5_d->digital_in) == 2){
//            puts("recording data");
//            do{
//                n = recv(sockfd, &buf, UR5_REALTIME_CI_SIZE, MSG_WAITALL);
//                if(n < 1){
//                    puts("connection to server lost");
//                    return 0;
//                }
//                ur5_d = parse_ur5_realtime_ci(ur5_rci, buf);
//                data_recv.push_back(*ur5_d);
//            }while(((int64_t) ur5_d->digital_in) == 2 && quit_program == false);
//        }
    }

    puts("disconnected from server\n");

    ofstream fd;

    fd.open("ur5_data.bin", ios::out | ios::binary);
    for(int packet=0; packet <data_recv.size();packet++){
//        fwrite((char *) &data_recv, sizeof(vector<ur5_data_rci>), data_recv.size(), fd);

        fd.write((char*) &data_recv[packet], sizeof(ur5_data_rci));
    }
    close(sockfd);
    fd.close();

    return 0;
}

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
#include <pthread.h>
#include <time.h>
#include "../libs/ur5_primary_secondary_interface.h"
#include "../libs/base_utils.h"


int main(int argc, char **argv){
    int sockfd, portno,ur5_data_fd;
    struct sockaddr_in serveraddr;

    struct Ur5PrimaryInterface ur5_primary;
    struct Ur5SecondaryInterface ur5_secondary;
    struct Ur5PrimaryInterface ur5_primary_last;
    struct Ur5SecondaryInterface ur5_secondary_last;
    char * ip_string= "";
    char *garbage= NULL;
    //    struct thread_arg args;

    bzero(&serveraddr, sizeof(serveraddr));


    // ---------------------- Checking arguments ------------------------
    // ------------------------------------------------------------------
    if (argc != 3) {
        fprintf(stderr, "usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    ip_string = argv[1];
    if(inet_aton(ip_string, &serveraddr.sin_addr) == 0){
        printf("ip address not valid");
        exit(1);
    }

    portno = (int) strtol(argv[2], &garbage, 10);

    // ------------------------------------------------------------------
    // ------------------------------------------------------------------

    printf("Start UR5 Client\n");


    // ----------------- Create Socket and Connect to Robot -------------
    // ------------------------------------------------------------------

    /* build the server's Internet address */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(portno);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        printf("failed\n");
        error("ERROR opening socket");
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

    // ------------------------------------------------------------------
    // ------------------------------------------------------------------

    printf("suceess\n");
    char * buf;
    int max_size_of_package;
    if(portno == 30001){
        max_size_of_package= UR5_PRIMARY_INTERFACE_MAX_SIZE+400;
        buf = (char *) calloc(max_size_of_package, sizeof(char));
        ur5_data_fd = fopen("ur5_primary_interface_packet","wb");
    }
    else if(portno == 30002){
        max_size_of_package= UR5_SECONDARY_INTERFACE_MAX_SIZE+400;
        buf = (char *) calloc(max_size_of_package, sizeof(char));
        ur5_data_fd = fopen("ur5_secondary_interface_packet","wb");
    }else{
        buf = NULL;
    }

    // -------------- Read from Socket and parse the Data ---------------
    // ------------------------------------------------------------------

    int i=0;
    unsigned int n=0;
    void*interface;
    if(portno == 30002 || portno == 30001){
        for(i=0; i < 200; i++){
            n = recv(sockfd, (void *) buf, max_size_of_package, 0);
            if(n < 1){
                puts("connection to server lost");
//                break;
                return 0;
            }
//            gettimeofday(&a,0);
            interface = parse_packages(buf);
            if(interface != NULL){
                if(portno == 30001){
                    ur5_primary = *(Ur5PrimaryInterface*) interface;
                    memcpy(&ur5_primary_last, &ur5_primary, sizeof(ur5_primary));
                }else if(portno == 30002){
                    ur5_secondary= *(Ur5SecondaryInterface*) interface;
                    memcpy(&ur5_secondary_last, &ur5_secondary, sizeof(ur5_secondary));
                }
                free(interface);
            }
            bzero(buf, sizeof(*buf));
        }
    }else{
        puts("not connected to secondary or primary interface");
    }

    // ------------------------------------------------------------------
    // ------------------------------------------------------------------

    puts("disconnected from server\n");

    close(sockfd);
    close(ur5_data_fd);


    return 0;
}

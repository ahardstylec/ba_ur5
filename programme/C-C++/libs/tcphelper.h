#ifndef _TCPHELPER_H
#define _TCPHELPER_H

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct connection_data{
    int sockfd;
    int newsockfd;
    int iflag;
    int port;
    socklen_t clilen;
    double initialize_direction;

    char * interface;
    char * ip_addr_s;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
};


char * get_interface_addr(char *interface, struct in_addr * ip_addr);

struct connection_data * new_connection_data(void);

void bind_socket(struct connection_data *);

void create_socket(int *);

#endif

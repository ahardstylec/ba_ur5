#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <netinet/in.h>
#include "tcphelper.h"
#include "helper.h"

char * get_interface_addr(char *interface, struct in_addr *ip_addr){
	// get ip addr of interface

    struct ifreq ifr;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    //Type of address to retrieve - IPv4 IP address
    ifr.ifr_addr.sa_family = AF_INET;

    //Copy the interface name in the ifreq structure
    strncpy(ifr.ifr_name , interface , IFNAMSIZ-1);

    if(ioctl(fd, SIOCGIFADDR, &ifr) == -1){
        perror("interface not found or offline");
        close(fd);
        exit(1);
    }
	close(fd);
 
	struct sockaddr_in* ipaddr = (struct sockaddr_in*)&ifr.ifr_addr;
	*ip_addr= ipaddr->sin_addr;


	return inet_ntoa(*ip_addr);
}

void create_socket(int *sockfd){
    int optval, on = 1;
    int rc;

    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sockfd < 0){
        error("ERROR opening socket");
    }
    setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}

void bind_socket(struct connection_data *con_data){
    if (bind(con_data->sockfd, (struct sockaddr *) &con_data->serv_addr, sizeof(con_data->serv_addr)) < 0){
        printf("ERROR on binding on port %i", con_data->port);
        exit(1);
    }
    if (listen(con_data->sockfd, 5) == -1) {
        puts("error on listen");
        exit(1);
    }
    printf("Server is running. listen on %s:%i\n", con_data->ip_addr_s, con_data->port);
}

struct connection_data * new_connection_data(){
    struct connection_data * temp_con_data=(struct connection_data *) calloc(1, sizeof(struct connection_data));
    temp_con_data->iflag=0;
    temp_con_data->port=8000;
    temp_con_data->interface = "lo";

    return temp_con_data;
}

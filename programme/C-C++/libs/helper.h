#ifndef _HELPER_H
#define _HELPER_H

#include <sys/socket.h>
#include <sys/types.h>
#include "tcphelper.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void read_args(int argc, char **argv, struct connection_data * server);
void timeval_diff(struct timeval *, struct timeval*, struct timeval*);
void error(char *);
void move_to_position(double*);

#endif

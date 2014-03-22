//#include <csignal>
//#include <cstdio>
//#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "robotinterface.h"


bool quit_program=false;
extern pthread_mutex_t quit_programm_mutex;

static void sigint_handler (int signal, siginfo_t * info, void *data) { 
    printf ("SIGINT\n");
    fflush(stdout);
    pthread_mutex_lock(&quit_programm_mutex);
    quit_program = true;
    pthread_mutex_unlock(&quit_programm_mutex);
//    printf("MODE: %d\n",robotinterface_get_robot_mode());
//    robotinterface_close();
    //signal(SIGINT, SIG_DFL);
}

static void sigpipe_handler (int signal, siginfo_t * info, void *data) { 
    printf ("SIGPIPE\n");
    fflush(stdout);
    //signal(SIGINT, SIG_DFL);
}


void  setup_sigint(){
    struct sigaction act;
    act.sa_sigaction = sigint_handler;
    act.sa_flags = SA_RESTART | SA_SIGINFO;
    sigemptyset (&act.sa_mask);
    if (sigaction (SIGINT, &act, NULL) < 0){
        perror ("sigaction");
        exit(0);
    }
}

void  setup_sigpipe(){
    struct sigaction act2;
    act2.sa_sigaction = sigpipe_handler;
    act2.sa_flags = SA_RESTART | SA_SIGINFO;
    sigemptyset (&act2.sa_mask);
    if (sigaction (SIGPIPE, &act2, NULL) < 0) {
        perror ("sigaction");
        exit(0);
    }
}

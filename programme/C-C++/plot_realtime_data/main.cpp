#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <vector>
#include <fstream>      // std::ifstream
#include <string>
#include "../libs/ur5_realtime_ci.h"
//#include "map"

using namespace std;
FILE *file_write;

void write_header(void){
    fprintf(file_write, "time;");
    for(int i=0; i<6;i++){
        fprintf(file_write, "position_actual%d;", i);
        fprintf(file_write, "velocity_actual%d;", i);
        fprintf(file_write, "current_actual%d;", i);
        fprintf(file_write, "current_target%d;", i);
        fprintf(file_write, "acceleration_target%d;", i);
    }
    fprintf(file_write, "\n");
}

void write_data(double time, ur5_data_rci *buf){
    fprintf(file_write, "%f;", time);
    for(int joint=0; joint<6;joint++){
        fprintf(file_write, "%f;", buf->q_actual[joint]);
        fprintf(file_write, "%f;", buf->qd_actual[joint]);
        fprintf(file_write, "%f;", buf->i_actual[joint]);
        fprintf(file_write, "%f;", buf->i_target[joint]);
        fprintf(file_write, "%f;", buf->qdd_target[joint]);
    }
    fprintf(file_write, "\n");
}

int main()
{
    ur5_data_rci buf;
    vector<ur5_data_rci> ur5_d_vector;
//    FILE * fd;
    ifstream fd;
    cout << "read data" << endl;
//    fd= fopen("ur5_data.bin", "r");
    file_write = fopen("pva_data.csv", "w");
    write_header();
    fd.open("ur5_data.bin", ios::in | ios::binary);
    for(int packet = 0; !fd.eof(); packet++){
        fd.read((char *)&buf, sizeof(ur5_data_rci));
        write_data(packet*0.008, &buf);
    }
    fclose(file_write);
    fd.close();
    return 0;
}


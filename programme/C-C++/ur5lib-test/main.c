#include <stdio.h>
#include <stdlib.h>
#include "../libs/ur5lib.h"
#include "../libs/robotinterface.h"

int main(int argc, char **argv)
{
    debug=true;
    if(argc < 2){

        return 1;
    }
//    debug_filename=argv[1];
//    printf("test ur5lib.. write file to %s\n", debug_filename);

    JointVector joint_home_rad={0.0000, -1.5708, 0.0000, -1.5708, 0.0000, 0.0000};
//    JointVector joint_home={0.0000, -90, 0.0000, -90, 0.0000, 0.0000};
//    JointVector init_two ={-6.5018, -95.3469, -29.8123, -194.7758, -59.0822, -62.3032};
//    JointVector init_right_side = {0.309199, -1.240009, 0.323497, -1.039543, 0.541075, 0.486453};
    JointVector initialP_rad = {-0.330294, -1.881878, -0.290919, -2.243194, -0.660115, -0.704284};

    JointVector viereck[4] = {{43.24,   -140.88,    -30.05, -12.83, 129.28, -0.27},
                              {130.67,  -140.87,    -30.05, -12.75, 44.08,  -0.27},
                              {130.67,  -187.90,    6.46,   -1.22,  43.35,  -0.27},
                              {43.24,   -187.90,    -30.05, 3.87,   100.32, -0.27}
                             };
    //    JointVector PosOne= {36.12,   -112.88,    -8.78, -24.53, 88.39, 16.64};
    MovePTPPacket move_ptp_packet;
    int i,h;
    for(i=0; i<4;i++){
        for(h=0;h<6;h++){
            viereck[i][h]= deg_to_rad(viereck[i][h]);
        }
    }

    calc_ptp_profile(initialP_rad, joint_home_rad, &move_ptp_packet, SINOIDENPROFIL);

    return 0;
}

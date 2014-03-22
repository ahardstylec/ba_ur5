#include "../libs/ur5_realtime_ci.h"

struct ur5_data_rci * parse_ur5_realtime_ci(struct ur5_realtime_ci *ur5_rci, char *buf){
    ur5_rci = (struct ur5_realtime_ci*) buf;

    ur5_rci->length = ntohl(ur5_rci->length);
    int i;
    for (i = 0; (i < (sizeof(ur5_rci->data_union.data_packed)/sizeof(double))); i++){
        ur5_rci->data_union.data_packed[i] = htobe64(ur5_rci->data_union.data_packed[i]);
    }
    return &ur5_rci->data_union.data;
}

void write_data_to_file(const char* filename, struct ur5_data_rci * ur5_d){

}

#include "../libs/ur5_primary_secondary_interface.h"

struct struct_swap_data{
    short array_size;
    short *data_offset[2];
    short loop_count;
};

short mode[10][2]={{8,1},{1,1},{1,1},{1,1},{1,1},{1,1},{1,1},{1,1},{1,1},{8,1}};
short joint[3][2]={{8,3},{4,3},{1,1}};
short tool[6][2]={{1,2},{8,2},{4,1},{1,1},{4,2},{1,1}};
short master[7][2]={{2,2},{1,2},{8,2},{1,2},{8,2},{4,4},{1,3}};
short cartesian[1][2]= {{8,6}};
short kindematics[1][2]= {{1,220}};
short configuration_arr[2][2]= {{8,67},{4,10}};
short force[1][2]={{8,7}};
short additional_arr[1][2]={{1,1}};

struct struct_swap_data swap_data_for_robot_mode_data= {10, mode, 1};
struct struct_swap_data swap_data_for_joint_data= {3, joint, 6};
struct struct_swap_data swap_data_for_tool_data= {6, tool, 1};
struct struct_swap_data swap_data_for_masterboard_data = {7, master, 1};
struct struct_swap_data swap_data_for_cartesian_data= {1,cartesian,1};
struct struct_swap_data swap_data_for_kinematics_data= {1,kindematics,1};
struct struct_swap_data swap_data_for_configuration_data= {2,configuration_arr,1};
struct struct_swap_data swap_data_for_force_mode_data= {1,force,1};
struct struct_swap_data swap_data_for_additional_data= {1,additional_arr,1};


char * package_type_to_s(unsigned char message_type, unsigned char robot_data_type){
    char * package;
    if(message_type == ROBOT_STATE){
        switch(robot_data_type){
        case ROBOT_MODE_DATA: package= "RobotModeData"; break;
        case JOINT_DATA_PACKAGE: package="JointDataPackage"; break;
        case TOOL_DATA: package="ToolData"; break;
        case MASTERBOAD_DATA: package= "MasterboadData"; break;
        case CARTESIAN_DATA: package="CartesianData"; break;
        case KINEMATICS_DATA: package= "KinematicsData"; break;
        case CONFIGURATION_DATA: package= "ConfigurationData"; break;
        case FORCE_MODE_DATA: package= "ForceModeData"; break;
        case ADDITIONAL_DATA: package= "AdditionalData"; break;
        default: package="Unknown package"; break;
        }
    }else if(message_type == ROBOT_MESSAGE){
        switch(robot_data_type){
        case VAR_MESSAGE: package= "VarMessage"; break;
        case VERSION_MESSAGE: package="VersionMessage"; break;
        case SECURITY_MESSAGE: package="SecurityMessage"; break;
        case ROBOT_COMM_MESSAGE: package= "RobotCommMessage"; break;
        case REQUEST_VALUE_MESSAGE: package="RequestValueMessage"; break;
        case KEY_MESSAGE: package= "KeyMessage"; break;
        case POPUP_MESSAGE: package= "PopupMessage"; break;
        case LABEL_MESSAGE: package= "LabelMessage"; break;
        case TEXT_MESSAGE: package = "TextMessage"; break;
        default: package="Unknown package"; break;
        }
    }else{
        package = "unknown package";
    }

    return package;
}

void swap_data(int size, void* var){
    switch(size){
    case 8:
        break;
    case 16:
        *(u_int16_t*)var = (u_int16_t) htobe16(*(u_int16_t*) var);
        break;
    case 32:
        *(u_int32_t*) var = (u_int32_t) htobe32(*(u_int32_t*) var);
        break;
    case 64:
        *(u_int64_t*)var = (u_int64_t) htobe64(*(u_int64_t*) var);
        break;
    }
}

void * parse_packages(char *buf){
    void * return_interface;
    unsigned char message_type;
    swap_data(32, &buf[0]);
    int32_t size_of_message = *(int32_t*) &buf[0];
    printf("size of message: %d\n", (int32_t) size_of_message);
    message_type =(unsigned char) buf[4];
    //    printf("message type is %u\n", message_type);
    Ur5PrimaryInterface * ur5_primary= malloc(sizeof(Ur5PrimaryInterface));
    Ur5SecondaryInterface * ur5_secondary=malloc(sizeof(Ur5SecondaryInterface));
    switch(message_type){
        case ROBOT_STATE:
            puts("MESSAGE TYPE IS ROBOT STATE");
            parse_robot_state(ur5_secondary, buf, size_of_message);
            return_interface = ur5_secondary;
            break;
        case ROBOT_MESSAGE:
            puts("MESSAGE TYPE IS ROBOT MESSAGE");
            parse_robot_message(ur5_primary, buf);
            return_interface = ur5_primary;
            break;
        default: return_interface = NULL; break;
    }
    return return_interface;
}

void parse_robot_state(Ur5SecondaryInterface * ur5_secondary, char * buf, int32_t size_of_message){
    int unknown_data_counter=0;
    int32_t buff_pointer=5;
    unsigned char current_package_type;
    int32_t package_size;

    // variables to swap data
    struct struct_swap_data *swap_data;
    struct struct_swap_data dont_swap_data = {0,{{0,0}},0};
    char *package;
    while(buff_pointer < size_of_message){
        package_size = *(int32_t*) &buf[buff_pointer];
        package_size = (int32_t) ntohl((u_int32_t) package_size);
//        printf("package size: %d\n", package_size);
        if(package_size == 0) break;
        current_package_type = (unsigned char) buf[buff_pointer+4];
//        printf("package type: %d\n",(unsigned char) current_package_type);
//        printf("package type name: %s\n", package_type_to_s(ROBOT_STATE, current_package_type));
        switch(current_package_type){
            case ROBOT_MODE_DATA:
                ur5_secondary->robot_mode_data = *(RobotModeData*) &buf[buff_pointer];
                package = &ur5_secondary->robot_mode_data;
                swap_data= &swap_data_for_robot_mode_data;
                break;
            case JOINT_DATA_PACKAGE:
                ur5_secondary->joint_data_package= *(JointDataPackage*) &buf[buff_pointer];
                package = &ur5_secondary->joint_data_package;
                swap_data= &swap_data_for_joint_data;
                break;
            case TOOL_DATA:
                ur5_secondary->tool_data= *(ToolData*) &buf[buff_pointer];
                package = &ur5_secondary->tool_data;
                swap_data= &swap_data_for_tool_data;
                break;
            case MASTERBOAD_DATA:
                ur5_secondary->masterboad_data= *(MasterboadData*) &buf[buff_pointer];
                package = &ur5_secondary->masterboad_data;
                swap_data= &swap_data_for_masterboard_data;
                break;
            case CARTESIAN_DATA:
                ur5_secondary->cartesian_data= *(CartesianData*) &buf[buff_pointer];
                package = &ur5_secondary->cartesian_data;
                swap_data= &swap_data_for_cartesian_data;
                break;
            case KINEMATICS_DATA:
                ur5_secondary->kinematics_data= *(KinematicsData*) &buf[buff_pointer];
                package = &ur5_secondary->kinematics_data;
                swap_data= &swap_data_for_kinematics_data;
                break;
            case CONFIGURATION_DATA:
                ur5_secondary->configuration_data= *(ConfigurationData*) &buf[buff_pointer];
                package = &ur5_secondary->configuration_data;
                swap_data= &swap_data_for_configuration_data;
                break;
            case FORCE_MODE_DATA:
                ur5_secondary->force_mode_data= *(ForceModeData*) &buf[buff_pointer];
                package = &ur5_secondary->force_mode_data;
                swap_data= &swap_data_for_force_mode_data;
                break;
            case ADDITIONAL_DATA:
                ur5_secondary->additional_data= *(AdditionalData*) &buf[buff_pointer];
                package = &ur5_secondary->additional_data;
                swap_data= &swap_data_for_additional_data;
                break;
            case UNKNOWN_PACKAGE:
                ur5_secondary->unknown_package = *(UnknownPackage*)&buf[buff_pointer];
                swap_data= &dont_swap_data;
                package = NULL;
                break;
            default:
                if(unknown_data_counter<5){
                    ur5_secondary->unknown_data_package[unknown_data_counter].package_length=package_size;
                    ur5_secondary->unknown_data_package[unknown_data_counter].package_type=current_package_type;
                    ur5_secondary->unknown_data_package[unknown_data_counter].data = (char *) &buf[buff_pointer];
                    unknown_data_counter++;
                }
                package = NULL;
                swap_data= &dont_swap_data;
                break;
        }
        swap_package_type(swap_data, current_package_type, package);
        buff_pointer+= package_size;
    }
    return;
}

void parse_robot_message(Ur5PrimaryInterface *ur5_primary, char *buf){
    int unknown_data_counter=0;
    int32_t buff_pointer=0;
    unsigned char current_package_type;
    int32_t package_size;
    current_package_type = (unsigned char) buf[buff_pointer+14];
    printf("package type: %d\n",(unsigned char) current_package_type);
    printf("package type name: %s\n", package_type_to_s(ROBOT_MESSAGE, current_package_type));
    switch(current_package_type){
        case TEXT_MESSAGE:/*0*/
            ur5_primary->text_message= *(TextMessage*)&buf[buff_pointer];
            parse_text_message_package(&ur5_primary->text_message);
            break;
        case LABEL_MESSAGE:/*1*/
            ur5_primary->label_message = *(LabelMessage *)&buf[buff_pointer];
            parse_label_message(&ur5_primary->label_message);
            break;
        case POPUP_MESSAGE:
            ur5_primary->popup_message = *(PopupMessage *)&buf[buff_pointer];
            parse_popup_message(&ur5_primary->popup_message);
            break;
        case VERSION_MESSAGE:
            ur5_primary->version_message = *(VersionMessage *)&buf[buff_pointer];
            parse_version_message(&ur5_primary->version_message);
            break;
        case SECURITY_MESSAGE:
            ur5_primary->security_message = *(SecurityMessage *)&buf[buff_pointer];
            parse_security_message(&ur5_primary->security_message);
            break;
        case ROBOT_COMM_MESSAGE:
            ur5_primary->robot_comm_message = *(RobotCommMessage *)&buf[buff_pointer];
            parse_robot_comm_message(&ur5_primary->robot_comm_message);
            break;
        case KEY_MESSAGE:
            ur5_primary->key_message = *(KeyMessage *)&buf[buff_pointer];
            parse_key_message(&ur5_primary->key_message);
            break;
        case VAR_MESSAGE:
            ur5_primary->var_message = *(VarMessage *)&buf[buff_pointer];
            parse_var_message(&ur5_primary->var_message);
            break;
        case REQUEST_VALUE_MESSAGE:
            ur5_primary->request_value_message = *(RequestValueMessage *)&buf[buff_pointer];
            parse_request_value_message(&ur5_primary->request_value_message);
            break;
    default:
        if(unknown_data_counter<5){
            ur5_primary->unknown_data_package[unknown_data_counter].package_length=package_size;
            ur5_primary->unknown_data_package[unknown_data_counter].package_type=current_package_type;
            ur5_primary->unknown_data_package[unknown_data_counter].data = (char *) &buf[buff_pointer];
            unknown_data_counter++;
        }
        break;
    }
    return;
}

parse_text_message_package(TextMessage* text_message){
    swap_data(64, &text_message->timestamp);
    int size_of_message = text_message->messageSize-TextMessageSize;
    parse_text_message(size_of_message, text_message->text_message);
}

parse_popup_message(PopupMessage* pop_message){
    swap_data(32, &pop_message->title_size);
    swap_data(64, &pop_message->timestamp);
    int size_of_message = pop_message->messageSize - PopupMessageSize - pop_message->title_size;
    parse_text_message(pop_message->title_size, pop_message->message_title);
    parse_text_message(size_of_message, pop_message->text_message);
}

parse_label_message(LabelMessage * label_message){
    swap_data(64, &label_message->timestamp);
    swap_data(32, &label_message->id);
    int size_of_message = label_message->messageSize - LabelMessageSize;
    parse_text_message(size_of_message, label_message->text_message);
}

parse_version_message(VersionMessage * version_message){
    puts("parse version message");
    swap_data(32, &version_message->svn_revision);
    swap_data(64, &version_message->timestamp);
    int size_of_message = version_message->messageSize - VersionMessageSize - version_message->project_name_size;
    parse_text_message(version_message->project_name_size, version_message->project_name);
    parse_text_message(size_of_message, version_message->build_date);
}

parse_robot_comm_message(RobotCommMessage * robot_comm_message){
    swap_data(64, &robot_comm_message->timestamp);
    swap_data(32, &robot_comm_message->robot_message_code);
    swap_data(32, &robot_comm_message->robot_message_argument);
    int size_of_message = robot_comm_message->messageSize - RobotCommMessageSize;
    parse_text_message(size_of_message, robot_comm_message->text_message);
}

parse_key_message(KeyMessage * key_message){
    swap_data(32, &key_message->title_size);
    swap_data(32, &key_message->robot_message_argument);
    swap_data(32, &key_message->robot_message_code);
    swap_data(64, &key_message->timestamp);
    int size_of_message = key_message->messageSize - KeyMessageSize - key_message->title_size;
    parse_text_message(key_message->title_size, key_message->message_title);
    parse_text_message(size_of_message, key_message->text_message);
}

parse_var_message(VarMessage * var_message){
    swap_data(64, &var_message->timestamp);
    swap_data(32, &var_message->code);
    swap_data(32, &var_message->argument);
    int size_of_message = var_message->messageSize - KeyMessageSize - var_message->title_size;
    parse_text_message(var_message->title_size, var_message->message_title);
    parse_text_message(size_of_message, var_message->text_message);
}

parse_security_message(SecurityMessage * security_message){
    swap_data(32, &security_message->robot_message_argument);
    swap_data(32, &security_message->robot_message_code);
    swap_data(64, &security_message->timestamp);
    int size_of_message = security_message->messageSize - SecurityMessageSize;
    parse_text_message(size_of_message, security_message->text_message);
}

parse_text_message(int size, char * text_message){
    //parse and swap message
    puts("parse text message start");
    char * message= calloc(size,1);
    memcpy(&message, &text_message, size);
    text_message= message;
    int i;
    for(i = 0; i< size; i++){
        swap_data(8, &text_message[i]);
    }
    puts("parse text message end");
    return;
}

parse_request_value_message(RequestValueMessage* req_message){
    swap_data(32, &req_message->request_id);
//    swap_data(8, &req_message->source);
    swap_data(64, &req_message->timestamp);
//    swap_data(64, &req_message->message_type);
    int size_of_message = req_message->messageSize - SecurityMessageSize;
    parse_text_message(size_of_message, req_message->text_message);
}

void swap_package_type(struct struct_swap_data *swap_data_arr, unsigned char package_type, void *package){
    int i,j,k;
    int offset = 5;
    for(k=0;k<swap_data_arr->loop_count; k++){
        for(i=0; i< swap_data_arr->array_size; i++){
            for(j=0; j< swap_data_arr->data_offset[i][1]; j++){
                switch(package_type){
                case ROBOT_MODE_DATA:
                    swap_data(swap_data_arr->data_offset[i][0], ((RobotModeDataArr*) package)[offset]);
                    break;
                case JOINT_DATA_PACKAGE:
                    swap_data(swap_data_arr->data_offset[i][0], ((JointDataPackageArr*) package)[offset]);
                    break;
                case TOOL_DATA:
                    swap_data(swap_data_arr->data_offset[i][0], ((ToolDataArr*) package)[offset]);
                    break;
                case MASTERBOAD_DATA:
                    swap_data(swap_data_arr->data_offset[i][0], ((MasterboardDataArr *) package)[offset]);
                    break;
                case CARTESIAN_DATA:
                    swap_data(swap_data_arr->data_offset[i][0], ((CartesianDataArr*) package)[offset]);
                    break;
                case KINEMATICS_DATA:
                    swap_data(swap_data_arr->data_offset[i][0], ((KinematicsDataArr*) package)[offset]);
                    break;
                case CONFIGURATION_DATA:
                    swap_data(swap_data_arr->data_offset[i][0], ((ConfigurationDataArr*) package)[offset]);
                    break;
                case FORCE_MODE_DATA:
                    swap_data(swap_data_arr->data_offset[i][0], ((ForceModeDataArr*) package)[offset]);
                    break;
                case ADDITIONAL_DATA:
                    swap_data(swap_data_arr->data_offset[i][0], ((AdditionalDataArr*) package)[offset]);
                    break;
                default: break;
                }
                offset+=swap_data_arr->data_offset[i][0];
            }
        }
    }
}

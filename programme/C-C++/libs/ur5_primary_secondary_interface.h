#ifndef _UR5_REALTIME_CI_H
#define _UR5_REALTIME_CI_H

#include <endian.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>

double rad_to_deg(double rad);
double deg_to_rad(double deg);

#define UR5_PRIMARY_INTERFACE_EUROMAP_MAX_SIZE 1208+11187
#define UR5_PRIMARY_INTERFACE_MAX_SIZE 1196+11187
#define UR5_SECONDARY_INTERFACE_EUROMAP_MAX_SIZE 1288
#define UR5_SECONDARY_INTERFACE_MAX_SIZE 1276
#define UR5_MESSAGE_SIZE 1000

#pragma pack(1)

// Package Types
#define ROBOT_MODE_DATA 0
#define JOINT_DATA_PACKAGE 1
#define TOOL_DATA 2
#define MASTERBOAD_DATA 3
#define CARTESIAN_DATA 4
#define KINEMATICS_DATA 5
#define CONFIGURATION_DATA 6
#define FORCE_MODE_DATA 7
#define ADDITIONAL_DATA 8
#define UNKNOWN_PACKAGE 9

typedef char RobotModeDataArr[29];
typedef char JointDataPackageArr[251];
typedef char ToolDataArr[37];
typedef char MasterboardDataEuromapArr[76];
typedef char MasterboardDataArr[53];
typedef char KinematicsDataArr[225];
typedef char CartesianDataArr[53];
typedef char ConfigurationDataArr[496];
typedef char ForceModeDataArr[61];
typedef char AdditionalDataArr[7];

// Package Type 0 is Robot Mode Data Package (29 bytes)
typedef struct RobotModeData{
    int32_t	package_length;
    unsigned char package_type;                  //	Type is 0 ("Robot Mode Data")
    u_int64_t Timestamp;
    bool 	is_real_robot_connected;    // PhysicalRobotConnected?
    bool 	is_real_robot_enabled;      // RealRobotEnabled?
    bool 	is_robot_power_on;          // RobotPowerOn?
    bool 	is_emergency_stopped;       // EmergencyStopped?
    bool 	is_security_stopped;        // SecurityStopped?
    bool 	is_program_running;         // ProgramRunning?
    bool 	is_program_paused;          // ProgramPaused?
    unsigned char 	robot_Mode;                 // See table Robot Modes
    double 	speed_fraction;             //
} RobotModeData;

//   Int    Robot modes
//----------------------------------------
//    0     ROBOT_RUNNING_MODE
//    1 	ROBOT_FREEDRIVE_MODE
//    2 	ROBOT_READY_MODE
//    3 	ROBOT_INITIALIZING_MODE
//    4 	ROBOT_SECURITY_STOPPED_MODE
//    5 	ROBOT_EMERGENCY_STOPPED_MODE
//    6 	ROBOT_FATAL_ERROR_MODE
//    7 	ROBOT_NO_POWER_MODE
//    8 	ROBOT_NOT_CONNECTED_MODE
//    9 	ROBOT_SHUTDOWN_MODE
//    10 	ROBOT_SAFEGUARD_STOP_MODE

// JointData  No Package!! it is used by JointDataPackage
typedef struct JointData{
    double 	q_actual; // 	Actual joint position
    double 	q_target; // 	Target joint position
    double 	qd_actual; // 	Actual joint speed
    float 	i_actual; // 	Actual joint current
    float 	v_actual; // 	Actual joint voltage
    float 	t_motor; // 	Joint motor temperature
    float 	t_micro; // 	Don't use, obsolete
    unsigned char 	joint_mode; // 	See table Joint Modes
} JointData;

// Package Type = 1 For Joint Data Packages. (251 bytes)
typedef struct JointDataPackage{
    int32_t  package_length; // Package Length (bytes) 	251 bytes
    unsigned char package_type; 	// Type is 1 ("Joint Data")
    JointData joint[6];
} JointDataPackage;

// Joint Mode Table

//    Num   Description
//------------------------------------------------
//    237 	JOINT_PART_D_CALIBRATION_MODE
//    238 	JOINT_BACKDRIVE_MODE
//    239 	JOINT_POWER_OFF_MODE
//    240 	JOINT_EMERGENCY_STOPPED_MODE
//    241 	JOINT_CALVAL_INITIALIZATION_MODE
//    242 	JOINT_ERROR_MODE
//    243 	JOINT_FREEDRIVE_MODE
//    244 	JOINT_SIMULATED_MODE
//    245 	JOINT_NOT_RESPONDING_MODE
//    246 	JOINT_MOTOR_INITIALISATION_MODE
//    247 	JOINT_ADC_CALIBRATION_MODE
//    248 	JOINT_DEAD_COMMUTATION_MODE
//    249 	JOINT_BOOTLOADER_MODE
//    250 	JOINT_CALIBRATION_MODE
//    251 	JOINT_STOPPED_MODE
//    252 	JOINT_FAULT_MODE

// Tool Data Package Type = 2 (37 Bytes)
typedef struct ToolData{
    int32_t 	package_length;             // (bytes) 	37 bytes
    unsigned char 	package_type;               // 	Type is 2 ("Tool Data")
    char 	is_analog_input_range2;
    char 	is_analog_input_range3;
    double 	is_analog_input2;
    double 	is_analog_iInput3;
    float 	is_tool_Voltage48V;
    unsigned char 	is_tool_OutputVoltage;
    float 	is_tool_current;
    float 	is_tool_temperature;
    unsigned char 	tool_mode;               //	(A subset of joint modes)
}ToolData;


//    Tool Modes
//    Mode (uchar) 	Description
//-----------------------------------
//    249 	TOOL_BOOTLOADER_MODE
//    250 	TOOL_RUNNING_MODE
//    251 	TOOL_IDLE_MODE


// MasterboardData Package Type = 3  (64 bytes)
typedef struct MasterboadData{
    int32_t	package_length;                     // (bytes) 	64 bytes
    unsigned char 	package_type;                   // Type is 3 ("Masterboard Data")
    short 	is_digital_input_bits;
    short 	is_digital_output_bits;
    char 	is_analog_input_range0;
    char 	is_analog_input_range1;
    double 	is_analog_input0;
    double 	is_AnalogInput1;
    char 	is_AnalogOutputDomain0;
    char 	is_AnalogOutputDomain;
    double 	AnalogOutput0;
    double 	AnalogOutput1;
    float 	masterboard_temperature;
    float 	robot_voltage48V;
    float 	robot_current;
    float 	master_io_urrent;
    char 	master_safety_state;            // See table MasterSafetyState
    unsigned char 	master_on_off_state;            // See table MasterOnOffState
    char 	euromap_67_installed;
} MasterboadData;


// MasterboardData Package Type = 3  (with Euromap: 76 bytes)
typedef struct MasterboadEuromapData{
    int32_t	package_length;                     // (bytes) 	64 bytes (with Euromap: 76 bytes)
    unsigned char 	package_type;                   // Type is 3 ("Masterboard Data")
    short 	is_digital_input_bits;
    short 	is_digital_output_bits;
    char 	is_analog_input_range0;
    char 	is_analog_input_range1;
    double 	is_analog_input0;
    double 	is_AnalogInput1;
    char 	is_AnalogOutputDomain0;
    char 	is_AnalogOutputDomain;
    double 	AnalogOutput0;
    double 	AnalogOutput1;
    float 	masterboard_temperature;
    float 	robot_voltage48V;
    float 	robot_current;
    float 	master_io_urrent;
    char 	master_safety_state;            // See table MasterSafetyState
    unsigned char 	master_on_off_state;            // See table MasterOnOffState
    char 	euromap_67_installed;
    int  	euromap_input_bits;             // If Euromap67 is installed
    int 	euromap_output_bits;            // If Euromap67 is installed
    short 	euromap_voltage;                // If Euromap67 is installed
    short 	euromap_urrent;                 // If Euromap67 is installed
} MasterboadEuromapData;

// Table Master Safety State
//  State   State                                   Comment
//------------------------------------------------------------------------------------
//    0 	SAFETY_STATE_UNDEFINED                  Used for shared state variables when communication is down
//    1 	SAFETY_STATE_BOOTLOADER                 Used for bootloading firmware over the robot bus
//    2 	SAFETY_STATE_FAULT                      Used for internal circuit/robot errors only
//    3 	SAFETY_STATE_BOOTING                    Waiting for communication to work, robot voltage to be under 4V. This state is also used to reboot the masterboard instead of faulting.
//    4 	SAFETY_STATE_INITIALIZING               Initializing masterboard (checking voltages etc.)
//    5 	SAFETY_STATE_ROBOT_EMERGENCY_STOP       (EA + EB + SBUS->Screen) Physical e-stop interface input activated
//    6 	SAFETY_STATE_EXTERNAL_EMERGENCY_STOP 	(EA + EB + SBUS->Euromap67) Physical e-stop interface input activated
//    7 	SAFETY_STATE_SAFEGUARD_STOP             (SA + SB + SBUS) Physical s-stop interface input
//    8 	SAFETY_STATE_OK

//Robot On Off State
//  State   Meanin                  Comment
//-----------------------------------------------------------

//    0 	ROBOT_48V_STATE_OFF             There is ~0V voltage on the robot
//    1 	ROBOT_48V_STATE_TURNING_ON      Voltage is raising (Soft Start)
//    2 	ROBOT_48V_STATE_ON              Power transistors is on, and the voltage is ~48V
//    3 	ROBOT_48V_STATE_TURNING_OFF

typedef struct CartesianData{
    int32_t package_length;             // (bytes) 	53 bytes
    unsigned char   package_type;       // type is 4 ("Cartesian Info")
    double x;                           // Tool vector, X-value
    double y;                           // Tool vector, Y-value
    double z;                           // 	Tool vector, Z-value
    double 	rx;                          // Rx: Rotation vector representation of the tool orientation
    double 	ry;                          //	Ry: Rotation vector representation of the tool orientation
    double 	rz;                          // Rz: Rotation vector representation of the tool orientation
} CartesianData;

// Package Type = 5 size(225)
typedef struct KinematicsData{
    int32_t package_length;
    unsigned char   package_type;
    char kinematics_info[220];
} KinematicsData;


typedef struct JointSpeedData{
    double 	joint_max_speed;
    double 	joint_max_acceleration;
}JointSpeedData;

// ConfigurationData Type = 6 (bytes) 	469 bytes
typedef struct ConfigurationData{
    int32_t package_length;             // (bytes) 	469 bytes
    unsigned char   package_type;       // type is 6 ("Configuration Data")
    double 	joint_min_limit;
    double 	joint_max_limit;
    JointSpeedData joint_speed[6];

    double 	v_joint_default;        //	Default speed limit
    double 	a_joint_default;        //	Default acceleration limit
    double 	v_tool_default;         //	Default tool speed limit
    double 	a_tool_default;         //	Default tool acceleration limit
    double 	eq_radius;              //  The characteristic size of the tool
    double 	DHa[6];                 // 	DH parameter 'a'
    double 	DHd[6];                 // 	DH parameter 'd'
    double 	DHalpha[6];             //	DH parameter 'alpha'
    double 	DHtheta[6];             //	DH parameter 'theta'

    int masterboard_version;
    int controller_box_type;
    int robot_type;
    int	robot_sub_type;
    int motor_type[6];  //	Have values 1 or 2
} ConfigurationData;

// ForceModeData Type = 7 (bytes) 	61 bytes
typedef struct ForceModeData{
    int32_t package_length;             // (bytes) 	61 bytes
    unsigned char   package_type;       // type is 6 ("Force Mode Data")
    double 	x;                          //Force mode frame, X-value
    double 	y;                          //Force mode frame, Y-value
    double 	z;                          //Force mode frame, Z-value
    double 	rx;         //	Force mode frame, Rx: Rotation vector representation of the orientation
    double 	ry;         //	Force mode frame, Ry: Rotation vector representation of the orientation
    double 	rz;         //	Force mode frame, Rz: Rotation vector representation of the orientation
    double 	robot_dexterity; // 	TCP dexterity
} ForceModeData;

//  AdditionalData Type = 8 (bytes) 	7 bytes
typedef struct AdditionalData{
    int32_t package_length;             // (bytes) 	7 bytes
    unsigned char   package_type;       // type is 6 ("Additional Data")
    bool 	TeachButtonPressed;
    bool 	TeachButtonEnabled;
} AdditionalData;

//  UnknownPackage Type = 8 (bytes) 	53 bytes
typedef struct UnknownPackage{
    int32_t package_length;             // (bytes) 	53 bytes
    unsigned char   package_type;       // type is 9
    char data[48];
}UnknownPackage;

// --- Message Packages Start ----------------------------------

// Robot Message Types

#define TEXT_MESSAGE            0
#define LABEL_MESSAGE           1
#define POPUP_MESSAGE           2
#define VERSION_MESSAGE         3
#define SECURITY_MESSAGE        5
#define ROBOT_COMM_MESSAGE      6
#define KEY_MESSAGE             7
#define VAR_MESSAGE             8
#define REQUEST_VALUE_MESSAGE   -1


// Message Sources
#define TOOL            6
#define WRIST3          5
#define WRIST2          4
#define WRIST1          3
#define ELBOW           2
#define SHOULDER        1
#define BASE            0
#define SAFETYSYS       -1
#define CONTROLLER      -2
#define RTMACHINE       -3
#define SIMULATED_ROBOT -4
#define GUI             -5
#define CONTROL_BOX     -6

// Message Types

#define ROBOT_STATE 16
#define ROBOT_MESSAGE 20
#define HMC_MESSAGE 22
#define MODBUS_INFO_MESSAGE 5


//  Message Type =   TEXT_MESSAGE Type = 0 (1015 Bytes)
#define TextMessageSize 15
typedef struct   TextMessage{
    int32_t messageSize;
    unsigned char message_type; // = ROBOT_MESSAGE = 20
    u_int64_t timestamp;
    char source;
    char robot_message_type;// = ROBOT_MESSAGE_POPUP = 0
    char *text_message; //[UR5_MESSAGE_SIZE];
}  TextMessage;

//  Message Type =  LABEL_MESSAGE Type = 1 (1019 Bytes)
#define LabelMessageSize 19
typedef struct LabelMessage{
    int32_t messageSize;
    unsigned char message_type; // = ROBOT_MESSAGE = 20
    u_int64_t timestamp;
    char source;
    char robot_message_type;// = ROBOT_MESSAGE_SECURITY = 1
    int id;
    char *text_message;//[UR5_MESSAGE_SIZE];
} LabelMessage;

//  Message Type =   POPUP_MESSAGE Type = 2 (1018 Bytes)
#define PopupMessageSize 18
typedef struct PopupMessage{
    int32_t messageSize;
    unsigned char message_type; // = ROBOT_MESSAGE = 20
    u_int64_t timestamp;
    char source;
    char robot_message_type;// = ROBOT_MESSAGE_POPUP = 2
    bool warning;
    bool error;
    unsigned char title_size;
    char *message_title; //[UR5_MESSAGE_SIZE];
    char *text_message; //[UR5_MESSAGE_SIZE];
}  PopupMessage;

//  Message Type = VERSION_MESSAGE Type = 3 (2023 Bytes)
#define VersionMessageSize 23
typedef struct VersionMessage{
    int messageSize;
    u_int64_t timestamp;
    unsigned char message_type; // = ROBOT_MESSAGE = 20
    char source;
    char robot_message_type; // = robot_message_version = 3
    char project_name_size;
    char *project_name;//[UR5_MESSAGE_SIZE];
    unsigned char major_version;
    unsigned char minor_version;
    int svn_revision;
    char *build_date;//[UR5_MESSAGE_SIZE];
} VersionMessage;

//  Message Type = SECURITY_MESSAGE Type = 5 (1023 Bytes)
#define SecurityMessageSize 23
typedef struct SecurityMessage{
    int messageSize;
    unsigned char message_type; // = ROBOT_MESSAGE = 20
    u_int64_t timestamp;
    char source;
    char robot_message_type;// = ROBOT_MESSAGE_SECURITY = 5
    int robot_message_code;
    int robot_message_argument;
    char *text_message;//[UR5_MESSAGE_SIZE];
} SecurityMessage;

//  Message Type = ROBOT_COMM_MESSAGE Type = 6 (1023 Bytes)
#define RobotCommMessageSize 23
typedef struct RobotCommMessage{
    int messageSize;
    unsigned char message_type; // = ROBOT_MESSAGE = 20
    u_int64_t timestamp;
    char source;
    char robot_message_type;// = ROBOT_MESSAGE_SECURITY = 6
    int robot_message_code;
    int robot_message_argument;
    char *text_message; //[UR5_MESSAGE_SIZE];
} RobotCommMessage;


//  Message Type = KEY_MESSAGE Type = 7 (2024 Bytes)
#define KeyMessageSize 24
typedef struct KeyMessage{
    int messageSize;
    unsigned char message_type; // = ROBOT_MESSAGE = 20
    u_int64_t timestamp;
    char source;
    char robot_message_type;// = ROBOT_MESSAGE_SECURITY = 7
    int robot_message_code;
    int robot_message_argument;
    unsigned char title_size;
    char *message_title;//[UR5_MESSAGE_SIZE];
    char *text_message;//[UR5_MESSAGE_SIZE];
} KeyMessage;


//  Message Type =   VAR_MESSAGE Type = 8 (1024 Bytes)
#define VarMessageSize 24
typedef struct VarMessage{
    int messageSize;
    unsigned char message_type; // = ROBOT_MESSAGE = 20
    u_int64_t timestamp;
    char source;
    char robot_message_type;// = ROBOT_MESSAGE_SECURITY = 8
    int code;
    int argument;
    unsigned char title_size;
    char *message_title;
    char *text_message; //[UR5_MESSAGE_SIZE];
}  VarMessage;

//  Message Type =   VAR_MESSAGE Type = 0-8 (1019 Bytes)
#define RequestValueMessageSize 19
typedef struct RequestValueMessage{
    int messageSize;
    unsigned char message_type; // = ROBOT_MESSAGE = 20
    u_int64_t timestamp;
    char source;
    char robot_message_type;// = ROBOT_MESSAGE_SECURITY = 0-8
    unsigned int request_id;
    char *text_message; //[UR5_MESSAGE_SIZE];
} RequestValueMessage;



typedef struct UnknownData{
    int32_t package_length;             // (bytes) 	100 bytes
    unsigned char   package_type;       // type is not known yet
    char *data;
}UnknownData;

// --- Message Packages End ----------------------------------

typedef struct Ur5PrimaryInterface{
    RobotModeData robot_mode_data;              // TYPE 0
    JointDataPackage joint_data_package;        // TYPE 1
    ToolData tool_data;                         // TYPE 2
    MasterboadData masterboad_data;             // TYPE 3
    CartesianData cartesian_data;               // TYPE 4
    KinematicsData kinematics_data;             // TYPE 5
    ConfigurationData configuration_data;       // TYPE 6
    ForceModeData force_mode_data;              // TYPE 7
    AdditionalData additional_data;             // TYPE 8

    UnknownPackage unknown_package;             // Type 9
    //--- Message Packages
    TextMessage text_message;                   // M_Type 0
    LabelMessage label_message;                 // M_Type 1
    PopupMessage popup_message;                 // M_Type 2
    VersionMessage version_message;             // M_Type 3
    SecurityMessage security_message;           // M_Type 5
    RobotCommMessage robot_comm_message;        // M_Type 6
    KeyMessage key_message;                     // M_Type 7
    VarMessage var_message;                     // M_Type 8
    RequestValueMessage request_value_message;  // M_Type 0-8
    UnknownData unknown_data_package[5];        // Type unknown
}Ur5PrimaryInterface;

typedef struct Ur5SecondaryInterface{
    RobotModeData robot_mode_data;              // TYPE 0
    JointDataPackage joint_data_package;        // TYPE 1
    ToolData tool_data;                         // TYPE 2
    MasterboadData masterboad_data;             // TYPE 3
    CartesianData cartesian_data;               // TYPE 4
    KinematicsData kinematics_data;             // TYPE 5
    ConfigurationData configuration_data;       // TYPE 6
    ForceModeData force_mode_data;              // TYPE 7
    AdditionalData additional_data;             // TYPE 8
    UnknownPackage unknown_package;             // Type 9
    UnknownData unknown_data_package[5];        // Type unknown
}Ur5SecondaryInterface;

#pragma pack()


//arg1 size: size of bits to swap
//arg2 var: pointer to variable to swap
void swap_data(int size, void* var);

//arg1: pointer to data which is parsed.
//returns pointer to interface data.
void * parse_packages(char *buf);

void parse_robot_state(Ur5SecondaryInterface * , char * , int32_t);
void parse_robot_message(Ur5PrimaryInterface *, char *);

#endif

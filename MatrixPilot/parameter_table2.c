// pyparam generated file - DO NOT EDIT

#include "defines.h" 

#if(SERIAL_OUTPUT_FORMAT == SERIAL_MAVLINK) 

#include "parameter_table.h"
#include "data_storage.h"
#include "../libUDB/ADchannel.h"


extern struct ADchannel udb_xaccel;
extern struct ADchannel udb_yaccel;
extern struct ADchannel udb_zaccel;
extern struct ADchannel udb_xrate;
extern struct ADchannel udb_yrate;
extern struct ADchannel udb_zrate;
extern int16_t height_target_min;
extern int16_t height_target_max;
extern int16_t height_margin;
extern fractional alt_hold_throttle_min;
extern fractional alt_hold_throttle_max;
extern int16_t alt_hold_pitch_min;
extern int16_t alt_hold_pitch_max;
extern int16_t alt_hold_pitch_high;
extern int16_t rtl_pitch_down;
extern int16_t minimum_groundspeed;
extern int16_t maximum_airspeed;
extern int16_t minimum_airspeed;
extern int16_t desiredSpeed;
extern int16_t minimum_groundspeed;
extern int16_t maximum_airspeed;
extern int16_t minimum_airspeed;
extern int16_t cruise_airspeed;
extern int16_t desiredSpeed;
extern int16_t airspeed_pitch_min_aspd;
extern int16_t airspeed_pitch_max_aspd;
extern int16_t airspeed_pitch_adjust_rate;
extern fractional airspeed_pitch_ki;
extern int16_t airspeed_pitch_ki_limit;


const mavlink_parameter_parser    mavlink_parameter_parsers[] = {
    { &mavlink_send_param_int16, &mavlink_set_param_int16, MAVLINK_TYPE_INT32_T},
    { &mavlink_send_param_Q14, &mavlink_set_param_Q14, MAVLINK_TYPE_FLOAT},
    { &mavlink_send_param_pwtrim, &mavlink_set_param_pwtrim, MAVLINK_TYPE_FLOAT},
    { &mavlink_send_param_gyroscale_Q14, &mavlink_set_param_gyroscale_Q14, MAVLINK_TYPE_FLOAT},
    { &mavlink_send_int_circular, &mavlink_set_int_circular, MAVLINK_TYPE_INT32_T},
    { &mavlink_send_dm_airspeed_in_cm, &mavlink_set_dm_airspeed_from_cm, MAVLINK_TYPE_INT32_T},
    { &mavlink_send_dm_airspeed_in_m, &mavlink_set_dm_airspeed_from_m, MAVLINK_TYPE_FLOAT},
    { &mavlink_send_cm_airspeed_in_m, &mavlink_set_cm_airspeed_from_m, MAVLINK_TYPE_FLOAT},
    { &mavlink_send_frame_anglerate, &mavlink_set_frame_anglerate, MAVLINK_TYPE_INT32_T},
    { &mavlink_send_dcm_angle, &mavlink_set_dcm_angle, MAVLINK_TYPE_INT32_T},
    };

/*
typedef union 
{
	float param_float;
	int32_t param_int32;
	uint32_t param_uint32;
} param_union_t;

typedef struct tag_mavlink_parameter 
{
	const char name[15];                // Name that will be displayed in the GCS
	param_union_t min;                  // Minimum allowed value for parameter
	param_union_t max;                  // Maximum allowed value for parameter
	udb_internal_type_e udb_param_type; // The internal UDB type for parsing
	uint8_t readonly;                   // Parameter is readonly (true) or Read / Write (false)
	uint8_t* pparam;                    // Reference to variable
	uint16_t param_size;                // parameter size in ram
} mavlink_parameter;
 */
#ifdef _MSC_VER
mavlink_parameter mavlink_parameters_list[] = {
#else
const mavlink_parameter mavlink_parameters_list[] = {
#endif // _MSC_VER
    {"PID_ROLLKP" , {0.0} , {0.5} , UDB_TYPE_Q14, PARAMETER_READWRITE, (void*) &rollkp, sizeof(rollkp) },
    {"PID_ROLLKD" , {0.0} , {0.5} , UDB_TYPE_GYROSCALE_Q14, PARAMETER_READWRITE, (void*) &rollkd, sizeof(rollkd) },
    {"PID_YAWKPAIL" , {0.0} , {0.5} , UDB_TYPE_Q14, PARAMETER_READWRITE, (void*) &yawkpail, sizeof(yawkpail) },
    {"PID_YAWKDAIL" , {0.0} , {0.5} , UDB_TYPE_GYROSCALE_Q14, PARAMETER_READWRITE, (void*) &yawkdail, sizeof(yawkdail) },
    {"PID_PITCHGAIN" , {0.0} , {0.5} , UDB_TYPE_Q14, PARAMETER_READWRITE, (void*) &pitchgain, sizeof(pitchgain) },
    {"PID_PITCHKD" , {0.0} , {0.5} , UDB_TYPE_Q14, PARAMETER_READWRITE, (void*) &pitchkd, sizeof(pitchkd) },
    {"PID_RUDELEVGAIN" , {0.0} , {0.5} , UDB_TYPE_Q14, PARAMETER_READWRITE, (void*) &rudderElevMixGain, sizeof(rudderElevMixGain) },
    {"PID_ROLLKPRUD" , {0.0} , {0.5} , UDB_TYPE_Q14, PARAMETER_READWRITE, (void*) &rollkprud, sizeof(rollkprud) },
    {"PID_YAWKPRUD" , {0.0} , {0.5} , UDB_TYPE_Q14, PARAMETER_READWRITE, (void*) &yawkprud, sizeof(yawkprud) },
    {"PID_YAWKDRUD" , {0.0} , {0.5} , UDB_TYPE_GYROSCALE_Q14, PARAMETER_READWRITE, (void*) &yawkdrud, sizeof(yawkdrud) },
    {"PID_ROLLKPRUD" , {0.0} , {0.5} , UDB_TYPE_Q14, PARAMETER_READWRITE, (void*) &rollkprud, sizeof(rollkprud) },
    {"PID_ROLLKDRUD" , {0.0} , {0.5} , UDB_TYPE_Q14, PARAMETER_READWRITE, (void*) &rollkdrud, sizeof(rollkdrud) },

    {"MAG_CAL_RAW0" , {-32767} , {32767} , UDB_TYPE_INT, PARAMETER_READWRITE, (void*) &rawMagCalib[0], sizeof(rawMagCalib[0]) },
    {"MAG_CAL_RAW1" , {-32767} , {32767} , UDB_TYPE_INT, PARAMETER_READWRITE, (void*) &rawMagCalib[1], sizeof(rawMagCalib[1]) },
    {"MAG_CAL_RAW2" , {-32767} , {32767} , UDB_TYPE_INT, PARAMETER_READWRITE, (void*) &rawMagCalib[2], sizeof(rawMagCalib[2]) },
    {"MAG_GAIN0" , {-32767} , {32767} , UDB_TYPE_INT, PARAMETER_READWRITE, (void*) &magGain[0], sizeof(magGain[0]) },
    {"MAG_GAIN1" , {-32767} , {32767} , UDB_TYPE_INT, PARAMETER_READWRITE, (void*) &magGain[1], sizeof(magGain[1]) },
    {"MAG_GAIN2" , {-32767} , {32767} , UDB_TYPE_INT, PARAMETER_READWRITE, (void*) &magGain[2], sizeof(magGain[2]) },
    {"MAG_OFFSET0" , {-32767} , {32767} , UDB_TYPE_INT, PARAMETER_READWRITE, (void*) &udb_magOffset[0], sizeof(udb_magOffset[0]) },
    {"MAG_OFFSET1" , {-32767} , {32767} , UDB_TYPE_INT, PARAMETER_READWRITE, (void*) &udb_magOffset[1], sizeof(udb_magOffset[1]) },
    {"MAG_OFFSET2" , {-32767} , {32767} , UDB_TYPE_INT, PARAMETER_READWRITE, (void*) &udb_magOffset[2], sizeof(udb_magOffset[2]) },
    {"MAG_DECLINATION" , {-180} , {180} , UDB_TYPE_INT_CIRCULAR, PARAMETER_READWRITE, (void*) &dcm_declination_angle.BB, sizeof(dcm_declination_angle.BB) },

    {"PWTRIM_AILERON" , {800.0} , {2200.0} , UDB_TYPE_PWTRIM, PARAMETER_READWRITE, (void*) &udb_pwTrim[AILERON_INPUT_CHANNEL], sizeof(udb_pwTrim[AILERON_INPUT_CHANNEL]) },
    {"PWTRIM_ELEVATOR" , {800.0} , {2200.0} , UDB_TYPE_PWTRIM, PARAMETER_READWRITE, (void*) &udb_pwTrim[ELEVATOR_INPUT_CHANNEL], sizeof(udb_pwTrim[ELEVATOR_INPUT_CHANNEL]) },
    {"PWTRIM_RUDDER" , {800.0} , {2200.0} , UDB_TYPE_PWTRIM, PARAMETER_READWRITE, (void*) &udb_pwTrim[RUDDER_INPUT_CHANNEL], sizeof(udb_pwTrim[RUDDER_INPUT_CHANNEL]) },
    {"PWTRIM_AILERON2" , {800.0} , {2200.0} , UDB_TYPE_PWTRIM, PARAMETER_READWRITE, (void*) &udb_pwTrim[AILERON_SECONDARY_INPUT_CHANNEL], sizeof(udb_pwTrim[AILERON_SECONDARY_INPUT_CHANNEL]) },
    {"PWTRIM_ROLL" , {800.0} , {2200.0} , UDB_TYPE_PWTRIM, PARAMETER_READWRITE, (void*) &udb_pwTrim[ROLL_INPUT_CHANNEL], sizeof(udb_pwTrim[ROLL_INPUT_CHANNEL]) },
    {"PWTRIM_PITCH" , {800.0} , {2200.0} , UDB_TYPE_PWTRIM, PARAMETER_READWRITE, (void*) &udb_pwTrim[PITCH_INPUT_CHANNEL], sizeof(udb_pwTrim[PITCH_INPUT_CHANNEL]) },
    {"PWTRIM_THROTTLE" , {800.0} , {2200.0} , UDB_TYPE_PWTRIM, PARAMETER_READWRITE, (void*) &udb_pwTrim[THROTTLE_INPUT_CHANNEL], sizeof(udb_pwTrim[THROTTLE_INPUT_CHANNEL]) },
    {"PWTRIM_YAW" , {800.0} , {2200.0} , UDB_TYPE_PWTRIM, PARAMETER_READWRITE, (void*) &udb_pwTrim[YAW_INPUT_CHANNEL], sizeof(udb_pwTrim[YAW_INPUT_CHANNEL]) },
    {"PWTRIM_FLAP" , {800.0} , {2200.0} , UDB_TYPE_PWTRIM, PARAMETER_READWRITE, (void*) &udb_pwTrim[FLAP_INPUT_CHANNEL], sizeof(udb_pwTrim[FLAP_INPUT_CHANNEL]) },
    {"PWTRIM_AIRBRAKE" , {800.0} , {2200.0} , UDB_TYPE_PWTRIM, PARAMETER_READWRITE, (void*) &udb_pwTrim[BRAKE_INPUT_CHANNEL], sizeof(udb_pwTrim[BRAKE_INPUT_CHANNEL]) },
    {"PWTRIM_SPOILER" , {800.0} , {2200.0} , UDB_TYPE_PWTRIM, PARAMETER_READWRITE, (void*) &udb_pwTrim[SPOILER_INPUT_CHANNEL], sizeof(udb_pwTrim[SPOILER_INPUT_CHANNEL]) },
    {"PWTRIM_CAMBER" , {800.0} , {2200.0} , UDB_TYPE_PWTRIM, PARAMETER_READWRITE, (void*) &udb_pwTrim[CAMBER_INPUT_CHANNEL], sizeof(udb_pwTrim[CAMBER_INPUT_CHANNEL]) },
    {"PWTRIM_CROW" , {800.0} , {2200.0} , UDB_TYPE_PWTRIM, PARAMETER_READWRITE, (void*) &udb_pwTrim[CROW_INPUT_CHANNEL], sizeof(udb_pwTrim[CROW_INPUT_CHANNEL]) },
    {"PWTRIM_CAMPITCH" , {800.0} , {2200.0} , UDB_TYPE_PWTRIM, PARAMETER_READWRITE, (void*) &udb_pwTrim[CAMERA_PITCH_INPUT_CHANNEL], sizeof(udb_pwTrim[CAMERA_PITCH_INPUT_CHANNEL]) },
    {"PWTRIM_CAM_YAW" , {800.0} , {2200.0} , UDB_TYPE_PWTRIM, PARAMETER_READWRITE, (void*) &udb_pwTrim[CAMERA_YAW_INPUT_CHANNEL], sizeof(udb_pwTrim[CAMERA_YAW_INPUT_CHANNEL]) },

    {"IMU_XACCEL_OFF" , {-32767} , {32767} , UDB_TYPE_INT, PARAMETER_READWRITE, (void*) &udb_xaccel.offset, sizeof(udb_xaccel.offset) },
    {"IMU_YACCEL_OFF" , {-32767} , {32767} , UDB_TYPE_INT, PARAMETER_READWRITE, (void*) &udb_yaccel.offset, sizeof(udb_yaccel.offset) },
    {"IMU_ZACCEL_OFF" , {-32767} , {32767} , UDB_TYPE_INT, PARAMETER_READWRITE, (void*) &udb_zaccel.offset, sizeof(udb_zaccel.offset) },
    {"IMU_XGYRO_OFF" , {-32767} , {32767} , UDB_TYPE_INT, PARAMETER_READWRITE, (void*) &udb_xrate.offset, sizeof(udb_xrate.offset) },
    {"IMU_YGYRO_OFF" , {-32767} , {32767} , UDB_TYPE_INT, PARAMETER_READWRITE, (void*) &udb_yrate.offset, sizeof(udb_yrate.offset) },
    {"IMU_ZGYRO_OFF" , {-32767} , {32767} , UDB_TYPE_INT, PARAMETER_READWRITE, (void*) &udb_zrate.offset, sizeof(udb_zrate.offset) },
    {"IMU_VREF_OFF" , {-32767} , {32767} , UDB_TYPE_INT, PARAMETER_READWRITE, (void*) &udb_vref.offset, sizeof(udb_vref.offset) },

    {"TH_H_TARGET_MIN" , {0} , {1} , UDB_TYPE_INT, PARAMETER_READWRITE, (void*) &height_target_min, sizeof(height_target_min) },
    {"TH_H_TARGET_MAX" , {0} , {1} , UDB_TYPE_INT, PARAMETER_READWRITE, (void*) &height_target_max, sizeof(height_target_max) },
    {"TH_H_MARGIN" , {1} , {500} , UDB_TYPE_INT, PARAMETER_READWRITE, (void*) &height_margin, sizeof(height_margin) },
    {"TH_T_HOLD_MIN" , {0} , {1} , UDB_TYPE_Q14, PARAMETER_READWRITE, (void*) &alt_hold_throttle_min, sizeof(alt_hold_throttle_min) },
    {"TH_T_HOLD_MAX" , {0} , {1} , UDB_TYPE_Q14, PARAMETER_READWRITE, (void*) &alt_hold_throttle_max, sizeof(alt_hold_throttle_max) },
    {"TH_P_HOLD_MIN" , {-89} , {0} , UDB_TYPE_INT, PARAMETER_READWRITE, (void*) &alt_hold_pitch_min, sizeof(alt_hold_pitch_min) },
    {"TH_P_HOLD_MAX" , {0} , {89} , UDB_TYPE_INT, PARAMETER_READWRITE, (void*) &alt_hold_pitch_max, sizeof(alt_hold_pitch_max) },
    {"TH_P_HIGH" , {0} , {89} , UDB_TYPE_INT, PARAMETER_READWRITE, (void*) &alt_hold_pitch_high, sizeof(alt_hold_pitch_high) },
    {"TH_P_RTL_DOWN" , {0} , {89} , UDB_TYPE_INT, PARAMETER_READWRITE, (void*) &rtl_pitch_down, sizeof(rtl_pitch_down) },

    {"ASPD_DESIRED" , {0} , {300.0} , UDB_TYPE_M_AIRSPEED_TO_DM, PARAMETER_READWRITE, (void*) &desiredSpeed, sizeof(desiredSpeed) },
    {"ASPD_MIN_GSPD" , {0} , {20000} , UDB_TYPE_M_AIRSPEED_TO_CM, PARAMETER_READWRITE, (void*) &minimum_groundspeed, sizeof(minimum_groundspeed) },
    {"ASPD_MIN" , {0} , {300.0} , UDB_TYPE_M_AIRSPEED_TO_CM, PARAMETER_READWRITE, (void*) &minimum_airspeed, sizeof(minimum_airspeed) },
    {"ASPD_MAX" , {0} , {300.0} , UDB_TYPE_M_AIRSPEED_TO_CM, PARAMETER_READWRITE, (void*) &maximum_airspeed, sizeof(maximum_airspeed) },
    {"ASPD_CRUISE" , {0} , {300.0} , UDB_TYPE_M_AIRSPEED_TO_CM, PARAMETER_READWRITE, (void*) &cruise_airspeed, sizeof(cruise_airspeed) },
    {"ASPD_P_MIN_ASPD" , {-90} , {90.0} , UDB_TYPE_DCM_ANGLE, PARAMETER_READWRITE, (void*) &airspeed_pitch_min_aspd, sizeof(airspeed_pitch_min_aspd) },
    {"ASPD_P_MAX_ASPD" , {-90} , {90.0} , UDB_TYPE_DCM_ANGLE, PARAMETER_READWRITE, (void*) &airspeed_pitch_max_aspd, sizeof(airspeed_pitch_max_aspd) },
    {"ASPD_P_RATE_LIM" , {1.0} , {720.0} , UDB_TYPE_FRAME_ANGLERATE, PARAMETER_READWRITE, (void*) &airspeed_pitch_adjust_rate, sizeof(airspeed_pitch_adjust_rate) },
    {"ASPD_P_KI" , {0.0} , {1.0} , UDB_TYPE_Q14, PARAMETER_READWRITE, (void*) &airspeed_pitch_ki, sizeof(airspeed_pitch_ki) },
    {"ASPD_P_KI_LIMIT" , {0.0} , {45.0} , UDB_TYPE_DCM_ANGLE, PARAMETER_READWRITE, (void*) &airspeed_pitch_ki_limit, sizeof(airspeed_pitch_ki_limit) },

    };

const uint16_t count_of_parameters_list = sizeof(mavlink_parameters_list) / sizeof(mavlink_parameter);


#endif 


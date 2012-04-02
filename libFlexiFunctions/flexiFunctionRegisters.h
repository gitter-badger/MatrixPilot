#ifndef FLEXIFUNCTION_REGISTERS_H
#define FLEXIFUNCTION_REGISTERS_H
// pyFEdit generated file - DO NOT EDIT


typedef enum
{
REG_NULL,
REG_PWIN_ROLL,
REG_PWIN_PITCH,
REG_PWIN_THROTTLE,
REG_PWIN_YAW,
REG_PWIN_FLAP,
REG_PWIN_CAMBER,
REG_PWIN_BRAKE,
REG_TRIM_POINT,
REG_TRIM_THROTTLE,
REG_APCON_ROLL,
REG_APCON_PITCH,
REG_APCON_THROTTLE,
REG_APCON_YAW,
REG_APCON_CAMBER,
REG_APCON_BRAKE,
REG_APCON_WAGGLE,
REG_APMODE_FULL,
REG_RADIO_MANUAL_MODE,
REG_APMODE_RADIO_ON,
REG_GAIN_MAN_MIX,
REG_MAN_GAIN_ROLL,
REG_MAN_GAIN_PITCH,
REG_MAN_GAIN_YAW,
REG_MAN_GAIN_THROTTLE,
REG_CON_ROLL,
REG_CON_PITCH,
REG_CON_YAW,
REG_CON_THROTTLE,
REG_CON_CAMBER,
REG_CON_BRAKE,
REG_CON_FLAP,
REG_AILERON_L,
REG_ELEVATOR,
REG_THROTTLE,
REG_RUDDER,
REG_AILERON_R,
REG_FLAPMID_L,
REG_FLAPMID_R,
REG_FLAP_L,
REG_FLAP_R,
REG_SPOILER,
REG_MAX
};

typedef enum
{
VIRTUAL_INPUT_PWIN_ROLL,
VIRTUAL_INPUT_PWIN_PITCH,
VIRTUAL_INPUT_PWIN_YAW,
VIRTUAL_INPUT_PWIN_THROTTLE,
VIRTUAL_INPUT_PWIN_FLAP,
VIRTUAL_INPUT_PWIN_CAMBER,
VIRTUAL_INPUT_PWIN_BRAKE,
VIRTUAL_INPUT_APCON_ROLL,
VIRTUAL_INPUT_APCON_PITCH,
VIRTUAL_INPUT_APCON_YAW,
VIRTUAL_INPUT_APCON_THROTTLE,
VIRTUAL_INPUT_APCON_FLAP,
VIRTUAL_INPUT_APCON_CAMBER,
VIRTUAL_INPUT_APCON_BRAKE,
VIRTUAL_INPUT_APCON_WAGGLE,
VIRTUAL_INPUT_APMODE_FULL,
VIRTUAL_INPUT_RADIO_MANUAL_MODE,
VIRTUAL_INPUT_APMODE_RADIO_ON,
};

typedef enum
{
VIRTUAL_OUTPUT_AILERON_L,
VIRTUAL_OUTPUT_ELEVATOR,
VIRTUAL_OUTPUT_THROTTLE,
VIRTUAL_OUTPUT_RUDDER,
VIRTUAL_OUTPUT_AILERON_R,
VIRTUAL_OUTPUT_FLAPMID_L,
VIRTUAL_OUTPUT_FLAPMID_R,
VIRTUAL_OUTPUT_FLAP_L,
VIRTUAL_OUTPUT_FLAP_R,
VIRTUAL_OUTPUT_SPOILER,
};

#endif

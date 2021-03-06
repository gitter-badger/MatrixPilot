// This file is part of MatrixPilot.
//
//    http://code.google.com/p/gentlenav/
//
// Copyright 2009-2011 MatrixPilot Team
// See the AUTHORS.TXT file for a list of authors of MatrixPilot.
//
// MatrixPilot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MatrixPilot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MatrixPilot.  If not, see <http://www.gnu.org/licenses/>.


#include "defines.h"
#include "../libUDB/heartbeat.h"
#include "mode_switch.h"
#include "airspeedCntrl.h"

//	routines to drive the PWM pins for the servos,
//	assumes the use of the 16MHz crystal.

// these are the desired pitch/roll angles (in DCM radians x.xx format)
// and yaw rate setpoint
int16_t pitch_setpoint, roll_setpoint, yaw_rate;
float xgain = 0;

// these are the final PWM outputs to the servos/ESC (2 * usec)
int16_t pitch_control, roll_control, yaw_control, throttle_control;

uint16_t wind_gain;

#if (USE_SBUS_INPUT == 1)
void parseSbusData(void);
extern boolean sbusDAV;
#endif

void manualPassthrough(void);

void init_servoPrepare(void) // initialize the PWM
{
    int16_t i;

#if (USE_NV_MEMORY == 1)
    if (udb_skip_flags.skip_radio_trim == 1)
        return;
#endif

    for (i = 0; i <= NUM_INPUTS; i++)
#if (FIXED_TRIMPOINT == 1)
        // don't send a valid throttle pulse yet (prevents ESC from arming)
        udb_pwIn[i] = ((i == THROTTLE_INPUT_CHANNEL) ? 0 : CHANNEL_TRIMPOINT);
    udb_pwTrim[i] = udb_pwIn[i];
#else
    // FIXME: looks like this assignment is useless
        udb_pwTrim[i] = ((i == THROTTLE_INPUT_CHANNEL) ? 0 : 3000);
    udb_pwTrim[i] = udb_pwIn[i];
#endif

#if (FIXED_TRIMPOINT == 1)
    // don't send a valid throttle pulse yet (prevents ESC from arming)
    for (i = 0; i <= NUM_OUTPUTS; i++)
        udb_pwOut[i] = ((i == THROTTLE_OUTPUT_CHANNEL) ? 0 : CHANNEL_TRIMPOINT);
#else
    for (i = 0; i <= NUM_OUTPUTS; i++)
        udb_pwOut[i] = ((i == THROTTLE_OUTPUT_CHANNEL) ? 0 : 3000);
#endif

#if (NORADIO == 1)
    udb_pwIn[MODE_SWITCH_INPUT_CHANNEL] = udb_pwTrim[MODE_SWITCH_INPUT_CHANNEL] = 4000;
#endif
}

// Called at HEARTBEAT_HZ

void dcm_servo_callback_prepare_outputs(void) {
#if (USE_SBUS_INPUT == 1)
    if (sbusDAV) {
        parseSbusData();
        #if (SILSIM == 1)
            // hardwired xgain
            xgain = 1.0/32;
        #else
            // channel 7 is xacc gain: scale PWM range of [2000,4000] to [0,1/8]
            xgain = ((float)(udb_pwIn[7] - 2000)) / (8 * 2000.0f);
            if (xgain < 0) xgain = 0;
        #endif
    }

#endif

    if (dcm_flags._.calib_finished) {
        if (udb_heartbeat_counter % (HEARTBEAT_HZ / 40) == 0) {
            flight_mode_switch_2pos_poll(); // we always want this called at 40Hz
        }
#if (DEADRECKONING == 1)
        process_flightplan();
#endif
#if (ALTITUDE_GAINS_VARIABLE == 1)
        airspeedCntrl();
#endif // ALTITUDE_GAINS_VARIABLE
        updateBehavior();
        wind_gain = wind_gain_adjustment();
        rollCntrl();
        yawCntrl();
        altitudeCntrl();
        pitchCntrl();
        servoMix();
#if (USE_CAMERA_STABILIZATION == 1)
        cameraCntrl();
#endif
        cameraServoMix();
        updateTriggerAction();
    } else {
        // otherwise, there is not anything to do
        manualPassthrough(); // Allow manual control while starting up
    }

    if (dcm_flags._.calib_finished) // start telemetry after calibration
    {
#if (SERIAL_OUTPUT_FORMAT == SERIAL_MAVLINK)
        mavlink_output();
#elif (SERIAL_OUTPUT_FORMAT == SERIAL_DEBUG)
        //extern void serial_output_nmea();
        //                serial_output_nmea();
        extern void serial_output_stest();
        // call serial_output_stest at 40 Hz
        if (udb_heartbeat_counter % (HEARTBEAT_HZ / 40) == 0) {
            serial_output_stest();
        }

#else
        // This is a simple check to send telemetry at 8hz
        //		if (udb_heartbeat_counter % 5 == 0)
        if (udb_heartbeat_counter % (HEARTBEAT_HZ / 8) == 0) {
            serial_output_8hz();
        }
#endif
    }

#if (USE_OSD == 1)
    osd_run_step();
#endif
}

void manualPassthrough(void) {
    roll_control = pitch_control = yaw_control = throttle_control = 0;
    servoMix();
}

//
//  SIL-udb.h
//  MatrixPilot-SIL
//
//  Created by Ben Levitt on 2/1/13.
//  Copyright (c) 2013 MatrixPilot. All rights reserved.
//

#include "libUDB.h"
#include "events.h"
#include "SIL-udb.h"
#include "UDBSocket.h"
#include "SIL-events.h"
#include "SIL-eeprom.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>


char leds[4] = {0, 0, 0, 0};
uint8_t lastLedBits = 0;

uint16_t udb_heartbeat_counter;

int16_t udb_pwIn[MAX_INPUTS];		// pulse widths of radio inputs
int16_t udb_pwTrim[MAX_INPUTS];	// initial pulse widths for trimming
int16_t udb_pwOut[MAX_OUTPUTS];		// pulse widths for servo outputs

union udb_fbts_byte udb_flags;

struct ADchannel udb_xaccel, udb_yaccel, udb_zaccel;	// x, y, and z accelerometer channels
struct ADchannel udb_xrate, udb_yrate, udb_zrate;	// x, y, and z gyro channels
struct ADchannel udb_vref;							// reference voltage
struct ADchannel udb_analogInputs[4];

fractional udb_magFieldBody[3];
fractional udb_magOffset[3];

union longww battery_current;	// battery_current._.W1 is in tenths of Amps
union longww battery_mAh_used;	// battery_mAh_used._.W1 is in mAh
union longww battery_voltage;	// battery_voltage._.W1 is in tenths of Volts
unsigned char rc_signal_strength;	// rc_signal_strength is 0-100 as percent of full signal

int16_t magMessage ;
int16_t vref_adj ;

int32_t gpsRate = 0;
int32_t serialRate = 0;

boolean showLEDs = 0;

SILSocket stdioSocket;
SILSocket gpsSocket, telemetrySocket;
SILSocket serialSocket;


boolean readUDBSockets(void);
void checkForLedUpdates(void);
void sil_handle_key_input(char c);


void udb_init(void)
{
	int16_t i;
	for (i=0; i<NUM_INPUTS; i++) {
		if (i == THROTTLE_INPUT_CHANNEL) {
			udb_pwIn[i] = 0;
			udb_pwTrim[i] = 0;
		}
		else {
			udb_pwIn[i] = 1500;
			udb_pwTrim[i] = 1500;
		}
	}
	for (i=0; i<NUM_OUTPUTS; i++) {
		udb_pwOut[i] = 0;
	}
	
	udb_flags.B = 0;
	
	udb_heartbeat_counter = 0;
	
	stdioSocket = SILSocket_init(SILSocketStandardInOut, 0, NULL, 0);
	
	gpsSocket = SILSocket_init((SILSIM_GPS_SERVER) ? SILSocketUDPServer : SILSocketUDPClient, SILSIM_GPS_PORT, NULL, 0);
	telemetrySocket = SILSocket_init((SILSIM_TELEMETRY_SERVER) ? SILSocketUDPServer : SILSocketUDPClient, SILSIM_TELEMETRY_PORT, NULL, 0);
	
	if (strlen(SILSIM_SERIAL_INPUT_DEVICE) > 0) {
		serialSocket = SILSocket_init(SILSocketSerial, 0, SILSIM_SERIAL_INPUT_DEVICE, SILSIM_SERIAL_INPUT_BAUD);
	}
	else {
		udb_flags._.radio_on = 1;
	}
}


#define UDB_STEP_TIME 25000
#define UDB_WRAP_TIME 1000000

void udb_run(void)
{
	struct timeval tv;
	struct timezone tz;
	int32_t currentTime = 0;
	int32_t nextHeartbeatTime = 0;
	
	gettimeofday(&tv,&tz);
	nextHeartbeatTime = tv.tv_usec + UDB_STEP_TIME;
	
	while (1) {
		if (!readUDBSockets()) {
			usleep(1000);
		}
		
		gettimeofday(&tv,&tz);
		currentTime = tv.tv_usec;
		
		if (currentTime > nextHeartbeatTime && (nextHeartbeatTime > UDB_STEP_TIME || currentTime < UDB_WRAP_TIME-UDB_STEP_TIME)) {
			udb_callback_read_sensors();
			udb_background_callback_periodic();
			//udb_magnetometer_callback_data_available();
			udb_servo_callback_prepare_outputs();
			checkForLedUpdates();
			
			udb_heartbeat_counter++;
			nextHeartbeatTime = nextHeartbeatTime + UDB_STEP_TIME;
			if (nextHeartbeatTime > UDB_WRAP_TIME) nextHeartbeatTime -= UDB_WRAP_TIME;
		}
		process_queued_events();
		writeEEPROMFileIfNeeded();
	}
}


void udb_background_trigger(void)
{
	udb_background_callback_triggered();
}


unsigned char udb_cpu_load(void)
{
	return 1; // 1% sounds reasonable for a fake cpu%
}


int16_t  udb_servo_pulsesat(int32_t pw)
{
	if ( pw > SERVOMAX ) pw = SERVOMAX ;
	if ( pw < SERVOMIN ) pw = SERVOMIN ;
	return (int16_t)pw ;
}


void udb_servo_record_trims(void)
{
	int16_t i;
	for (i=0; i <= NUM_INPUTS; i++)
		udb_pwTrim[i] = udb_pwIn[i] ;
	
	return ;
}


void udb_set_action_state(boolean newValue)
{
	// not simulated
	(void)newValue;   // unused parameter
}


void udb_a2d_record_offsets(void)
{
	UDB_XACCEL.offset = UDB_XACCEL.value ;
	udb_xrate.offset = udb_xrate.value ;
	UDB_YACCEL.offset = UDB_YACCEL.value - ( Y_GRAVITY_SIGN ((int16_t)(2*GRAVITY)) ); // opposite direction
	udb_yrate.offset = udb_yrate.value ;
	UDB_ZACCEL.offset = UDB_ZACCEL.value ;
	udb_zrate.offset = udb_zrate.value ;
	udb_vref.offset = udb_vref.value ;

}


void printLEDStatus(void)
{
	printf("LEDs: %c %c %c %c\n",
		   (leds[0]) ? 'R' : '-',
		   (leds[1]) ? 'G' : '-',
		   (leds[2]) ? 'O' : '-',
		   (leds[3]) ? 'B' : '-');
}


void checkForLedUpdates(void)
{
	uint8_t newLedBits = 0;
	if (leds[0]) newLedBits |= 1;
	if (leds[1]) newLedBits |= 2;
	if (leds[2]) newLedBits |= 4;
	if (leds[3]) newLedBits |= 8;
	
	if (lastLedBits	!= newLedBits) {
		if (showLEDs) {
			printLEDStatus();
		}
		lastLedBits	= newLedBits;
	}
}


#define BUFLEN 512

boolean readUDBSockets(void)
{
	unsigned char buffer[BUFLEN];
	int32_t bytesRead;
	int16_t i;
	boolean didRead = false;
	
	if (gpsSocket) {
		bytesRead = SILSocket_read(gpsSocket, buffer, BUFLEN);
		if (bytesRead < 0) {
			SILSocket_close(gpsSocket);
			gpsSocket = NULL;
		}
		else {
			for (i=0; i<bytesRead; i++) {
				udb_gps_callback_received_byte(buffer[i]);
			}
			if (bytesRead>0) didRead = true;
		}
	}
	
	if (telemetrySocket) {
		bytesRead = SILSocket_read(telemetrySocket, buffer, BUFLEN);
		if (bytesRead < 0) {
			SILSocket_close(telemetrySocket);
			telemetrySocket = NULL;
		}
		else {
			for (i=0; i<bytesRead; i++) {
				udb_serial_callback_received_byte(buffer[i]);
			}
			if (bytesRead>0) didRead = true;
		}
	}
	
	if (serialSocket) {
		bytesRead = SILSocket_read(serialSocket, buffer, BUFLEN);
		if (bytesRead < 0) {
			SILSocket_close(serialSocket);
			serialSocket = NULL;
		}
		else {
			if (bytesRead >= 20 && buffer[0]==0xFF && buffer[1]==0xEE) {
				for (i=0; i<NUM_INPUTS; i++) {
					udb_pwIn[i+1] = (uint16_t)(buffer[i*2+2])*256 + buffer[i*2+3];
					if ((i+1) == FAILSAFE_INPUT_CHANNEL) {
						udb_flags._.radio_on = (udb_pwIn[i] >= FAILSAFE_INPUT_MIN && udb_pwIn[i] <= FAILSAFE_INPUT_MAX);
					}
				}
			}
			if (bytesRead>0) didRead = true;
		}
	}
	
	if (stdioSocket) {
		bytesRead = SILSocket_read(stdioSocket, buffer, BUFLEN);
		for (i=0; i<bytesRead; i++) {
			sil_handle_key_input(buffer[i]);
		}
		if (bytesRead>0) didRead = true;
	}
		
	return didRead;
}


void sil_rc_input_adjust(int inChannelIndex, int delta)
{
	udb_pwIn[inChannelIndex] = udb_servo_pulsesat(udb_pwIn[inChannelIndex] + delta);
}


void sil_reset(void)
{
	printf("\nTODO: Implement Reset\n");
}


#define KEYPRESS_INPUT_DELTA 50

void sil_handle_key_input(char c)
{
	switch (c) {
		case '?':
			printf("\n");
			printf("1/2/3 = mode manual/stabilized/waypoint\n");
			printf("w/s   = throttle up/down\n");
			printf("a/d   = rudder left/right\n");
			printf("i/k   = elevetor forward/back\n");
			printf("j/l   = aileron left/right\n");
			printf("\n");
			printf("z     = zero the sticks\n");
			printf("shift-L     = toggle LEDs\n");
			printf("r     = reset\n");
			break;
			
		case 'w':
			sil_rc_input_adjust(THROTTLE_INPUT_CHANNEL, KEYPRESS_INPUT_DELTA);
			break;
			
		case 's':
			sil_rc_input_adjust(THROTTLE_INPUT_CHANNEL, -KEYPRESS_INPUT_DELTA);
			break;
			
		case 'a':
			sil_rc_input_adjust(RUDDER_INPUT_CHANNEL, KEYPRESS_INPUT_DELTA);
			break;
			
		case 'd':
			sil_rc_input_adjust(RUDDER_INPUT_CHANNEL, -KEYPRESS_INPUT_DELTA);
			break;
			
		case 'i':
			sil_rc_input_adjust(ELEVATOR_INPUT_CHANNEL, KEYPRESS_INPUT_DELTA);
			break;
			
		case 'k':
			sil_rc_input_adjust(ELEVATOR_INPUT_CHANNEL, -KEYPRESS_INPUT_DELTA);
			break;
			
		case 'j':
			sil_rc_input_adjust(AILERON_INPUT_CHANNEL, KEYPRESS_INPUT_DELTA);
			break;
			
		case 'l':
			sil_rc_input_adjust(AILERON_INPUT_CHANNEL, -KEYPRESS_INPUT_DELTA);
			break;
			
		case 'z':
			udb_pwIn[AILERON_INPUT_CHANNEL] = udb_pwTrim[AILERON_INPUT_CHANNEL];
			udb_pwIn[ELEVATOR_INPUT_CHANNEL] = udb_pwTrim[ELEVATOR_INPUT_CHANNEL];
			udb_pwIn[RUDDER_INPUT_CHANNEL] = udb_pwTrim[RUDDER_INPUT_CHANNEL];
			break;
			
		case '1':
			udb_pwIn[MODE_SWITCH_INPUT_CHANNEL] = MODE_SWITCH_THRESHOLD_LOW - 1;
			break;
			
		case '2':
			udb_pwIn[MODE_SWITCH_INPUT_CHANNEL] = MODE_SWITCH_THRESHOLD_LOW + 1;
			break;
			
		case '3':
			udb_pwIn[MODE_SWITCH_INPUT_CHANNEL] = MODE_SWITCH_THRESHOLD_HIGH + 1;
			break;
			
		case 'L':
			showLEDs = !showLEDs;
			if (showLEDs) {
				printf("\n");
				printLEDStatus();
			}
			break;
			
		case 'r':
			sil_reset();
			break;
			
		default:
			break;
	}
}



//////////////////////////////////////////////////////////
// GPS and Serial
//////////////////////////////////////////////////////////

void udb_gps_set_rate(int32_t rate)
{
	gpsRate = rate;
}


boolean udb_gps_check_rate(int32_t rate)
{
	return (rate == gpsRate);
}


// Call this function to initiate sending a data to the GPS
void udb_gps_start_sending_data(void)
{
	if (!gpsSocket) return;
	
	unsigned char buffer[BUFLEN];
	int16_t c;
	int16_t pos=0;
	
	while ((c = udb_gps_callback_get_byte_to_send()) != -1) {
		buffer[pos++] = c;
	};
	
	int16_t bytesWritten = SILSocket_write(gpsSocket, (unsigned char*)buffer, pos);
	
	if (bytesWritten < 0) {
		SILSocket_close(gpsSocket);
		gpsSocket = NULL;
	}
}


void udb_serial_set_rate(int32_t rate)
{
	serialRate = rate;
}


boolean udb_serial_check_rate(int32_t rate)
{
	return (serialRate == rate);
}


// Call this function to initiate sending a data to the serial port
void udb_serial_start_sending_data(void)
{
	if (!telemetrySocket) return;
	
	unsigned char buffer[BUFLEN];
	int16_t c;
	int16_t pos=0;
	
	while ((c = udb_serial_callback_get_byte_to_send()) != -1) {
		buffer[pos++] = c;
	}
	
	int16_t bytesWritten = SILSocket_write(telemetrySocket, (unsigned char*)buffer, pos);
	
	if (bytesWritten == -1) {
		SILSocket_close(telemetrySocket);
		telemetrySocket = NULL;
	}
}


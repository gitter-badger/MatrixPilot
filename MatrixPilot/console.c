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
#include "MDD File System/FSIO.h"
#include "MDD File System/FSDefs.h"

#include "ymodem.h"
#include "../libUDB/libUDB.h"
#include "../libUDB/interrupt.h"
#include "../libDCM/estAltitude.h"
#include "../libUDB/uart.h"
#include <string.h>
#include <stdio.h>

#if (USE_CONFIGFILE == 1)
#include "config.h"
#include "redef.h"
#endif // USE_CONFIGFILE

#if (CONSOLE_UART != 0)

#define LOWORD(a) ((WORD)(a))
#define HIWORD(a) ((WORD)(((DWORD)(a) >> 16) & 0xFFFF))

void AT45D_FormatFS(void);

typedef struct tagCmds {
	int index;
	void (*fptr)(char*);
	const char const * cmdstr;
} cmds_t;


int cmdlen = 0;
char cmdstr[32];
int show_cpu_load = 0;


static void cmd_ver(char* arg)
{
	printf("MatrixPilot v0.1, " __TIME__ " " __DATE__ "\r\n");
}

static void cmd_format(char* arg)
{
#if (BOARD_TYPE == AUAV3_BOARD)
//	printf("formatting dataflash\r\n");
//	AT45D_FormatFS();
#endif // BOARD_TYPE
}

static void cmd_start(char* arg)
{
	printf("starting.\r\n");
	show_cpu_load = 1;
}

static void cmd_stop(char* arg)
{
	printf("stopped.\r\n");
	show_cpu_load = 0;
}

static void cmd_on(char* arg)
{
	printf("on.\r\n");
	SRbits.IPL = 0; // turn on all interrupt priorities
}

static void cmd_off(char* arg)
{
	printf("off.\r\n");
	SRbits.IPL = 7; // turn off all interrupt priorities
}

static void cmd_cpuload(char* arg)
{
	printf("CPU Load %u%%\r\n", udb_cpu_load());
}

static void cmd_crash(char* arg)
{
	static int i;
	char buffer[32];

	sprintf(buffer, "overflowing stack %u.\r\n", i++);
	printf(buffer);
	cmd_crash(arg);
}

static void cmd_adc(char* arg)
{
//	printf("ADC vcc %u, 5v %u, rssi %u\r\n", udb_vcc.value, udb_5v.value, udb_rssi.value);
}

static void cmd_barom(char* arg)
{
	printf("Barometer temp %i, pres %u, alt %u, agl %u\r\n",
	       get_barometer_temperature(),
	       (uint16_t)get_barometer_pressure(),
	       (uint16_t)get_barometer_altitude(),
	       (uint16_t)get_barometer_agl_altitude()
	      );
}

static void cmd_magno(char* arg)
{
}

static void cmd_options(char* arg)
{
#if (USE_CONFIGFILE == 1)
	printf("ROLL_STABILIZATION_AILERONS: %u\r\n", ROLL_STABILIZATION_AILERONS);
	printf("ROLL_STABILIZATION_RUDDER: %u\r\n", ROLL_STABILIZATION_RUDDER);
	printf("PITCH_STABILIZATION: %u\r\n", PITCH_STABILIZATION);
	printf("YAW_STABILIZATION_RUDDER: %u\r\n", YAW_STABILIZATION_RUDDER);
	printf("YAW_STABILIZATION_AILERON: %u\r\n", YAW_STABILIZATION_AILERON);
	printf("AILERON_NAVIGATION: %u\r\n", AILERON_NAVIGATION);
	printf("RUDDER_NAVIGATION: %u\r\n", RUDDER_NAVIGATION);
	printf("ALTITUDEHOLD_STABILIZED: %u\r\n", ALTITUDEHOLD_STABILIZED);
	printf("ALTITUDEHOLD_WAYPOINT: %u\r\n", ALTITUDEHOLD_WAYPOINT);
	printf("RACING_MODE: %u\r\n", RACING_MODE);
#endif
}

static void cmd_gains(char* arg)
{
#if (USE_CONFIGFILE == 1)
	printf("YAWKP_AILERON: %f\r\n", (double)gains.YawKPAileron);
	printf("YAWKD_AILERON: %f\r\n", (double)gains.YawKDAileron);
	printf("ROLLKP: %f\r\n", (double)gains.RollKP);
	printf("ROLLKD: %f\r\n", (double)gains.RollKD);
	printf("AILERON_BOOST: %f\r\n", (double)gains.AileronBoost);
	printf("PITCHGAIN: %f\r\n", (double)gains.Pitchgain);
	printf("PITCHKD: %f\r\n", (double)gains.PitchKD);
	printf("RUDDER_ELEV_MIX: %f\r\n", (double)gains.RudderElevMix);
	printf("ROLL_ELEV_MIX: %f\r\n", (double)gains.RollElevMix);
	printf("ELEVATOR_BOOST: %f\r\n", (double)gains.ElevatorBoost);
	printf("YAWKP_RUDDER: %f\r\n", (double)gains.YawKPRudder);
	printf("YAWKD_RUDDER: %f\r\n", (double)gains.YawKDRudder);
	printf("ROLLKP_RUDDER: %f\r\n", (double)gains.RollKPRudder);
	printf("ROLLKD_RUDDER: %f\r\n", (double)gains.RollKDRudder);
	printf("RUDDER_BOOST: %f\r\n", (double)gains.RudderBoost);
	printf("RTL_PITCH_DOWN: %f\r\n", (double)gains.RtlPitchDown);
	printf("HEIGHT_TARGET_MAX: %f\r\n", (double)gains.HeightTargetMax);
	printf("HEIGHT_TARGET_MIN: %f\r\n", (double)gains.HeightTargetMin);
	printf("ALT_HOLD_THROTTLE_MIN: %f\r\n", (double)gains.AltHoldThrottleMin);
	printf("ALT_HOLD_THROTTLE_MAX,: %f\r\n", (double)gains.AltHoldThrottleMax);
	printf("ALT_HOLD_PITCH_MIN: %f\r\n", (double)gains.AltHoldPitchMin);
	printf("ALT_HOLD_PITCH_MAX: %f\r\n", (double)gains.AltHoldPitchMax);
	printf("ALT_HOLD_PITCH_HIGH: %f\r\n", (double)gains.AltHoldPitchHigh);
#endif
}

static void printbin16(int a)
{
	unsigned int i;
	for (i = 0x8000; i > 0; i >>= 1) {
		if (a & i) printf("1");
		else printf("0");
	}
}

const char* byte_to_binary(int x)
{
	static char b[9];
	int z;

	b[0] = '\0';
	for (z = 128; z > 0; z >>= 1) {
		strcat(b, ((x & z) == z) ? "1" : "0");
	}
	return b;
}

const char* word_to_binary(int x)
{
	static char b[17];
	unsigned int z;

	b[0] = '\0';
	for (z = 0x8000; z > 0; z >>= 1) {
		strcat(b, ((x & z) == z) ? "1" : "0");
	}
	return b;
}

void gentrap(void);

static void cmd_trap(char* arg)
{
	gentrap();
}

static void cmd_reg(char* arg)
{
#if (BOARD_TYPE == AUAV3_BOARD)
	printf("USB Registers:\r\n");
	printf("\tU1OTGSTAT = %s\r\n", word_to_binary(U1OTGSTAT));
	printf("\tU1OTGCON  = %s\r\n", word_to_binary(U1OTGCON));
	printf("\tU1STAT    = %s\r\n", word_to_binary(U1STAT));
	printf("\tU1CON     = %s\r\n", word_to_binary(U1CON));
	printf("\tU1CNFG1   = %s\r\n", word_to_binary(U1CNFG1));
	printf("\tU1CNFG2   = %s\r\n", word_to_binary(U1CNFG2));
	printf("\tU1OTGIR   = %s\r\n", word_to_binary(U1OTGIR));
	printf("\tU1OTGIE   = %s\r\n", word_to_binary(U1OTGIE));

	printf("IC Registers:\r\n");
	printf("\tIC1CON1 = %s %04x\r\n", word_to_binary(IC1CON1), IC1CON1);
	printf("\tIC1CON2 = %s %04x\r\n", word_to_binary(IC1CON2), IC1CON2);
	printf("\tIC2CON1 = %s %04x\r\n", word_to_binary(IC2CON1), IC2CON1);
	printf("\tIC2CON2 = %s %04x\r\n", word_to_binary(IC2CON2), IC2CON2);
#endif // BOARD_TYPE
/*
UxOTGSTAT: USB OTG STATUS REGISTER
VBUSVD: A-VBUS Valid Indicator bit
1 = The VBUS voltage is above VA_VBUS_VLD (as defined in the USB OTG Specification) on the A device
0 = The VBUS voltage is below VA_VBUS_VLD on the A device


UxOTGCON: USB ON-THE-GO CONTROL REGISTER
bit 3 VBUSON: VBUS Power-on bit(1)
1 = VBUS line is powered
0 = VBUS line is not powered

UxCNFG2: USB CONFIGURATION REGISTER 2
 */
}

#if (RECORD_FREE_STACK_SPACE == 1)
extern uint16_t maxstack;
#endif

static void cmd_stack(char* arg)
{
#if (RECORD_FREE_STACK_SPACE == 1)
	printf("maxstack %x\r\n", maxstack);
	printf("SP_start %x\r\n", SP_start());
	printf("SP_limit %x\r\n", SP_limit());
	printf("SP_current %x\r\n", SP_current());
	printf("stack usage %u\r\n", maxstack - SP_start());
#else
	printf("stack reporting disabled.\r\n");
#endif
}

static void cmd_reset(char* arg)
{
	asm("reset");
}

static void cmd_help(char* arg);

void log_close(void);

static void cmd_close(char* arg)
{
#if (USE_TELELOG == 1)
	log_close();
#endif
}

extern unsigned long ymodem_receive(unsigned char *buf, unsigned long length);
extern unsigned long ymodem_send(unsigned char *buf, unsigned long size, char* filename);

static void cmd_send(char* arg)
{
#if (USE_YMODEM == 1)
	unsigned char* buf;
	unsigned long length;
	unsigned long result;

	result = ymodem_receive(buf, length);
#endif
}

static void cmd_receive(char* arg)
{
#if (USE_YMODEM == 1)
	unsigned char* buf;
	unsigned long size;
	char* filename;
	unsigned long result;

	result = ymodem_send(buf, size, filename);
#endif
}
/*
// Summary: A structure used for searching for files on a device.
// Description: The SearchRec structure is used when searching for file on a device.  It contains parameters that will be loaded with
//              file information when a file is found.  It also contains the parameters that the user searched for, allowing further
//              searches to be perfomed in the same directory for additional files that meet the specified criteria.
typedef struct
{
    char            filename[FILE_NAME_SIZE_8P3 + 2];   // The name of the file that has been found
    unsigned char   attributes;                     // The attributes of the file that has been found
    unsigned long   filesize;                       // The size of the file that has been found
    unsigned long   timestamp;                      // The last modified time of the file that has been found (create time for directories)
	#ifdef SUPPORT_LFN
		BOOL			AsciiEncodingType;          // Ascii file name or Non-Ascii file name indicator
		unsigned short int *utf16LFNfound;		    // Pointer to long file name found in UTF16 format
		unsigned short int utf16LFNfoundLength;     // LFN Found length in terms of words including the NULL word at the last.
	#endif
    unsigned int    entry;                          // The directory entry of the last file found that matches the specified attributes. (Internal use only)
    char            searchname[FILE_NAME_SIZE_8P3 + 2]; // The 8.3 format name specified when the user began the search. (Internal use only)
    unsigned char   searchattr;                     // The attributes specified when the user began the search. (Internal use only)
    unsigned long   cwdclus;                        // The directory that this search was performed in. (Internal use only)
    unsigned char   initialized;                    // Check to determine if the structure was initialized by FindFirst (Internal use only)
} SearchRec;
 */
static void cmd_dir(char* arg)
{
#if (USE_TELELOG == 1 || USE_CONFIGFILE == 1)
	SearchRec rec;
	char* fileName = "*.*";

//int FindFirst(const char* fileName, unsigned int attr, SearchRec* rec);
//int FindNext(SearchRec* rec); 

	if (arg != NULL) {
		fileName = arg;
	}
	if (FindFirst(fileName, ATTR_MASK, &rec) != -1) {
		do {
			printf("%s\r\n", rec.filename);
		} while (FindNext(&rec) != -1);
	}
#endif
}

//size_t FSfread(void *ptr, size_t size, size_t n, FSFILE *stream);
static void cmd_cat(char* arg)
{
#if (USE_TELELOG == 1 || USE_CONFIGFILE == 1)
	char buf[2];
	FSFILE* fp;

	printf("cmd_cat(%s)\r\n", arg);

	fp = FSfopen(arg, "r");
	if (fp != NULL) {
		while (FSfread(buf, 1, sizeof(char), fp) == 1) {
			printf("%c", buf[0]);
		}
		FSfclose(fp);
	}
#endif
}

double gcdist(double lat1, double lon1, double lat2, double lon2);

double gcdist(double lat1, double lon1, double lat2, double lon2) // Compute distance from [lat1,lon1] to [lat2,lon2]
{
	double result;

//	double pow(double, double);
//	double p1, p2;

//	p1 = pow((sin((lat1 - lat2) / 2)), 2);
//	p2 = 
	result = 2 * asin(sqrt( pow((sin((lat1 - lat2) / 2)), 2) + cos(lat1) * cos(lat2) * pow((sin((lon1 - lon2) / 2)), 2)));
	return result;
}

//	  radians = degrees * 3.1415926 / 180;

static void cmd_nav(char* arg)
{
	double lat1 = -0.025244442;
	double lon1 = 1.892460508;
	double lat2 = -0.645946356;
	double lon2 = -3.052929928;
	double dist;

	dist = gcdist(lat1, lon1, lat2, lon2);
	printf("lat1 %f lon1 %f lat2 %f lon2 %f\r\n", lat1, lon1, lat2, lon2);
	printf("gcdist = %f rad\r\n", dist);

	dist = dist * 180 * 60 / 3.1415926;
	printf("gcdist = %f Nm\r\n", dist);
}
/*
	deg:min:sec	rad			rad	nm
latitude1	S1.4464	-0.025244442		distance	1.36993292	4709.482
longitude1	W108.43	1.892460508				deg
latitude2	S37.01	-0.645946356		bearing 1->2	4.057135924	232.4568
longitude2	E174.92	-3.052929928		bearing 2->1	1.449270091	83.0371
 */

/*
lat1 -0.025244 lon1 1.892460 lat2 -0.645946 lon2
gcdist = 1.369933

lat1 -0.025244 lon1 1.892460 lat2 -0.645946 lon2 -3.052930
gcdist = 1.369933 rad
gcdist = 4709.484375 Nm
 */

const cmds_t cmdslist[] = {
	{ 0, cmd_help,   "help" },
	{ 0, cmd_ver,    "ver" },
	{ 0, cmd_format, "format" },
	{ 0, cmd_start,  "start" },
	{ 0, cmd_stop,   "stop" },
	{ 0, cmd_on,     "on" },
	{ 0, cmd_off,    "off" },
	{ 0, cmd_stack,  "stack" },
	{ 0, cmd_reg,    "reg" },
	{ 0, cmd_adc,    "adc" },
	{ 0, cmd_barom,  "bar" },
	{ 0, cmd_cpuload,"cpu" },
	{ 0, cmd_magno,  "mag" },
	{ 0, cmd_crash,  "crash" },
	{ 0, cmd_gains,  "gains" },
	{ 0, cmd_options,"options" },
	{ 0, cmd_reset,  "reset" },
	{ 0, cmd_trap,   "trap" },
	{ 0, cmd_close,  "close" },
	{ 0, cmd_send,   "send" },
	{ 0, cmd_receive,"receive" },
	{ 0, cmd_dir,    "dir" },
	{ 0, cmd_cat,    "cat" },
	{ 0, cmd_nav,    "nav" },
};

static void cmd_help(char* arg)
{
	int i;

	printf("Commands:\r\n");
	for (i = 0; i < (sizeof(cmdslist)/sizeof(cmdslist[0])); i++) {
		printf("\t%s\r\n", cmdslist[i].cmdstr);
	}
}

static void command(char* cmdstr, int cmdlen)
{
	int i;
	char* argstr = NULL;

	for (i = 0; i < cmdlen; i++) {
		if (cmdstr[i] == ' ') {
			cmdstr[i] = '\0';
			argstr = cmdstr + i + 1;
		}
	}
	for (i = 0; i < (sizeof(cmdslist)/sizeof(cmdslist[0])); i++) {
		if (strcmp(cmdslist[i].cmdstr, cmdstr) == 0) {
			cmdslist[i].fptr(argstr);
		}
	}
}

void console_inbyte(char ch)
{
	if (cmdlen < sizeof(cmdstr)) {
		cmdstr[cmdlen] = ch;
		if ((ch == '\r') || (ch == '\n')) {
			cmdstr[cmdlen] = '\0';
//			cmdlen = 0;
			if (strlen(cmdstr) > 0) {
//				putch('\r');
				printf("\r");
				command(cmdstr, cmdlen);
			}
			cmdlen = 0;
		} else {
//			putch(ch);
			printf("%c", ch);
			cmdlen++;
		}
	} else {
		cmdlen = 0;
	}
}

void console(void)
{
#if (CONSOLE_UART != 9)
	if (kbhit()) {
		char ch = getch();
		console_inbyte(ch);
	}
#endif
}

#endif // CONSOLE_UART

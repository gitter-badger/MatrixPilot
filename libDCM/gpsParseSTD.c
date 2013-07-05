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


#include "libDCM_internal.h"
#include "gpsParseCommon.h"
#include "../libUDB/heartbeat.h"


#if (GPS_TYPE == GPS_STD)

// Parse the GPS messages, using the binary interface.
// The parser uses a state machine implemented via a pointer to a function.
// Binary values received from the GPS are directed to program variables via a table
// of pointers to the variable locations.
// Unions of structures are used to be able to access the variables as long, ints, or bytes.

static void msg_A0(uint8_t inchar);
static void msg_A2(uint8_t inchar);
static void msg_PL1(uint8_t inchar);
static void msg_PL2(uint8_t inchar);
//static void msg_MSG2(uint8_t inchar);
static void msg_MSG41(uint8_t inchar);
static void msg_MSGU(uint8_t inchar);
static void msg_B0(uint8_t inchar);
static void msg_B3(uint8_t inchar);

//void (*msg_parse)(uint8_t inchar) = &msg_B3;
static union intbb payloadlength;
static int16_t store_index = 0;
static uint8_t un;
static uint8_t svs_;

//static union longbbbb xpg_, ypg_, zpg_;
//static union intbb    xvg_, yvg_, zvg_;
//static uint8_t mode1_, mode2_;
//static uint8_t svsmin = 24;
//static uint8_t svsmax = 0;

//static union longbbbb lat_gps_, lon_gps_, alt_sl_gps_;
static union longbbbb tow_;
static union intbb sog_gps_, cog_gps_, climb_gps_;
static union intbb nav_valid_, nav_type_, week_no_;
//static uint8_t hdop_;
static union intbb checksum_; // included at the end of the GPS message
static union intbb calculated_checksum; // calculated locally
#define INVALID_CHECKSUM -1

const char bin_mode[] = "$PSRF100,0,19200,8,1,0*39\r\n"; // turn on binary
const uint8_t mode[] = {
	0x86,
	0x00,0x00,0x4B,0x00,
	0x08,
	0x01,
	0x00,
	0x00 
};
const uint16_t mode_length = 9;

/*
uint8_t * const msg2parse[] = {
	&xpg_.__.B3, &xpg_.__.B2,
	&xpg_.__.B1, &xpg_.__.B0,
	&ypg_.__.B3, &ypg_.__.B2,
	&ypg_.__.B1, &ypg_.__.B0,
	&zpg_.__.B3, &zpg_.__.B2,
	&zpg_.__.B1, &zpg_.__.B0,
	&xvg_._.B1,  &xvg_._.B0,
	&yvg_._.B1,  &yvg_._.B0,
	&zvg_._.B1,  &zvg_._.B0,
	&mode1_, 
	&un, 
	&mode2_, 
	&un, &un, &un, &un, &un, &un,
	&svs_,
	&un, &un, &un, &un, &un, &un, 
	&un, &un, &un, &un, &un, &un,
	&un, &un };
*/

uint8_t* const msg41parse[] = {
	&nav_valid_._.B1, &nav_valid_._.B0,
	&nav_type_._.B1,  &nav_type_._.B0,
	// &un, &un, &un, &un, &un, &un,
	&week_no_._.B1, &week_no_._.B0,
	&tow_.__.B3, &tow_.__.B2, &tow_.__.B1, &tow_.__.B0,
	&un, &un, &un, &un, &un, &un,
	&un, &un, &un, &un, &un, &un,
	&lat_gps_.__.B3,  &lat_gps_.__.B2,  &lat_gps_.__.B1,  &lat_gps_.__.B0,
	&lon_gps_.__.B3, &lon_gps_.__.B2, &lon_gps_.__.B1, &lon_gps_.__.B0,
	&un, &un, &un, &un,
	&alt_sl_gps_.__.B3, &alt_sl_gps_.__.B2, &alt_sl_gps_.__.B1, &alt_sl_gps_.__.B0,
	&un, 
	&sog_gps_._.B1, &sog_gps_._.B0,
	&cog_gps_._.B1, &cog_gps_._.B0,
	&un, &un,
	&climb_gps_._.B1, &climb_gps_._.B0,
	&un, &un, &un, &un, &un, &un, &un, &un, &un, &un,
	&un, &un, &un, &un, &un, &un, &un, &un, &un, &un,
	&un, &un, &un, &un, &un, &un, &un, &un, &un, &un,
	&un, &un, &un, &un, &un, &un, &un, &un, &un, &un,
	&svs_,
	&hdop_._.B0,
	&un,
	&checksum_._.B1, &checksum_._.B0
};


//	if nav_valid is zero, there is valid GPS data that can be used for navigation.
static boolean gps_nav_valid_(void)
{
	return (nav_valid_.BB == 0);
}

static void gps_startup_sequence_(int16_t gpscount)
{
// this gets called at HEARTBEAT_HZ with gpscount from GPS_COUNT to zero

	if (gpscount == HEARTBEAT_HZ)
		udb_gps_set_rate(4800);
	else if (gpscount == (3 * HEARTBEAT_HZ) / 4)
		// set the GPS to use binary mode
		gpsoutline((char*)bin_mode);
	else if (gpscount == HEARTBEAT_HZ / 2)
		// command GPS to select which messages are sent, using NMEA interface
		gpsoutbin(mode_length, mode);
	else if (gpscount == HEARTBEAT_HZ / 4)
		// Switch to 19200 baud
		udb_gps_set_rate(19200);
}

/*
int16_t hex_count = 0;
const char convert[] = "0123456789ABCDEF";
const char endchar = 0xB3;

void hex_out(char outchar)
//	Used for debugging purposes, converts to HEX and outputs to the debugging USART
//	Only the first 5 bytes following a B3 are displayed.
{
	if (hex_count > 0) 
	{
		U1TXREG = convert[ ((outchar>>4) & 0x0F) ];
		U1TXREG = convert[ (outchar & 0x0F) ];
		U1TXREG = ' ';
		hex_count --;
	}
	if (outchar == endchar)
	{
		hex_count = 5;
		U1TXREG = '\r';
		U1TXREG = '\n';
	}
}
*/

//	The parsing routines follow. Each routine is named for the state in which the routine is applied.
//	States correspond to the portions of the binary messages.
//	For example, msg_B3 is the routine that is applied to the byte received after a B3 is received.
//	If an A0 is received, the state machine transitions to the A0 state.

static void msg_B3(uint8_t gpschar)
{
	if (gpschar == 0xA0)
	{
		msg_parse = &msg_A0;
	}
	else
	{
		// error condition
	}
}

static void msg_A0(uint8_t gpschar)
{
	if (gpschar == 0xA2)
	{
		store_index = 0;
		msg_parse = &msg_A2;
	}
	else
	{
		msg_parse = &msg_B3;	// error condition
	}
}

static void msg_A2(uint8_t gpschar)
{
	payloadlength._.B1 = gpschar;
	msg_parse = &msg_PL1;
}

static void msg_PL1(uint8_t gpschar)
{
	payloadlength._.B0 = gpschar;
	payloadlength.BB++; // -1 for msgType, +2 for checksum int16_t
	msg_parse = &msg_PL2;
}

static void msg_PL2(uint8_t gpschar)
{
	//	the only SiRF message being used by MatrixPilot is 41.
	switch (gpschar) {
		/*
		case 0x02 : {
			if (payloadlength.BB == sizeof(msg2parse)>>1)
			{
				msg_parse = &msg_MSG2;
			}
			else
			{
				msg_parse = &msg_B3;
			}
			break;
		}
		*/
		case 0x29 : {
			if (payloadlength.BB == sizeof(msg41parse)>>1)
			{
				calculated_checksum.BB = gpschar;
				msg_parse = &msg_MSG41;
			}
			else
			{
				calculated_checksum.BB = INVALID_CHECKSUM; // bad payload length
				msg_parse = &msg_B3;
			}
			break;
		}
		default : {
			calculated_checksum.BB = INVALID_CHECKSUM; // wrong message type
			msg_parse = &msg_MSGU;
			break;
		}
	}
}

/*
static void msg_MSG2(uint8_t gpschar)
{
	if (payloadlength.BB > 0)
	{
		*msg2parse[store_index++] = gpschar;
		payloadlength.BB--;
	}
	else
	{
		if (gpschar == 0xB0)
		{
			msg_parse = &msg_B0;
		}
		else
		{
			msg_parse = &msg_B3;  // error condition
		}
	}
}
*/

static void msg_MSG41(uint8_t gpschar)
{
	if (payloadlength.BB > 0)
	{
		*msg41parse[store_index++] = gpschar;
		if (payloadlength.BB > 2) // Don't include the sent checksum bytes in the checksum calculation
		{
			calculated_checksum.BB += gpschar;
		}
		payloadlength.BB--;
	}
	else
	{
		if (gpschar == 0xB0)
		{
			msg_parse = &msg_B0;
		}
		else
		{
			msg_parse = &msg_B3;  // error condition
		}
	}
}

static void msg_MSGU(uint8_t gpschar)
{
	if (payloadlength.BB > 0)
	{
		payloadlength.BB--;
	}
	else
	{
		if (gpschar == 0xB0)
		{
			msg_parse = &msg_B0;
		}
		else
		{
			msg_parse = &msg_B3; // error condition
		}
	}
}

static void msg_B0(uint8_t gpschar)
{
	if (gpschar == 0xB3)
	{
		int16_t masked = calculated_checksum.BB & 0x7FFF;
		if (calculated_checksum.BB != INVALID_CHECKSUM && checksum_.BB == masked)
		{
			udb_background_trigger(); // parsing is complete and valid, schedule navigation
		}
		msg_parse = &msg_B3;
	}
	else
	{
		msg_parse = &msg_B3; // error condition
	}
}

static void gps_commit_data_(void)
{
	week_no     = week_no_;
	tow         = tow_;
	lat_gps     = lat_gps_;
	lon_gps     = lon_gps_;
	alt_sl_gps  = alt_sl_gps_;
	sog_gps     = sog_gps_;
	cog_gps     = cog_gps_;
	climb_gps   = climb_gps_;
	hdop        = hdop_._.B0;
	//xpg         = xpg_;
	//ypg         = ypg_;
	//zpg         = zpg_;
	//xvg         = xvg_;
	//yvg         = yvg_;
	//zvg         = zvg_;
	//mode1       = mode1_;
	//mode2       = mode2_;
	svs         = svs_;
}

void init_gps_std(void)
{
	msg_parse = &msg_B3;
	gps_startup_sequence = &gps_startup_sequence_;
	gps_nav_valid = &gps_nav_valid_;
	gps_commit_data = &gps_commit_data_;
}

#endif // (GPS_TYPE == GPS_STD)

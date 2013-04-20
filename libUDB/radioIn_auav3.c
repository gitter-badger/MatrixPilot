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


#include "libUDB_internal.h"
#include "oscillator.h"
#include "interrupt.h"
#include <stdio.h>

#if (BOARD_TYPE == AUAV3_BOARD)

#include "defines.h"

#if (FLYBYWIRE_ENABLED == 1)
#include "FlyByWire.h"
#include "mode_switch.h"
#endif

//	Measure the pulse widths of the servo channel inputs from the radio.
//	The dsPIC makes this rather easy to do using its capture feature.

//	One of the channels is also used to validate pulse widths to detect loss of radio.

//	The pulse width inputs can be directly converted to units of pulse width outputs to control
//	the servos by simply dividing by 2.

int16_t udb_pwIn[NUM_INPUTS+1] ;	// pulse widths of radio inputs
int16_t udb_pwTrim[NUM_INPUTS+1] ;	// initial pulse widths for trimming

int16_t failSafePulses = 0 ;
int16_t noisePulses = 0 ;


#if (USE_PPM_INPUT != 1)
uint16_t rise[NUM_INPUTS+1] ;	// rising edge clock capture for radio inputs

#else
uint16_t rise_ppm ;				// rising edge clock capture for PPM radio input
#endif

#if (FREQOSC == 128000000LL)
#define TMR_FACTOR 4
#elif (FREQOSC == 64000000LL)
#define TMR_FACTOR 1
#elif (FREQOSC == 32000000LL)
#define TMR_FACTOR 2
#else
#error Invalid Oscillator Frequency
#endif

#define MIN_SYNC_PULSE_WIDTH (14000/TMR_FACTOR)	// 3.5ms
/*
#if (FREQOSC == 128000000LL)
#define MIN_SYNC_PULSE_WIDTH 3500	// 3.5ms
#elif (FREQOSC == 64000000LL)
#define MIN_SYNC_PULSE_WIDTH 14000	// 3.5ms
#elif (FREQOSC == 32000000LL)
#define MIN_SYNC_PULSE_WIDTH 7000	// 3.5ms
#else
#error Invalid Oscillator Frequency
#endif
 */

#if (FLYBYWIRE_ENABLED == 1)
int set_udb_pwIn(int pwm, int index)
{
	#if (NORADIO == 0)
	// It's kind of a bad idea to override the radio mode input
	if (MODE_SWITCH_INPUT_CHANNEL == index)
		return (pwm * TMR_FACTOR / 2);
	#endif

	if (udb_pwIn[MODE_SWITCH_INPUT_CHANNEL] < MODE_SWITCH_THRESHOLD_LOW)
		return get_fbw_pwm(index);
	else
		return (pwm * TMR_FACTOR / 2);
}
#else
#define set_udb_pwIn(pwm, b) (pwm * TMR_FACTOR / 2)
#endif // FLYBYWIRE_ENABLED


void udb_init_capture(void)
{
	int16_t i;

#if(USE_NV_MEMORY == 1)
	if(udb_skip_flags.skip_radio_trim == 0)
	{	
#endif
		for (i=0; i <= NUM_INPUTS; i++)
	#if (FIXED_TRIMPOINT == 1)
			if(i == THROTTLE_OUTPUT_CHANNEL)
				udb_pwTrim[i] = udb_pwIn[i] = THROTTLE_TRIMPOINT;
			else
				udb_pwTrim[i] = udb_pwIn[i] = CHANNEL_TRIMPOINT;			
	#else
			udb_pwTrim[i] = udb_pwIn[i] = 0 ;
	#endif
#if(USE_NV_MEMORY == 1)
	}
#endif
	
	TMR2 = 0 ; 				// initialize timer
#if (FREQOSC == 128000000LL)
	T2CONbits.TCKPS = 2 ;	// prescaler = 64 option
#else
	T2CONbits.TCKPS = 1 ;	// prescaler = 8 option
#endif
	T2CONbits.TCS = 0 ;		// use the internal clock
	T2CONbits.TON = 1 ;		// turn on timer 2
	
	//	configure the capture pins
	IC1CON1bits.ICTSEL = 1 ;  // use timer 2
	IC1CON1bits.ICM = 1 ; // capture every edge
        TRISDbits.TRISD0 = 1; // I1

	IC1CON2bits.SYNCSEL = 0x0C;

	_IC1IP = 6 ;
	_IC1IF = 0 ;
	if (NUM_INPUTS > 0) _IC1IE = 1 ;
	
#if (USE_PPM_INPUT != 1)
        IC2CON1 = IC1CON1;
        IC3CON1 = IC1CON1;
        IC4CON1 = IC1CON1;
        IC5CON1 = IC1CON1;
        IC6CON1 = IC1CON1;
        IC7CON1 = IC1CON1;
        IC8CON1 = IC1CON1;

        TRISDbits.TRISD1 = 1;   // I2
        TRISDbits.TRISD8 = 1;   // I3
        TRISAbits.TRISA15 = 1;  // I4
        TRISAbits.TRISA14 = 1;  // I5
        TRISAbits.TRISA5 = 1;   // I6
        TRISAbits.TRISA4 = 1;   // I7
        TRISFbits.TRISF8 = 1;   // I8

	//	set the interrupt priorities to 6
	_IC2IP = 6;
        _IC3IP = 6;
        _IC4IP = 6;
        _IC5IP = 6;
        _IC6IP = 6;
        _IC7IP = 6;
        _IC8IP = 6;
	
	//	clear the interrupts:
	_IC2IF = 0;
        _IC3IF = 0;
        _IC4IF = 0;
        _IC5IF = 0;
        _IC6IF = 0;
        _IC7IF = 0;
        _IC8IF = 0;
	
	//	enable the interrupts:
	if (NUM_INPUTS > 1) _IC2IE = 1 ; 
	if (NUM_INPUTS > 2) _IC3IE = 1 ; 
	if (NUM_INPUTS > 3) _IC4IE = 1 ; 
	if (NUM_INPUTS > 4) _IC5IE = 1 ; 
	if (NUM_INPUTS > 5) _IC6IE = 1 ; 
	if (NUM_INPUTS > 6) _IC7IE = 1 ; 
	if (NUM_INPUTS > 7) _IC8IE = 1 ;
#endif
}


#if (USE_PPM_INPUT != 1)

// Input Channel 1
void __attribute__((__interrupt__,__no_auto_psv__)) _IC1Interrupt(void)
{
	indicate_loading_inter ;
	interrupt_save_set_corcon ;
	
	uint16_t time = 0;	
	_IC1IF = 0 ; // clear the interrupt
	while ( IC1CON1bits.ICBNE )
	{
		time = IC1BUF ;
	}
	
#if ( NORADIO != 1 )
	if (PORTDbits.RD8)
	{
		 rise[1] = time ;
	}
	else
	{
		udb_pwIn[1] = set_udb_pwIn(time - rise[1], 1) ;
		
#if ( FAILSAFE_INPUT_CHANNEL == 1 )
		if ( (udb_pwIn[FAILSAFE_INPUT_CHANNEL] > FAILSAFE_INPUT_MIN) && (udb_pwIn[FAILSAFE_INPUT_CHANNEL] < FAILSAFE_INPUT_MAX ) )
		{
			failSafePulses++ ;
		}
		else
		{
			noisePulses++ ;
		}
#endif
	
	}
#endif
	
	interrupt_restore_corcon ;
	return ;
}


// Input Channel 2
void __attribute__((__interrupt__,__no_auto_psv__)) _IC2Interrupt(void)
{
	indicate_loading_inter ;
	interrupt_save_set_corcon ;
	
	uint16_t time = 0;
	_IC2IF = 0 ; // clear the interrupt
	while ( IC2CON1bits.ICBNE )
	{
		time = IC2BUF ;
	}
	
#if ( NORADIO != 1 )
	if (PORTDbits.RD9)
	{
		 rise[2] = time ;
	}
	else
	{
		udb_pwIn[2] = set_udb_pwIn(time - rise[2], 2) ;
		
#if ( FAILSAFE_INPUT_CHANNEL == 2 )
		if ( (udb_pwIn[FAILSAFE_INPUT_CHANNEL] > FAILSAFE_INPUT_MIN) && (udb_pwIn[FAILSAFE_INPUT_CHANNEL] < FAILSAFE_INPUT_MAX ) )
		{
			failSafePulses++ ;
		}
		else
		{
			noisePulses++ ;
		}
#endif
	
	}	
#endif
	
	interrupt_restore_corcon ;
	return ;
}


// Input Channel 3
void __attribute__((__interrupt__,__no_auto_psv__)) _IC3Interrupt(void)
{
	indicate_loading_inter ;
	interrupt_save_set_corcon ;
	
	uint16_t time = 0;
	_IC3IF = 0 ; // clear the interrupt
	while ( IC3CON1bits.ICBNE )
	{
		time = IC3BUF ;
	}
	
#if ( NORADIO != 1 )
	if (PORTDbits.RD10)
	{
		 rise[3] = time ;
	}
	else
	{
		udb_pwIn[3] = set_udb_pwIn(time - rise[3], 3) ;
		
#if ( FAILSAFE_INPUT_CHANNEL == 3 )
		if ( (udb_pwIn[FAILSAFE_INPUT_CHANNEL] > FAILSAFE_INPUT_MIN) && (udb_pwIn[FAILSAFE_INPUT_CHANNEL] < FAILSAFE_INPUT_MAX ) )
		{
			failSafePulses++ ;
		}
		else
		{
			noisePulses++ ;
		}
#endif
	
	}
#endif
	
	interrupt_restore_corcon ;
	return ;
}


// Input Channel 4
void __attribute__((__interrupt__,__no_auto_psv__)) _IC4Interrupt(void)
{
	indicate_loading_inter ;
	interrupt_save_set_corcon ;
	
	uint16_t time = 0;
	_IC4IF =  0 ; // clear the interrupt
	while ( IC4CON1bits.ICBNE )
	{
		time = IC4BUF ;
	}
	
#if ( NORADIO != 1 )
	if (PORTDbits.RD11)
	{
		 rise[4] = time ;
	}
	else
	{
		udb_pwIn[4] = set_udb_pwIn(time - rise[4], 4) ;
		
#if ( FAILSAFE_INPUT_CHANNEL == 4 )
		if ( (udb_pwIn[FAILSAFE_INPUT_CHANNEL] > FAILSAFE_INPUT_MIN) && (udb_pwIn[FAILSAFE_INPUT_CHANNEL] < FAILSAFE_INPUT_MAX ) )
		{
			failSafePulses++ ;
		}
		else
		{
			noisePulses++ ;
		}
#endif
	
	}
#endif
	
	interrupt_restore_corcon ;
	return ;
}


// Input Channel 5
void __attribute__((__interrupt__,__no_auto_psv__)) _IC5Interrupt(void)
{
	indicate_loading_inter ;
	interrupt_save_set_corcon ;
	
	uint16_t time = 0;
	_IC5IF =  0 ; // clear the interrupt
	while ( IC5CON1bits.ICBNE )
	{
		time = IC5BUF ;
	}
	
#if ( NORADIO != 1 )
	if (PORTDbits.RD12)
	{
		 rise[5] = time ;
	}
	else
	{
		udb_pwIn[5] = set_udb_pwIn(time - rise[5], 5) ;
		
#if ( FAILSAFE_INPUT_CHANNEL == 5 )
		if ( (udb_pwIn[FAILSAFE_INPUT_CHANNEL] > FAILSAFE_INPUT_MIN) && (udb_pwIn[FAILSAFE_INPUT_CHANNEL] < FAILSAFE_INPUT_MAX ) )
		{
			failSafePulses++ ;
		}
		else
		{
			noisePulses++ ;
		}
#endif
	
	}
#endif
	
	interrupt_restore_corcon ;
	return ;
}


// Input Channel 6
void __attribute__((__interrupt__,__no_auto_psv__)) _IC6Interrupt(void)
{
	indicate_loading_inter ;
	interrupt_save_set_corcon ;
	
	uint16_t time = 0;
	_IC6IF =  0 ; // clear the interrupt
	while ( IC6CON1bits.ICBNE )
	{
		time = IC6BUF ;
	}
	
#if ( NORADIO != 1 )
	if (PORTDbits.RD13)
	{
		 rise[6] = time ;
	}
	else
	{
		udb_pwIn[6] = set_udb_pwIn(time - rise[6], 6) ;
		
#if ( FAILSAFE_INPUT_CHANNEL == 6 )
		if ( (udb_pwIn[FAILSAFE_INPUT_CHANNEL] > FAILSAFE_INPUT_MIN) && (udb_pwIn[FAILSAFE_INPUT_CHANNEL] < FAILSAFE_INPUT_MAX ) )
		{
			failSafePulses++ ;
		}
		else
		{
			noisePulses++ ;
		}
#endif
	
	}
#endif
	
	interrupt_restore_corcon ;
	return ;
}


// Input Channel 7
void __attribute__((__interrupt__,__no_auto_psv__)) _IC7Interrupt(void)
{
	indicate_loading_inter ;
	interrupt_save_set_corcon ;
	
	uint16_t time = 0;
	_IC7IF =  0 ; // clear the interrupt
	while ( IC7CON1bits.ICBNE )
	{
		time = IC7BUF ;
	}
	
#if ( NORADIO != 1 )
	if (PORTDbits.RD14)
	{
		 rise[7] = time ;
	}
	else
	{
		udb_pwIn[7] = set_udb_pwIn(time - rise[7], 7) ;
		
#if ( FAILSAFE_INPUT_CHANNEL == 7 )
		if ( (udb_pwIn[FAILSAFE_INPUT_CHANNEL] > FAILSAFE_INPUT_MIN) && (udb_pwIn[FAILSAFE_INPUT_CHANNEL] < FAILSAFE_INPUT_MAX ) )
		{
			failSafePulses++ ;
		}
		else
		{
			noisePulses++ ;
		}
#endif
	
	}
#endif
	
	interrupt_restore_corcon ;
	return ;
}


// Input Channel 8
void __attribute__((__interrupt__,__no_auto_psv__)) _IC8Interrupt(void)
{
	indicate_loading_inter ;
	interrupt_save_set_corcon ;
	
	uint16_t time = 0;
	_IC8IF =  0 ; // clear the interrupt
	while ( IC8CON1bits.ICBNE )
	{
		time = IC8BUF ;
	}
	
#if ( NORADIO != 1 )
	if (PORTDbits.RD15)
	{
		 rise[8] = time ;
	}
	else
	{
		udb_pwIn[8] = set_udb_pwIn(time - rise[8], 8) ;
		
#if ( FAILSAFE_INPUT_CHANNEL == 8 )
		if ( (udb_pwIn[FAILSAFE_INPUT_CHANNEL] > FAILSAFE_INPUT_MIN) && (udb_pwIn[FAILSAFE_INPUT_CHANNEL] < FAILSAFE_INPUT_MAX ) )
		{
			failSafePulses++ ;
		}
		else
		{
			noisePulses++ ;
		}
#endif	
	}
#endif
	
	interrupt_restore_corcon ;
	return ;
}

#else // #if (USE_PPM_INPUT == 1)

#if (PPM_SIGNAL_INVERTED == 1)
#define PPM_PULSE_VALUE 0
#else
#define PPM_PULSE_VALUE 1
#endif

uint8_t ppm_ch = 0 ;

// PPM Input on Channel 1
#ifndef USE_PPM_ROBD
void __attribute__((__interrupt__,__no_auto_psv__)) _IC1Interrupt(void)
{
	indicate_loading_inter ;
	interrupt_save_set_corcon ;
	
	uint16_t time = 0;	
	_IC1IF = 0 ; // clear the interrupt
	while ( IC1CON1bits.ICBNE )
	{
		time = IC1BUF ;
	}
	
#if ( NORADIO != 1 )

	if (_RD8 == PPM_PULSE_VALUE)
	{
		uint16_t pulse = time - rise_ppm ;
		rise_ppm = time ;
		
		if (pulse > MIN_SYNC_PULSE_WIDTH)			//sync pulse
		{
			ppm_ch = 1 ;
		}
		else
		{
			if (ppm_ch > 0 && ppm_ch <= PPM_NUMBER_OF_CHANNELS)
			{
				if (ppm_ch <= NUM_INPUTS)
				{
					udb_pwIn[ppm_ch] = pulse ;
					
					if ( ppm_ch == FAILSAFE_INPUT_CHANNEL )
					{
						if ( udb_pwIn[FAILSAFE_INPUT_CHANNEL] > FAILSAFE_INPUT_MIN && udb_pwIn[FAILSAFE_INPUT_CHANNEL] < FAILSAFE_INPUT_MAX )
						{
							failSafePulses++ ;
						}
						else
						{
							noisePulses++ ;
						}
					}
				}
				ppm_ch++ ;		//scan next channel
			}
		}
	}
#endif

	interrupt_restore_corcon ;
	return ;
}
#else  // USE_PPM_ROBD

void __attribute__((__interrupt__,__no_auto_psv__)) _IC1Interrupt(void)
{
	indicate_loading_inter ;
	interrupt_save_set_corcon ;
	
	unsigned int time = 0 ;	
	_IC1IF = 0 ; // clear the interrupt
	while ( IC1CON1bits.ICBNE )
	{
		time = IC1BUF ;
	}
#if ( NORADIO != 1 )
	unsigned int pulse = time - rise_ppm ;
	rise_ppm = time ;

	if (_RD0 == PPM_PULSE_VALUE)
	{
//		printf("%u\r\n", pulse);
		if (pulse > MIN_SYNC_PULSE_WIDTH)			//sync pulse
		{
			ppm_ch = 1 ;
		}
	}
	else
	{
//		printf("%u %u\r\n", ppm_ch, pulse);	
		if (ppm_ch > 0 && ppm_ch <= PPM_NUMBER_OF_CHANNELS)
		{
			if (ppm_ch <= NUM_INPUTS)
			{
				udb_pwIn[ppm_ch] = set_udb_pwIn(pulse, ppm_ch);

				if ( ppm_ch == FAILSAFE_INPUT_CHANNEL && udb_pwIn[FAILSAFE_INPUT_CHANNEL] > FAILSAFE_INPUT_MIN && udb_pwIn[FAILSAFE_INPUT_CHANNEL] < FAILSAFE_INPUT_MAX )
				{
					failSafePulses++ ;
				}
			}
			ppm_ch++ ;		//scan next channel
		}
	}
#endif

//static int foo = 0;
//	if (foo++ > 160) {
//		foo = 0;
//		printf("FS: %u\r\n", udb_pwIn[FAILSAFE_INPUT_CHANNEL]);
//	}

	interrupt_restore_corcon ;
	return ;
}
#endif // USE_PPM_ROBD

#endif // USE_PPM_INPUT

#endif // BOARD_TYPE

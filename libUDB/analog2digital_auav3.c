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

#if (BOARD_TYPE == AUAV3_BOARD)

//	Variables.

#if (NUM_ANALOG_INPUTS >= 1)
struct ADchannel udb_analogInputs[NUM_ANALOG_INPUTS] ; // 0-indexed, unlike servo pwIn/Out/Trim arrays
#endif
struct ADchannel udb_vcc ;
struct ADchannel udb_5v ;
struct ADchannel udb_rssi ;


// Number of locations for ADC buffer = 6 (AN0,15,16,17,18) x 1 = 6 words
// Align the buffer. This is needed for peripheral indirect mode
#define NUM_AD_CHAN 7
__eds__ int16_t  BufferA[NUM_AD_CHAN] __attribute__((eds,space(dma),aligned(32))) ;
__eds__ int16_t  BufferB[NUM_AD_CHAN] __attribute__((eds,space(dma),aligned(32))) ;


//int16_t vref_adj ;
int16_t sample_count ;

#if (RECORD_FREE_STACK_SPACE == 1)
uint16_t maxstack = 0 ;
#endif


#define ALMOST_ENOUGH_SAMPLES 216 // there are 222 or 223 samples in a sum

void udb_init_ADC( void )
{
	sample_count = 0 ;
	
	AD1CON1bits.FORM   = 3 ;	// Data Output Format: Signed Fraction (Q15 format)
	AD1CON1bits.SSRC   = 7 ;	// Sample Clock Source: Auto-conversion
	AD1CON1bits.ASAM   = 1 ;	// ADC Sample Control: Sampling begins immediately after conversion
	AD1CON1bits.AD12B  = 1 ;	// 12-bit ADC operation
	
	AD1CON2bits.CSCNA = 1 ;		// Scan Input Selections for CH0+ during Sample A bit
	AD1CON2bits.CHPS  = 0 ;		// Converts CH0
	
	AD1CON3bits.ADRC = 0 ;		// ADC Clock is derived from Systems Clock
	AD1CON3bits.ADCS = 11 ;		// ADC Conversion Clock Tad=Tcy*(ADCS+1)= (1/40M)*12 = 0.3us (3333.3Khz)
								// ADC Conversion Time for 12-bit Tc=14*Tad = 4.2us
	AD1CON3bits.SAMC = 1 ;		// No waiting between samples
	
	AD1CON2bits.VCFG = 0 ;		// use supply as reference voltage
	
	AD1CON1bits.ADDMABM = 1 ; 	// DMA buffers are built in sequential mode
	AD1CON2bits.SMPI    = (NUM_AD_CHAN-1) ;	
	AD1CON4bits.DMABL   = 0 ;	// Each buffer contains 1 word
	
	
	AD1CSSL = 0x0000 ;
	AD1CSSH = 0x0000 ;

        // power-on default is all analog inputs selected

        ANSELA = 0; // disable all analog inputs on port A
        ANSELC = 0; // disable all analog inputs on port C
        ANSELD = 0; // disable all analog inputs on port D
        ANSELE = 0; // disable all analog inputs on port E
        ANSELG = 0; // disable all analog inputs on port G

        // enable specific analog inputs on port B
        // AN6:9,13:15 map to:
        // AUAV3 inputs ANA2:3, ANA0:1, V, I, RS
        int16_t mask = ((1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 13) | (1 << 14) | (1 << 15));
        ANSELB = mask;

        // set analog pins as inputs
        TRISB |= mask;

//	include voltage monitor inputs

	_CSS13 = 1 ;		// Enable AN13 for channel scan
	_CSS14 = 1 ;		// Enable AN14 for channel scan
	_CSS15 = 1 ;		// Enable AN15 for channel scan

	
//  include the extra analog input pins
	_CSS6 = 1 ;		// Enable AN6 for channel scan
	_CSS7 = 1 ;		// Enable AN7 for channel scan
	_CSS8 = 1 ;		// Enable AN8 for channel scan
	_CSS9 = 1 ;		// Enable AN9 for channel scan
 	
	_AD1IF = 0 ;		// Clear the A/D interrupt flag bit
	_AD1IP = 5 ;		// priority 5
	AD1CON1bits.ADON = 1;	// Turn on the A/D converter
	_AD1IE = 0 ;		// Do Not Enable A/D interrupt
	
	
//  DMA Setup
	DMA0CONbits.AMODE = 2;	// Configure DMA for Peripheral indirect mode
	DMA0CONbits.MODE  = 2;  // Configure DMA for Continuous Ping-Pong mode
	DMA0PAD=(int16_t)&ADC1BUF0;
	DMA0CNT = NUM_AD_CHAN-1;					
	DMA0REQ = 13 ;		// Select ADC1 as DMA Request source
	
	DMA0STAH = 0x0000;
	DMA0STAL = __builtin_dmaoffset(BufferA) ;
	DMA0STBH = 0x0000;
	DMA0STBL = __builtin_dmaoffset(BufferB) ;
	
	IFS0bits.DMA0IF = 0 ;	// Clear the DMA interrupt flag bit
        IEC0bits.DMA0IE = 1 ;	// Set the DMA interrupt enable bit
	_DMA0IP = 5 ;		// Set the DMA ISR priority
	
	DMA0CONbits.CHEN = 1 ;	// Enable DMA
	
	return ;
}


uint8_t DmaBuffer = 0 ;

void __attribute__((__interrupt__,__no_auto_psv__)) _DMA0Interrupt(void)
{
	indicate_loading_inter ;
	interrupt_save_set_corcon ;
	
#if (RECORD_FREE_STACK_SPACE == 1)
	uint16_t stack = WREG15 ;
	if ( stack > maxstack )
	{
		maxstack = stack ;
	}
#endif
	
#if (HILSIM != 1)
	__eds__ int16_t *CurBuffer = (DmaBuffer == 0) ? BufferA : BufferB ;
	udb_vcc.input = CurBuffer[A_VCC_BUFF-1] ;
	udb_5v.input =  CurBuffer[A_5V_BUFF-1] ;
	udb_rssi.input =  CurBuffer[A_RSSI_BUFF-1] ;
	
#if (NUM_ANALOG_INPUTS >= 1)
	udb_analogInputs[0].input = CurBuffer[analogInput1BUFF-1] ;
#endif
#if (NUM_ANALOG_INPUTS >= 2)
	udb_analogInputs[1].input = CurBuffer[analogInput2BUFF-1] ;
#endif
#if (NUM_ANALOG_INPUTS >= 3)
	udb_analogInputs[2].input = CurBuffer[analogInput3BUFF-1] ;
#endif
#if (NUM_ANALOG_INPUTS >= 4)
	udb_analogInputs[3].input = CurBuffer[analogInput4BUFF-1] ;
#endif
	
#endif
	
	DmaBuffer ^= 1 ;			// Switch buffers
	IFS0bits.DMA0IF = 0 ;		// Clear the DMA0 Interrupt Flag
	
	
	if ( udb_flags._.a2d_read == 1 ) // prepare for the next reading
	{
		udb_flags._.a2d_read = 0 ;
//		udb_xrate.sum = udb_yrate.sum = udb_zrate.sum = 0 ;
//		udb_xaccel.sum = udb_yaccel.sum = udb_zaccel.sum = 0 ;
#ifdef VREF
//		udb_vref.sum = 0 ;
#endif
	udb_vcc.sum = 0 ; 
	udb_5v.sum = 0 ;
	udb_rssi.sum = 0 ;
#if (NUM_ANALOG_INPUTS >= 1)
		udb_analogInputs[0].sum = 0;
#endif
#if (NUM_ANALOG_INPUTS >= 2)
		udb_analogInputs[1].sum = 0 ;
#endif
#if (NUM_ANALOG_INPUTS >= 3)
		udb_analogInputs[2].sum = 0;
#endif
#if (NUM_ANALOG_INPUTS >= 4)
		udb_analogInputs[3].sum = 0 ;
#endif
		sample_count = 0 ;
	}
	
	//	perform the integration:

	udb_vcc.sum += udb_vcc.input ;
	udb_5v.sum +=  udb_5v.input ;
	udb_rssi.sum +=  udb_5v.input ;

#if (NUM_ANALOG_INPUTS >= 1)
	udb_analogInputs[0].sum += udb_analogInputs[0].input ;
#endif
#if (NUM_ANALOG_INPUTS >= 2)
	udb_analogInputs[1].sum += udb_analogInputs[1].input ;
#endif
#if (NUM_ANALOG_INPUTS >= 3)
	udb_analogInputs[2].sum += udb_analogInputs[2].input ;
#endif
#if (NUM_ANALOG_INPUTS >= 4)
	udb_analogInputs[3].sum += udb_analogInputs[3].input ;
#endif
	sample_count ++ ;
	
	//	When there is a chance that data will be read soon,
	//  have the new average values ready.
	if ( sample_count > ALMOST_ENOUGH_SAMPLES )
	{	

		udb_vcc.value = __builtin_divsd( udb_vcc.sum, sample_count ) ;
		udb_5v.value = __builtin_divsd( udb_5v.sum, sample_count ) ;
		udb_rssi.value = __builtin_divsd( udb_rssi.sum, sample_count ) ;
		
#if (NUM_ANALOG_INPUTS >= 1)
		udb_analogInputs[0].value = __builtin_divsd( udb_analogInputs[0].sum, sample_count ) ;
#endif
#if (NUM_ANALOG_INPUTS >= 2)
		udb_analogInputs[1].value = __builtin_divsd( udb_analogInputs[1].sum, sample_count ) ;
#endif
#if (NUM_ANALOG_INPUTS >= 3)
		udb_analogInputs[2].value = __builtin_divsd( udb_analogInputs[2].sum, sample_count ) ;
#endif
#if (NUM_ANALOG_INPUTS >= 4)
		udb_analogInputs[3].value = __builtin_divsd( udb_analogInputs[3].sum, sample_count ) ;
#endif
	}
	
	interrupt_restore_corcon ;
	return ;
}

#endif

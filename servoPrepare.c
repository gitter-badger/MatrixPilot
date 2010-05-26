#include "libDCM.h"
#include "defines.h"

//	routines to drive the PWM pins for the servos,
//	assumes the use of the 16MHz crystal.

int gpscount ; // counter to initialize GPS
int calibcount ; // number of PWM pulses before control is turned on

char fourHertzCounter = 0 ;
boolean startTelemetry = 0 ;


void manualPassthrough( void ) ;


void init_servoPrepare( void )	// initialize the PWM
{
	calibcount = 400 ;	// wait 400 PWM pulses before turning on the control (10 seconds)
	gpscount = 1000 ;	// wait 25 seconds for GPS to initialize

	int i;
	for (i=0; i <= NUM_OUTPUTS; i++)
		if (i != THROTTLE_OUTPUT_CHANNEL)
			udb_pwOut[i] = 3000 ;
	
#if (NORADIO == 1)
	udb_pwIn[MODE_SWITCH_INPUT_CHANNEL] = udb_pwTrim[MODE_SWITCH_INPUT_CHANNEL] = 4000 ;
#endif
	
	return ;
}


void dcm_servo_callback_prepare_outputs(void)
{
	// This is a simple counter to do stuff at 4hz
	fourHertzCounter++ ;
	if ( fourHertzCounter >= 10 )
	{
		if ( startTelemetry )
		{
			serial_output_4hz() ;
		}
		fourHertzCounter = 0 ;
	}
	
	
	switch ( calibcount ) {
		// case 0 is when the control is up and running
			
		case 0: {
			updateBehavior() ;
			rollCntrl() ;
			yawCntrl() ;
			altitudeCntrl();
			pitchCntrl() ;
			servoMix() ;
#if ( USE_CAMERA_STABILIZATION == 1 )
			cameraCntrl();
#endif
			updateTriggerAction() ;
			break ;
		}
			
		case 1: {
			// almost ready to turn the control on, save the input offsets
			dcm_calibrate() ;
			manualPassthrough() ;	// Allow manual control while starting up
			startTelemetry = 1 ;
			break ;
		}
		default: {
			// otherwise, there is not anything to do
			manualPassthrough() ;	// Allow manual control while starting up
			break ;
		}
	}
	
	// count down the startup counter to 0
	if ( calibcount > 0 ) calibcount-- ;

	
	// count down the startup counter to 0
	gps_startup_sequence(gpscount) ;

	if ( gpscount > 0 ) gpscount-- ;
	
	return ;
}

void manualPassthrough( void )
{
	roll_control = pitch_control = yaw_control = altitude_control = 0 ;
	servoMix() ;
	
	return ;
}


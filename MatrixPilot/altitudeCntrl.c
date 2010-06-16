#include "p30f4011.h"
#include "definesRmat.h"
#include "defines.h"
#include "options.h"

union longww throttleFiltered = { 0 } ;

#define THROTTLEFILTSHIFT 12

#define DEADBAND 150

#define MAXTHROTTLE			( 2.0*SERVORANGE*ALT_HOLD_THROTTLE_MAX  )
#define FIXED_WP_THROTTLE	( 2.0*SERVORANGE*RACING_MODE_WP_THROTTLE  )

#define THROTTLEHEIGHTGAIN (((ALT_HOLD_THROTTLE_MAX - ALT_HOLD_THROTTLE_MIN )*2.0*SERVORANGE )/(HEIGHT_MARGIN*2.0))

#define PITCHATMAX (ALT_HOLD_PITCH_MAX*(RMAX/57.3))
#define PITCHATMIN (ALT_HOLD_PITCH_MIN*(RMAX/57.3))
#define PITCHATZERO (ALT_HOLD_PITCH_HIGH*(RMAX/57.3))

#define PITCHHEIGHTGAIN ((PITCHATMAX - PITCHATMIN) / (HEIGHT_MARGIN*2.0)) 

#define HEIGHTTHROTTLEGAIN (( 1.5*HEIGHT_TARGET_MAX* 1024.0 ) / ( SERVORANGE*SERVOSAT ))

int pitchAltitudeAdjust = 0 ;
boolean filterManual = false;

int desiredHeight ;

extern struct waypointparameters goal ;

void normalAltitudeCntrl(void) ;
void manualThrottle(int throttleIn) ;
void hoverAltitudeCntrl(void) ;


void altitudeCntrl(void)
{
	if ( STABILIZE_HOVERING && current_orientation == F_HOVER )
	{
		hoverAltitudeCntrl() ;
	}
	else
	{
		normalAltitudeCntrl() ;
	}
	
	return ;
}


void normalAltitudeCntrl(void)
{
	union longww throttleAccum ;
	union longww pitchAccum ;
	int throttleIn ;
	int throttleInOffset ;
	union longww heightError ;
	
	if ( flags._.radio_on == 1 )
	{
		throttleIn = pwIn[THROTTLE_INPUT_CHANNEL] ;
		// keep the In and Trim throttle values within 2000-4000 to account for
		// Spektrum receivers using failsafe values below 2000.
		throttleInOffset = pulsesat( pwIn[THROTTLE_INPUT_CHANNEL] ) - pulsesat( pwTrim[THROTTLE_INPUT_CHANNEL] ) ;
	}
	else
	{
		throttleIn = pwTrim[THROTTLE_INPUT_CHANNEL] ;
		throttleInOffset = 0 ;
	}
	
	if ( flags._.altitude_hold_throttle || flags._.altitude_hold_pitch )
	{
		if ( THROTTLE_CHANNEL_REVERSED ) throttleInOffset = - throttleInOffset ;
		
		if ( flags._.GPS_steering )
		{
			if ( desired_behavior._.climbout )
			{
				desiredHeight = goal.height ;
			}
			else
			{
				desiredHeight = goal.fromHeight + (((goal.height - goal.fromHeight) * (long)progress_to_goal)>>12)  ;
			}
		}
		else
		{
			desiredHeight =(( __builtin_mulss((int)( HEIGHTTHROTTLEGAIN ), throttleInOffset )) >> 11) ;
			if (desiredHeight < (int)(HEIGHT_TARGET_MIN)) desiredHeight = (int)(HEIGHT_TARGET_MIN) ;
		}
		
		if ( throttleInOffset < (int)(DEADBAND) && flags._.radio_on )
		{
			pitchAltitudeAdjust = 0 ;
			throttleAccum.WW  = 0 ;
		}
		else
		{
			heightError._.W1 = - desiredHeight ;
			heightError.WW += IMUlocationz.WW ;
			heightError.WW = heightError.WW >> 13 ;
			if ( heightError._.W0 < (- (int)(HEIGHT_MARGIN*8.0)) )
			{
				throttleAccum.WW = (int)(MAXTHROTTLE) ;
				pitchAltitudeAdjust = (int)(PITCHATMAX) ;
			}
			else if ( heightError._.W0 > (int)(HEIGHT_MARGIN*8.0) )
			{
				throttleAccum.WW = 0 ;
				pitchAltitudeAdjust = (int)(PITCHATZERO) ;
			}
			else
			{
				throttleAccum.WW = (int)(MAXTHROTTLE) + (__builtin_mulss( (int)(THROTTLEHEIGHTGAIN), ( - heightError._.W0 - (int)(HEIGHT_MARGIN*8.0) ) )>>3 );
				if ( throttleAccum.WW > (int)(MAXTHROTTLE) ) throttleAccum.WW = (int)(MAXTHROTTLE) ;
				pitchAccum.WW = __builtin_mulss( (int)(PITCHHEIGHTGAIN) , - heightError._.W0 - (int)(HEIGHT_MARGIN*8.0 ))>>3 ;
				pitchAltitudeAdjust = (int)(PITCHATMAX) + pitchAccum._.W0 ; 
			}
			
#if (RACING_MODE == 1)
			if ( flags._.GPS_steering )
			{
				throttleAccum.WW = FIXED_WP_THROTTLE ;
			}
#endif
		}
		
		if ( !flags._.altitude_hold_throttle )
		{
			manualThrottle(throttleIn) ;
		}
		else if ( flags._.GPS_steering && desired_behavior._.land )
		{
			pitchAltitudeAdjust = 0 ;
			
			throttleFiltered.WW += (((long)(pwTrim[THROTTLE_INPUT_CHANNEL] - throttleFiltered._.W1 ))<<THROTTLEFILTSHIFT ) ;
			altitude_control = throttleFiltered._.W1 - throttleIn ;
		}
		else
		{
			// Servo reversing is handled in servoMix.c
			int throttleOut = pulsesat( pwTrim[THROTTLE_INPUT_CHANNEL] + throttleAccum.WW ) ;
			throttleFiltered.WW += (((long)( throttleOut - throttleFiltered._.W1 )) << THROTTLEFILTSHIFT ) ;
			altitude_control = throttleFiltered._.W1 - throttleIn ;
		}
		
		if ( !flags._.altitude_hold_pitch )
		{
			pitchAltitudeAdjust = 0 ;
		}
		
		filterManual = true;
	}
	else
	{
		pitchAltitudeAdjust = 0 ;
		manualThrottle(throttleIn) ;
	}
	
	return ;
}


void manualThrottle( int throttleIn )
{
	throttleFiltered.WW += (((long)( throttleIn - throttleFiltered._.W1 )) << THROTTLEFILTSHIFT ) ;
	
	if (filterManual) {
		// Continue to filter the throttle control value in manual mode to avoid large, instant
		// changes to throttle value, which can burn out a brushed motor.  But after fading over
		// to the new throttle value, stop applying the filter to the throttle out to allow
		// faster control.
		altitude_control = throttleFiltered._.W1 - throttleIn ;
		if (altitude_control < 10) filterManual = false ;
	}
	else {
		altitude_control = 0 ;
	}
}


// For now, hovering does not attempt to control the throttle, and instead
// gives manual throttle control back to the pilot.
void hoverAltitudeCntrl(void)
{
	int throttleIn = ( flags._.radio_on == 1 ) ? pwIn[THROTTLE_INPUT_CHANNEL] : pwTrim[THROTTLE_INPUT_CHANNEL] ;
	
	throttleFiltered.WW += (((long)( throttleIn - throttleFiltered._.W1 )) << THROTTLEFILTSHIFT ) ;
	
	if (filterManual) {
		// Continue to filter the throttle control value in manual mode to avoid large, instant
		// changes to throttle value, which can burn out a brushed motor.  But after fading over
		// to the new throttle value, stop applying the filter to the throttle out to allow
		// faster control.
		altitude_control = throttleFiltered._.W1 - throttleIn ;
		if (altitude_control < 10) filterManual = false ;
	}
	else {
		altitude_control = 0 ;
	}
}

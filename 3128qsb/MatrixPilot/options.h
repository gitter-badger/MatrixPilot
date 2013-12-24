
/*  Release 3128qsb-aps-t107.02, supports UDB4, UDB5 (AUAV3 WIP)

    Setup: UDB4, SF HMC5883L magnetometer, Sonar Maxbotix MB1230, BMP085 Barometer(WIP), 406 Std. GPS, Breeze 2000 V-tail
    
	Baseline: MatrixPilot trunk rel. 3128
  
    Last modified date  Dec 22, 2013

    MODIFICATIONS :

    Iteration: MxPilot-3125nqsb-aps-brz2k-t106

	t00 -  Mods:  work in sonar modifications and enhancements from MxPilot-1782qbs-aps-brz2k-t105
		-  Code changes:
			o - radioIn.c: consolidated to combine sonarIn.c plus new algorithm for pin and channel assignment and 
				data capture
			o - added radioIn.c udb_init_sonar() initialization in libUDB.c, pointers in libUDB_internal.h
			o - sonarCntrl.c formula (binary operations), function call and parameters changes 
			o - propagated function call / parameters changes throughout  radioIn.c, option.h,  altitudeCntrl.c, libUDB.c
				libUDB_internal.h, telemetry.c, defines.h 
			o - telemetry.c auto adjust serial baud rate when sonar and barometer is in use
			o - telemetry.c added sonar and barometer (rough in- DEV WIP) and serial log output debug options
			o - defines.h added "boolean setjmp(void);" (L197) declaration used in main.c to resolve compile warning 
  			o - options.h: reorganized/reconstituted to enforce logical groupings of parameters, some moved from other 
                header files, for ease of use and provide single point of access to all basic and essential setup params 
			o - options.h added ON_SETUP_WARNINGS define in analog2digitia_UDB4.c, mcu.c and osdspi.c to turn off/on test 
                setup warning messages during build for debug purposes 
		-  Outcome:  BUG solved with new revised sonar algorithm, now providing consistently good sonar data reading
			o - RESOLVED: BUG solved with new revised sonar algorithm, has sustained good sonar data and readings
			o - All sensors works well with consistently good data read good
			o - All modes, activates with switch changes and control changes
		-  Log: MxPilot-3125nqsb-aps-brz2k-t106-00a

	t01a-  Mods:  work in logo, sonar and barometer modifications and enhancements from MxPilot-1782qbs-aps-brz2k-t105
		-  Code changes:
			o - flightplan-logo.h, flightplan-logo.c: added new systems parameters and sonar and barometer related 
				data and fuctions; 
			o - defines.h:  added sonar and barometer related data (external) pointers  
			o - options.h: transfered osd.h's video format definition, included I2C2_QUEUED_ON to test queued I2C2 driver
			o - I2C2.c (driver), magnetometer.c, barometer.c: added new queued algorithm and pointers
			o - I2C.h: added new queued algorithm pointers & headers "oscillator.h" and "events.h" to solve compile bugs
			o - libUDB.c: trigger the I2C1 and I2C2 (QUEUED) service-drivers routine to run at 4HZ
			o - libDCM.c, estYawDrift.c: added triggers for queued I2C2 based magnetometer and barometer algorithms
			o - barometerCntrl.c, barometerCntrl.h (..\MatrixPilot): replaced estAltitude.c and estAltitude.h in ..\libDCM 
				with new barometer algorithms.. name and location change normalizes overall structure
			o - estYawDrift.c: added dcm_flags._.mag_drift_req = 1 #if (MAG_YAW_DRIFT == 1)
			o - telemetry.c: barometer data included in the log output
			o - deadReckoning.c: added algo to apply sonar and barometer altitude if flagged on below and if used in LOGO
			o - osd_spi.c, mcu.c and gpsParseCommon.c: added SETUP_WARNINGS_ON algo to set on/off warning during build
			o - navigate.c: roughed-in sonar origin altitude alternate us if SONAR_ALTITUDE is turned on..
			o - barometerCntrl.c ~h and barometer.c: updated data types to use <stdint.h>
		-  Outcome:  
			o - Compiles clean for UDB4 and UDB5
			o - Sonar feed is good, all controls tested working including modes switching, 
			o - BUG: Barometer data's is not feeding in, no feed at all
		-  Logs: MxPilot-3125nqsb-aps-brz2k-t106-01a.TXT

	t01b-  Mods:  trace and debug I2C2 programs and compare diff with MxPilot-1782qbs-aps-brz2k-t105
		-  Code changes:
			o - libUDBc, I2C.h, I2C2.c I2C1.c. replaced declarations, pointers and change into small case some i2c2_ini calls 
		-  Outcome:  
			o - Compiles clean for UDB4 and UDB5
			o - Sonar feed is good, all controls tested working including modes switching,
			o - Barometer temperature is now feeding but BUG: still no pressure data capture
		-  Logs: MxPilot-3125nqsb-aps-brz2k-t106-01b.TXT

	t01c-f Mods:  reverted barometerCntrl.c ~h and barometer.c incl. updated data types back to orig. MxPilot-1782qbs-aps-brz2k-t105
		-  Code changes:
			o - barometerCntrl.c ~h and barometer.c ~ h revert data types back to orig. MxPilot-1782qbs-aps-brz2k-t105, 
				flow and nesting re-structured
		-  Outcome:  
			o - Compiles clean for UDB4 and UDB5
			o - Sonar feed is good, all controls tested working including modes switching,
			o - still faild to produce barometer data
		-  Logs: MxPilot-3125nqsb-aps-brz2k-t106-01c.TXT


    Iteration: MxPilot-3128qsb-aps-brz2k-t106.02

	t02 -  Mods:  getting barometer feed data synchronized and validated, updated to include 3128 modifications (Robert)
		-  Code changes:
			o - radioIn.c, sonarIn.c: fixed-resolves AUAV3 compile errors
			o - barometerCntrl.c: WIP

	TODOS:	1)check and sync barometer data feed
			2)setup_origin(); in navigate.c, look into this call to capture lauch takeoff point and angle for LOGO landing support
*/

// ACKNOWLEDGEMENTS AND SPECIAL THANKS :
// MatrixPilot Group, especially Robert, Guilio and Pete for the contributions and assistance in the sonar and 
//   	barometer enhancements
// Creator:        Bill Premerlani's UAV Dev Board 
// See the AUTHORS.TXT file for a list of authors of MatrixPilot.

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



///////////////////////////////////  I.  BASIC CONFIGURATION   ////////////////////////////////////
// 
//   Section to manage: Airframe, Board Deployment, Mandatory Devices, Input and Output Connections,  
//                                       Options and Finetuning
//
///////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////    I.1 AIRFRAME, BOARD ORIENTATION & MANDATORY GPS  //////////////////////////
//
//  Supported:
//    AIRFRAME_STANDARD         Elevator, and Ailerons and/or Rudder control
//    AIRFRAME_VTAIL            Ailerons(optional), and Elevator and Rudder as V-tail controls
//    AIRFRAME_DELTA            Aileron and Elevator as Elevons, and Rudder(optional)
//  Currently Not Supported:
//    AIRFRAME_HELI             
//    AIRFRAME_QUAD           
//
#define AIRFRAME_TYPE                       AIRFRAME_STANDARD  //AIRFRAME_VTAIL

// BOARD ORIENTATION
//      For UDB4, X arrow points to the front, GPS connectors are on the front.
//      For AUAV3, airplane symbol points to the front, GPS connector is at rear.
//
// The following 6 orientations have the board parallel with the ground.
// ORIENTATION_FORWARDS:  Component-side up,   GPS connector front
// ORIENTATION_BACKWARDS: Component-side up,   GPS connector back
// ORIENTATION_INVERTED:  Component-side down, GPS connector front
// ORIENTATION_FLIPPED:   Component-side down, GPS connector back
// ORIENTATION_YAWCW:     Component-side up,   GPS connector to the right
// ORIENTATION_YAWCCW:    Component-side up,   GPS connector to the left
// ORIENTATION_ROLLCW: 	  board rolled 90 degrees clockwise, from point of view of the pilot
// ORIENTATION_ROLLCW180: board rolled 90 degrees clockwise,
//        from point of view of the pilot, then rotate the board 180 around the Z axis of the plane,
//        so that the GPS connector points toward the tail of the plane
#define BOARD_ORIENTATION                   ORIENTATION_FORWARDS

// MANDATORY GPS
// Set to GPS_STD, GPS_UBX_2HZ, GPS_UBX_4HZ, GPS_MTEK, GPS_NMEA
#define GPS_TYPE                            GPS_STD   // 
//#define DEFAULT_GPS_BAUD                    57600   // added for GPS_NMEA support

// RADIO/FULL AUTONOMOUS ON/OFF
// Set this to 1 if you want the UAV Dev Board to fly your plane without a radio transmitter or
// receiver. (Totally autonomous.)  Recommended ONLY for simulation and debugging.  
// Note that  there's no manual control to fall back on if things go wrong, when turned on.  
// Also may not be legal in your area. Check and conform with your local laws, rules and regulations.
#define NORADIO                             0


/////////////////////////    I.2 INPUT CHANNEL CONFIGURATION  //////////////////////////
// 
// 
// Use a single PPM input connection from the RC receiver to the UDB on RC input channel 1.
// The 8 standard output channels remain unaffected.  2 additional output channels are available
// on pins RA4 and RA1.
// 
// For all boards:
// If you're not sure, leave USE_PPM_INPUT set to 0.
// PPM_NUMBER_OF_CHANNELS is the number of channels sent on the PWM signal.  This is
// often different from the NUM_INPUTS value below, and should usually be left at 8.
// 
#define USE_PPM_INPUT						1
#define PPM_NUMBER_OF_CHANNELS				8
#define PPM_SIGNAL_INVERTED					0
#define PPM_ALT_OUTPUT_PINS					0

// NUM_INPUTS:
// For classic boards: Set to 1-5 (or 1-8 when using PPM input)
//   1-4 enables only the first 1-4 of the 4 standard input channels
//   5 also enables E8 as the 5th input channel
//  For UDB4 boards: Set to 1-8
#define NUM_INPUTS                          8

// INPUT CHANNELS OPTIONS
//   - If you're set up to use Rudder Navigation (like MatrixNav), then you may want to swap
//     the aileron and rudder channels so that rudder is CHANNEL_1, and aileron is 5.
// This setup works only with a 8C PPM encoder and an 8 channel, minimum, TX-RX (DX8)
/*  DEFAULT
#define THROTTLE_INPUT_CHANNEL				CHANNEL_3
#define AILERON_INPUT_CHANNEL				CHANNEL_1
#define ELEVATOR_INPUT_CHANNEL				CHANNEL_2
#define RUDDER_INPUT_CHANNEL				CHANNEL_5
#define MODE_SWITCH_INPUT_CHANNEL			CHANNEL_4
#define CAMERA_PITCH_INPUT_CHANNEL			CHANNEL_UNUSED
#define CAMERA_YAW_INPUT_CHANNEL			CHANNEL_UNUSED
#define CAMERA_MODE_INPUT_CHANNEL			CHANNEL_UNUSED
#define OSD_MODE_SWITCH_INPUT_CHANNEL		CHANNEL_UNUSED
#define PASSTHROUGH_A_INPUT_CHANNEL			CHANNEL_UNUSED
#define PASSTHROUGH_B_INPUT_CHANNEL			CHANNEL_UNUSED
#define PASSTHROUGH_C_INPUT_CHANNEL			CHANNEL_UNUSED
#define PASSTHROUGH_D_INPUT_CHANNEL			CHANNEL_UNUSED
*/
/*  Other options relative to the Radio TX type:
//  Multiplex 12-16 C M-Link W/ 8C PPM encoder             		 physical PPM IN and direct connected board IN channels to MPX Tx / 8 TO 11C RX
#define THROTTLE_INPUT_CHANNEL				CHANNEL_1            // PPM/C1 to RxC3  Throttle
#define AILERON_INPUT_CHANNEL				CHANNEL_2 		     // PPM/C2 to RxC1  Ail
#define ELEVATOR_INPUT_CHANNEL				CHANNEL_3            //	PPM/C3 to RxC2  Elev
#define RUDDER_INPUT_CHANNEL				CHANNEL_4            // PPM/C4 to RxC4  Rudder
#define MODE_SWITCH_INPUT_CHANNEL			CHANNEL_5            // PPM/C5 to RxC6  MPX AUX 2 3p toggle, FM (flight mode)
#define LOGO_A_CHANNEL						CHANNEL_6		  	 // PPM/C6 to RxC5  MPX Gear 3p toggle, 1st LOGO plan change 		  
#define LOGO_B_CHANNEL						CHANNEL_7		  	 // PPM/C7 to RxC7  MPX Aux1 3p toggle, 2nd LOGO plan change 
#define LOGO_C_CHANNEL						CHANNEL_8		  	 // PPM/C8 to RxC5  MPX Aux1, 3rd LOGO HI/LO speed select
//DIRECT-CONNECT 			 				>>>>>>>>			 // RX/C9 3p toggle to Landing Gear
//DIRECT-CONNECT 			 				>>>>>>>>			 // RX/C10 3p toggle to Spoiler / air brake OR crow/butterfly mixing
//DIRECT-CONNECT 			 				>>>>>>>>			 // RX/C11 3p toggle to camber-thermal / normal / reflex-speed with deg. slider mixing
//
//  BASIC 7 CHANNEL RX, W/O an 8C PPM                            physical IN channels to MPX Tx / 7C RX
#define THROTTLE_INPUT_CHANNEL				CHANNEL_2            // Input 2 to RxC3  Throttle
#define AILERON_INPUT_CHANNEL				CHANNEL_3 		     // Input 3 to RxC1  Ail
#define ELEVATOR_INPUT_CHANNEL				CHANNEL_4            //	Input 4 to RxC2  Elev
#define RUDDER_INPUT_CHANNEL				CHANNEL_5            // Input 5 to RxC4  Rudder
#define MODE_SWITCH_INPUT_CHANNEL			CHANNEL_6            // Input 6 to RxC6  MPX AUX 2 3p toggle, FM
#define LOGO_A_CHANNEL						CHANNEL_7		  	 // Input 7 to RxC5  MPX Gear 3p toggle, 1st LOGO plan change 	  
#define LOGO_B_CHANNEL						CHANNEL_8		  	 // Input 8 to RxC7  MPX Aux1 3p toggle, 2nd LOGO plan change 
//  
//  Specktrum DX8 W/O PPM C1 & used for Voltage sensor           physical IN channels to AR8000 Rx / DX8 Tx
#define THROTTLE_INPUT_CHANNEL				CHANNEL_2            // Input 2 to RxC1  Throttle
#define AILERON_INPUT_CHANNEL				CHANNEL_3 		     // Input 3 to RxC2  Ail
#define ELEVATOR_INPUT_CHANNEL				CHANNEL_4            //	Input 4 to RxC3  Elev
#define RUDDER_INPUT_CHANNEL				CHANNEL_5            // Input 5 to RxC4  Rudder
#define MODE_SWITCH_INPUT_CHANNEL			CHANNEL_6            // Input 6 to RxC6  AUX1, flight mode
#define LOGO_A_CHANNEL						CHANNEL_7		  	 // Input 7 to RxC6  AUX2,3p toggle, 1st LOGO plan change 	  
#define LOGO_B_CHANNEL						CHANNEL_8		  	 // Input 8 to RxC7  AUX3,3p toggle, 2nd LOGO plan change 
*/
//  Specktrum DX8                                                physical PPM IN channels to AR8000 Rx / DX8 Tx
#define THROTTLE_INPUT_CHANNEL				CHANNEL_1            // PPM/C1 to RxC1  Throttle
#define AILERON_INPUT_CHANNEL				CHANNEL_2 		     // PPM/C2 to RxC2  Ail
#define ELEVATOR_INPUT_CHANNEL				CHANNEL_3            //	PPM/C3 to RxC3  Elev
#define RUDDER_INPUT_CHANNEL				CHANNEL_4            // PPM/C4 to RxC4  Rudder
#define MODE_SWITCH_INPUT_CHANNEL			CHANNEL_5            // PPM/C5 to RxC6  AUX1, flight mode
#define LOGO_A_CHANNEL						CHANNEL_6		  	 // PPM/C6 to RxC7  AUX2,3p toggle, 1st LOGO plan change 	  
#define LOGO_B_CHANNEL						CHANNEL_7		  	 // PPM/C7 to RxC8  AUX3,3p toggle, 2nd LOGO plan change 
#define LOGO_C_CHANNEL						CHANNEL_8		  	 // PPM/C8 to RxC5  GR,2p toggle, 3rd LOGO HI/LO speed select
// Unused 
#define CAMERA_PITCH_INPUT_CHANNEL			CHANNEL_UNUSED       
#define CAMERA_YAW_INPUT_CHANNEL			CHANNEL_UNUSED       
#define CAMERA_MODE_INPUT_CHANNEL			CHANNEL_UNUSED
#define OSD_MODE_SWITCH_INPUT_CHANNEL		CHANNEL_UNUSED
#define PASSTHROUGH_A_INPUT_CHANNEL			CHANNEL_UNUSED
#define PASSTHROUGH_B_INPUT_CHANNEL			CHANNEL_UNUSED
#define PASSTHROUGH_C_INPUT_CHANNEL			CHANNEL_UNUSED
#define PASSTHROUGH_D_INPUT_CHANNEL			CHANNEL_UNUSED

// MODE INPUT AND SWITCH SETUP
// Ideally a 3-position switch on your transmitter
// Often the Flap channel will be controlled by a 3-position switch.
// These are the thresholds for the cutoffs between low and middle, and between middle and high.
// Normal signals should fall within about 2000 - 4000.
#define MODE_SWITCH_THRESHOLD_LOW           2600
#define MODE_SWITCH_THRESHOLD_HIGH          3400

// TWO POSITION SWITCH SETUP
// Setting MODE_SWITCH_TWO_POSITION to 1,  allows a two state mode switch on the transmitter to be used
// to create three flight modes. When switch is "Down" the plane always reverts to Manual. When "Up",
// the plane moves to Stabilized". If the user is in stabilized ("Up"), and then the user toggles
// the switch to Down, Up, Down, Up, then the plane moves to autonomous.
// Each toggle must be achieved with a limited time period ( 1/2 a second ) and not faster than 1/40th of a second.
// When in Autonomous, a move to "Down" puts the switch state  back to Manual. And a futher move to "Up", will put the
// switch state back in stabilized. The important design concept is that Manual position is always Manual state immediately.
// Stabilized position is Stabilized mode unless you try  hard to reach Autonomous mode.
// Set MODE_SWITCH_TWO_POSITION to 0 for a normal three position mode switch.
#define MODE_SWITCH_TWO_POSITION            0


/////////////////////////    I.3 OUTPUT CHANNEL CONFIGURATION  //////////////////////////
// 
// NOTE: If USE_PPM_INPUT is enabled above, up to 9 outputs are available.)
// For UDB4/5 boards: Set to 3-8 (or up to 10 using pins RA4 and RA1.)
// For AUAV3 boards:  Set to 3-8 (or up to 11 using pins RE1, RA6 and RA7.)
//                               (this needs developing, so contact the list)
#define NUM_OUTPUTS							5

// Channel numbers for each output
// Use as is, or edit to match your setup.
//   - Only assign each channel to one output purpose
//   - If you don't want to use an output channel, set it to CHANNEL_UNUSED
//   - If you're set up to use Rudder Navigation (like MatrixNav), then you may want to swap
//     the aileron and runner channels so that rudder is CHANNEL_1, and aileron is 5.
// 
// NOTE: If your board is powered from your ESC through the throttle cable, make sure to
// connect THROTTLE_OUTPUT_CHANNEL to one of the built-in Outputs (1, 2, or 3) to make
// sure your board gets power.
/*  DEFAULT
#define THROTTLE_OUTPUT_CHANNEL				CHANNEL_3
#define AILERON_OUTPUT_CHANNEL				CHANNEL_1
#define ELEVATOR_OUTPUT_CHANNEL				CHANNEL_2
#define RUDDER_OUTPUT_CHANNEL				CHANNEL_4
#define AILERON_SECONDARY_OUTPUT_CHANNEL	CHANNEL_UNUSED
#define CAMERA_PITCH_OUTPUT_CHANNEL			CHANNEL_UNUSED
#define CAMERA_YAW_OUTPUT_CHANNEL			CHANNEL_UNUSED
#define TRIGGER_OUTPUT_CHANNEL				CHANNEL_UNUSED
#define PASSTHROUGH_A_OUTPUT_CHANNEL		CHANNEL_UNUSED
#define PASSTHROUGH_B_OUTPUT_CHANNEL		CHANNEL_UNUSED
#define PASSTHROUGH_C_OUTPUT_CHANNEL		CHANNEL_UNUSED
#define PASSTHROUGH_D_OUTPUT_CHANNEL		CHANNEL_UNUSED
*/
//                                                                physical pin to servo/controls connections
//  UDB4/UDB3/PPM_ALT_OUTPUT_PINS=1 OPTIONS 
#define THROTTLE_OUTPUT_CHANNEL				CHANNEL_3            // Out3 to ESC/BL Motor  
#define AILERON_OUTPUT_CHANNEL				CHANNEL_4            // Out4/IN3/RE0 to AIL 1
#define ELEVATOR_OUTPUT_CHANNEL				CHANNEL_2            // Out2 to Elevator 
#define RUDDER_OUTPUT_CHANNEL				CHANNEL_1            // Out1 to Rudder 
#define AILERON_SECONDARY_OUTPUT_CHANNEL	CHANNEL_5	     	 // Out5/IN2/RE2 to AIL 2
// Unused 
#define CAMERA_PITCH_OUTPUT_CHANNEL			CHANNEL_UNUSED
#define CAMERA_YAW_OUTPUT_CHANNEL			CHANNEL_UNUSED
#define TRIGGER_OUTPUT_CHANNEL				CHANNEL_UNUSED
#define PASSTHROUGH_A_OUTPUT_CHANNEL		CHANNEL_UNUSED
#define PASSTHROUGH_B_OUTPUT_CHANNEL		CHANNEL_UNUSED
#define PASSTHROUGH_C_OUTPUT_CHANNEL		CHANNEL_UNUSED
#define PASSTHROUGH_D_OUTPUT_CHANNEL		CHANNEL_UNUSED


/////////////////////////    I.4 SERVOS CONFIGURATION  //////////////////////////
//
// SERVO REVERSING 
// For any of these that are set to 1, that servo will be sent reversed controls.
// Note that your servo reversing settings here should match what you set on your transmitter.
// 
#define AILERON_CHANNEL_REVERSED            1 // 0 for breeze vtail glider, 1 for standard
#define ELEVATOR_CHANNEL_REVERSED           0 // 1 for breeze vtail glider, 0 for standard
#define RUDDER_CHANNEL_REVERSED             1 // 0 for breeze vtail glider, 1 for standard
#define AILERON_SECONDARY_CHANNEL_REVERSED  0
#define THROTTLE_CHANNEL_REVERSED           0
#define CAMERA_PITCH_CHANNEL_REVERSED       0
#define CAMERA_YAW_CHANNEL_REVERSED         0

// Set this to 1 if you need to switch the left and right elevon or vtail surfaces
//
#define ELEVON_VTAIL_SURFACES_REVERSED      0



////////////////////////   II. CONFIGURING OPTIONAL SENSORS AND DEVICES   /////////////////////////
// 
//    Section to manage: Magnetormeter, Barometer, Sonar, OSD, Current-Voltage-RSSI and CAM gimbal 
//                                       Setup and Finetuning
//
///////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////    II.1. MAGNETOMETER (DIGITAL COMPASS) SUPPORT   ////////////////////////
///
// Define MAG_YAW_DRIFT to be 1 to use magnetometer for yaw drift correction.
// Otherwise, if set to 0 the GPS will be used.
// If you select this option, you also need to set magnetometer options in
// the magnetometerOptions.h file, including declination and magnetometer type.
#define MAG_YAW_DRIFT 						1

// Define which magnetometer is used
// HMC5843 is the 3DRobotics HMC5843 (now out of production).
// HMC5883L is the 3DRobotics HMC5883L
// HMC5883L_SF is the SparkFun HMC5883L
#define HMC5883L_SF

// Define magneticDeclination to be the magnectic declination, in degrees, measured
// clockwise from the north, east is plus, west is minus.
//  Mississauga, ON is Lat 45.58 N and Long 79.65 W, Mag. Decl. therefore is 10deg21' W or -10.35 degrees
//  Bennet Field Springvale, ON is Lat 42deg58' N and Long 80deg9' W, Mag. Decl. therefore is 9deg48' W or -9.48 degrees
#define MAGNETICDECLINATION 				-10.35  // 0

// Set to 0 for fixed declination angle or 1 for variable declination angle
#define DECLINATIONANGLE_VARIABLE 			0

// #define LED_RED_MAG_CHECK 1 if you want the RED LED to indicate the magnetometer is not working.
// #define LED_RED_MAG_CHECK 0 if you want the RED LED to indicate control mode.
#define LED_RED_MAG_CHECK                   0

// The following 4 supported orientations have the mag level with the ground.
// MAG_FORWARDS:  Component-side up,   edge connector front
// MAG_BACKWARDS: Component-side up,   edge connector back
// MAG_INVERTED:  Component-side down, edge connector front
// MAG_FLIPPED:   Component-side down, edge connector back
// MAG_DIRECT:    Magnetometer mounted in an orientation that permits a direct connection to a UDB4 or UDB5
// Note: right now, if MAG_DIRECT is selected, UDB board orientation must be ORIENTATION_FORWARDS
// For 3DRobotics mags, for MAG_DIRECT the mag mounts over the UDB, component side down.
// For SparkFun HMC5883L, for MAG_DIRECT the mag mounts over the UDB, component side up.
#define MAG_FORWARDS


/////////////////////////     II.2. SONAR AGL ALTITUDE SENSOR SUPPORT      ////////////////////////
//
// Works with UDB4 only with the sensor attached to the UDB4 or UDB5 INPUT C8 
// Supports MAXBOTIX MB1230 and 2200 for MB1260 XL
// http://www.maxbotix.com/Ultrasonic_Sensors/MB1230.htm
// This option is only usable with LOGO enhancements.
// Provides for a soft landing LOGO function using sonar AGL (above ground level) altitude.
// Note: Set to 1 to enable a sonar sensor device and for debugging option 
//     SERIAL_OUTPUT_FORMAT may be defined as SERIAL_SONAR for debugging purposes
//     If this is turned off, when used in LOGO, ALT_SNR will have a 0 default value
//     Turned on, the baud rate for the SERIAL_OUTPUT_FORMAT will default to 57600.
//     Adjust any serial telemetry logging and transmitting device to same baud rate.
#define USE_SONAR							1

// Works with UDB4, UDB5 and AUAV3. This feature can only be combined with USE_SONAR set to 1, below.
// Design for supporting autonomous and soft precision landing flight plan in LOGO.  
// Otherwise, set to 0 to use GPS and IMU estimated ALTITUDE. 
// When enabled, sonar altitude will be used to recalibrate altitude in navigation (navigate.c) and 
// deadreckoning within .2 to 4m altitude range, depending on the sensor class used.
#define SONAR_ALTITUDE 						1

// This specifies the tested effective and vendor max range of the type of sonar being used: . 
// 	400 cm (4 m) effective and 750 cm max (vendor rated) for an MB1230 XL-MaxSonar-EZ3 
// 	500 cm (5 m) effective and 1000 cm max (10 m vendor rated) for an  MB1261 XL-MaxSonar-EZL1
#define EFFECTV_SONAR_ALTRANGE                400 // Def 450 Reliable Sonar measurement distance (centimeters) 
#define MAXIMUM_SONAR_ALTRANGE                700 // Distance in centimeters close to vendor max. range spec
#define SONAR_MINIMUM_VALREADS                  3 // Def 3, number of readings used threshold, for  integrity/error 
                                                  //   filtering, will slow down the aglaltitude feed and 


////////////////     II.3. BAROMETER PRESURE (ALTIMETER) AND TEMPERATURE SUPPORT    ///////////////
//
// Works with  only UDB4. The barometer sensor must be plugged in at the UDB4's I2C2 pins.
//
// This feature is optionally combined with BAROMETER ALTITUDE set to 1, above.
// When enabled, barometer altitude will be available as systems value in LOGO functions and in 
// telemetry.  With BAROMETER ALTITUDE set to 1, above and BAR_ALT~ LOGO function used, barometer 
// altitude will be used in deadreckoning for altitude correction, on ground or 20m above altitude 
// range, complementing the sonar's altitude range if enabled below and above.
// Recommended sensors:  BMP085 with .25 m accuracy and altitude range anywhere between -1640 to about 
// 29,000 ft  (manufacturer's data). 
// uncomment to enable for testing and debugging barometer program only
// 	
#define USE_BAROMETER						1  

// Define BAROMETER_ALTITUDE to be 1 to use barometer for altitude correction.
// Otherwise, if set to 0 only the GPS will be used.
// If you select this option, you also need to correctly set the ASL_GROUND_ALT in centimeters
// to your takeoff location altitude at the time of initialisation.
// If set to 1, barometer altitude will be used to recalibrate altitude in navigation and 
// deadreckoning, starting 0 to 20m above altitude range, depending on the whether a SONAR sensor 
// is attached and if so, what sonar sensor class used is used.
#define BAROMETER_ALTITUDE                  1  // UNTESTED

// Home position fix above-sea-level(ASL) ground altitude in centimeter, USED BY sonar and barometer
//   altitude computation when USE_PA_PRESSURE is set to 0 and a barometer sensor is enabled
//   eg. PN: Home front yard 13200, OMFC, SF 16950
#define ASL_GROUND_ALT						13200	// above sea level (ASL) altitude in centimeters

// USE_PA_PRESSURE 
// if set to 1, will use hPA PA_PRESSURE and if set to 2, will use MC_PRESSURE mercury inch based on 
// METAR/station computation
#define USE_PA_PRESSURE						1    

// PA_PRESSURE below is for Ontario, Canada as of 11-16-2012, 1029.6826 hPA from
// http://www.wunderground.com/cgi-bin/findweather/hdfForecast?query=Mississauga%2C+Canada
#define PA_PRESSURE							101300   // in (1013 pa) hPA  [set USE_PA_PRESSURE to 1]
#define MC_PRESSURE							3016     // 30.16 in. mercury (METAR), set USE_PA_PRESSURE to 2

//  To use realtime ground pressure set to 1, or otherwise will use static-altimeter-calibrated 
//  ground pressure for ASL and AGL computations. 
//  IMPORTANT: Use realtime only for ground/terrain based vehicles to constantly update
//     ground reference. Also, this is only applicable with PA_PRESSURE set to 1 or 2.
#define USE_REALTIME_GRDPRES 				0 

// Barometer oversampling [OSS] can be set from 0 thru 3
//				  [ms]  Ave/cur/uA   [hPA]      [m]
// Set  Samples  Time  /1 sample   RMS noise  RMS noise
//  0	 1		  4.5		3		  0.06		0.5  [ultra low power]
//  1	 2		  7.5		5		  0.05		0.4  [standard]
//  2	 4		 13.5		7		  0.04		0.3  [high resolution]
//  3	 8		 25.5		12		  0.03		0.25 [ultra high resolution]
#define OSS 								1

// EXTENDED FLIGHT RANGE (EXPERIMENTAL AND NOT FLIGHT TESTED)
// Optionally enable experimental extended range navigation support (merged from ballon launch branch)
//
//#define USE_EXTENDED_NAV

// USING BAROMETRIC PA (EXPERIMENTAL AND NOT FLIGHT TESTED, WILL WORK ONLY WITH EXTENDED FLIGHT RANGE)
// Uncomment the ff. to use barometer pressure altitude instead of GPS altitude
//    in the gpsParseCommon.c program
//
//#define USE_PRESSURE_ALT


//  BAROMETER ALGORITHM DUMMY TEST VALUES    
//
//#define TEST_WITH_DATASHEET_VALUES 	//  uncomment to use test values 


// TESTING runtime trigger location:
//  0- barometerCntrl.c (def); 1- gpsParseCommon.c; 2- libDCM.c; 
// 
#define ALTRUN_TRIG	 						0   

// Alternate barometer calibrations trigger locations for BarometerCalOne() & ~CalTwo() functions. 
//  If set to 0, calibration (~CalOne()) is triggered 1st in states.c then ~CalTwo() in libDCM.c
//  Options 0- states.c & libDCM.c (def); 1- both in navigate.c;
//
#define ALTCAL_TRIG	 						0   

//  MAGNETOMETER AND BAROMETER i2c mode,  0 to turnoff and 1 to use queued I2C2 driver
//
#define I2C2_QUEUED_ON                		0

//  estBarometerAltitude() estimation algorithm, uncomment to calculate data based on 
//  full use of barometer parameters, comment to use simple calculation
//
#define EST_BAROMETER_ALLDATA


//////////////////////   II.4. TELEMETRY, ON SCREEN DISPLAY (OSD) SETUP     ////////////////////
//
// On Screen Display
// USE_OSD enables the OSD system.  OSD Layout can be customized in the osd_layout.h file.
// Option for USE_OSD:  OSD_NONE. OSD_NATIVE, OSD_REMZIBI
#define USE_OSD                         OSD_NATIVE

// OSD_VIDEO_FORMAT can be set to either OSD_NTSC, or OSD_PAL
#define OSD_VIDEO_FORMAT                OSD_PAL

// set this to 1 to use the SPI peripheral, 0 to bit-bash 
// while osd_spi.c identifies SPI1 port (CK1 DO1 DI1) used for native osd applies to UDB5
#define USE_OSD_SPI     				1  

// Serial Output Format (Can be SERIAL_NONE, SERIAL_DEBUG, SERIAL_ARDUSTATION, SERIAL_UDB,
// SERIAL_UDB_EXTRA,SERIAL_MAVLINK, SERIAL_CAM_TRACK, or SERIAL_OSD_REMZIBI)
// This determines the format of the output sent out the spare serial port.
// Note that SERIAL_OSD_REMZIBI only works with a ublox GPS.
// SERIAL_UDB_EXTRA will add additional telemetry fields to those of SERIAL_UDB.
// SERIAL_UDB_EXTRA can be used with the OpenLog without characters being dropped.
// SERIAL_UDB_EXTRA may result in dropped characters if used with the XBEE wireless transmitter.
// SERIAL_CAM_TRACK is used to output location data to a 2nd UDB, which will target its camera at this plane.
// SERIAL_MAVLINK is a bi-directional binary format for use with QgroundControl, HKGCS or MAVProxy 
// SERIAL_MAVLINK is only supported on the UDB4 to ensure that sufficient RAM is available.
// SERIAL_UDB_SONAR for debugging sonar sensor displays data feed, should be combined with USE_SONAR set to 1
// SERIAL_UDB_BAROMETER for debugging sonar sensor displays data feed, should be combined with USE_BAROMETER set to 1
// Note that SERIAL_MAVLINK defaults to using a baud rate of 57600 baud (other formats default to 19200)
#define SERIAL_OUTPUT_FORMAT                SERIAL_UDB_EXTRA

// Serial Output BAUD rate for either standard telemetry streams or MAVLink
//  19200, 38400, 57600, 115200, 230400, 460800, 921600 
#define SERIAL_BAUDRATE                     57600


/////////////////////     II.5. ANALOG INPUT CURRENT-VOLTAGE-RSSI SENSORS      ////////////////////
//
// NUM_ANALOG_INPUTS: Set to 0, 1, or 2
//   1 enables Radio In 1 as an analog Input
//   2 also enables Radio In 2 as another analog Input
//   NOTE: Can only be set this higher than 0 if USE_PPM_INPUT is enabled above.
// For UDB4 boards: Set to 0-4.  Analog pins are AN15 - AN18.
#define NUM_ANALOG_INPUTS					2  // >>>>>>  VERIFY IF THIS STILL WORKS AS IT USED TOO

// Channel numbers for each analog input
//   - Only assign each channel number to one analog sensor
//   - If you don't want to use an output channel, set it to CHANNEL_UNUSED
//   - Only 2 analog inputs are available, so you can't use all the defined analog
//     sensors at once
//
// ANALOG_CURRENT_INPUT_CHANNEL and ANALOG_VOLTAGE_INPUT_CHANNEL let you plug in and
// use this Voltage/Current sensor board from SparkFun:
//    http://www.sparkfun.com/products/9028
// Just plug the ground and signal lines of the chosen current input channel into the
// ground and current outputs of the current sensor, and the signal line of the chosen
// voltage input channel to the voltage output from the current sensor.  Values for
// instantaneous current, voltage, and mAh used will become available for use with the
// OSD layout.
//
// ANALOG_RSSI_INPUT_CHANNEL lets you connect your RC Receiver's RSSI output to your
// UDB, in order to see the RC signal strength on your OSD.  Just plug RSSI and ground
// from your Receiver to Input2's signal and ground on your UDB.  If you use this feature,
// you'll also need to set up the RSSI_MIN_SIGNAL_VOLTAGE and RSSI_MAX_SIGNAL_VOLTAGE
// to match your Receiver's RSSI format.  Note that some receivers use a higher voltage to 
// represent a lower signal strength, so you may need to set MIN higher than MAX.

#define ANALOG_CURRENT_INPUT_CHANNEL        CHANNEL_2
#define ANALOG_VOLTAGE_INPUT_CHANNEL        CHANNEL_1
#define ANALOG_RSSI_INPUT_CHANNEL           CHANNEL_UNUSED

#define MAX_CURRENT                         900 // 90.0 Amps max for the sensor from SparkFun (in tenths of Amps)
#define CURRENT_SENSOR_OFFSET               10  // Add 1.0 Amp to whatever value we sense

#define MAX_VOLTAGE                         543 // 54.3 Volts max for the sensor from SparkFun (in tenths of Volts)
#define VOLTAGE_SENSOR_OFFSET               0   // Add 0.0 Volts to whatever value we sense

// RSSI - RC Receiver signal strength
#define RSSI_MIN_SIGNAL_VOLTAGE             0.5     // Voltage when RSSI should show 0%
#define RSSI_MAX_SIGNAL_VOLTAGE             3.3     // Voltage when RSSI should show 100%


//////////////   II.6. CAMERA GIMBAL STABILIZATION, TARGETTING AND TRIGERING SETUP   //////////////
//
// Set this value to 1, for camera to be stabilized using camera options further below.
#define USE_CAMERA_STABILIZATION			0

// Trigger Action
// Use the trigger to do things like drop an item at a certain waypoint, or take a photo every
// N seconds during certain waypoint legs.

// TRIGGER_TYPE can be set to TRIGGER_TYPE_NONE, TRIGGER_TYPE_SERVO, or TRIGGER_TYPE_DIGITAL.
// If using TRIGGER_TYPE_SERVO, set the TRIGGER_OUTPUT_CHANNEL above to choose which output channel
// receives trigger events, and set the TRIGGER_SERVO_LOW and TRIGGER_SERVO_HIGH values below.
// If using TRIGGER_TYPE_DIGITAL, the trigger will be on pin RE4.  In this case make sure to set
// NUM_OUTPUTS to be less than 6 to avoid a conflict between digital output and servo output on
// that pin.

// TRIGGER_ACTION can be: TRIGGER_PULSE_HIGH, TRIGGER_PULSE_LOW, TRIGGER_TOGGLE, or TRIGGER_REPEATING
// The trigger action output is always either low or high.  In servo mode, low and high are servo
// values set below.  In digital mode, low and high are 0V and 5V on pin RE4.
// The action is triggered when starting on a waypoint leg that includes the F_TRIGGER flag (see the
// waypoints.h file).
// If set to TRIGGER_PULSE_HIGH or TRIGGER_PULSE_LOW, then the output will pulse high or low for the
// number of milliseconds set by TRIGGER_PULSE_DURATION.
// If set to TRIGGER_TOGGLE, the output will just switch from high to low, or low to high each time
// the action is triggered.
// If set to TRIGGER_REPEATING, then during any waypoint leg with F_TRIGGER set, high pulses will be
// sent every TRIGGER_REPEAT_PERIOD milliseconds.

// Note, durations in milliseconds are rounded down to the nearest 25ms.
#define TRIGGER_TYPE                        TRIGGER_TYPE_NONE
#define TRIGGER_ACTION                      TRIGGER_PULSE_HIGH
#define TRIGGER_SERVO_LOW                   2000
#define TRIGGER_SERVO_HIGH                  4000
#define TRIGGER_PULSE_DURATION              250
#define TRIGGER_REPEAT_PERIOD               4000


// There are three camera modes within MatrixPilot
//  Canera Mode 1: No stabilisation for camera pitch or yaw
//  Camera Mode 2: Stabilisation of camera pitch but not yaw.
//  Camera Mode 3: Camera targetting. The camera is aimed at a GPS location.
// Control of camera modes
// If CAMERA_MODE_INPUT_CHANNEL is assigned to a channel in the channels section of
// options.h then a three position switch can be used to select between the three camera
// stabilization modes. The following min and max values should work for most transmitters.
#define CAMERA_MODE_THRESHOLD_LOW          2600
#define CAMERA_MODE_THRESHOLD_HIGH         3400

// If you do not have a spare channel for CAMERA_MODE_INPUT_CHANNEL then,
// If CAMERA_MODE_INPUT_CHANNEL is defined as CHANNEL_UNUSED :-
//  In UDB Manual Mode the camera is fixed straight ahead. (Camera mode 1)
//  In UDB Stabilized Mode, the camera stabilizes in the pitch axis but stabilizes a constant yaw
//     relative to the plane's frame of reference. (Camera mode 2).
//  In Waypoint Mode, the direction of the camera is driven from a flight camera plan in waypoints.h
// In all three flight modes, if you set CAMERA_INPUT_CHANNEL then the transmitter camera controls
// will be mixed into the camera stabilisation. This allows a pilot to override the camera stabilization dynamically
// during flight and point the camera at a specific target of interest.

// Setup and configuration of camera targetting at installation of camera servos:-
// To save cpu cycles, you will need to pre-compute the tangent of the desired pitch of the camera
// when in stabilized mode. This should be expressed in 2:14 format. 
// Example: You require the camera to be pitched down by 15 degrees from the horizon in stabilized mode.
// Paste the following line into a google search box (without the //)
// tan((( 15 /180 )* 3.1416 ))* 16384
// The result, as an integer, will be 4390. Change the angle, 15, for whatever angle you would like.
// Note that CAM_TAN_PITCH_IN_STABILIZED_MODE should not exceed 32767 (integer overflows to negative).
#define CAM_TAN_PITCH_IN_STABILIZED_MODE    1433    // 1443 is 5 degrees of pitch. Example: 15 degrees is 4389
#define CAM_YAW_IN_STABILIZED_MODE          0       // in degrees relative to the plane's yaw axis.    Example: 0

// All number should be integers
#define CAM_PITCH_SERVO_THROW               95      // Camera lens rotation at maximum PWM change (2000 to 4000), in degrees.          
#define CAM_PITCH_SERVO_MAX                 85      // Max pitch up that plane can tilt and keep camera level, in degrees.  
#define CAM_PITCH_SERVO_MIN                -22      // Max pitch down that plane can tilt and keep camera level, in degrees. 
#define CAM_PITCH_OFFSET_CENTRED            38      // Offset in degrees of servo that results in a level camera.           
                                                    // Example: 30 would mean that a centered pitch servo points the camera
                                                    // 30 degrees down from horizontal when looking to the front of the plane.

#define CAM_YAW_SERVO_THROW                 350     // Camera yaw movement for maximum yaw PWM change (2000 to 4000) in Degrees. 
#define CAM_YAW_SERVO_MAX                   130     // Max positive yaw of camera relative to front of plane in Degrees.              
#define CAM_YAW_SERVO_MIN                  -130     // Min reverse  yaw of camera relative to front of plane in Degrees.   
#define CAM_YAW_OFFSET_CENTRED              11      // Yaw offset in degrees that results in camera pointing forward. 

// Camera test mode will move the yaw from + 90 degrees to + 90 degrees every 5 seconds. (180 degree turn around)
// That will show whether the CAM_PITCH_SERVO_THROW value is set correctly for your servo.
// Once the camera rotates correctly through 180 degrees, then you can adjust CAM_PITCH_OFFSET_CENTRED to center the camera.
// In Camera test mode, pitch angle changes permanently to 90 degrees down in stabilized mode, and  0 (level) in Manual Mode.
#define CAM_TESTING_OVERIDE                 0       // Set to 1 for camera to move to test angles in stabilized mode.
#define CAM_TESTING_YAW_ANGLE               90      // e.g. 90 degrees. Will try to swing 90 degrees left, then 90 degrees right
#define CAM_TESTING_PITCH_ANGLE             90      // In degrees.

// Set this to 1 to ignore camera target data from the flightplan, and instead use camera target data coming in on the serial port.
// This data can be generated by another UDB running MatrixPilot, using SERIAL_CAM_TRACK.
// NOTE: When using camera tracking, both UDBs must be set to use the same fixed origin location.
#define CAM_USE_EXTERNAL_TARGET_DATA        0



///////////////    III. FLIGHT BEHAVIOR AND NAV PARAMETERS SETUP & FINETUNING    //////////////////
// 
//    Section to manage:  basic setup, control gains and rates, flight navigation thresholds
//            normal and failsave flight plan management and vehicle identification             
//									and communications
//
///////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////    III.1  STABILIZATION AND AUTO PILOT BASIC SETUP    ///////////////////////
// Enable/Disable core features of this firmware
//
// Roll, Pitch, and Yaw Stabilization
// Set any of these to 0 to disable the stabilization in that axis.
#define ROLL_STABILIZATION_AILERONS         1
#define ROLL_STABILIZATION_RUDDER           1 //0
#define PITCH_STABILIZATION                 1
#define YAW_STABILIZATION_RUDDER            1
#define YAW_STABILIZATION_AILERON           1

// Aileron and Rudder Navigation
// Set either of these to 0 to disable use of that control surface for navigation.
#define AILERON_NAVIGATION                  1
#define RUDDER_NAVIGATION                   1

// Cross track margin, in meters
// This is used when the cross track option is attached to a waypoint
// It defines the amount of cross track error at which the cross tracking
// bearing adjustment saturates at 45 degree. You can also think of it
// as the reciprocal of the cross tracking gain.
// A larger value of cross track margin is more stable, a smaller one
// holds the cross track error to smaller values.
// 64 meters is probably the largest value you might use on a fast model jet (more than 50 meters/sec)
// Use 32 meters for 20 to 50 meters/sec, and 16 meters for less than that.
#define CROSS_TRACK_MARGIN                  16 // def 32

// Wind Gain Adjustment
// This is an option for modulating the navigation gains in flight
// to maintain a constant turn radius in heavy winds in waypoing mode.
// Define WIND_GAIN_ADJUSTMENT as 1 to turn this feature on.
#define WIND_GAIN_ADJUSTMENT                0

// Altitude Hold
// Use altitude hold in stabilized mode?  In waypoint mode?
// Each of these settings can be AH_NONE, AH_FULL, or AH_PITCH_ONLY
//  - In waypoint mode, the target altitude is defined by the waypoints or logo program.
//  - In stabilized mode, when ALTITUDEHOLD_STABILIZED is set to AH_PITCH_ONLY, the target
// altitude is whatever altitude the plane was at when switched into stabilized mode.
//  - In stabilized mode, when ALTITUDEHOLD_STABILIZED is set to AH_FULL, the target
// altitude is determined by the position of the throttle stick on the transmitter.
// NOTE: even when set to AH_NONE, MatrixPilot will still try to stabilize pitch as long
// as PITCH_STABILIZATION is set to 1 above, but will not aim for any specific altitude.
#define ALTITUDEHOLD_STABILIZED             AH_FULL  // def AH_PITCH_ONLY
#define ALTITUDEHOLD_WAYPOINT               AH_FULL

// Speed Control
// If you define SPEED_CONTROL to be 1, MatrixPilot will take air speed into account
// in the altitude controls, and will trim the throttle and pitch to maintain air speed.
// Define DESIRED_SPEED to be the air speed that you want, in meters/second.
#define SPEED_CONTROL                       0
#define DESIRED_SPEED                       10.0    // meters/second

// Inverted flight
// Set these to 1 to enable stabilization of inverted flight in stabilized and/or waypoint modes.
#define INVERTED_FLIGHT_STABILIZED_MODE     1
#define INVERTED_FLIGHT_WAYPOINT_MODE       1

// Hovering
// Set these to 1 to enable stabilization of hovering in stabilized and/or waypoint modes.
#define HOVERING_STABILIZED_MODE            0
#define HOVERING_WAYPOINT_MODE              0

// Note: As of MatrixPilot 3.0, Dead Reckoning and Wind Estimation are automatically enabled.

// Racing Mode
// Setting RACING_MODE to 1 will keep the plane at a set throttle value while in waypoint mode.
// RACING_MODE_WP_THROTTLE is the throttle value to use, and should be set between 0.0 and 1.0.
// Racing performance can be improved by disabling cross tracking for your waypoints.
#define RACING_MODE                         0
#define RACING_MODE_WP_THROTTLE             1.0


////////////////////    III.2  CONTROL GAINS, LIMITS AND RATES FINETUNING    //////////////////////
//
// All gains should be positive real numbers.
// Proportional gains should be less than 4.0.
// Rate gains should be less than 0.8.
// Proportional gains include ROLLKP, YAWKP_AILERON, AILERON_BOOST, PITCHGAIN,
// RUDDER_ELEV_MIX, ROLL_ELEV_MIX, ELEVATOR_BOOST, YAWKP_RUDDER, ROLLKP_RUDDER,
// MANUAL_AILERON_RUDDER_MIX, RUDDER_BOOST, HOVER_ROLLKP, HOVER_PITCHGAIN, HOVER_YAWKP
// Rate gains include ROLLKD, YAWKD_AILERON, PITCHKD, YAWKD_RUDDER, ROLLKD_RUDDER,
// HOVER_ROLLKD, HOVER_PITCHKD, HOVER_YAWKD

// SERVOSAT limits servo throw by controlling pulse width saturation.
// set it to 1.0 if you want full servo throw, otherwise set it to the portion that you want
#define SERVOSAT                            1.0

// Aileron/Roll Control Gains
// ROLLKP is the proportional gain, approximately 0.25
// ROLLKD is the derivative (gyro) gain, approximately 0.125
// YAWKP_AILERON is the proportional feedback gain for ailerons in response to yaw error
// YAWKD_AILERON is the derivative feedback gain for ailerons in response to yaw rotation
// AILERON_BOOST is the additional gain multiplier for the manually commanded aileron deflection
/* Def.:
#define ROLLKP                              0.20
#define ROLLKD                              0.05
#define YAWKP_AILERON                       0.10
#define YAWKD_AILERON                       0.05
#define AILERON_BOOST                       1.00
*/
#define ROLLKP								0.25  // 0.25
#define ROLLKD								0.07  // 0.08
#define YAWKP_AILERON						0.09  // 0.08, 0.18
#define YAWKD_AILERON						0.05
#define AILERON_BOOST						1.00

// Elevator/Pitch Control Gains
// PITCHGAIN is the pitch stabilization gain, typically around 0.125
// PITCHKD feedback gain for pitch damping, around 0.0625
// RUDDER_ELEV_MIX is the degree of elevator adjustment for rudder and banking
// AILERON_ELEV_MIX is the degree of elevator adjustment for aileron
// ELEVATOR_BOOST is the additional gain multiplier for the manually commanded elevator deflection
/* Def.:
#define PITCHGAIN                           0.10
#define PITCHKD                             0.04
#define RUDDER_ELEV_MIX                     0.20
#define ROLL_ELEV_MIX                       0.05
#define ELEVATOR_BOOST                      0.50
*/
#define PITCHGAIN							0.11  // 0.12
#define PITCHKD								0.04  // 0.07
#define RUDDER_ELEV_MIX						0.16  // reduced for vtail configuration
#define ROLL_ELEV_MIX						0.06
#define ELEVATOR_BOOST						0.55

// Neutral pitch angle of the plane (in degrees) when flying inverted
// Use this to add extra "up" elevator while the plane is inverted, to avoid losing altitude.
#define INVERTED_NEUTRAL_PITCH              8.2 // def. 9.0

// Rudder/Yaw Control Gains
// YAWKP_RUDDER is the proportional feedback gain for rudder navigation
// YAWKD_RUDDER is the yaw gyro feedback gain for the rudder in reponse to yaw rotation
// ROLLKP_RUDDER is the feedback gain for the rudder in response to the current roll angle
// ROLLKD_RUDDER is the feedback gain for the rudder in response to the rate of change roll angle
// MANUAL_AILERON_RUDDER_MIX is the fraction of manual aileron control to mix into the rudder when
// in stabilized or waypoint mode.  This mainly helps aileron-initiated turning while in stabilized.
// RUDDER_BOOST is the additional gain multiplier for the manually commanded rudder deflection
/*
#define YAWKP_RUDDER                        0.05
#define YAWKD_RUDDER                        0.05
#define ROLLKP_RUDDER                       0.06
#define ROLLKD_RUDDER                       0.05
#define MANUAL_AILERON_RUDDER_MIX           0.00
#define RUDDER_BOOST                        1.00
*/
#define YAWKP_RUDDER						0.06   //  0.08, 0.11
#define YAWKD_RUDDER						0.06   //  0.12
#define ROLLKP_RUDDER						0.08 
#define ROLLKD_RUDDER						0.05
#define MANUAL_AILERON_RUDDER_MIX			0.02
#define RUDDER_BOOST						1.00

// Gains for Hovering
// Gains are named based on plane's frame of reference (roll means ailerons)
// HOVER_ROLLKP is the roll-proportional feedback gain applied to the ailerons while navigating a hover
// HOVER_ROLLKD is the roll gyro feedback gain applied to ailerons while stabilizing a hover
// HOVER_PITCHGAIN is the pitch-proportional feedback gain applied to the elevator while stabilizing a hover
// HOVER_PITCHKD is the pitch gyro feedback gain applied to elevator while stabilizing a hover
// HOVER_PITCH_OFFSET is the neutral pitch angle for the plane (in degrees) while stabilizing a hover
// HOVER_YAWKP is the yaw-proportional feedback gain applied to the rudder while stabilizing a hover
// HOVER_YAWKD is the yaw gyro feedback gain applied to rudder while stabilizing a hover
// HOVER_YAW_OFFSET is the neutral yaw angle for the plane (in degrees) while stabilizing a hover
// HOVER_PITCH_TOWARDS_WP is the max angle in degrees to pitch the nose down towards the WP while navigating
// HOVER_NAV_MAX_PITCH_RADIUS is the radius around a waypoint in meters, within which the HOVER_PITCH_TOWARDS_WP
//                            value is proportionally scaled down.
#define HOVER_ROLLKP                        0.05
#define HOVER_ROLLKD                        0.05
#define HOVER_PITCHGAIN                     0.2
#define HOVER_PITCHKD                       0.25
#define HOVER_PITCH_OFFSET                  0.0        // + leans towards top, - leans towards bottom
#define HOVER_YAWKP                         0.2
#define HOVER_YAWKD                         0.25
#define HOVER_YAW_OFFSET                    0.0
#define HOVER_PITCH_TOWARDS_WP             30.0
#define HOVER_NAV_MAX_PITCH_RADIUS         20


///////////////////////    III.3  ALTITUDE HOLD AND THROTTLE SETUP    /////////////////////////////
// 
// These settings are only used when Altitude Hold is enabled above.

// Min and Max target heights in meters.  These only apply to stabilized mode.
#define HEIGHT_TARGET_MIN                   4.0  //25.00
#define HEIGHT_TARGET_MAX                   500.0  // def. 100m, 500m is 1640.42ft, 600m is 1968.5ft 

// The range of altitude within which to linearly vary the throttle
// and pitch to maintain altitude.  A bigger value makes altitude hold
// smoother, and is suggested for very fast planes.
#define HEIGHT_MARGIN                       6 // def. 10

// Use ALT_HOLD_THROTTLE_MAX when below HEIGHT_MARGIN of the target height.
// Interpolate between ALT_HOLD_THROTTLE_MAX and ALT_HOLD_THROTTLE_MIN
// when within HEIGHT_MARGIN of the target height.
// Use ALT_HOLD_THROTTLE_MIN when above HEIGHT_MARGIN of the target height.
// Throttle values are from 0.0 - 1.0.
#define ALT_HOLD_THROTTLE_MIN               0.38  // def. 0.35
#define ALT_HOLD_THROTTLE_MAX               0.75  // def. 1.0

// Use ALT_HOLD_PITCH_MAX when below HEIGHT_MARGIN of the target height.
// Interpolate between ALT_HOLD_PITCH_MAX and ALT_HOLD_PITCH_MIN when
// within HEIGHT_MARGIN of the target height.
// Use ALT_HOLD_PITCH_HIGH when above HEIGHT_MARGIN of the target height.
// Pitch values are in degrees.  Negative values pitch the plane down.
#define ALT_HOLD_PITCH_MIN                 -16.0  //def 15
#define ALT_HOLD_PITCH_MAX                  15.0
#define ALT_HOLD_PITCH_HIGH                -16.0  //def 15

//////////////////////////    III.4  FLIGHT PLAN AND FAILSAFE SETUP    ////////////////////////////
// Flight Plan handling
//
// You can define your flightplan either using the UDB Waypoints format, or using UDB Logo
// Set this to either FP_WAYPOINTS or FP_LOGO
// The Waypoint definitions and options are located in the waypoints.h file.
// The Logo flight plan definitions and options are located in the flightplan-logo.h file.
#define FLIGHT_PLAN_TYPE                    FP_LOGO

// Set this to either LOGO_SIMPLE or LOGO_VARIABLE
// With LOGO_SIMPLE AUTO MODE invokes fix LOGO flight plan.
// With LOGO_VARIABLE, AUTO MODE invokes LOGO fight plan with pattern and properties 
//   controlled from a radio TX,  while in flight
//#define LOGO_TYPE                    		LOGO_VARIABLE  //WIP, to accommodate an option for beginners

// Return To Launch Pitch Down in degrees, a real number.
// this is the real angle in degrees that the nose of the plane will pitch downward during a return to launch.
// it is used to increase speed (and wind penetration) during a return to launch.
// set it to zero if you do not want to use this feature.
// This only takes effect when entering RTL mode, which only happens when the plane loses the transmitter signal.
#define RTL_PITCH_DOWN                      0.0

// The Failsafe Channel is the RX channel that is monitored for loss of signal
// Make sure this is set to a channel you actually have plugged into the UAV Dev Board!
//
// For a receiver that remembers a failsafe value for when it loses the transmitter signal,
// like the Spektrum AR6100, you can program the receiver's failsafe value to a value below
// the normal low value for that channel.  Then set the FAILSAFE_INPUT_MIN value to a value
// between the receiver's programmed failsafe value and the transmitter's normal lowest
// value for that channel.  This way the firmware can detect the difference between a normal
// signal, and a lost transmitter.
//
// FAILSAFE_INPUT_MIN and _MAX define the range within which we consider the radio on.
// Normal signals should fall within about 2000 - 4000.
#define FAILSAFE_INPUT_CHANNEL              THROTTLE_INPUT_CHANNEL
#define FAILSAFE_INPUT_MIN                  1500
#define FAILSAFE_INPUT_MAX                  4500

// FAILSAFE_TYPE controls the UDB's behavior when in failsafe mode due to loss of transmitter
// signal.  (Set to FAILSAFE_RTL or FAILSAFE_MAIN_FLIGHTPLAN.)
//
// When using FAILSAFE_RTL (Return To Launch), the UDB will begin following the RTL flight plan
// as defined near the bottom of the waypoints.h or flightplan-logo.h files.  By default, this
// is set to return to a point above the location where the UDB was powered up, and to loiter there.
// See the waypoints.h or flightplan-logo.h files for info on modifying this behavior.
//
// When set to FAILSAFE_MAIN_FLIGHTPLAN, the UDB will instead follow the main flight plan as
// defined in either waypoints.h or flightplan-logo.h.  If the UDB was already in waypoint mode
// when it lost signal, the plane will just continue following the main flight plan without
// starting them over.  And if the transmitter is still in waypoint mode when the UDB sees it
// again, the UDB will still continue following the main flight plan without restarting.  If
// the UDB loses signal while not in waypoint mode, it will start the main flight plan from the
// beginning.
#define FAILSAFE_TYPE                       FAILSAFE_RTL

// When FAILSAFE_HOLD is set to 1, then once Failsafe has engaged, and you have subsequently
// regained your RC TX-RX connection, you will need to manually change the Mode Switch in order
// to exit Failsafe mode.  This avoids the situation where your plane flies in and out of range,
// and keeps switching into and out of Failsafe mode, which depending on your configuration,
// could be confusing and/or dangerous.
#define FAILSAFE_HOLD                       1


///////////////    IV. FLIGHT SIMULATION, COMMUNICATION, ADVANCE OPTIONS SETUP    ////////////////
// 
//    Section to manage:  WIP description
//
///////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////   IV.1  HARDWARE IN THE LOOP, FLIGHT SIMULATION AND IDENTITY   //////////////////
//
// Hardware In the Loop Simulation
// Only set this to 1 for testing in the simulator.  Do not try to fly with this set to 1!
// See the MatrixPilot wiki for more info on using HILSIM.
// HILSIM_BAUD is the serial speed for communications with the X-Plane plugin.  Default is
// now 38400.  Make sure the X-Plane plugin's Setup file has its speed set to match.
#define HILSIM                              0
#define HILSIM_USB                          0           // AUAV3 only (under development)
#define HILSIM_BAUD                         38400


// MAVLink requires an aircraft Identifier (I.D) as it is deaigned to control multiple aircraft
// Each aircraft in the sky will need a unique I.D. in the range from 0-255
#define MAVLINK_SYSID                       138

// Vehicle and Pilot Identification

// Once you are flying your plane and swapping flights and telemetry with other's across
// the world, you may like to fill in some of the fields below. This will be embedded in your
// telemetry, and used to make more interesting flights in Google Earth.
// ID_VEHICLE_MODEL_NAME provides indication of what model of plane, quad, car etc you are using
// ID_VEHICLE_REGISTRATION should be short (less than 12 continuous characters with no space
// it will be used in Google Earth as the folder name containing your flights.
// ID_LEAD_PILOT is your lead pilot flight name or alias e.g. "UAV Flight Director"
// ID_DIY_DRONES_URL should be the URL of your member page on DIY Drones.
// That will allow Google Earth viewers of your flights to click straight through to your latest discussions.
// EXAMPLE:-
//#define ID_VEHICLE_MODEL_NAME "Multiplex Twinstar 2"
//#define ID_VEHICLE_REGISTRATION "TW2-PDH-UK"
//#define ID_LEAD_PILOT "Pete Hollands"
//#define ID_DIY_DRONES_URL "http://www.diydrones.com/profile/PeterHollands"
/*
#define ID_VEHICLE_MODEL_NAME               "Not Defined"
#define ID_VEHICLE_REGISTRATION             "Not Defined"
#define ID_LEAD_PILOT                       "Not Defined"
#define ID_DIY_DRONES_URL                   "http://www.diydrones.com"
*/
#define ID_VEHICLE_MODEL_NAME "brz2k-t06-01"
#define ID_VEHICLE_REGISTRATION "MP-APS4-3128"
#define ID_LEAD_PILOT "DB-EZFLIER"
#define ID_DIY_DRONES_URL ""

/////////////////////////    IV.2  DATA COM AND OTHER ADVANCE OPTIONS    //////////////////////////
//
// Fly-By-Datalink Configure
// This allows flight of an aircraft using data instead of an RC transmitter using the app found
// in /Tools/FlyByDatalink/UDB_FlyByDatalink.exe. This app takes input from a typical off-the-shelf
// gaming joystick and transmits it to the UDB over serial or IP. The joystick used for development
// was the Logitech Attack3. This data overrides the PWM inputs allowing for direct control of the flight
// surfaces. While this is enabled, instead of the usual manual/stabilized/WP flight modes, it's
// FBDL/stabilized/WP. For saftey reasons, an RC transmitter is still required for flight to set the modes.
#define FLY_BY_DATALINK_ENABLED             0

// The UDB4/5 has two UART's, while the AUAV3 has four UART's.
// Three MatrixPilot features are currently defined for using a UART. 
// These being the GPS, Telemetry and a 'debug' console.
// Therefore UDB4/5 is one UART short, the AUAV3 has one UART extra.
//
// CONSOLE_UART specfies which UART is used for stdio support, aka the console.
// Set CONSOLE_UART to 1, 2, 3 or 4 to enable the console on UART of that number.
// Setting CONSOLE_UART to 0 disables console support.
// On the UDB4/5, optionally specifying console support on UART 1 or 2 overrides 
// the default usage of that UART, being the GPS and Telemetry respectively.
// CONSOLE_UART 3 and 4 options are only available with the AUAV3 board.
// Thus UDB4/5 options are 0, 1, or 2  AUAV3 options are 0, 3, or 4
#define CONSOLE_UART                        0

// The following define is used to enable vertical initialization for VTOL
// To enable vertical initialization, uncomment the line
//#define INITIALIZE_VERTICAL

// Optionally enable the new power saving idle mode of the MCU during mainloop
#define USE_MCU_IDLE                        1


///////////////////////////////////    IV.3  DEBUG OPTIONS      ///////////////////////////////////
// Debugging defines

// The following can be used to do a ground check of stabilization without a GPS.
// If you define TestGains, stabilization functions
// will be enabled, even without GPS or Tx turned on. (Tx is optional)
// uncomment this line if you want to test your gains without using GPS
// #define TestGains                        

// Set this to 1 to calculate and print out free stack space
#define RECORD_FREE_STACK_SPACE             0

// Define USE_DEBUG_IO to enable DPRINT macro to call printf(..)
//#define USE_DEBUG_IO


//////////////////////////   IV.4  ADDITIONAL SETUP OPTIONS FOR AUAV3    ////////////////////////////
//
// At present, the AUAV3 schematic and 'installation & basic connections' document
// are drafts and hence there is some inconsistency in labelling conventions.
//
// The following standard labelling convention is proposed.
//
// AUAV3 schematic:
//        TLM      -    PORT1
//        OSD      -    PORT2
//        UART3    -    PORT3
//        GPS      -    PORT4
//
// 'AUAV3 Installation and Basic Connections' document:
//        OUART1   -    PORT1
//        OUART2   -    PORT2
//        UART3    -    PORT3
//        GPS      -    PORT4
//
////////////////////////////////////////////////////////////////////////////////
// On the AUAV3, the external UART connections are known as ports 1 through 4.
// The definitions below specifies which feature maps to an external port.
//
// NOTE: on the AUAV3, do not confuse the CONSOLE_UART definition with the 
// external port assignment.
// Assign the console to an internal UART with CONSOLE_UART, map this console to
// external port connection with DBG_PORT.
#define GPS_PORT                            4
#define TLM_PORT                            3
#define DBG_PORT                            1

// Set this to 1 to enable logging telemetry to dataflash on AUAV3
#define USE_TELELOG                         0

// Set this to 1 to enable loading options settings from a config file on AUAV3
#define USE_CONFIGFILE                      0

// Set this to 1 to enable the USB stack on AUAV3
#define USE_USB                             0

// Set this to 1 to enable the Mass Storage Driver support over USB on AUAV3
#define USE_MSD                             0

// SETUP_WARNINGS ON/OFF comment to turnoff debug and setup test warnings during compile
//
//#define SETUP_WARNINGS_ON  

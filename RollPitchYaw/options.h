
#ifdef ConfigRed
#include "ConfigRed.h"
#endif

#ifdef ConfigGreen
#include "ConfigGreen.h"
#endif

#define LONGDEG_2_BYTECIR 305
// = (256/360)*((256)**4)/(10**7)

#define COURSEDEG_2_BYTECIR 466
 // = (256/360)*((256)**2)/(10**2)

#define FILTERSHIFT 3 
//#define FILTERSHIFT 6
// filter shift divide 

#define GRAVITY ((long long)(5280/SCALEACCEL)) 
// gravity in AtoD/2 units

#define RADPERSEC ((long long)5632)
// one radian per second, in AtoD/2 units

#define GRAVITYM ((long long)980) 
// 100 times gravity, meters/sec/sec

#define CENTRISCALE (long long) (((long long)519168)*GRAVITY)/(RADPERSEC*GRAVITYM)
// scale factor in multiplying omega times velocity to get centrifugal acceleration

#define CENTRIFSAT (long long) (GRAVITYM*RADPERSEC)/(GRAVITY*((long long)32))
// saturation limit for the centrifugal adjustment to avoid numeric overflow

#define RMAX   0b0100000000000000	//	1.0 in 2.14 fractional format
#define RMAX15 0b0110000000000000	//	1.5 in 2.14 format

#define indicate_loading_main LATEbits.LATE0 = 0
#define indicate_loading_inter LATEbits.LATE0 = 1



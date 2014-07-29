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


#ifndef SERVOPREPARE_H
#define SERVOPREPARE_H


extern int16_t throttle_control;
extern int16_t pitch_control;
extern int16_t roll_control;
extern int16_t yaw_control;
extern uint16_t wind_gain;

extern int16_t tiltError[3] ;
extern int16_t RotationRateError[3] ;

void init_servoPrepare(void);


#endif // SERVOPREPARE_H

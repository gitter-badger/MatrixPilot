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
#include "../libDCM/gpsParseCommon.h"
#include "config.h"

#if (USE_TELELOG == 1)
#include "telemetry_log.h"
#endif

#if (USE_USB == 1)
#include "preflight.h"
#endif

#if (CONSOLE_UART != 0)
#include "console.h"
#endif


void init_tasks(void)
{
	while (1)
	{
#if (USE_TELELOG == 1)
		telemetry_log();
#endif
#if (USE_USB == 1)
		USBPollingService();
#endif
#if (CONSOLE_UART != 0 && SILSIM == 0)
		console();
#endif
		udb_run();
	}
	return 0;
}

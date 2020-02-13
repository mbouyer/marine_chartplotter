/*
 * Copyright (c) 2020 Manuel Bouyer
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "NMEA2000.h"
#include "nmea2000_defs_rx.h"
#include "nmea2000_defs_tx.h"
#include <lv_edisplay/edisplay_data.h>

bool private_command_status_rx::handle(const nmea2000_frame &f)
{
	int heading = f.frame2int16(0);
	uint8_t error = f.frame2uint8(2);
	uint8_t r_mode = f.frame2uint8(3);
	int8_t rudder = f.frame2uint8(4);
	uint8_t r_slot = f.frame2uint8(5);
	double headingd;
	if (addr != f.getsrc()) {
		setdst_auto(f.getsrc());
		addr = f.getsrc();
	}
	if (heading == HEADING_INVALID) 
		headingd = -1;
	else
		headingd = rad2deg(heading);

	edisp_set_auto_status(mode, headingd, error, slot);
	return true;
}

void private_command_status_rx::setdst_auto(int addr)
{
	unsigned int dst_pgns[] = {PRIVATE_COMMAND_ENGAGE,
			   PRIVATE_COMMAND_ERRACK,
			   PRIVATE_COMMAND_FACTORS,
			   PRIVATE_COMMAND_FACTORS_REQUEST,
			   PRIVATE_COMMAND_ACUATOR,
			   PRIVATE_REMOTE_CONTROL,
	};

	for (int i = 0; i < sizeof(dst_pgns) / sizeof(dst_pgns[0]); i++) {
		nmea2000P->get_frametx(nmea2000P->get_tx_bypgn(dst_pgns[i]))->setdst(addr);
	}
}

bool private_command_factors_rx::handle(const nmea2000_frame &f)
{
	char slot = f.frame2uint8(0);
	int err = f.frame2int16(1);
	int dif = f.frame2int16(3);
	int dif2 = f.frame2int16(5);
	return true;
}

bool private_remote_control_rx::handle(const nmea2000_frame &f)
{
	char type = f.frame2uint8(0);
	char subtype = f.frame2uint8(1);
	switch(type) {
	case CONTROL_LIGHT:
		switch(subtype) {
		case CONTROL_LIGHT_OFF:
			edisp_set_light(LIGHT_MODE_OFF);
			break;
		case CONTROL_LIGHT_ON:
			edisp_set_light(LIGHT_MODE_ON);
			break;
		case CONTROL_LIGHT_REV:
			edisp_set_light(LIGHT_MODE_REV);
			break;
		}
		break;
	}
	return true;
}

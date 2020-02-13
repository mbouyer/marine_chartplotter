/*
 * Copyright (c) 2019 Manuel Bouyer
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
#include "nmea2000_defs_tx.h"
#include "nmea2000_defs_rx.h"
#include <lv_edisplay/edisplay_data.h>

static int command_address = 0;

bool
private_command_engage_tx::senddata(double heading, uint8_t auto_mode, uint8_t params_slot)
{
	bool ret;
	int headingr;
	if (heading < 0)
		headingr = HEADING_INVALID;
	else
		headingr = deg2rad(heading);
	int162frame(headingr, 0);
	uint82frame(auto_mode, 2);
	uint82frame(params_slot, 3);
	valid = true;
	ret = nmea2000P->send_bypgn(PRIVATE_COMMAND_ENGAGE, true);
	valid = false;
	return ret;
}

bool
private_command_acuator_tx::senddata(int8_t move)
{
	bool ret;
	int82frame(move, 0);
	ret = nmea2000P->send_bypgn(PRIVATE_COMMAND_ACUATOR, true);
	valid = false;
	return ret;
}

bool
private_command_errack_tx::senddata(uint8_t ack_errors)
{
	bool ret;
	uint82frame(ack_errors, 0);
	valid = true;
	ret = nmea2000P->send_bypgn(PRIVATE_COMMAND_ERRACK, true);
	valid = false;
	return ret;
}

bool
private_command_factors_tx::senddata(int8_t slot, int err, int dif, int dif2)
{
	bool ret;
	int82frame(slot, 0);
	int162frame(err, 1);
	int162frame(dif, 3);
	int162frame(dif2, 5);
	valid = true;
	ret = nmea2000P->send_bypgn(PRIVATE_COMMAND_FACTORS, true);
	valid = false;
	return ret;
}

bool
private_command_factors_request_tx::senddata(int8_t slot)
{
	bool ret;
	int82frame(slot, 0);
	valid = true;
	ret = nmea2000P->send_bypgn(PRIVATE_COMMAND_FACTORS_REQUEST, true);
	valid = false;
	return ret;
}

bool
private_remote_control_tx::senddata(uint8_t addr, int8_t type, int8_t subtype, const int8_t *data, int size)
{
	bool ret;
	int82frame(type, 0);
	int82frame(subtype, 1);
	for (int i = 0; i < size - 2; i++) {
		int82frame(data[i], i + 2);
	}
	setdst(addr);
	frame->can_dlc = size;
	valid = true;
	ret = nmea2000P->send_bypgn(PRIVATE_REMOTE_CONTROL, true);
	valid = false;
	return ret;
}

#define N2K_CONTROL_MOB     0x00
#define N2K_CONTROL_MOB_MARK        0x00
#define N2K_CONTROL_MOB_SIZE 2
#define N2K_CONTROL_LIGHT   0x01
#define N2K_CONTROL_LIGHT_OFF       0x00
#define N2K_CONTROL_LIGHT_ON        0x01
#define N2K_CONTROL_LIGHT_VAL       0x02  
#define N2K_CONTROL_LIGHT_REV       0x03
#define N2K_CONTROL_LIGHT_SIZE     2      
#define N2K_CONTROL_LIGHT_VAL_SIZE 3      
#define N2K_CONTROL_RESET   0x02
#define N2K_CONTROL_RESET_SIZE 1
#define N2K_CONTROL_MUTE    0x02
#define N2K_CONTROL_MUTE_SIZE 2
#define N2K_CONTROL_BEEP    0x04
#define N2K_CONTROL_BEEP_SHORT      0x00  
#define N2K_CONTROL_BEEP_LONG       0x01  
#define N2K_CONTROL_BEEP_SIZE 2
#define N2K_CONTROL_REMOTE_RADIO    0x05  

extern "C" {

bool
n2ks_auto_engage(double heading, uint8_t auto_mode, uint8_t slot)
{
        private_command_engage_tx *f = (private_command_engage_tx *)nmea2000P->get_frametx(nmea2000P->get_tx_bypgn(PRIVATE_COMMAND_ENGAGE));
	return f->senddata(heading, auto_mode, slot);
}

bool
n2ks_auto_errack(uint8_t err)
{
        private_command_errack_tx *f = (private_command_errack_tx *)nmea2000P->get_frametx(nmea2000P->get_tx_bypgn(PRIVATE_COMMAND_ERRACK));
	return f->senddata(err);
}

bool
n2ks_auto_factors(int8_t slot, int err, int dif, int dif2)
{
        private_command_factors_tx *f = (private_command_factors_tx *)nmea2000P->get_frametx(nmea2000P->get_tx_bypgn(PRIVATE_COMMAND_FACTORS));
	return f->senddata(slot, err, dif, dif2);
}

bool
n2ks_auto_factors_request(int8_t slot)
{
        private_command_factors_request_tx *f = (private_command_factors_request_tx *)nmea2000P->get_frametx(nmea2000P->get_tx_bypgn(PRIVATE_COMMAND_FACTORS_REQUEST));
	return f->senddata(slot);
}

bool
n2ks_auto_acuator(int8_t move)
{
        private_command_factors_request_tx *f = (private_command_factors_request_tx *)nmea2000P->get_frametx(nmea2000P->get_tx_bypgn(PRIVATE_COMMAND_ACUATOR));
	return f->senddata(move);
}

bool
n2ks_control_mob()
{
        private_remote_control_tx *f = (private_remote_control_tx *)nmea2000P->get_frametx(nmea2000P->get_tx_bypgn(PRIVATE_REMOTE_CONTROL));
	return f->senddata(NMEA2000_ADDR_GLOBAL,
	    CONTROL_MOB, CONTROL_MOB_MARK, NULL, CONTROL_MOB_SIZE);
}

bool
n2ks_control_light_mode(int mode)
{
	int8_t n2k_mode;
	switch(mode) {
	case LIGHT_MODE_OFF:
		n2k_mode = CONTROL_LIGHT_OFF;
		break;
	case LIGHT_MODE_ON:
		n2k_mode = CONTROL_LIGHT_ON;
		break;
	case LIGHT_MODE_REV:
		n2k_mode = CONTROL_LIGHT_REV;
		break;
	default:
		return false;
	}

        private_remote_control_tx *f = (private_remote_control_tx *)nmea2000P->get_frametx(nmea2000P->get_tx_bypgn(PRIVATE_REMOTE_CONTROL));
	return f->senddata(NMEA2000_ADDR_GLOBAL,
	    CONTROL_LIGHT, n2k_mode, NULL, CONTROL_LIGHT_SIZE);
}

void
n2k_set_command_address(int addr)
{
	static const unsigned int dst_pgns[] = {PRIVATE_COMMAND_ENGAGE,
			   PRIVATE_COMMAND_ERRACK,
			   PRIVATE_COMMAND_FACTORS,
			   PRIVATE_COMMAND_FACTORS_REQUEST,
			   PRIVATE_COMMAND_ACUATOR,
	};

	command_address = addr;

	for (int i = 0; i < sizeof(dst_pgns) / sizeof(dst_pgns[0]); i++) {
		nmea2000P->get_frametx(nmea2000P->get_tx_bypgn(dst_pgns[i]))->setdst(addr);
	}
}

} // extern "C"

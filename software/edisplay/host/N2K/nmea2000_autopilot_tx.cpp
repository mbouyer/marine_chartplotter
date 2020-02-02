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

bool
private_command_engage_tx::senddata(double heading, uint8_t auto_mode, uint8_t params_slot)
{
	bool ret;
	int162frame(heading * 1000, 0);
	uint82frame(auto_mode, 2);
	uint82frame(params_slot, 3);
	valid = true;
	ret = nmea2000P->send_bypgn(PRIVATE_COMMAND_ENGAGE, true);
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
private_remote_control_tx::senddata(int8_t type, int8_t subtype)
{
	bool ret;
	int82frame(type, 0);
	int82frame(subtype, 1);
	valid = true;
	ret = nmea2000P->send_bypgn(PRIVATE_REMOTE_CONTROL, true);
	valid = false;
	return ret;
}

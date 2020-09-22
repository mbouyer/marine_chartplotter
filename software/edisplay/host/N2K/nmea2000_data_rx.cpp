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
#include "nmea2000_defs_rx.h"
#include <lv_edisplay/edisplay_data.h>

bool nmea2000_attitude_rx::handle(const nmea2000_frame &f)
{
	int cap = f.frame2int16(1);
	int roll = f.frame2int16(5);
	edisp_set_attitude(rad2deg(cap), rad2deg(roll));
	return true;
}

bool nmea2000_cogsog_rx::handle(const nmea2000_frame &f)
{
	unsigned int cog = f.frame2uint16(2);
	int sog = f.frame2uint16(4);
	edisp_set_cogsog(urad2deg(cog), sog);
	return true;
}

bool nmea2000_xte_rx::handle(const nmea2000_frame &f)
{
	int xte = f.frame2int32(2);
	edisp_set_xte(xte);
	return true;
}

bool nmea2000_navdata_rx::fast_handle(const nmea2000_frame &f)
{
	unsigned int capp, distp;
	uint32_t wp;

	capp = f.frame2uint16(14);
	distp = f.frame2uint32(1);
	wp  = f.frame2uint32(20);
	edisp_set_navdata(urad2deg(capp), distp, wp);
	return true;
}

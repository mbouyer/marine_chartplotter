/*
 *  Copyright (c) 2019 Manuel Bouyer.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef EDISPLAY_DATA_H
#define EDISPLAY_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

void edisp_set_attitude(int, int);
void edisp_set_cogsog(int, int);
void edisp_set_xte(int);
void edisp_set_navdata(int, int, uint32_t);
void edisp_set_winddata(int, int);
void edisp_set_light(int);
#define LIGHT_MODE_OFF 0
#define LIGHT_MODE_ON 1
#define LIGHT_MODE_REV 2
void edisp_control_page(int);

void edisp_set_auto_status(uint8_t, double, uint8_t, uint8_t);
void edisp_set_auto_factors(uint8_t, int, int, int);
/* has to match nmea2000_pgn.h value from canbus_autopilot */
#define AUTO_OFF	0x0000
#define AUTO_STANDBY	0x0001
#define AUTO_HEAD	0x0002

#define AUTO_ERR_OUTPUT		0x01
#define AUTO_ERR_OVERLOAD	0x02
#define AUTO_ERR_RUDDER		0x04
#define AUTO_ERR_ATTITUDE	0x08

bool n2ks_auto_engage(double, uint8_t, uint8_t);

bool n2ks_auto_errack(uint8_t);
bool n2ks_auto_factors(int8_t, int, int, int);
bool n2ks_auto_factors_request(int8_t);
bool n2ks_auto_acuator(int8_t);
bool n2ks_control_mob(void);
bool n2ks_control_light_mode(int);

void n2k_set_command_address(int);

#define NPARAMS 6

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*EDISPLAY_DATA_H*/

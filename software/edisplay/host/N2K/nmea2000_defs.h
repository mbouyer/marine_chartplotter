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

#ifndef NMEA2000_DEFS_H_
#define NMEA2000_DEFS_H_

class nmea2000_desc {
    public:
        const char *descr;
	const bool isuser;
	const int pgn;
	bool enabled;

	inline nmea2000_desc(const char *desc, bool isuser, int pgn) :
	    descr(desc), isuser(isuser), pgn(pgn) {enabled = false;}
	virtual ~nmea2000_desc() {};
};

#define NMEA2000_PRIORITY_HIGH          0
#define NMEA2000_PRIORITY_SECURITY      1
#define NMEA2000_PRIORITY_CONTROL       3
#define NMEA2000_PRIORITY_REQUEST       6
#define NMEA2000_PRIORITY_INFO          6
#define NMEA2000_PRIORITY_ACK           6
#define NMEA2000_PRIORITY_LOW           7

#define NMEA2000_ADDR_GLOBAL    255
#define NMEA2000_ADDR_NULL      254
#define NMEA2000_ADDR_MAX       251

#define NMEA2000_INDUSTRY_GROUP 4

#define ISO_ADDRESS_CLAIM	60928U
#define ISO_REQUEST		59904U

#define NMEA2000_DATETIME	129033U
#define NMEA2000_ATTITUDE	127257U
#define NMEA2000_RATEOFTURN	127251U
#define NMEA2000_COGSOG		129026U
#define NMEA2000_XTE		129283U
#define NMEA2000_NAVDATA	129284U

#define PRIVATE_COMMAND_STATUS	61846U
#define 	HEADING_INVALID 32767
#define PRIVATE_COMMAND_ACUATOR	38400U
#define PRIVATE_COMMAND_ENGAGE	38656U
#define PRIVATE_COMMAND_ERRACK	38912U
#define PRIVATE_COMMAND_FACTORS	39168UL
#define PRIVATE_COMMAND_FACTORS_REQUEST	39424U
#define PRIVATE_REMOTE_CONTROL	39680U
#define 	CONTROL_MOB     0x00
#define 	CONTROL_MOB_MARK        0x00
#define 	CONTROL_MOB_SIZE 2
#define 	CONTROL_LIGHT   0x01
#define 	CONTROL_LIGHT_OFF       0x00
#define 	CONTROL_LIGHT_ON        0x01
#define 	CONTROL_LIGHT_VAL       0x02  
#define 	CONTROL_LIGHT_REV       0x03
#define 	CONTROL_LIGHT_SIZE     2      
#define 	CONTROL_LIGHT_VAL_SIZE 3      
#define 	CONTROL_RESET   0x02
#define 	CONTROL_RESET_SIZE 1
#define 	CONTROL_MUTE    0x02
#define 	CONTROL_MUTE_SIZE 2
#define 	CONTROL_BEEP    0x04
#define 	CONTROL_BEEP_SHORT      0x00  
#define 	CONTROL_BEEP_LONG       0x01  
#define 	CONTROL_BEEP_SIZE 2
#define 	CONTROL_REMOTE_RADIO    0x05  

inline double rad2deg(int rad)
{
	double deg;
	deg = (double)rad / 174.53293F;
	if (deg < 0)
		deg += 360.0F;
	return deg;
}

inline double urad2deg(unsigned int rad)
{
	double deg;
	deg = (double)rad / 174.53293F;
	return deg;
}

inline int deg2rad(double deg)
{
        double rad;

	if (deg > 180)
		deg -= 360;

	rad = deg * 174.53293F;
	return (int)(rad + 0.5);
}

inline unsigned int udeg2rad(double deg)
{
        double rad;

	rad = deg * 174.53293F;
	return (unsigned int)(rad + 0.5);
}

#endif

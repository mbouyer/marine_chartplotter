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

#ifndef NMEA2000_FRAME_RX_H_
#define NMEA2000_FRAME_RX_H_
#include "nmea2000_frame.h"
#include "nmea2000_defs.h"
#include <array>

class nmea2000_frame_rx : public nmea2000_desc {
    public:
	inline nmea2000_frame_rx() :
	    nmea2000_desc(NULL, false, -1) { enabled = true; }

	inline nmea2000_frame_rx(const char *desc, bool isuser, int pgn) :
	    nmea2000_desc(desc, isuser, pgn) { enabled = true; }
	virtual ~nmea2000_frame_rx() {};
	virtual bool handle(const nmea2000_frame &) { return false;}
};

class nmea2000_fastframe_rx : public nmea2000_frame_rx, public nmea2000_frame {
    public:
	inline nmea2000_fastframe_rx() : nmea2000_frame_rx() { init(); }
	inline nmea2000_fastframe_rx(const char *desc, bool isuser, int pgn) : nmea2000_frame_rx(desc, isuser, pgn) { init() ; }
	virtual ~nmea2000_fastframe_rx() {};
	bool handle(const nmea2000_frame &);
	virtual bool fast_handle(const nmea2000_frame &) { return false;}
    private:
	uint8_t _userdata[233];
	int ident;
	int id;
	int len;
	inline void init()
	    {
	      data = &_userdata[0];
	      ident = -1;
	      len = 0;
	      id = 0;
	    }
};

class nmea2000_attitude_rx : public nmea2000_frame_rx {
    public:
	inline nmea2000_attitude_rx() :
	    nmea2000_frame_rx("NMEA2000 attitude", true, NMEA2000_ATTITUDE) {};
	virtual ~nmea2000_attitude_rx() {};
	bool handle(const nmea2000_frame &f);
};

class nmea2000_cogsog_rx : public nmea2000_frame_rx {
    public:
	inline nmea2000_cogsog_rx() :
	    nmea2000_frame_rx("NMEA2000 COG/SOG", true, NMEA2000_COGSOG) {};
	virtual ~nmea2000_cogsog_rx() {};
	bool handle(const nmea2000_frame &f);
};

class nmea2000_xte_rx : public nmea2000_frame_rx {
    public:
	inline nmea2000_xte_rx() :
	    nmea2000_frame_rx("NMEA2000 XTE", true, NMEA2000_XTE) {};
	virtual ~nmea2000_xte_rx() {};
	bool handle(const nmea2000_frame &f);
};

class nmea2000_navdata_rx : public nmea2000_fastframe_rx {
    public:
	inline nmea2000_navdata_rx() :
	    nmea2000_fastframe_rx("NMEA2000 NAVDATA", true, NMEA2000_NAVDATA) {};
	virtual ~nmea2000_navdata_rx() {};
	bool fast_handle(const nmea2000_frame &f);
};

class nmea2000_rx {
    public:
	inline nmea2000_rx() {};

	bool handle(const nmea2000_frame &);
	const nmea2000_desc *get_byindex(u_int);
	int get_bypgn(int);
	void enable(u_int, bool);

    private:
	nmea2000_attitude_rx attitude;
	nmea2000_cogsog_rx cogsog;
	nmea2000_xte_rx xte;
	nmea2000_navdata_rx navdata;

	std::array<nmea2000_frame_rx *,4> frames_rx = { {
	    &attitude,
	    &cogsog,
	    &xte,
	    &navdata,
	} };
};

#endif // NMEA2000_FRAME_RX_H_

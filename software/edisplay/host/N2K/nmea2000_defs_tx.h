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

#ifndef NMEA2000_FRAME_TX_H_
#define NMEA2000_FRAME_TX_H_
#include "nmea2000_frame.h"
#include "nmea2000_defs.h"
#include <array>
#include <time.h>
#include <assert.h>

class NMEA0183;

class nmea2000_frame_tx : public nmea2000_frame, public nmea2000_desc {
    public:
	bool  valid;

	inline nmea2000_frame_tx() :
	    nmea2000_frame(),
	    nmea2000_desc(NULL, false, -1)
	    {valid = 0; }

	inline nmea2000_frame_tx(const char *desc, bool isuser, u_int pgn, u_int pri, u_int len) : nmea2000_frame(), nmea2000_desc(desc, isuser, pgn)
	    {
		valid = 0;
		assert((len & 0xff) <= 8);
		frame->can_id = ((pri & 0x7) << 26) |
		    (pgn << 8);
		frame->can_id |= CAN_EFF_FLAG;
		frame->can_dlc = len;
	    }

	 virtual ~nmea2000_frame_tx() {};

	inline void setsrc(int src)
	    {
		frame->can_id = (frame->can_id & ~0xff) | (src & 0xff);
	    }
	inline void setdst(int dst) {
		assert(is_pdu1());
		frame->can_id =
		   (frame->can_id & ~0xff00) | ((dst & 0xff) << 8);
	}

	virtual bool send(int);
};

class nmea2000_fastframe_tx : public nmea2000_frame_tx {
public:
	inline nmea2000_fastframe_tx() : nmea2000_frame_tx(), fastlen(233) { init(); }
	inline nmea2000_fastframe_tx(const char *desc, bool isuser, u_int pgn, u_int pri, u_int len) : nmea2000_frame_tx(desc, isuser, pgn, pri, 8), fastlen(len) { init(); }
	virtual ~nmea2000_fastframe_tx();
	virtual bool send(int);
protected:
	const int fastlen;
private:
	uint8_t *userdata;
	uint8_t ident;
	inline void init()
	    { userdata = (uint8_t *)malloc(fastlen);
	      data = userdata; ; 
	      ident = 0;
	    }
};

class iso_address_claim_tx : public nmea2000_frame_tx {
    public:
	inline iso_address_claim_tx() : nmea2000_frame_tx("ISO address claim", false, ISO_ADDRESS_CLAIM, NMEA2000_PRIORITY_REQUEST, 8) {} ;

	inline void setdata(u_int uniquenumn, u_int manuf, u_int devfunc, u_int devclass, u_int devinst, u_int systinst)
	{
	    data[0] = (uniquenumn >> 0) & 0xff;
	    data[1] = (uniquenumn >> 8) & 0xff;
	    data[2] = (uniquenumn >> 16) & 0x1f;
	    data[2] |= (manuf << 5) & 0xe0;
	    data[3] = manuf >> 3;
	    data[4] = devinst;
	    data[5] = devfunc;
	    data[6] = devclass << 1;
	    data[7] = 0x80 | (NMEA2000_INDUSTRY_GROUP << 4) | systinst;
	};
};

#if 0
class n2k_attitude_tx : public nmea2000_frame_tx {
    public:
	inline n2k_attitude_tx() : nmea2000_frame_tx("NMEA2000 attitude", true, NMEA2000_ATTITUDE, NMEA2000_PRIORITY_INFO, 8) { };

	void update(double, double, double, uint8_t);
};

class n2k_rateofturn_tx : public nmea2000_frame_tx {
    public:
	inline n2k_rateofturn_tx() : nmea2000_frame_tx("NMEA2000 rate of turn", true, NMEA2000_RATEOFTURN, NMEA2000_PRIORITY_INFO, 8) { };
	virtual ~n2k_rateofturn_tx() {};
	void update(double, uint8_t);
};

class n2k_navdata_tx : public nmea2000_fastframe_tx {
    public:
	static const int navdata_size = 34;
	inline n2k_navdata_tx() : nmea2000_fastframe_tx("NMEA2000 Navigation Data", true, NMEA2000_NAVDATA, NMEA2000_PRIORITY_INFO, navdata_size) { apb_ok = false; rmb_ok = false; current_bearing = -1;}
	virtual ~n2k_navdata_tx() {};
	virtual void nmeasentence(NMEA0183 *, int, uint8_t);
	virtual void positionfix(PlugIn_Position_Fix_Ex *, int, uint8_t);
    private:
	bool apb_ok;
	bool rmb_ok;
	double current_bearing;
	double current_sog;
};

class n2k_xte_tx : public nmea2000_frame_tx {
    public:
	inline n2k_xte_tx() : nmea2000_frame_tx("NMEA2000 Cross-Track Error", true, NMEA2000_XTE, NMEA2000_PRIORITY_INFO, 6) { };
	virtual ~n2k_xte_tx() {};
	virtual void nmeasentence(NMEA0183 *, int, uint8_t);
};
#endif


class nmea2000_tx {
    public:
	nmea2000_tx();
	~nmea2000_tx();

	const nmea2000_desc *get_byindex(u_int);
	int get_bypgn(int);
	void enable(u_int, bool);

	bool send_frame(int sock, int pgn, bool force = false);
	void setsrc(int);
	nmea2000_frame_tx *get_frametx(u_int);

	iso_address_claim_tx iso_address_claim;
#if 0
	n2k_attitude_tx n2k_attitude;
	n2k_rateofturn_tx n2k_rateofturn;
	n2k_navdata_tx n2k_navdata;
	n2k_xte_tx n2k_xte;
#endif
    private:

	std::array<nmea2000_frame_tx *,1> frames_tx = { {
		&iso_address_claim,
#if 0
		&n2k_attitude,
		&n2k_rateofturn,
		&n2k_navdata,
		&n2k_xte,
#endif
	} };
	uint8_t sid;
};

#endif // NMEA2000_FRAME_TX_H_

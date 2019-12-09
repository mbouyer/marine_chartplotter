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

#include <err.h>
#include <iostream>
#include "NMEA2000.h"
#include "nmea2000_defs_tx.h"
#include "nmea2000_defs_rx.h"

bool nmea2000_rx::handle(const nmea2000_frame &n2kf)
{
	int pgn = n2kf.getpgn();

	for (u_int i = 0; i < frames_rx.size(); i++) {
		if (frames_rx[i]->pgn == pgn) {
			if (frames_rx[i]->enabled)
				return frames_rx[i]->handle(n2kf);
			return false;
		}
	}
	return false;
}

const nmea2000_desc * nmea2000_rx::get_byindex(u_int i) {
	if (i >= frames_rx.size()) {
		return NULL;
	};
	return frames_rx[i];
}

int nmea2000_rx::get_bypgn(int pgn) {
	for (u_int i = 0; i < frames_rx.size(); i++) {
		if (frames_rx[i]->pgn == pgn) {
			return i;
		}
	}
	return -1;
}

void nmea2000_rx::enable(u_int i, bool en)
{
	if (i < frames_rx.size()) {
		frames_rx[i]->enabled = en;
	}
}

bool nmea2000_fastframe_rx::handle(const nmea2000_frame &f)
{
#define FASTPACKET_IDX_MASK 0x1f
#define FASTPACKET_ID_MASK  0xe0
	unsigned char _idx = (f.frame2uint8(0) & FASTPACKET_IDX_MASK);
	unsigned char _id = (f.frame2uint8(0) & FASTPACKET_ID_MASK);

	if (_idx == 0) {
		/* new packet */
		ident = _id;
		len = f.frame2uint8(1);
		for (int i = 0; i < 6 && len > 0; i++) {
			data[i] = f.frame2uint8(i+2);
			len--;
		}
	} else if (ident == _id) {
		int i, j;
		/* i = 6 + (_idx - 1) * 7 : i = _idx * 7 - 1 */
		for (i = _idx * 7 - 1, j = 1;
		    i < sizeof(_userdata) && j < 8 && len > 0; i++, j++) {
			data[i] = f.frame2uint8(j);
			len--;
		}
	}
	if (len == 0) {
		return fast_handle(*this);
	}
	return true;
}

nmea2000_tx::nmea2000_tx()
{
	sid = 0;
};

nmea2000_tx::~nmea2000_tx()
{
}

const nmea2000_desc *nmea2000_tx::get_byindex(u_int i) {
	if (i >= frames_tx.size()) {
		return NULL;
	};
	return frames_tx[i];
}

nmea2000_frame_tx *nmea2000_tx::get_frametx(u_int i) {
        if (i >= frames_tx.size()) {  
		return NULL;
	};
	return frames_tx[i];
}

int nmea2000_tx::get_bypgn(int pgn) {
	for (u_int i = 0; i < frames_tx.size(); i++) {
		if (frames_tx[i]->pgn == pgn) {
			return i;
		}
	}
	return -1;
}

void nmea2000_tx::enable(u_int i, bool en)
{
	if (i < frames_tx.size()) {
		frames_tx[i]->enabled = en;
	}
}

bool nmea2000_tx::send_frame(int sock, int pgn, bool force) {
	for (u_int i = 0; i < frames_tx.size(); i++) {
		if (frames_tx[i]->pgn == pgn) {
			if (frames_tx[i]->enabled || force) {
				return frames_tx[i]->send(sock);
			} else {
				return false;
			}
		}
	}
	return false;
}

void nmea2000_tx::setsrc(int src) {
	for (u_int i = 0; i < frames_tx.size(); i++) {
		frames_tx[i]->setsrc(src);
	}
}

bool nmea2000_frame_tx::send(int sock) {
	if (!valid)
		return false;

	if (write(sock, frame, sizeof(struct can_frame)) < 0) {
		warn("send %s", descr);
		return false;
	}
	return true;
}

nmea2000_fastframe_tx::~nmea2000_fastframe_tx()
{
	free(userdata);
}

bool nmea2000_fastframe_tx::send(int sock)
{
	int i;
	int n;
	if (!valid)
		return false;

	for (i = 0, n = 0; i < fastlen; n++) {
		frame->data[0] = (ident << 5) | n ;
		if (n == 0) {
			frame->data[1] = fastlen;
			memcpy(&frame->data[2], &data[i], 6);
			i += 6;
			frame->can_dlc = 8;
		} else {
			int remain = fastlen - i;
			if (remain > 7)
				remain = 7;
			memcpy(&frame->data[1], &data[i], remain);
			frame->can_dlc = remain + 1;
			i += remain;
		}
		if (write(sock, frame, sizeof(struct can_frame)) < 0) {
			warn("send %s (%d)", descr, n);
			return false;
		}
	}
	ident = (ident + 1) & 0x7;
	return true;
}

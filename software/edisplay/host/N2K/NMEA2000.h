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

#ifndef NMEA2000_H_
#define NMEA2000_H_

#include <pthread.h>
#include "nmea2000_defs.h"

class nmea2000_frame;
class nmea2000_rx;
class nmea2000_tx;
class nmea2000_frame_tx;

class nmea2000 {
   public:
    nmea2000(const char *);
    ~nmea2000(void);

    void Init(void);

    inline void setcanif(const char *ifn) {canif = ifn;}
    inline const char *getcanif() {return canif;}
    int getaddress(void) { return myaddress; }
    inline void getconfig(int *un, int * di, int *mf)
	{ *un = uniquenumber; *di = deviceinstance; *mf = manufcode; }
    inline void setconfig(int un, int di, int mf)
	{ uniquenumber = un; deviceinstance = di; manufcode = mf; }
    const nmea2000_desc *get_tx_byindex(int);
    int get_tx_bypgn(int);
    nmea2000_frame_tx *get_frametx(int i);
    bool send_bypgn(int pgn, bool force = false);

    void tx_enable(int, bool);
    const nmea2000_desc *get_rx_byindex(int);
    int get_rx_bypgn(int);
    void rx_enable(int, bool);
    static void * rx_thread(void *p);

  private:
    volatile bool thread_running;
    pthread_t thread;
    int myaddress;
    const char *canif;
    int deviceinstance;
    int uniquenumber;
    int manufcode;
    nmea2000_rx *nmea2000_rxP;
    nmea2000_tx *nmea2000_txP;
    int sock;
    enum {
	UNCONF, DOINGCONF, DOCLAIM, CLAIMING, CLAIMED
    } state;
    struct timeval claim_date;
    bool configure();
    void parse_frame(const nmea2000_frame &);
    void handle_address_claim(const nmea2000_frame &);
    void handle_iso_request(const nmea2000_frame &);
    bool send_address_claim();
};

extern nmea2000 *nmea2000P;

#endif

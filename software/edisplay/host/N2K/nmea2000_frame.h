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

#ifndef NMEA2000_FRAME_H_
#define NMEA2000_FRAME_H_

#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#ifdef __NetBSD__
#include <netcan/can.h>
#else
#include <linux/can.h>
#include <linux/can/raw.h>
#endif


class nmea2000_frame {
    public:
	inline nmea2000_frame() {init();}
	inline nmea2000_frame(struct can_frame *f)
	    { frame  = f; data = f->data;}
	virtual ~nmea2000_frame() {};
	inline bool is_pdu1() const
	    { return (((frame->can_id >> 16) & 0xff) < 240); };
	    
	inline int getsrc() const { return (frame->can_id & 0xff); };
	inline int getdst() const
	    { return (is_pdu1() ? (frame->can_id >> 8) & 0xff : -1); };
	inline int getpgn() const
	    { return (is_pdu1() ? (frame->can_id >> 8) & 0x1ff00 : (frame->can_id >> 8) & 0x1ffff); }

	inline int getpri() const { return ((frame->can_id >> 26) & 0x7); };
	inline int getlen() const { return (frame->can_dlc); };
	inline const unsigned char *getdata() const {return (data); };
	inline ssize_t readframe(int s) {
	    return read(s, frame, sizeof(struct can_frame));
	}

	inline int8_t frame2int8(int i) const
	    {return (data[i]);}
	inline int16_t frame2int16(int i) const
	    {return ((int16_t)data[i+1] << 8) | data[i];}
	inline int32_t frame2int32(int i) const
	    {return ((int32_t)data[i+3] << 24) |
	            ((int32_t)data[i+2] << 16) |
	            ((int32_t)data[i+1] << 8) |
		    data[i];
	     }
	inline uint8_t frame2uint8(int i) const
	    {return (data[i]);}
	inline uint16_t frame2uint16(int i) const
	    {return ((uint16_t)data[i+1] << 8) | data[i];}
	inline uint32_t frame2uint24(int i) const
	    {return ((uint32_t)data[i+2] << 16) |
	            ((uint32_t)data[i+1] << 8) |
		    data[i];
	    }
	inline uint32_t frame2uint32(int i) const
	    {return ((uint32_t)data[i+3] << 24) |
	            ((uint32_t)data[i+2] << 16) |
	            ((uint32_t)data[i+1] << 8) |
		    data[i];
	    }
	inline void int82frame(int8_t v, int i)
	    {data[i] = v;}
	inline void int162frame(int16_t v, int i)
	    {
		uint16_t *uv = (uint16_t *)(void *)&v;
		uint162frame(*uv, i);
	    }
	inline void int242frame(int32_t v, int i)
	    {
		uint32_t *uv = (uint32_t *)(void *)&v;
		uint242frame(*uv, i);
	    }
	inline void int322frame(int32_t v, int i)
	    {
		uint32_t *uv = (uint32_t *)(void *)&v;
		uint322frame(*uv, i);
	    }
	inline void uint82frame(u_int8_t v, int i)
	    {data[i] = v;}
	inline void uint162frame(u_int16_t v, int i)
	    {data[i+1] = (v >> 8) & 0xff; data[i] = v & 0xff;}
	inline void uint242frame(u_int32_t v, int i)
	    {
		data[i+2] = (v >> 16) & 0xff;
		data[i+1] = (v >>  8) & 0xff;
		data[i] = v & 0xff;
	    }
	inline void uint322frame(u_int32_t v, int i)
	    {
		data[i+3] = (v >> 24) & 0xff;
		data[i+2] = (v >> 16) & 0xff;
		data[i+1] = (v >>  8) & 0xff;
		data[i] = v & 0xff;
	    }
	    
    protected:
	struct can_frame *frame;
	uint8_t *data;
    private:
	struct can_frame _frame;
	inline void init() 
	    { frame = &_frame;
	      memset(frame, 0, sizeof(struct can_frame));
	      data = frame->data;
	    }
};

#endif

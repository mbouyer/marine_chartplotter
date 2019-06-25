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

#include <pic18fregs.h>
#include <stdint.h>
#include <stdbool.h>
#include "lcd.h"
#include "spi.h"


void
initLCD(void)
{
	int i;
	RST=0; /* reset display */
	for (i = 0; i < 1000; i++)
		; /* wait */
	RST=1;
	CD = COMMAND;   

	sendSPI(0xf1);  /* reset */
	sendSPI(0x67); /* display start line 0 */

	sendSPI(0xC0);  /* COM Direction normal */
	sendSPI(0xA4);  /* Set all Pixel on */
	sendSPI(0x40);  
	sendSPI(0x50);  

	sendSPI(0xB2);  /* Set Bias 1/9 (Duty 1/65) */
	sendSPI(0xEB);  /* Booster, Regulator and Follower on */
	sendSPI(0x81);  /* Set Contrast */
	sendSPI(0x5F);  /* .. */


	sendSPI(0xFA);  /* Temperature compensation */
	sendSPI(0xC6);  /* .. */
	sendSPI(0xAF);  /* Display on */

	cleardisplay();
}

void
cleardisplay()
{
	unsigned char page;
	/* initialize display ram */
	for (page = 0; page < DISPLAY_H; page++) {
		clearline(page);
	}
}

static void
clearline(unsigned char line) __wparam
{
	unsigned char col;
	
	CD = COMMAND;
	sendSPI(PAGE_ADDR(line));
	sendSPI(COL_ADDR_H(0));
	sendSPI(COL_ADDR_L(0));
	CD = DATA;
	for (col = 0; col < DISPLAY_W; col++) {
		sendSPI(0x00);
	}
}

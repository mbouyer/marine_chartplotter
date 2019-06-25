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


#ifndef LCD_H
#define	LCD_H

#include <stdint.h>
#include <stdbool.h>



#define SI              RPOR1    // SDO2 (MSSP2)
#define CLK             RPOR0    // SCK2 (MSSP2)

#define CD              LATAbits.LATA6    
#define CS0             LATAbits.LATA7    // Chip select 
#define RST             LATAbits.LATA3    // Reset


#define COMMAND         0
#define DATA            1

#define _XTAL_FREQ  4000000
#define PAGE_ADDR(c) ((unsigned char)0x60 | c)
#define COL_ADDR_H(c) ((unsigned char)0x10 | (c >> 4))
#define COL_ADDR_L(c) ((unsigned char)0x00 | (c & 0x0f))
#define DISPLAY_W	160
#define DISPLAY_H	32


void initLCD(void);
 void cleardisplay(void);
static void clearline(unsigned char) __wparam;


#endif	/* LCD_H */

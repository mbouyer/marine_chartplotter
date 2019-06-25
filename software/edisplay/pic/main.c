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
#include <stdio.h> 
#include "usb.h"
#include "lcd.h"
#include "spi.h"


unsigned char sending;

extern char stack; 
extern char stack_end;
byte i,rr,rb;
#pragma stack 0x100 256

static char counter_1hz;
static volatile short counter_10hz;
#define TIMER2_10HZ 4
static volatile unsigned char softintrs;
#define INT_10HZ         (unsigned char)0x01
#define INT_NEEDAD       (unsigned char)0x02
#define INT_XMIT         (unsigned char)0x04
#define INT_ADDONE       (unsigned char)0x08
#define INT_DOAD         (unsigned char)0x10
 
#define TIMER0_5MS 192 /* 48 without PLL */

void _reset (void) __naked __interrupt 0;
void _startup (void) __naked;
static void usart_putchar(unsigned char arg) __wparam;
static void InitializeSystem(void);
void USBTasks(void);
char *txbuf_prod; /* producer index in txbuf */
char *txbuf_cons; /* consumer index in txbuf */

#define UART_BUFSIZE 256
#define UART_BUFSIZE_MASK 0xff


static char ad_channel;
static long ad_i_result;
unsigned int ad_v_result;
unsigned int counter=0;

#define ENCODER_A PORTBbits.RB4        /* Encoder A Pin */
#define ENCODER_B PORTBbits.RB5        /* Encoder B Pin */

unsigned char    encoder_A;
unsigned char    encoder_B;
unsigned char    encoder_A_prev=0;

char txBuffer[VENDOR_INPUT_REPORT_BYTES];
char rxBuffer[VENDOR_OUTPUT_REPORT_BYTES];
char rxBuffer2[VENDOR_OUTPUT_REPORT_BYTES];

static void
Pressed() 
{
	T2CONbits.TMR2ON = 0;

	if(ad_v_result < 1700 && ad_v_result > 1300)
		VENDORTxReport("RED BUTTON PRESSED", 19);
	else if(ad_v_result < 1300 && ad_v_result > 1000)
		VENDORTxReport("LEFT BLACK BUTTON PRESSED", 25);
	else if(ad_v_result < 1000 && ad_v_result > 700)
		VENDORTxReport("RIGHT BLACK BUTTON PRESSED", 26);
	else if(ad_v_result < 700 && ad_v_result > 400)
		VENDORTxReport("YELLOW BUTTON PRESSED", 22);
		
	T2CONbits.TMR2ON = 1;
	for(rr=0;rr<VENDOR_OUTPUT_REPORT_BYTES;rr++)
		rxBuffer[rr]='\n';

}


char uart_txbuf[UART_BUFSIZE];
unsigned char uart_txbuf_prod, uart_txbuf_cons;

void
__stream_usart_putchar(unsigned char c) __wparam __naked
{
	c;
	__asm
	call _usart_putchar;
	return;
	__endasm;
}

static void
usart_putchar(unsigned char c) __wparam
{
	unsigned char new_uart_txbuf_prod;

again:
	new_uart_txbuf_prod = (uart_txbuf_prod + 1) & UART_BUFSIZE_MASK;
	while (new_uart_txbuf_prod == uart_txbuf_cons) {
		PIE3bits.TX2IE = 1; /* ensure we'll make progress */
	}
	uart_txbuf[uart_txbuf_prod] = c;
	uart_txbuf_prod = new_uart_txbuf_prod;
 	PIE3bits.TX2IE = 1; /* start transmit  */

	if (c == '\n') {
		c = '\r';
		goto again;
	}
}

PUTCHAR(c) /* Macro */
{
	usart_putchar(c);
}

char
getchar(void)
{
	char c;
	while (!PIR3bits.RC2IF); /* wait for a char */
	c = RCREG2;
	if (RCSTA2bits.OERR) {
		RCSTA2bits.CREN = 0;
		RCSTA2bits.CREN = 1;
	}
	return c;
}

void
flushtx(void)
{
	while (PIE3bits.TX2IE)
		;
}


static void
process_char(char c) __wparam
{
	if (c == 'r') {
		printf("reset\r\n"); flushtx();
		__asm__("reset");
	} 
}

void main(void)
{
	static byte cdc_trf_state_l;
	softintrs = 0;

	counter_10hz = TIMER2_10HZ;
	counter_1hz = 10;

	stdout = STREAM_USART; /* Use the macro PUTCHAR with printf */

	INTCON2bits.RBPU = 1;
	ANCON0 = 0xf0; /* an0-an3 analog, an4-an7 digital */
	ANCON1 = 0x3f; /* an8-12 digital */

	TRISBbits.TRISB3 = 0;
	PORTBbits.RB3 = 0;

	/* switch PLL on */
	OSCTUNEbits.PLLEN = 1;

	/* configure sleep mode: PRI_IDLE */
	OSCCONbits.SCS = 0;
	OSCCONbits.IDLEN = 1;

	/* everything is low priority by default */
	IPR1 = 0;
	IPR2 = 0;
	IPR3 = 0;
	IPR4 = 0;
	IPR5 = 0;
	INTCON = 0;
	INTCON2 &= 0xf8;
	INTCON3 = 0;

	INTCONbits.GIE=0;
    	EECON2 = 0x55;
   	EECON2 = 0xAA;
    	EECON2 = 0x00; /* unlock PPS */

    	RPOR0= 11;	/* map spi clock to PR0 PIN */
   	RPOR1= 10;	/* map spi function to PR1 PIN */

   	EECON2 = 0x55;
   	EECON2 = 0xAA;
   	EECON2 = 0x01; /* lock PPS */

	INTCONbits.GIE=1;

	PIR2bits.BCL1IF=0;
        TRISAbits.TRISA7= 0;
	TRISAbits.TRISA0= 0;
	TRISAbits.TRISA1= 0;
	LATAbits.LATA7 = 0; /* chip select CS1B */

	TRISAbits.TRISA6 = 0;
        TRISAbits.TRISA3 = 0;

	RST = 1;
	initSPI();    /* intilize spi connexion */
        initLCD();    /* intilize LCD  */

	RCONbits.IPEN=1; /* enable interrupt priority */

	/* configure timer0 as free-running counter at 46.875Khz */     
	T0CON = 0x07; /* b00000111: internal clock, 1/256 prescaler */  
	INTCONbits.TMR0IF = 0;
	INTCONbits.TMR0IE = 0; /* no interrupt */
	T0CONbits.TMR0ON = 1;

#if TIMER2_10HZ == 4
	/* configure timer2 for 1Khz interrupt */
	T2CON = 0x22; /* b00100010: postscaller 1/5, prescaler 1/16 */  
	PR2 = 150; /* 1khz output */  
#elif TIMER2_10HZ == 1000
	/* configure timer2 for 10Khz interrupt */
	T2CON = 0x22; /* b00100010: postscaller 1/5, prescaler 1/16 */  
	PR2 = 15; /* 10khz output */  
#elif TIMER2_10HZ == 2000
	/* configure timer2 for 20Khz interrupt */
	T2CON = 0x21; /* b00100001: postscaller 1/5, prescaler 1/4 */  
	PR2 = 29; /* 20khz output */  
#else
#error "unknown TIMER2_10HZ"
#endif
	T2CONbits.TMR2ON = 1;
	PIR1bits.TMR2IF = 0;
	IPR1bits.TMR2IP = 1; /* high priority interrupt */
	PIE1bits.TMR2IE = 1;

	/* configure UART for 57600Bps at 48Mhz */
	SPBRG2 = 12;

	/* init TX buffer pointers */
	uart_txbuf_cons = uart_txbuf_prod = 0;

	INTCONbits.GIE_GIEH=1;  /* enable high-priority interrupts */   
	INTCONbits.PEIE_GIEL=1; /* enable low-priority interrrupts */   
	TRISBbits.TRISB4 = 1;
	TRISBbits.TRISB5 = 1;
	TRISA = 0x04; 		/*set all digital I/O to inputs */
	TRISAbits.TRISA2 = 1; /*Disable the output driver for pin RA2/AN2  */

	ADCON0bits.VCFG = 0; /*Vdd is the +ve reference  */
	ADCON1bits.ADCS = 0b001; /*Fosc/8 is the conversion clock */
	ADCON0bits.CHS = 2; /*select analog input, AN2  */
	ADCON0bits.ADON = 1; /*Turn on the ADC */

#if 0
	ADCON0 = 0xc1; /* b11000001 */

	/* clk = fosc/64, tacq = 4tad (5.33us) */
	ADCON1 = 0x96; /* b10010110 */

	/* clk = fosc/64, tacq = 0tad */
	ADCON1 = 0x86; /* b10000110 */

	/* ANCON already set up */    

	/* start calibration */       
	ADCON1bits.ADCAL = 1;
	ADCON0bits.GO_NOT_DONE =1;    
	while (ADCON0bits.GO_NOT_DONE)
	  ; /* wait */
	ADCON1bits.ADCAL = 0;
#endif
	/* enable watch dog timer */  
	WDTCON = 0x01;

#include "version.h"
	fprintf(STREAM_USER, "\nthermoUSB v%x.%x", REV_MAJOR, REV_MINOR);
	fprintf(STREAM_USER, BUILD);  
#undef BUILD
	flushtx();
	PORTBbits.RB3 = 0;

	/* initialize USB */
	UCFG = _UPUEN| _FSEN | 0 /* MODE_PP */;
	deviceState = DETACHED;
	remoteWakeup = 0x00;
	currentConfiguration = 0x00;

	PORTBbits.RB3 = 0;
	
	while(1) {
		counter++;
		__asm__("clrwdt");
		
		while (PIR3bits.RC2IF) { /* a char is ready on serial */
			process_char(RCREG2);
			if (RCSTA2bits.OERR) {
			    RCSTA2bits.CREN = 0;
			    RCSTA2bits.CREN = 1;
			}
		}
		ADRESL=0;
		ADRESH=0;
		ADCON0bits.GO = 1; /*start the conversion */

		while(ADCON0bits.GO==1){}; /*wait for the conversion to end  */
		ad_v_result = (ADRESH*8)+ADRESL; /*combine the 10 bits of the conversion */

		EnableUSBModule();  /* enable usb  */
		/* As long as we aren't in test mode (UTEYE), process */
	       /* USB transactions. */
		if(UCFGbits.UTEYE != 1)
			ProcessUSBTransactions();
		/* Application specific tasks      */

		if ((deviceState >= CONFIGURED) && (UCONbits.SUSPND==0))
		{
			if(counter>2200){  /* buttons rebond  */
				Pressed(); /* detect which button has been pressed  */
				counter=0;
			}	

			rb = VENDORRxBulk(rxBuffer, VENDOR_OUTPUT_REPORT_BYTES);    /* read data from bulk */
			if(rb != 0) {
				T2CONbits.TMR2ON = 0;  /* disable TMR 2 to avoid interruption while sending on spi  */
				CD=DATA;     /* data will be sent  */
#if 0
				for(sending=0;sending<rb;sending++){

					sendSPI((uint8_t)rxBuffer[sending]); /* send data to LCD 				 */
				}
#else
				DMACON1 = 0x24; /* auto-inc, half duplex TX */
				DMACON2 = 0x60; /* 64 Tcy inter-byte delay */
				DMABCH = 0;
				DMABCL = (rb - 1);
				TXADDRH = ((int)rxBuffer >> 8);
				TXADDRL = ((int)rxBuffer & 0xff);
				CS0 = 0;
				DMACON1bits.DMAEN = 1;
				while (DMACON1bits.DMAEN)
					; /* wait */
				CS0 = 1;
#endif
				T2CONbits.TMR2ON = 1; /* enable TMR2 */
			}
		
			rr = VENDORRxReport(rxBuffer2, VENDOR_OUTPUT_REPORT_BYTES);    /* read data from bulk */
			if(rr != 0) {
				if(rxBuffer2[0]==0){    /* PIC command  */
					if(rxBuffer2[1]==1){
					  LATBbits.LATB3 = 1;
					}
					if(rxBuffer2[1]==0){
						  LATBbits.LATB3 = 0;
					}
				} else {
					CD=COMMAND;     /* LCD command */
					for(sending=1;sending<rr;sending++){
						sendSPI((uint8_t)rxBuffer2[sending]); /* send command to LCD 				 */
					}
				}

				/* clean up rxbuffer */
				for(rr=0;rr<VENDOR_OUTPUT_REPORT_BYTES;rr++){
					rxBuffer[rr]=' ';
					rxBuffer2[rr]=' ';
					txBuffer[rr]=' ';
				}
			}
		}
		/*__asm__("sleep"); */
	}
}


/** V E C T O R  R E M A P P I N G *******************************************/

/*extern void _startup (void);        // See c018i.c in your C18 compiler dir */

void _reset (void) __naked __interrupt 0
{
	__asm__("goto __startup");
}

void _startup (void) __naked
{
	__asm
	/* Initialize the stack pointer */
	lfsr 1, _stack_end
	lfsr 2, _stack_end
	clrf _TBLPTRU, 0    /* 1st silicon doesn't do this on POR */

	/*
	 * initialize the flash memory access configuration. this is harmless
	 * for non-flash devices, so we do it on all parts.
	 */
	bsf _EECON1, 7, 0
	bcf _EECON1, 6, 0
	__endasm ;

	/* Call the user's main routine */
	main();

	__asm__("reset");
}

/*
 * high priority interrupt. Split in 2 parts; one for the entry point
 * where we'll deal with timer0, then jump to another address
 * as we don't have enough space before the low priority vector
 */
void _irqh (void) __naked __shadowregs __interrupt 1
{
	__asm
	bcf   _PIR1, 1
	goto _irqh_timer2
	__endasm ;

}

void irqh_timer2(void) __naked
{
	/*
	 * no sdcc registers are automatically saved,
	 * so we have to be carefull with C code !
	 */
	counter_10hz--;
	if (counter_10hz == 0) {
		counter_10hz = TIMER2_10HZ;
		softintrs |= INT_10HZ;
		encoder_A = ENCODER_A;
		encoder_B = ENCODER_B;

		if((!encoder_A) && (encoder_A_prev)) {
			/* A has gone from high to low */
			if(encoder_B) {
				VENDORTxReport("ROTARY CLOCK ", 13);
			}   
			else {  /* A has gone from low to high */
				VENDORTxReport("ROTARY anti-CLOCK ", 18);
			}   
		}   
		encoder_A_prev = encoder_A;     /* Store value for next time */
	}
	__asm
	retfie 1
	nop
	__endasm;
}

void _irq (void) __interrupt 2 /* low priority */
{
	if (PIE1bits.ADIE && PIR1bits.ADIF) {
		PIE1bits.ADIE = 0;
		/* process A/D in main loop */
	}
	if (PIE3bits.TX2IE && PIR3bits.TX2IF) {
		if (uart_txbuf_prod == uart_txbuf_cons) {
			PIE3bits.TX2IE = 0; /* buffer empty */
		} else {
			/* Place char in TXREG - this starts transmition */
			TXREG2 = uart_txbuf[uart_txbuf_cons]; 
			uart_txbuf_cons = (uart_txbuf_cons + 1) & UART_BUFSIZE_MASK;
		}
	}
}

;
; Copyright (c) 2016 Manuel Bouyer.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions
; are met:
; 1. Redistributions of source code must retain the above copyright
;    notice, this list of conditions and the following disclaimer.
; 2. Redistributions in binary form must reproduce the above copyright
;    notice, this list of conditions and the following disclaimer in the
;    documentation and/or other materials provided with the distribution.
;
; THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
; IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
; OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
; IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
; INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
; NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
; DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
; THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
; THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;
;

LIST   P=PIC18F25K22
#include <p18f25k22.inc>

	CBLOCK 0x00C
	softint : 1
	status : 1
	needclock : 1
	cmpt1: 1
	cmpt2: 1
	tod0 : 1 ; TOD counter
	tod1 : 1
	tod2 : 1
	tod3 : 1
	tod0_sn : 1 ; snapshot of TOD counter
	tod1_sn : 1
	tod2_sn : 1
	tod3_sn : 1
	toddiv : 1
	portb_cache : 1
	portb_prev : 1
	ledr : 1
	ledg : 1
	pwofftmr : 1
	i2caddr : 1
	i2creg : 1
	i2cstart : 1
; BMC registers available from I2C
	bmc_status : 1
	bmc_templ : 1
	bmc_temph : 1
	bmc_12vl : 1
	bmc_12vh : 1
	bmc_command : 1
	bmc_bklpwm : 1
	ENDC
BMC_VERSION	EQU	0x11
BMC_STATUS_PWSW	EQU	0
BMC_STATUS_BKLON EQU	1
BMC_STATUS_INTR EQU	2

BMC_CMD_PWROFF	EQU	0

SOFTINT_32K	EQU	0
SOFTINT_AD	EQU	1
SOFTINT_KEY	EQU	2
SOFTINT_LED	EQU	3
SOFTINT_SSP	EQU	4

STATUS_12VON 	EQU	0
STATUS_PWRON 	EQU	1
STATUS_PWRKEY	EQU	2

CLOCK_TMR4	EQU	0
CLOCK_TMR3	EQU	1
CLOCK_TMR2	EQU	2
CLOCK_UART1	EQU	7

MTXIF   EQU     TX1IF
MRCIF   EQU     RC1IF
MPIR    EQU     PIR1
MTXREG  EQU     TXREG1
MRCREG  EQU     RCREG1
MRCSTA  EQU     RCSTA1

C_A20_INT	EQU	2
C_A20_EN	EQU	5
A_12V_SENSE	EQU	0
A_CTN_SENSE	EQU	2
A_CTN_PWR	EQU	3
A_36VG_EN	EQU	4
B_LEDG		EQU	0
B_LEDR		EQU	1
B_PWM_EN	EQU	3
B_SW_PWR	EQU	4
B_LCD_BKL	EQU	5

ADC_12V_MIN	EQU	0x13e	; about 7V

IIC_DS1672	EQU	0xd0 ; DS1672 at 0x68
IIC_BMC		EQU	0xd2 ; BMC at 0x69


	ORG 0x0200
	bra start
	ORG 0x0208
	bra intr_hi
	ORG 0x0218
	; low priority interrupts
	btfss INTCON, RBIF
	bra irq_timer4
	movf PORTB, w
	clrf TMR4 ;  reset timer4
	bcf PIR5, TMR4IF
	bsf T4CON, TMR4ON ; start timer4
	bsf needclock, CLOCK_TMR4
	bcf INTCON, RBIF
irq_timer4:
	btfss PIR5, TMR4IF
	bra irq_ad
	; timer4 expired: stop timer4 and record portb status
	bcf T4CON, TMR4ON ; stop timer4
	bcf PIR5, TMR4IF
	movff PORTB, portb_cache
	bcf needclock, CLOCK_TMR4
	bsf softint, SOFTINT_KEY
irq_ad:
	btfss PIR1, ADIF
	bra irq_timer3
	bcf PIR1, ADIF
	bsf softint, SOFTINT_AD
irq_timer3:
	btfss PIR2, TMR3IF
	bra irq_ssp1
	bcf   PIR2, TMR3IF
	bsf softint, SOFTINT_LED
irq_ssp1:
	btfss PIR1, SSP1IF
	retfie 0
	bcf PIR1, SSP1IF
	bsf softint, SOFTINT_SSP
	retfie 0

intr_hi:
	btfss PIR1, TMR1IF
	retfie 1 ; no interrupt
	bcf PIR1, TMR1IF ; clear interrupt
IFNDEF USE_SOSC
	decfsz toddiv, f ; XXX tmp divizor
	retfie 1
	movlw d'122'
	movwf toddiv
ELSE
	bsf	TMR1H, 7
ENDIF
	bsf softint, SOFTINT_32K
	incfsz tod0, f
	retfie 1
	movlw 0
	addwfc tod1, f
	addwfc tod2, f
	addwfc tod3, f
	retfie 1

hsrt DA "marinegps bmc V0\0";
crlfstr DA "\r\n\0";

start:
	clrf INTCON
	clrf softint
	clrf status
	clrf needclock
	clrf bmc_status
	clrf bmc_command
	setf	bmc_bklpwm
; setup port early, so that outputs are not left floating for too long
;	setup portA: 2 analog inputs, one output
	clrf LATA
	movlw b'00000101'
	banksel ANSELA
	movwf ANSELA, b ; porta 0 and 2 analog input
	bcf TRISA, A_36VG_EN
	bcf TRISA, A_CTN_PWR

;	setup portC: 2 outputs
	banksel ANSELC
	clrf ANSELC, b
	clrf LATC
	bcf TRISC, C_A20_INT
	bcf TRISC, C_A20_EN

;	setup portB: LED outputs, and pullup on power switch
	clrf LATB
	movlw b'11111100'
	movwf TRISB ; portb0-1 output
	movlw 0x10 ; pullup on portb5
	movwf WPUB
	bcf INTCON2, RBPU

; disable modules that we don't need
	movlw	b'10110110'; uart1, timer1, timer4 on
	banksel PMD0
	movwf PMD0, b

	movlw	b'11011111'; everything off
	; banksel PMD1 same bank as PMD0
	movwf PMD1, b

	movlw	b'00001110'; A/D on
	; banksel PMD2 same bank as PMD0
	movwf PMD2, b

	movlw LOW hsrt
	call txstrcrlf

	bsf RCON, IPEN ; enable interrupts priority
	; setup timer1
	clrf T1GCON
	clrf TMR1L
IFNDEF USE_SOSC
	clrf TMR1H
	movlw d'122'
	movwf toddiv
ELSE
	movlw 0x80
	movwf TMR1H
ENDIF
	bsf IPR1, TMR1IP ; timer 1 high priority
	bcf PIR1, TMR1IF ; clear timer1 irq
IFNDEF	USE_SOSC
	movlw b'00000101'
ELSE
	movlw b'10001101'
ENDIF
	movwf T1CON ; start timer1
	clrf tod0
	clrf tod1
	clrf tod2
	clrf tod3

	; setup timer4, used for key debounce
	movlw b'01111011'
	movwf T4CON ; 1:16 prescaler and postscaler, timer off
	setf PR4 ; period 255
	bcf PIR5, TMR4IF ; clear timer irq

	; setup intrerrupt on change for PORTB/4
	movlw b'00010000'
	movwf IOCB
	movf PORTB, w ; read to init interrupt on change
	movwf portb_cache
	movwf portb_prev
	bcf INTCON, RBIF ; 

	; setup A/D 
	movlw b'00001000'
	movwf ADCON1 ; ref+ to FVR, ref- to GND
	movlw b'10111111'
	movwf ADCON2 ; right justified, 20 Tad wait, Frc clock
	bcf PIR1, ADIF

	; enable interrupts
	bsf PIE1, TMR1IE ; enable timer1 irq
	bcf IPR5, TMR4IP ; timer4 low priority
	bsf PIE5, TMR4IE ; enable timer4 irq
	bcf IPR1, ADIP ; A/D low priority
	bsf PIE1, ADIE   ; enable A/D irq
	bcf INTCON2, RBIP ; irq on change low priority
	bsf INTCON, RBIE ; enable portb irq-on-change
	bsf INTCON, GIEH ; enable high-priority irq
	bsf INTCON, GIEL ; enable low-priority irq

	bsf needclock, CLOCK_UART1; XXX

	movlw	b'00000101'
	movwf	ledr
	movlw	b'01010000'
	movwf	ledg
	call	start_led

mainloop: 
	bcf OSCCON, IDLEN ; can clock be off while sleeping ?
	tstfsz needclock
	bsf OSCCON, IDLEN ; needclock not 0, need to keep clock running
	tstfsz	softint
	bra mainloop_work
	sleep
	nop
mainloop_work:
	btfsc softint, SOFTINT_32K
	rcall do_softint32k
	btfsc softint, SOFTINT_AD
	rcall do_softintad
	btfsc softint, SOFTINT_KEY
	rcall do_softintkey
	btfsc softint, SOFTINT_LED
	rcall do_softintled
	btfsc softint, SOFTINT_SSP
	rcall do_softintssp
	btfss PORTB, B_LCD_BKL
	bra mainloop_bkoff ; backlight off
	btfss	status, STATUS_PWRON
	bra mainloop_bkoff ; A20 off, backlight should be off
	; backlight is on
	btfsc	bmc_status, BMC_STATUS_BKLON
	bra	mainloop	; was already on
	bsf	bmc_status, BMC_STATUS_BKLON
	; configure timer2
	bcf	PMD0, TMR2MD
	movlw	b'00000010'; 1:16 prescaler
	movwf	T2CON;
	clrf	TMR2
	setf	PR2; period=255 (976Hz)
	bcf	PIR1, TMR2IF
	bsf	T2CON, TMR2ON; timer2 on
	bsf	needclock, CLOCK_TMR2
	bcf	PMD1, CCP2MD
	banksel	CCPTMRS0
	clrf	CCPTMRS0, b
	clrf	ECCP2AS
	movlw	b'00111100'
	movwf	CCP2CON
	movff	bmc_bklpwm, CCPR2L
	bcf	TRISB, B_PWM_EN
	bra	mainloop


mainloop_bkoff:
	;backlight is off
	btfsc	bmc_status, BMC_STATUS_BKLON
	rcall	do_pwm_off
	bra	mainloop

do_pwm_off
	; turn off PWM and timer2
	bsf	TRISB, B_PWM_EN
	bcf	bmc_status, BMC_STATUS_BKLON
	clrf	CCP2CON
	clrf	T2CON
	bsf	PMD0, TMR2MD
	bsf	PMD1, CCP2MD
	bcf	needclock, CLOCK_TMR2
	return

do_softint32k:
	bcf softint, SOFTINT_32K
	btfss MPIR, MRCIF; did we receive a char ?
	bra do_softint32k_1
	movf MRCREG, w
	sublw 'r' ; is the received char a 'r' ?
	bnz do_softint32k_1
	reset
do_softint32k_1:
	btfsc	bmc_command, BMC_CMD_PWROFF
	bra	do_softint32k_dopwroff
	movf	pwofftmr, w
	bz	do_softint32k_ad; power off timer not active
	decfsz	pwofftmr, f
	bra	do_softint32k_ad; power off timer not expired
	; key timer expired
do_softint32k_dopwroff
	movlw	b'00001111'
	movwf	ledr
	movlw	b'11000000'
	movwf	ledg
	call	start_led ; one red blink followed by a green
	call	a20_off;
do_softint32k_ad:
	movlw b'01111100'
	andwf ADCON0, w ; test selected channel
	bnz ad_startan0
	;btfss status, STATUS_12VON
	;bra ad_startan0 ; no 12V, no need to check temp
	; we were doing an0, start an2
	bsf LATA, A_CTN_PWR ; power up CTN
	movlw b'00001001' ; select channel 2, ad on
	movwf ADCON0
	movlw b'00000100'
	movwf ADCON1 ; ref+ to A_CTN_PWR, ref- to GND
	bra ad_start

ad_startan0:
	; we were doing an2, start an0
	movlw b'10100000'
	banksel VREFCON0
	movwf VREFCON0, b ; FVR on, 2.048V
	movlw b'00000001' ; select channel 0, ad on
	movwf ADCON0
	movlw b'00001000'
	movwf ADCON1 ; ref+ to FVR, ref- to GND
wait_vfr:
	btfss VREFCON0, FVRST
	bra wait_vfr
	bcf PIR1, ADIF
ad_start
	bsf ADCON0, GO
	bsf PIE1, ADIE
	return;

do_softintad
	bcf softint, SOFTINT_AD
	movlw b'01111100'
	andwf ADCON0, w ; test selected channel
	bz ad_donean0
	; an2 done
	bcf LATA, A_CTN_PWR ; power down CTN
	movff ADRESH, bmc_temph
	movff ADRESL, bmc_templ
	bcf ADCON0, ADON
	movlw '2'
	bra ad_print

ad_donean0:
	; an0 done
	banksel VREFCON0
	clrf VREFCON0, b ; FVR off
	movff ADRESH, bmc_12vh
	movff ADRESL, bmc_12vl
	bcf ADCON0, ADON
	movlw	LOW ADC_12V_MIN
	subwf	bmc_12vl, w
	movlw	HIGH ADC_12V_MIN
	subwfb	bmc_12vh, w
	btfsc	STATUS, C
	bra	ad0_12von
	btfss	status, STATUS_12VON
	bra	ad0_12voff1
	; batt below ADC_12V_MIN
	; turn off power converters
	bcf	LATA, A_36VG_EN
	call	a20_off
	setf	ledr ; one long red blink
	clrf	ledg
	call	start_led
	bcf	status, STATUS_12VON
ad0_12voff1
	movlw	 '0'
	bra	ad_print
ad0_12von
	; batt above ADC_12V_MIN
	btfsc	status, STATUS_12VON
	bra	ad0_12von1
	bsf	LATA, A_36VG_EN ; turn on 3v6 power
	setf	ledg ; one long green blink
	clrf	ledr
	call	start_led
	bsf	status, STATUS_12VON
ad0_12von1
	movlw	 '1'
ad_print
	call dotx
	movlw ' '
	call dotx
	movf ADRESH, w
	call printhex
	swapf ADRESL, w
	call printhex
	movf ADRESL, w
	call printhex
	movlw ' '
	call dotx
	swapf SSP1STAT, w
	call printhex
	movf SSP1STAT, w
	call printhex
	movlw ' '
	call dotx
	swapf SSP1CON1, w
	call printhex
	movf SSP1CON1, w
	call printhex
	call crlf
	return

do_softintled
	bcf	softint, SOFTINT_LED
	tstfsz	ledr
	bra	do_led_on
	bcf	LATB, B_LEDR
	tstfsz	ledg
	bra	do_led_on
	; led off, stop timer3
	bcf	LATB, B_LEDG
	bcf	T3CON, TMR3ON
	bsf	PMD0, TMR3MD
	bcf	needclock, CLOCK_TMR3
	bcf	PIE2, TMR3IE ; disable timer3 irq
	return

start_led
	; enable and start timer3
	bcf	PMD0, TMR3MD
	bcf	PIR2, TMR3IF
	bcf	IPR2, TMR3IP ; timer3 low priority
	bsf	PIE2, TMR3IE ; enable timer3 irq
	movlw	b'00110000'
	movwf	T3CON ; Fosc/4, 1:8 prescale
	clrf	T3GCON
	clrf	TMR3H
	clrf	TMR3L
	bsf	T3CON, TMR3ON ; start timer3
	bsf	needclock, CLOCK_TMR3
do_led_on
	btfsc	ledr, 0
	bsf	LATB, B_LEDR
	btfss	ledr, 0
	bcf	LATB, B_LEDR
	bcf	ledr, 0
	rrncf	ledr, f
	btfsc	ledg, 0
	bsf	LATB, B_LEDG
	btfss	ledg, 0
	bcf	LATB, B_LEDG
	bcf	ledg, 0
	rrncf	ledg, f
	return

do_softintkey
	bcf softint, SOFTINT_KEY
	movf portb_cache, w
	andlw b'00010000'
	cpfseq portb_prev
	bra key_did_change
	return ; key did not change
key_did_change
	movwf portb_prev
	btfsc portb_cache, B_SW_PWR
	bra keyup
	; key pressed
	movlw 'd'
	call dotx
	btfss	status, STATUS_PWRON
	bra	keyd_doon
	bsf	status, STATUS_PWRKEY
	movlw	4
	movwf	pwofftmr ; start power off timer
	bsf	bmc_status, BMC_STATUS_PWSW
	bsf	bmc_status, BMC_STATUS_INTR
	bsf	LATC, C_A20_INT
	return
keyd_doon:
	; power key pressed, we're off, try to power on
	btfss	status, STATUS_12VON
	bra	keyd_12voff
	; 12v present, we can turn A20 on
	movlw	b'00001111'
	movwf	ledg
	clrf	ledr
	call	start_led; one long red blink
	bsf	status, STATUS_PWRON
	bsf	LATC, C_A20_EN
	; configure I2C
	bcf	PMD1, MSSP1MD
	clrf	SSP1STAT
	movlw	b'00010110'
	movwf	SSP1CON1 ; i2c 7bit slave mode
	movlw	b'00000001'
	movwf	SSP1CON2 ; enable clock stretching
	movlw	b'00000000'
	movwf	SSP1CON3
	movlw	b'11111100'
	movwf	SSP1MSK
	movlw	IIC_DS1672
	movwf	SSP1ADD ; match address 0x68 and 0x69
	bcf	IPR1, SSP1IP ; ssp irq low priority
	bcf	PIR1, SSP1IF
	bsf	PIE1, SSP1IE ; enable ssp interrupt
	bsf	SSP1CON1, SSPEN ; enable ssp
	return
keyd_12voff:
	; key press but no 12v: one long red blink
	movlw	b'00001111'
	movwf	ledr
	clrf	ledg
	call	start_led; one long red blink
	return;

keyup: 
	; key released
	clrf	pwofftmr ; key released, don't force power off
	bcf	status, STATUS_PWRKEY
	bsf	bmc_status, BMC_STATUS_INTR
	bsf	LATC, C_A20_INT
	movlw 'u'
	call dotx
	movlw ' '
	call dotx
	bcf PIE1, TMR1IE ; disable timer1 irq
	swapf tod3, w
	call printhex
	movf tod3, w
	call printhex
	swapf tod2, w
	call printhex
	movf tod2, w
	call printhex
	swapf tod1, w
	call printhex
	movf tod1, w
	call printhex
	swapf tod0, w
	call printhex
	movf tod0, w
	call printhex
	bsf PIE1, TMR1IE ; re-enable timer1 irq
	call crlf
	return 

a20_off:
	rcall	do_pwm_off
	bcf	LATC, C_A20_EN
	bcf	LATC, C_A20_INT
	bsf	bmc_status, BMC_STATUS_INTR
	bcf	status, STATUS_PWRON
	; disable i2c
	bcf	PIE1, SSP1IE
	bcf	SSP1CON1, SSPEN
	bsf	PMD1, MSSP1MD
	bcf	bmc_command, BMC_CMD_PWROFF
	return

do_softintssp:
	bcf	softint, SOFTINT_SSP
	btfss	SSP1STAT, D_NOT_A
	bra	ssp_addr
	btfss	SSP1STAT, R_NOT_W
	bra	ssp_write
ssp_read
	; this is a read
	movlw	IIC_DS1672
	cpfseq	i2caddr
	bra	read_bmc
	movf	i2creg, w
	bz	ssp_read_ds_0
	decf	WREG, w
	bz	ssp_read_ds_1
	decf	WREG, w
	bz	ssp_read_ds_2
	decf	WREG, w
	bz	ssp_read_ds_3
	; unimplemented register
	clrf	SSP1BUF
ssp_inc_and_send
	incf	i2creg, f
	bsf	SSP1CON1, CKP
	return
ssp_read_ds_0
	; get snapshot
	movff	tod0, tod0_sn
	movff	tod1, tod1_sn
	movff	tod2, tod2_sn
	movff	tod3, tod3_sn
	movf	tod0_sn, w
	cpfseq	tod0
	bra	ssp_read_ds_0 ; counter changed, re-read
	movff	tod0_sn, SSP1BUF
	bra	ssp_inc_and_send
ssp_read_ds_1
	movff	tod1_sn, SSP1BUF
	bra	ssp_inc_and_send
ssp_read_ds_2
	movff	tod2_sn, SSP1BUF
	bra	ssp_inc_and_send
ssp_read_ds_3
	movff	tod3_sn, SSP1BUF
	bra	ssp_inc_and_send

read_bmc
	movf	i2creg, w
	bz	read_bmc_0
	decf	WREG, w
	bz	read_bmc_1
	decf	WREG, w
	bz	read_bmc_2
	decf	WREG, w
	bz	read_bmc_3
	decf	WREG, w
	bz	read_bmc_4
	decf	WREG, w
	bz	read_bmc_5
	decf	WREG, w
	bz	read_bmc_6
	decf	WREG, w
	bz	read_bmc_7
	; unimplemented register
	clrf	SSP1BUF
	bra	ssp_inc_and_send

read_bmc_0
	movlw	BMC_VERSION
	movwf	SSP1BUF
	bra	ssp_inc_and_send
read_bmc_1
	movff	bmc_status, SSP1BUF
	clrf	bmc_status	; clear status and irq
	bcf	LATC, C_A20_INT
	bra	ssp_inc_and_send
read_bmc_2
	movff	bmc_templ, SSP1BUF
	bra	ssp_inc_and_send
read_bmc_3
	movff	bmc_temph, SSP1BUF
	bra	ssp_inc_and_send
read_bmc_4
	movff	bmc_12vl, SSP1BUF
	bra	ssp_inc_and_send
read_bmc_5
	movff	bmc_12vh, SSP1BUF
	bra	ssp_inc_and_send
read_bmc_6
	movff	bmc_command, SSP1BUF
read_bmc_7
	movff	bmc_bklpwm, SSP1BUF
	bra	ssp_inc_and_send

ssp_write
	tstfsz	i2cstart
	bra	ssp_write_data
	movff	SSP1BUF, i2creg
	bsf	SSP1CON1, CKP
	incf	i2cstart, f
	movlw	's'
	rcall	dotx
	movlw	'r'
	rcall	dotx
	swapf	i2creg, w
	rcall 	printhex
	movf	i2creg, w
	rcall	printhex
	rcall	crlf
	return

ssp_write_data
	movlw	IIC_DS1672
	cpfseq	i2caddr
	bra	write_bmc
	movf	i2creg, w
	bz	ssp_write_ds_0
	decf	WREG, w
	bz	ssp_write_ds_1
	decf	WREG, w
	bz	ssp_write_ds_2
	decf	WREG, w
	bz	ssp_write_ds_3
	; unimplemented register, ignore
ssp_write_unimpl
	movf	SSP1BUF, w
	bra ssp_inc_and_send

ssp_write_ds_0
	; lsb write: reset second counter
	clrf	TMR1L
	clrf	TMR1H
	movff	SSP1BUF, tod0
	bra ssp_inc_and_send
ssp_write_ds_1
	movff	SSP1BUF, tod1
	bra ssp_inc_and_send
ssp_write_ds_2
	movff	SSP1BUF, tod2
	bra ssp_inc_and_send
ssp_write_ds_3
	movff	SSP1BUF, tod3
	bra ssp_inc_and_send

write_bmc
	movlw	6 ; bmc_command
	cpfseq	i2creg
	bra	write_bmc_pwm
	movff	SSP1BUF, bmc_command	
	bra	ssp_inc_and_send
write_bmc_pwm
	movlw	7 ; bmc_pwm
	cpfseq	i2creg
	bra	ssp_write_unimpl
	movf	SSP1BUF, w
	movwf	bmc_bklpwm	
	btfss	bmc_status, BMC_STATUS_BKLON
	bra	ssp_inc_and_send
	movwf	CCPR2L
	bra	ssp_inc_and_send

ssp_addr:
	movff	SSP1BUF, i2caddr
	bcf	i2caddr, 0 ; clear R/W bit
	clrf	i2cstart
	movlw	's'
	rcall	dotx
	movlw	'a'
	rcall	dotx
	swapf	i2caddr, w
	rcall 	printhex
	movf	i2caddr, w
	rcall	printhex
	rcall	crlf
	btfsc	SSP1STAT, R_NOT_W
	bra	ssp_read
	bsf	SSP1CON1, CKP
	return

w64k:
	clrf cmpt2
w2
	clrf cmpt1
w1
	decfsz cmpt1, f
	bra w1
	decfsz cmpt2, f
	bra w2
	return;

txstrcrlf
	rcall txstr
crlf  
	movlw LOW crlfstr;
txstr 
	movwf TBLPTRL;
	movlw 0x02
	movwf TBLPTRH;
	clrf TBLPTRU;
txstrloop
	tblrd*+;
	movf TABLAT, w;
	bz doreturn;
	rcall dotx;
	bra txstrloop;
printhex
	andlw 0x0f;
	sublw 9;
	bc decimal;
	sublw '@';
	bra dotx;
decimal 
	sublw '9';
dotx  
	btfss MPIR, MTXIF; 
	bra dotx;
	movwf MTXREG;
doreturn: 
	return;

	END

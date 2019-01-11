EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:LCDdisplays
LIBS:texas-local
LIBS:control-main-cache
LIBS:microchip-local
LIBS:switches-local
LIBS:edisplay-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 2
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L SW_PUSH_SMALL_H_LED SW1
U 1 1 58D69C17
P 4700 3600
F 0 "SW1" H 4780 3710 50  0000 C CNN
F 1 "SW_PUSH_SMALL_H" H 5060 3540 50  0001 C CNN
F 2 "Buttons_Switches_SMD:SW_SP3T_PCM13" H 4700 3800 50  0001 C CNN
F 3 "" H 4700 3800 50  0000 C CNN
	1    4700 3600
	1    0    0    -1  
$EndComp
$Comp
L SW_PUSH_SMALL_H SW4
U 1 1 58D69C1E
P 6100 3600
F 0 "SW4" H 6180 3710 50  0000 C CNN
F 1 "SW_PUSH_SMALL_H" H 6460 3540 50  0001 C CNN
F 2 "Buttons_Switches_SMD:SW_SPST_B3S-1000" H 6100 3800 50  0001 C CNN
F 3 "" H 6100 3800 50  0000 C CNN
	1    6100 3600
	1    0    0    -1  
$EndComp
$Comp
L SW_PUSH_SMALL_H SW5
U 1 1 58D69C25
P 6100 4150
F 0 "SW5" H 6200 4100 50  0000 C CNN
F 1 "SW_PUSH_SMALL_H" H 6460 4090 50  0001 C CNN
F 2 "Buttons_Switches_SMD:SW_SPST_B3S-1000" H 6100 4350 50  0001 C CNN
F 3 "" H 6100 4350 50  0000 C CNN
	1    6100 4150
	1    0    0    -1  
$EndComp
$Comp
L SW_PUSH_SMALL_H SW2
U 1 1 58D69C2C
P 4700 4150
F 0 "SW2" H 4800 4100 50  0000 C CNN
F 1 "SW_PUSH_SMALL_H" H 5060 4090 50  0001 C CNN
F 2 "Buttons_Switches_SMD:SW_SPST_B3S-1000" H 4700 4350 50  0001 C CNN
F 3 "" H 4700 4350 50  0000 C CNN
	1    4700 4150
	1    0    0    -1  
$EndComp
$Comp
L R R5
U 1 1 58D69C33
P 6800 3050
F 0 "R5" H 6950 3050 50  0000 C CNN
F 1 "2K" H 6650 3050 50  0000 C CNN
F 2 "Resistors_ThroughHole:Resistor_Horizontal_RM7mm" V 6730 3050 50  0001 C CNN
F 3 "" H 6800 3050 50  0000 C CNN
	1    6800 3050
	-1   0    0    1   
$EndComp
$Comp
L R R6
U 1 1 58D69C3A
P 6800 3450
F 0 "R6" H 6950 3450 50  0000 C CNN
F 1 "3K" H 6650 3450 50  0000 C CNN
F 2 "Resistors_ThroughHole:Resistor_Horizontal_RM7mm" V 6730 3450 50  0001 C CNN
F 3 "" H 6800 3450 50  0000 C CNN
	1    6800 3450
	-1   0    0    1   
$EndComp
$Comp
L R R7
U 1 1 58D69C41
P 6800 3850
F 0 "R7" H 6950 3850 50  0000 C CNN
F 1 "5K" H 6650 3850 50  0000 C CNN
F 2 "Resistors_ThroughHole:Resistor_Horizontal_RM7mm" V 6730 3850 50  0001 C CNN
F 3 "" H 6800 3850 50  0000 C CNN
	1    6800 3850
	-1   0    0    1   
$EndComp
$Comp
L R R8
U 1 1 58D69C48
P 6800 4250
F 0 "R8" H 6950 4250 50  0000 C CNN
F 1 "10K" H 6650 4250 50  0000 C CNN
F 2 "Resistors_ThroughHole:Resistor_Horizontal_RM7mm" V 6730 4250 50  0001 C CNN
F 3 "" H 6800 4250 50  0000 C CNN
	1    6800 4250
	-1   0    0    1   
$EndComp
Wire Wire Line
	4850 3600 5950 3600
Wire Wire Line
	4850 4150 5950 4150
Wire Wire Line
	5450 3450 5450 4150
Connection ~ 5450 3600
Wire Wire Line
	4850 3750 5450 3750
Connection ~ 5450 3750
Wire Wire Line
	6800 3600 6800 3700
Wire Wire Line
	6800 4000 6800 4100
Wire Wire Line
	6800 4050 4550 4050
Wire Wire Line
	4550 4050 4550 4150
Connection ~ 6800 4050
Wire Wire Line
	6800 3650 6250 3650
Wire Wire Line
	6250 3650 6250 3600
Connection ~ 6800 3650
Wire Wire Line
	6800 3200 6800 3300
Wire Wire Line
	6800 3250 4550 3250
Wire Wire Line
	4550 3250 4550 3600
Connection ~ 6800 3250
Wire Wire Line
	6250 4150 6450 4150
Wire Wire Line
	6800 4500 6800 4400
Wire Wire Line
	6450 4150 6450 4500
Wire Wire Line
	6450 4500 6800 4500
Wire Wire Line
	5250 2750 5250 3450
Wire Wire Line
	5250 3450 5450 3450
Text HLabel 4300 2750 1    60   Input ~ 0
LED
Text HLabel 5250 2750 1    60   Input ~ 0
GND
Text HLabel 6800 2750 1    60   Input ~ 0
BUTTON
Wire Wire Line
	6800 2750 6800 2900
Wire Wire Line
	4300 2750 4300 3750
Wire Wire Line
	4300 3750 4550 3750
$EndSCHEMATC

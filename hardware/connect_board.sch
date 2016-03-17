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
LIBS:connect_board-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
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
L HE10-10 P3
U 1 1 56C8223B
P 5150 3700
F 0 "P3" H 5150 4250 50  0000 C CNN
F 1 "HE10-10" H 5150 3100 50  0000 C CNN
F 2 "local:he10s-10d" H 5150 3700 50  0001 C CNN
F 3 "" H 5150 3700 50  0000 C CNN
	1    5150 3700
	1    0    0    -1  
$EndComp
$Comp
L CONN_01X04 P2
U 1 1 56C8223C
P 3250 3850
F 0 "P2" H 3250 4100 50  0000 C CNN
F 1 "CONN_01X04" V 3350 3850 50  0000 C CNN
F 2 "phoenix-local:phoenix_1770979" H 3250 3850 50  0001 C CNN
F 3 "" H 3250 3850 50  0000 C CNN
	1    3250 3850
	-1   0    0    1   
$EndComp
$Comp
L CONN_01X04 P1
U 1 1 56C8223D
P 2650 3850
F 0 "P1" H 2650 4100 50  0000 C CNN
F 1 "CONN_01X04" V 2750 3850 50  0000 C CNN
F 2 "phoenix-local:phoenix_1770979" H 2650 3850 50  0001 C CNN
F 3 "" H 2650 3850 50  0000 C CNN
	1    2650 3850
	-1   0    0    1   
$EndComp
Text Label 4100 4100 0    60   ~ 0
CAN_+12V
Text Label 4100 3900 0    60   ~ 0
CAN_GND
Text Label 4100 3700 0    60   ~ 0
CANL
Text Label 4100 3500 0    60   ~ 0
CANH
Wire Wire Line
	3550 4100 4600 4100
Wire Wire Line
	2850 4000 3550 4000
Wire Wire Line
	2850 3900 4600 3900
Connection ~ 3450 3900
Wire Wire Line
	4600 3700 3900 3700
Wire Wire Line
	3900 3800 3900 3700
Wire Wire Line
	2850 3800 3900 3800
Connection ~ 3450 3800
Wire Wire Line
	4600 3500 3550 3500
Wire Wire Line
	2850 3700 3550 3700
Wire Wire Line
	3550 3700 3550 3500
Connection ~ 3450 3700
Wire Wire Line
	3550 4000 3550 4100
Connection ~ 3450 4000
Text Label 5800 4100 0    60   ~ 0
A_TX
Text Label 5800 3900 0    60   ~ 0
B_TX
Text Label 5800 3500 0    60   ~ 0
A_RX
Text Label 5800 3700 0    60   ~ 0
B_RX
$Comp
L JUMPER3 JP1
U 1 1 56C8223E
P 6550 4100
F 0 "JP1" H 6600 4000 50  0000 L CNN
F 1 "JUMPER3" H 6550 4200 50  0001 C BNN
F 2 "FS_Pin_Headers:Pin_Header_Straight_1x03" H 6550 4100 50  0001 C CNN
F 3 "" H 6550 4100 50  0000 C CNN
	1    6550 4100
	-1   0    0    1   
$EndComp
$Comp
L JUMPER3 JP2
U 1 1 56C8223F
P 6600 3650
F 0 "JP2" H 6650 3550 50  0000 L CNN
F 1 "JUMPER3" H 6600 3750 50  0001 C BNN
F 2 "FS_Pin_Headers:Pin_Header_Straight_1x03" H 6600 3650 50  0001 C CNN
F 3 "" H 6600 3650 50  0000 C CNN
	1    6600 3650
	1    0    0    -1  
$EndComp
$Comp
L CONN_01X03 P5
U 1 1 56C82240
P 7750 3850
F 0 "P5" H 7750 4050 50  0000 C CNN
F 1 "CONN_01X03" V 7850 3850 50  0001 C CNN
F 2 "FS_Terminal_Blocks:TerminalBlock_Pheonix_MPT-2.54mm_3pol" H 7750 3850 50  0001 C CNN
F 3 "" H 7750 3850 50  0000 C CNN
	1    7750 3850
	1    0    0    -1  
$EndComp
$Comp
L CONN_01X03 P4
U 1 1 00000000
P 7750 3400
F 0 "P4" H 7750 3600 50  0000 C CNN
F 1 "CONN_01X03" V 7850 3400 50  0001 C CNN
F 2 "FS_Terminal_Blocks:TerminalBlock_Pheonix_MPT-2.54mm_3pol" H 7750 3400 50  0001 C CNN
F 3 "" H 7750 3400 50  0000 C CNN
	1    7750 3400
	1    0    0    -1  
$EndComp
Text Label 7000 3500 0    60   ~ 0
VHF_GND
Text Label 7050 3950 0    60   ~ 0
RS232_RX
Text Label 7050 3750 0    60   ~ 0
VHF_RX_B
Text Label 7050 3850 0    60   ~ 0
VHF_RX_A
Wire Wire Line
	5700 4100 6300 4100
Wire Wire Line
	6800 4100 6800 3950
Wire Wire Line
	6800 3950 7550 3950
Wire Wire Line
	6550 3850 7550 3850
Wire Wire Line
	6550 3850 6550 4000
Wire Wire Line
	6600 3750 7550 3750
Wire Wire Line
	6850 3650 6850 3500
Wire Wire Line
	6850 3500 7550 3500
Wire Wire Line
	5700 3900 6350 3900
Wire Wire Line
	6350 3900 6350 3650
Wire Wire Line
	5700 3700 6250 3700
Wire Wire Line
	6250 3700 6250 3400
Wire Wire Line
	6250 3400 7550 3400
Wire Wire Line
	5700 3500 6100 3500
Wire Wire Line
	6100 3500 6100 3300
Wire Wire Line
	6100 3300 7550 3300
$EndSCHEMATC

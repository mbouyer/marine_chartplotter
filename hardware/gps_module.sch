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
LIBS:microstack_gps
LIBS:regul-local
LIBS:microchip-local
LIBS:gps_module-cache
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
L microstack_GPS U101
U 1 1 56C07B34
P 3650 2900
F 0 "U101" H 3650 3400 60  0000 C CNN
F 1 "microstack_GPS" H 3650 2350 60  0000 C CNN
F 2 "local:microstack" H 3650 2900 60  0001 C CNN
F 3 "" H 3650 2900 60  0000 C CNN
	1    3650 2900
	1    0    0    -1  
$EndComp
$Comp
L HE10-10 P101
U 1 1 56C07B35
P 6000 2950
F 0 "P101" H 6000 3500 50  0000 C CNN
F 1 "HE10-10" H 6000 2350 50  0000 C CNN
F 2 "local:he10s-10d" H 6000 2950 50  0001 C CNN
F 3 "" H 6000 2950 50  0000 C CNN
	1    6000 2950
	1    0    0    -1  
$EndComp
Text Label 5300 2550 2    60   ~ 0
VBATT
Text Label 6800 2750 0    60   ~ 0
VCC
Text Label 6800 2950 0    60   ~ 0
GND
Text Label 5200 2750 2    60   ~ 0
GPS_RX
Text Label 5200 2950 2    60   ~ 0
GPS_TX
Text Label 5200 3150 2    60   ~ 0
PPS
$Comp
L CONN_01X01 P102
U 1 1 00000000
P 4850 2300
F 0 "P102" H 4850 2400 50  0000 C CNN
F 1 "CONN_01X01" V 4950 2300 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x01" H 4850 2300 50  0001 C CNN
F 3 "" H 4850 2300 50  0000 C CNN
	1    4850 2300
	-1   0    0    1   
$EndComp
$Comp
L MCP1754ST-3302E/MB U102
U 1 1 56C07842
P 8150 2750
F 0 "U102" H 8250 2600 50  0000 C CNN
F 1 "MCP1703A" H 8150 2900 50  0000 C CNN
F 2 "local:SOT-223-Tto2" H 8150 2750 50  0001 C CNN
F 3 "" H 8150 2750 50  0000 C CNN
	1    8150 2750
	1    0    0    -1  
$EndComp
$Comp
L C_Small C101
U 1 1 56C07927
P 7750 2950
F 0 "C101" H 7760 3020 50  0000 L CNN
F 1 "1µF" H 7760 2870 50  0000 L CNN
F 2 "Capacitors_SMD:C_0805_HandSoldering" H 7750 2950 50  0001 C CNN
F 3 "" H 7750 2950 50  0000 C CNN
	1    7750 2950
	1    0    0    -1  
$EndComp
$Comp
L C_Small C102
U 1 1 56C07999
P 8550 2950
F 0 "C102" H 8560 3020 50  0000 L CNN
F 1 "1µF" H 8560 2870 50  0000 L CNN
F 2 "Capacitors_SMD:C_0805_HandSoldering" H 8550 2950 50  0001 C CNN
F 3 "" H 8550 2950 50  0000 C CNN
	1    8550 2950
	1    0    0    -1  
$EndComp
NoConn ~ 6550 2550
NoConn ~ 6550 3150
NoConn ~ 6550 3350
$Comp
L PWR_FLAG #FLG01
U 1 1 56C07CB4
P 7750 2550
F 0 "#FLG01" H 7750 2645 50  0001 C CNN
F 1 "PWR_FLAG" H 7750 2730 50  0000 C CNN
F 2 "" H 7750 2550 50  0000 C CNN
F 3 "" H 7750 2550 50  0000 C CNN
	1    7750 2550
	1    0    0    -1  
$EndComp
$Comp
L PWR_FLAG #FLG02
U 1 1 56C07CF0
P 7150 3700
F 0 "#FLG02" H 7150 3795 50  0001 C CNN
F 1 "PWR_FLAG" H 7150 3880 50  0000 C CNN
F 2 "" H 7150 3700 50  0000 C CNN
F 3 "" H 7150 3700 50  0000 C CNN
	1    7150 3700
	0    1    1    0   
$EndComp
Wire Wire Line
	6550 2750 7850 2750
Wire Wire Line
	6550 2950 7200 2950
Wire Wire Line
	4550 2750 5450 2750
Wire Wire Line
	4800 2950 5450 2950
Wire Wire Line
	4900 3150 5450 3150
Wire Wire Line
	4050 3300 4800 3300
Wire Wire Line
	4800 3300 4800 2950
Wire Wire Line
	4050 3150 4550 3150
Wire Wire Line
	4550 3150 4550 2750
Wire Wire Line
	4050 3000 4900 3000
Wire Wire Line
	4900 3000 4900 3150
Wire Wire Line
	4050 2850 5250 2850
Wire Wire Line
	5250 2850 5250 3350
Wire Wire Line
	5250 3350 5450 3350
Wire Wire Line
	8450 2750 9150 2750
Wire Wire Line
	9150 2750 9150 4050
Wire Wire Line
	9150 4050 2750 4050
Wire Wire Line
	2750 4050 2750 3150
Wire Wire Line
	2750 3150 3200 3150
Wire Wire Line
	7200 2950 7200 3150
Wire Wire Line
	7200 3150 8550 3150
Wire Wire Line
	8150 3150 8150 2950
Wire Wire Line
	7000 2950 7000 3700
Wire Wire Line
	3050 3700 7150 3700
Wire Wire Line
	3050 3700 3050 2550
Wire Wire Line
	3050 2550 3200 2550
Connection ~ 7000 2950
Wire Wire Line
	7750 2550 7750 2850
Connection ~ 7750 2750
Wire Wire Line
	7750 3050 7750 3150
Connection ~ 7750 3150
Wire Wire Line
	8550 3150 8550 3050
Connection ~ 8150 3150
Wire Wire Line
	8550 2850 8550 2750
Connection ~ 8550 2750
Connection ~ 7000 3700
Wire Wire Line
	5050 2300 5300 2300
Wire Wire Line
	5300 2300 5300 2550
Wire Wire Line
	5300 2550 5450 2550
$EndSCHEMATC

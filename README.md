Hardware and software for an opencpn-based chartplotter
The kardware has been designed with kicad 4.0.x
The chartplotter is built around a A20-based single-board computer
from Olimex (https://www.olimex.com/Products/OLinuXino/A20/A20-OLinuXIno-LIME2/)
and a 15" TFT full HD display also from Olimex
(https://www.olimex.com/Products/OLinuXino/LCD/LCD-OLinuXino-15.6FHD)

The hardware consist of 4 boards
- a breakout board for the Lime2, also providing power supplies,
  an audio amplifier, RS422 and CAN interfaces, and a I2C connection to
  a PIC18 for power management and TOD clock.
- a board holding buttons
- a board holding a GPS module
- a board for connections to other electronic equipements (VHF, sensors, ...)
  via a single flat cable.

for more details see the wiki: https://github.com/mbouyer/marine_chartplotter/wiki

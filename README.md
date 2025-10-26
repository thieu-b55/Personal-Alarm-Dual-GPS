# Personal-Alarm-Dual-GPS

# Warning: The volume of the buzzer may cause hearing damage.

# Built with

SIM A7670E 		4G successor to the well-known 2G SIM800 modules with a built-in GNSS receiver.

The "E" after 7670 indicates the continent. For countries other than Europe, please consult the manual.

SAM M-10Q		Ublox GNSS module

STM32G431CB	STM32 board with compact dimensions


# Activate alarm

Press the alarm button

12 x 1 Watt red LEDs flash at a frequency of 25 Hz (while sending SMS the LEDs keep on burning, after sending it flashes)

Extremely powerful alarm signal (120dB)

The buzzer is disabled in the program using "//" (line 145). To avoid hearing damage, 	it is recommended to only activate it once everything has been tested and is ready for 	use.

SMS to a maximum of 5 mobile numbers with a pre-programmed text followed by the current location

# Test

First press the test button, followed by the alarm button.

Then release the alarm button first, and then the test button.

A text message is sent to the number in position 6.

The LEDs light for 1 second.

The buzzer activates for 1 second.

# Usage

Fully programmable by sending an SMS to the module.

The module has 7 programmable positions.

All positions have a maximum length of 50 characters.

The module can only be programmed if PRGM_EN (PA1) is FALSE.

After programming disconnect PA1 from GND


# Positions 1 through 5

Enter here the number of the contacts that should be notified with an SMS

Send an SMS in the format

#1 +32123456789#

SMS starts and ends with a #

After the first #, enter the number of the position to be programmed. If it is already programmed, the contents will be erased and overwritten.

Blank

Contact number in international format

SMS ends with a #

Response module via SMS to the number that sent the SMS

pos : 1   tel : +32123456789

# Position 6

Enter your mobile phone number here.

During testing, a text message will be sent to this number.

#6 +32987654321#

Response module via SMS to the number that sent the SMS

pos : 6   tel : +32987654321


# Position 7

Enter the pre-programmed text here that will be sent via SMS

For teenagers in dire need of money, the message might look like this:

#7 NO MONEY…BRING 100€ URGENTLY#

Response module via SMS to the number that sent the SMS

pos : 7   txt :  NO MONEY…BRING 100€ URGENTLY


#  #LIST#

module responds with

pos : 1  tel: +32123456789

pos : 2  tel: +32123456789

pos : 3  tel: +32123456789

…

pos : 7 txt: NO MONEY…BRING 100€ URGENTLY


#  #5 DEL#
removes position 5

Answer module

pos 5 : deleted









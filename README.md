# Personal-Alarm-Dual-GPS

# Warning: The volume of the buzzer may cause hearing damage.

# Built with

SIM A7670E 		4G successor to the well-known 2G SIM800 modules with a built-in GNSS receiver.

The "E" after 7670 indicates the continent. For countries other than Europe, please consult the manual.

SAM M-10Q		Ublox GNSS module

STM32G431CB	STM32 board with compact dimensions


# Activate alarm

Press the alarm button

12 x 1 Watt red LEDs flash at a frequency of 25 Hz

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








The buzzer activates for 1 second.

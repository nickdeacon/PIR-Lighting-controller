# PIR-Lighting-controller
Sonoff CH4 Pro2 firmware to create a security lighting system
This is a firmware update for the Sonoff 4CH Pro device to create a PIR based security light system. It has two main additions to the basic unit, allowing up to 4 RF devices to be registered against each of the four output relay channels, and includes an ADC input for light levels, so that switching only occurs at a light level below the pre-set level.
It is intended for use with the Sonoff PIR units, as well as the Sonoff keyfob, and allows each channel to be switched from a variety of sources.
This software will allow the unit to function as a security lighting system without relying on a connection to the eWeLink or other internet based home automation system. That said, individual lighting zones can be triggered via these also.
This firmware runs on the STM32f030C6T6 chip, and you will need a means of programming the chip using the SWDIO and SWCLK pins. I have found that the STM32F0308-DISCO development board is an ideal means to do this.
I recommend that you also download the ST-LINK Utility which will allow you to backup the original firmware, and program the unit back to its original state if required.

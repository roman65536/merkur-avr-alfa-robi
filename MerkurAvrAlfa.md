# Introduction #

This is alternative Firmware for the Merkur Alfa Robot (www.merkurtoys.cz)


# Details #

I added some new features to the original [Merkur](http://www.merkurtoys.cz/en/products/robotic-tracer) firmware, such as variable speed control for both motors also both directions. This is done by software PWM found at http://www.mikrocontroller.net/topic/114624 .
As it is now, when you turn on the Robot it beeps four time at different frequencies.
When you press forward the speed of the motors increase. By pressing backward, it decrease. Pressing Function 2 button it stops. The original stuff is still in the code, however the dip switches don't have any function.

# Background Informations #
The Ucontroller used in Merkur AVR Alfa, is an [AVR Mega8a](http://www.atmel.com/Images/Atmel-8159-8-bit-AVR-microcontroller-ATmega8A_datasheet.pdf).
Motor Drivers are [Toshiba TA7291S](http://www.toshiba.com/taec/components2/Datasheet_Sync/261/3604.pdf).
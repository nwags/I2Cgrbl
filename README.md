I2Cgrbl
=======

grbl with I2C input (address=4)


I wanted to control a grbl board using i2c. The grbl board is set as a slave board with the address 
arbitrarily set to 4. It can be changed in nwi.c. This is based off of twi. I plan on cleaning this 
up at some point and adding feedback through i2c similar to the current feedback through the serial 
connection.

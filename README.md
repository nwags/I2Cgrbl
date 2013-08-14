I2Cgrbl
=======

grbl with I2C input (address=4)


I wanted to control a grbl board using i2c. The grbl board is set as a slave board with the address 
arbitrarily set to 4. It can be changed in nwi.c. This is based off of twi.

I have a working bluetooth multi-master i2c grbl setup now for 2-way communication between mobile 
device and grbl, but I'm still testing it and I'll post the code for the various parts in the near future. 
The set-up is:

Android device(using just blueterm right now) <=BT=> BT Arduino <=i2c=> GRBL Arduino

Also, I'm currently looking for a full-time job...

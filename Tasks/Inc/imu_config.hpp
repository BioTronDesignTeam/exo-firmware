#include "stm32h7xx_hal.h"

/*
pc6 pd14 pd15 pf3 pg12 can be used for gpio

pb8 pb9 i2c1 is first bus
pb10 pb11 i2c2 is the second bus

pd12 pd13 i2c4 is the third bus

Pins to connect: 3.3v gnd scl sda int rst may be needed but theres also d1 pin which controls I2C addressing (pull HIGH for alternate)
*/

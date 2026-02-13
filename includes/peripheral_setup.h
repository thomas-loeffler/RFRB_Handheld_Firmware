
#include <stdint.h>

void GPIO_setup(void);
void i2c_setup(void);
void spi_setup(void);

#define CS_PIN 21 // GP21 / physical pin 27 (SPI CS pin)

#define DIO0_PIN 15  //  DIO0


#define SC_PIN 0 // GP0 / physical pin 1

#define DIP1 10 // GP10 / physical pin 14
#define DIP2 11 // GP11 / physical pin 15
#define DIP3 12 // GP12 / physical pin 16
#define DIP4 13 // GP13 / physical pin 17


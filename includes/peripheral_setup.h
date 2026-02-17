
//////////////////////////////////////
//             INCLUDES             //
//////////////////////////////////////

#include <stdint.h>



//////////////////////////////////////
//       FUNCTION PROTOTYPES        //
//////////////////////////////////////

void GPIO_setup(void);
void i2c_setup(void);
void spi_setup(void);
void radio_irq_setup(void);
void radio_irq_handler(uint gpio, uint32_t events);




//////////////////////////////////////
//            VARIABLES             //
//////////////////////////////////////

extern volatile bool radio_event;





//////////////////////////////////////
//             DEFINES              //
//////////////////////////////////////

// ===== Simple Cycle Pin =====
#define CYCLE 0 // GP0 / physical pin 1


// ============ I2C ============
#define I2C_SDA 4 // GP4 / physical pin 6
#define I2C_SCL 5 // GP5 / physical pin 7


// ======= DIP Switches =======
#define DIP1 10 // GP10 / physical pin 14
#define DIP2 11 // GP11 / physical pin 15
#define DIP3 12 // GP12 / physical pin 16
#define DIP4 13 // GP13 / physical pin 17


// ============ SPI ============
#define SPI_MISO 16  // GP16 / physical pin 21
#define SPI_CS 17    // GP17 / physical pin 22
#define SPI_SCK 18   // GP18 / physical pin 24
#define SPI_MOSI 19  // GP19 / physical pin 25


// ===== Other Radio Pins =====
#define RADIO_RST 20 // GP20 / physical pin 26
#define RADIO_IRQ 21 // GP21 / physical pin 27
#define RADIO_EN 22 // GP22 / physical pin 29





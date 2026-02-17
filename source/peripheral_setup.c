
//////////////////////////////////////
//             INCLUDES             //
//////////////////////////////////////

// Standard includes / hardware headers
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

// User defined headers
#include "peripheral_setup.h"



//////////////////////////////////////
//             VARIABLES            //
//////////////////////////////////////

volatile bool radio_event = false;





//////////////////////////////////////
//       FUNCTION DEFINITIONS       //
//////////////////////////////////////

void GPIO_setup(void) {
    
    // Initialize the simple cycle pin as an output and set high to start
    gpio_init(CYCLE); // GP0 / physical pin 1
    gpio_set_dir(CYCLE, GPIO_OUT);
    gpio_put(CYCLE, 1);  


    // Initialize the DIP switch pins as inputs with pull-up resistors
    gpio_init(DIP1); // GP10 / physical pin 14
    gpio_init(DIP2); // GP11 / physical pin 15
    gpio_init(DIP3); // GP12 / physical pin 16
    gpio_init(DIP4); // GP13 / physical pin 17

    gpio_set_dir(DIP1, GPIO_IN);
    gpio_set_dir(DIP2, GPIO_IN);
    gpio_set_dir(DIP3, GPIO_IN);
    gpio_set_dir(DIP4, GPIO_IN);

    gpio_pull_up(DIP1);
    gpio_pull_up(DIP2);
    gpio_pull_up(DIP3);
    gpio_pull_up(DIP4);
}






void i2c_setup(void){
    // Use i2c0, 400 kHz speed
    i2c_init(i2c0, 400 * 1000);

    // Set up the GPIO pins to the I2C function
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);   // GP4 / physical pin 6)
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);   // GP5 / Physical pin 7)

    // Enable pull-ups (typical for I2C)
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}






void spi_setup(void){
    // Use spi0, 4 MHz speed
    spi_init(spi0, 4 * 1000 * 1000); 

    spi_set_format( spi0,
                    8,          // bits per transfer
                    SPI_CPOL_0, // Clock polarity: idle low
                    SPI_CPHA_0, // Clock phase: first edge / rising edge
                    SPI_MSB_FIRST);


    gpio_set_function(SPI_MISO, GPIO_FUNC_SPI); // GP16 / physical pin 21
    gpio_set_function(SPI_SCK, GPIO_FUNC_SPI); // GP18 / physical pin 24
    gpio_set_function(SPI_MOSI, GPIO_FUNC_SPI); // GP19 / physical pin 25
    

    gpio_init(SPI_CS); // GP17 / physical pin 22
    gpio_set_dir(SPI_CS, GPIO_OUT);
    gpio_put(SPI_CS, 1); // deselect device
}






void radio_irq_setup(void) {

    gpio_init(RADIO_IRQ); // GP21 / physical pin 27
    gpio_set_dir(RADIO_IRQ, GPIO_IN);
    gpio_pull_down(RADIO_IRQ);  // So pin is typically low until radio asserts it high for irq

    gpio_set_irq_enabled_with_callback(
        RADIO_IRQ,          // GP21 / physical pin 27
        GPIO_IRQ_EDGE_RISE, // Trigger on rising edge
        true,               // Enable the interrupt
        &radio_irq_handler  // Callback function to handle the interrupt
    );

}


void radio_irq_handler(uint gpio, uint32_t events) {
    // Set flag 
    radio_event = true;
}






/*

writing example
uint8_t data = 0x55;

gpio_put(CS_PIN, 0);                 // select device
spi_write_blocking(spi0, &data, 1);  // send
gpio_put(CS_PIN, 1);                 // deselect


reading and writing at the same time example:
uint8_t tx = 0x00;
uint8_t rx;

gpio_put(CS_PIN, 0);
spi_write_read_blocking(spi0, &tx, &rx, 1);
gpio_put(CS_PIN, 1);


example reading a packet form the radio:

uint8_t rx_buffer[64];   // storage for packet

gpio_put(CS, 0);
spi_write_blocking(spi0, &fifo_read_cmd, 1);
spi_read_blocking(spi0, 0x00, rx_buffer, packet_length);
gpio_put(CS, 1);



*/


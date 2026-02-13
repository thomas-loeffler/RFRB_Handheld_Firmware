
#include "pico/stdlib.h"

#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

#include "peripheral_setup.h"



void GPIO_setup(void) {
    
    // Initialize the simple cycle pin as an output and set high to start
    gpio_init(SC_PIN);
    gpio_set_dir(SC_PIN, GPIO_OUT);
    gpio_put(SC_PIN, 1);  


    // Initialize the DIP switch pins as inputs with pull-up resistors
    gpio_init(DIP1);
    gpio_init(DIP2);
    gpio_init(DIP3);
    gpio_init(DIP4);

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
    gpio_set_function(4, GPIO_FUNC_I2C);   // SDA (Physical pin 6)
    gpio_set_function(5, GPIO_FUNC_I2C);   // SCL (Physical pin 7)

    // Enable pull-ups (typical for I2C)
    gpio_pull_up(4);
    gpio_pull_up(5);
}


void spi_setup(void){
    // Use spi0, 4 MHz speed
    spi_init(spi0, 4 * 1000 * 1000); 

    spi_set_format( spi0,
                    8,        // bits per transfer
                    SPI_CPOL_0,
                    SPI_CPHA_0,
                    SPI_MSB_FIRST);

    gpio_set_function(18, GPIO_FUNC_SPI); // SCK
    gpio_set_function(19, GPIO_FUNC_SPI); // MOSI
    gpio_set_function(16, GPIO_FUNC_SPI); // MISO


    gpio_init(CS_PIN); // GP21 / physical pin 27
    gpio_set_dir(CS_PIN, GPIO_OUT);
    gpio_put(CS_PIN, 1); // deselect device

}


gpio_set_irq_enabled_with_callback(
    DIO0_PIN, 
    GPIO_IRQ_EDGE_RISE,   // trigger on rising edge (LOW→HIGH)
    true,                 // enable
    &dio0_isr             // callback function
);


void dio0_isr(uint gpio, uint32_t events) {
    // This function is called when DIO0 triggers
    // gpio = pin number, events = what triggered it
    // Set a flag or handle packet received
    printf("Packet ready!\n");
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


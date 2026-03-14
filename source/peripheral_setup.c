
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
#include "display_functions.h" // just for now



//////////////////////////////////////
//             VARIABLES            //
//////////////////////////////////////

volatile bool radio_event = false;





//////////////////////////////////////
//       FUNCTION DEFINITIONS       //
//////////////////////////////////////

void GPIO_setup(void) {
    
    // ---------- Simple Cycle Pin ----------
    // Initialize the simple cycle pin as an output and set high to start
    gpio_init(CYCLE); // GP0 / physical pin 1
    gpio_set_dir(CYCLE, GPIO_OUT);
    gpio_put(CYCLE, 0);  

    // ---------- Test Input Pin ----------
    // Pin for various tests
    gpio_init(TEST); // GP28 / physical pin 34
    gpio_set_dir(TEST, GPIO_IN);
    gpio_pull_down(TEST); // normally grounded, triggers when sent high
    




    // ---------- DIP Switches ----------
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

    // ---------- Radio Reset Pin ----------
    gpio_init(RADIO_RST); // GP20 / physical pin 26
    gpio_set_dir(RADIO_RST, GPIO_OUT);
    gpio_put(RADIO_RST, 0); // reset active high, so start with it low
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
    
    // gpio_pull_up(SPI_MISO); // suggested but not needed for functionality

    gpio_init(SPI_CS); // GP17 / physical pin 22 
    gpio_set_dir(SPI_CS, GPIO_OUT);
    gpio_put(SPI_CS, 1); // deselect device
    //gpio_pull_up(SPI_CS); // suggested but not needed for functionality
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



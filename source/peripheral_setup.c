
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "peripheral_setup.h"



void GPIO_setup(void) {
    // gpio_init initializes the specified GPIO pin to GPIO function
    // gpio_set_dir sets the pin to be as an input or output (data direction)
    // gpio_put sets the pin to high or low
    
    gpio_init(SC_PIN);
    gpio_set_dir(SC_PIN, GPIO_OUT);
    gpio_put(SC_PIN, 1);  



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
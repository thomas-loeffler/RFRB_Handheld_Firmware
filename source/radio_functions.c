
//////////////////////////////////////
//             INCLUDES             //
//////////////////////////////////////

// Standard includes / hardware headers
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <stdio.h>
//#include <string.h>

// User defined headers
#include "radio_functions.h"
#include "peripheral_setup.h" // For GPIO pin definitions



//////////////////////////////////////
//       FUNCTION DEFINITIONS       //
//////////////////////////////////////

// reg is the register you're writing too, and value what you want to write to that register
void rfm69_spi_write(uint8_t reg, uint8_t value) {
    gpio_put(SPI_CS, 0);             // CS low to select the chip
    uint8_t buf[2] = { reg | 0x80, value }; // MSB=1 for write
    spi_write_blocking(spi0, buf, 2);
    gpio_put(SPI_CS, 1);             // CS high to finish
}

// reg is youre reading from, and the return value is the data read from that register
uint8_t rfm69_spi_read(uint8_t reg) {
    gpio_put(SPI_CS, 0);             // select the chip
    uint8_t out_buf[2] = { reg & 0x7F, 0x00 }; // MSB=0 for read
    uint8_t in_buf[2] = { 0 };
    spi_write_read_blocking(spi0, out_buf, in_buf, 2);
    gpio_put(SPI_CS, 1);             // deselect
    return in_buf[1];                // second byte is the data
}



void radio_reset(void) {
    gpio_put(RADIO_RST, 0);
    sleep_ms(10);
    gpio_put(RADIO_RST, 1);
    sleep_ms(10);
}



void check_version(void) {
    uint8_t version = rfm69_spi_read(0x10); // RegVersion
    printf("RFM69 version: 0x%02X\n", version);
    if (version != 0x24) {
        printf("SPI communication failed!\n");
    } else {
        printf("SPI OK\n");
    }
}

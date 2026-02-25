
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
#include "radio_registers.h" // For register definitions and bit masks



const uint8_t SYNC_WORD[4] = {0xCA, 0xFE, 0xBA, 0xBE}; // Sync word to use for the radio


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


// Shouldn't need to reset the radio on every boot, but this is useful for testing and ensures a known state
void radio_reset(void) {
    gpio_put(RADIO_RST, 1);
    sleep_ms(1); // Datasheet specifies at least 100us high, so 1ms is plenty
    gpio_put(RADIO_RST, 0);
    sleep_ms(5); // Ready after 5ms
}


// Debugging function
void check_version(void) {
    uint8_t version = rfm69_spi_read(0x10); // RegVersion
    printf("RFM69 version: 0x%02X\n", version);
    if (version != 0x24) {
        printf("SPI communication failed!\n");
    } else {
        printf("SPI OK\n");
    }
}

void radio_setup(void) {

    rfm69_spi_write(REG_OPMODE, MODE_STANDBY); // Set to standby mode (if not already) to allow writing to registers

    rfm69_spi_write(REG_DATAMODUL, MY_DATAMODUL); // Packet mode, FSK, no shaping

    rfm69_spi_write(REG_BITRATEMSB, BR_100kb_MSB); // 100kbps bitrate
    rfm69_spi_write(REG_BITRATELSB, BR_100kb_LSB);

    rfm69_spi_write(REG_FDEVMSB, FDEV_50k_MSB); // 50kHz frequency deviation
    rfm69_spi_write(REG_FDEVLSB, FDEV_50k_LSB);

    rfm69_spi_write(REG_FRFMSB, FRF_915_MSB); // 915MHz frequency
    rfm69_spi_write(REG_FRFMID, FRF_915_MID);
    rfm69_spi_write(REG_FRFLSB, FRF_915_LSB); // Always write LSB last since freqnecy only updates on LSB write

    rfm69_spi_write(REG_PALEVEL, MY_TX_POWER); // PA1 on, PA2 off, OutputPower = 31 = +13dBm

    rfm69_spi_write(REG_PARAMP, MY_PARAMP); // Set PA ramp-up time to default (40us)

    rfm69_spi_write(REG_OCP, MY_OCP); // OCP on, 95mA limit

    rfm69_spi_write(REG_LNA, MY_LNA_RX_POWER); // 50 ohm input impedance, ACG on

    rfm69_spi_write(REG_RXBW, MY_RXBW); // Rx bandwidth 167kHz

    rfm69_spi_write(REG_AFCBW, MY_AFCBW); // AFC bandwidth 200kHz, wider than RxBw for better AFC


    /*
    rfm69_spi_write(REG_PREAMBLEMSB, MY_PREAMBLE_MSB);
    rfm69_spi_write(REG_PREAMBLELSB, MY_PREAMBLE_LSB); // 4 preamble bytes

    rfm69_spi_write(REG_SYNC_CONFIG, MY_SYNC_CONFIG); // Sync on, 4 bytes sync word, 0 bit tolerance
	rfm69_spi_write(REG_SYNCVALUE1, SYNC_WORD[0]);
	rfm69_spi_write(REG_SYNCVALUE2, SYNC_WORD[1]);
	rfm69_spi_write(REG_SYNCVALUE3, SYNC_WORD[2]);
	rfm69_spi_write(REG_SYNCVALUE4, SYNC_WORD[3]);

    rfm69_spi_write(REG_PACKETCONFIG1, 0x90); // Fixed length packets, CRC on, discard bad packets, no address filtering

    rfm69_spi_write(REG_PAYLOADLENGTH, MY_PAYLOAD_LENGTH); // Maximum packet length 64 bytes

    rfm69_spi_write(REG_FIFOTHRESH, MY_FIFOTHRESH); // Transmit if fifo not empty, threshhold = 0

    rfm69_spi_write(REG_PACKETCONFIG2, 0x32); //InterPacketRxDelay 80us, auto RX restart on, AES encryption off
    */


}


void verify_radio_setup(void) {
    // read back and verify all configured registers are correct

    uint8_t opmode = rfm69_spi_read(REG_OPMODE); //set mode to standby

    uint8_t datamodul = rfm69_spi_read(REG_DATAMODUL); // Packet mode, FSK, no shaping

    uint8_t bitrate_msb = rfm69_spi_read(REG_BITRATEMSB); // 100kbps bitrate
    uint8_t bitrate_lsb = rfm69_spi_read(REG_BITRATELSB);

    uint8_t fdev_msb = rfm69_spi_read(REG_FDEVMSB); // 50kHz frequency deviation
    uint8_t fdev_lsb = rfm69_spi_read(REG_FDEVLSB);

    uint8_t frf_msb = rfm69_spi_read(REG_FRFMSB); // 915MHz frequency
    uint8_t frf_mid = rfm69_spi_read(REG_FRFMID);
    uint8_t frf_lsb = rfm69_spi_read(REG_FRFLSB);

    uint8_t palevel = rfm69_spi_read(REG_PALEVEL); // PA1 on, PA2 off, OutputPower = 31 = +13dBm

    uint8_t paramp = rfm69_spi_read(REG_PARAMP); // Default PA ramp-up time (40us)

    uint8_t ocp = rfm69_spi_read(REG_OCP); // OCP on, 95mA limit

    uint8_t lna = rfm69_spi_read(REG_LNA); // 50 ohm input impedance, ACG on

    uint8_t rxbw = rfm69_spi_read(REG_RXBW); // Rx bandwidth 167kHz

    uint8_t afcbw = rfm69_spi_read(REG_AFCBW); // AFC bandwidth 200kHz, wider than RxBw for better AFC




    printf("OPMODE      : 0x%02X\n", opmode);
    printf("DATAMODUL   : 0x%02X\n", datamodul);
    printf("BITRATE MSB : 0x%02X\n", bitrate_msb);
    printf("BITRATE LSB : 0x%02X\n", bitrate_lsb);
    printf("FDEV MSB    : 0x%02X\n", fdev_msb);
    printf("FDEV LSB    : 0x%02X\n", fdev_lsb);
    printf("FRF MSB     : 0x%02X\n", frf_msb);
    printf("FRF MID     : 0x%02X\n", frf_mid);
    printf("FRF LSB     : 0x%02X\n", frf_lsb);
    printf("PALEVEL     : 0x%02X\n", palevel);
    printf("PARAMP      : 0x%02X\n", paramp);
    printf("OCP         : 0x%02X\n", ocp);
    printf("LNA         : 0x%02X\n", lna);
    printf("RXBW        : 0x%02X\n", rxbw);
    printf("AFCBW       : 0x%02X\n", afcbw);
    

	if (opmode      == 0x04 &&
        datamodul   == 0x00 &&
        bitrate_msb == 0x01 &&
        bitrate_lsb == 0x40 &&
        fdev_msb    == 0x03 &&
        fdev_lsb    == 0x33 &&
        frf_msb     == 0xE4 &&
        frf_mid     == 0xC0 &&
        frf_lsb     == 0x00 &&
        palevel     == 0x5F &&
        paramp      == 0x09 &&
        ocp         == 0x1A &&
        lna         == 0x00 &&
        rxbw        == 0x51 &&
        afcbw       == 0x49) {
        printf("\nRadio setup SUCCESSFUL!\n \n");
    } else {
        printf("\nRadio setup FAILED\n \n");
    }
}


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

    // ---------- Sync Word Configuration ----------
    rfm69_spi_write(REG_SYNC_CONFIG, 0x98); // Sync on, 4 bytes sync word, 0 bit tolerance
	rfm69_spi_write(REG_SYNCVALUE1, SYNC_WORD[0]);
	rfm69_spi_write(REG_SYNCVALUE2, SYNC_WORD[1]);
	rfm69_spi_write(REG_SYNCVALUE3, SYNC_WORD[2]);
	rfm69_spi_write(REG_SYNCVALUE4, SYNC_WORD[3]);

    rfm69_spi_write(REG_DATAMODUL, 0x00); // Packet mode, FSK, no shaping

    rfm69_spi_write(REG_BITRATEMSB, BR_100kb_MSB); // 100kbps bitrate
    rfm69_spi_write(REG_BITRATELSB, BR_100kb_LSB);

    rfm69_spi_write(REG_FDEVMSB, FDEV_50k_MSB); // 50kHz frequency deviation
    rfm69_spi_write(REG_FDEVLSB, FDEV_50k_LSB);

    rfm69_spi_write(REG_FRFMSB, FRF_915_MSB); // 915MHz frequency
    rfm69_spi_write(REG_FRFMID, FRF_915_MID);
    rfm69_spi_write(REG_FRFLSB, FRF_915_LSB); // Always write LSB last since freqnecy only updates on LSB write

    rfm69_spi_write(REG_RXBW, 0x4A); // Rx bandwidth 100kHz

    rfm69_spi_write(REG_PACKETCONFIG1, 0x90); // Fixed length packets, CRC on, discard bad packets, no address filtering

    rfm69_spi_write(REG_PACKETCONFIG2, 0x32); //InterPacketRxDelay 80us, auto RX restart on, AES encryption off

    rfm69_spi_write(REG_PARAMP, DEFAULT_PARAMP); // Set PA ramp-up time to default (40us)


}


void verify_radio_setup(void) {
    // read back and verify all configured registers are correct
    uint8_t mode = rfm69_spi_read(REG_OPMODE);

    uint8_t sync_config = rfm69_spi_read(REG_SYNC_CONFIG);
    uint8_t sync_val1   = rfm69_spi_read(REG_SYNCVALUE1);
	uint8_t sync_val2   = rfm69_spi_read(REG_SYNCVALUE2);
	uint8_t sync_val3   = rfm69_spi_read(REG_SYNCVALUE3);
	uint8_t sync_val4   = rfm69_spi_read(REG_SYNCVALUE4);

    uint8_t data_modul = rfm69_spi_read(REG_DATAMODUL);

    uint8_t bitrate_msb = rfm69_spi_read(REG_BITRATEMSB);
    uint8_t bitrate_lsb = rfm69_spi_read(REG_BITRATELSB);

    uint8_t fdev_msb = rfm69_spi_read(REG_FDEVMSB);
    uint8_t fdev_lsb = rfm69_spi_read(REG_FDEVLSB);

    uint8_t frf_msb = rfm69_spi_read(REG_FRFMSB);
    uint8_t frf_mid = rfm69_spi_read(REG_FRFMID);
    uint8_t frf_lsb = rfm69_spi_read(REG_FRFLSB);

    uint8_t rx_bw = rfm69_spi_read(REG_RXBW);

    uint8_t packet_config1 = rfm69_spi_read(REG_PACKETCONFIG1);

    uint8_t packet_config2 = rfm69_spi_read(REG_PACKETCONFIG2);

    uint8_t pa_ramp_time = rfm69_spi_read(REG_PARAMP);





    printf("RegOpMode     : 0x%02X (expected 0x04)\n", mode);

    printf("RegSyncConfig : 0x%02X (expected 0x98)\n", sync_config);
    printf("RegSyncValue1 : 0x%02X (expected 0xCA)\n", sync_val1);
    printf("RegSyncValue2 : 0x%02X (expected 0xFE)\n", sync_val2);
    printf("RegSyncValue3 : 0x%02X (expected 0xBA)\n", sync_val3);
    printf("RegSyncValue4 : 0x%02X (expected 0xBE)\n", sync_val4);

    printf("RegDataModul  : 0x%02X (expected 0x00)\n", data_modul);

    printf("RegBitrateMSB : 0x%02X (expected 0x01)\n", bitrate_msb);
    printf("RegBitrateLSB : 0x%02X (expected 0x40)\n", bitrate_lsb);

    printf("RegFdevMSB    : 0x%02X (expected 0x03)\n", fdev_msb);
    printf("RegFdevLSB    : 0x%02X (expected 0x33)\n", fdev_lsb);
	
    printf("RegFrfMSB     : 0x%02X (expected 0xE4)\n", frf_msb);
    printf("RegFrfMID     : 0x%02X (expected 0xC0)\n", frf_mid);
    printf("RegFrfLSB     : 0x%02X (expected 0x00)\n", frf_lsb);

    printf("RegRxBw       : 0x%02X (expected 0x4A)\n", rx_bw);

    printf("RegPacketConfig1 : 0x%02X (expected 0x90)\n", packet_config1);

    printf("RegPacketConfig2 : 0x%02X (expected 0x32)\n", packet_config2);

    printf("RegPaRamp       : 0x%02X (expected 0x09)\n", pa_ramp_time);

	if (mode == 0x04 && 
        sync_config == 0x98 &&
        sync_val1   == 0xCA &&
        sync_val2   == 0xFE &&
        sync_val3   == 0xBA &&
        sync_val4   == 0xBE &&
        data_modul  == 0x00 &&
        bitrate_msb == 0x01 &&
        bitrate_lsb == 0x40 &&
        fdev_msb    == 0x03 &&
        fdev_lsb    == 0x33 &&
        frf_msb     == 0xE4 &&
        frf_mid     == 0xC0 &&
        frf_lsb     == 0x00 &&
        rx_bw       == 0x4A &&
        packet_config1 == 0x90 &&
        packet_config2 == 0x32 &&
        pa_ramp_time == 0x09) {
        printf("\nRadio setup SUCCESSFUL!\n \n");
    } else {
        printf("\nRadio setup FAILED\n \n");
    }
}

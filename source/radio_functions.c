
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

    rfm69_spi_write(REG_AFCFEI, MY_AFCFEI);

    rfm69_spi_write(REG_DIOMAPPING1, MY_DIOMAPPING1_PACKETSENT);
    rfm69_spi_write(REG_DIOMAPPING2, MY_DIOMAPPING2);

    rfm69_spi_write(REG_RSSITHRESH, MY_RSSITHRESH);

    rfm69_spi_write(REG_RXTIMEOUT1, MY_RXTIMEOUT1);
    rfm69_spi_write(REG_RXTIMEOUT2, MY_RXTIMEOUT2);

    rfm69_spi_write(REG_PREAMBLEMSB, MY_PREAMBLE_MSB);
    rfm69_spi_write(REG_PREAMBLELSB, MY_PREAMBLE_LSB); // 4 preamble bytes


    
    rfm69_spi_write(REG_SYNC_CONFIG, MY_SYNC_CONFIG); // Sync on, 4 bytes sync word, 0 bit tolerance
	rfm69_spi_write(REG_SYNCVALUE1, SYNC_WORD[0]);
	rfm69_spi_write(REG_SYNCVALUE2, SYNC_WORD[1]);
	rfm69_spi_write(REG_SYNCVALUE3, SYNC_WORD[2]);
	rfm69_spi_write(REG_SYNCVALUE4, SYNC_WORD[3]);

    rfm69_spi_write(REG_PACKETCONFIG1, MY_PACKETCONFIG1); // Fixed length packets, CRC on, discard bad packets, no address filtering

    rfm69_spi_write(REG_PAYLOADLENGTH, MY_PAYLOAD_LENGTH); // Maximum packet length 64 bytes

    rfm69_spi_write(REG_FIFOTHRESH, MY_FIFOTHRESH); // Transmit if fifo not empty, threshhold = 0

    rfm69_spi_write(REG_PACKETCONFIG2, MY_PACKETCONFIG2); //InterPacketRxDelay 80us, auto RX restart on, AES encryption off

    rfm69_spi_write(REG_TESTDAGC, MY_TESTDAGC);


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

    uint8_t afcfei = rfm69_spi_read(REG_AFCFEI);

    uint8_t diomap1 = rfm69_spi_read(REG_DIOMAPPING1);
    uint8_t diomap2 = rfm69_spi_read(REG_DIOMAPPING2);

    uint8_t rssithresh = rfm69_spi_read(REG_RSSITHRESH);

    uint8_t rxtimeout1 = rfm69_spi_read(REG_RXTIMEOUT1);
    uint8_t rxtimeout2 = rfm69_spi_read(REG_RXTIMEOUT2);

    uint8_t preamblemsb = rfm69_spi_read(REG_PREAMBLEMSB);
    uint8_t preamblelsb = rfm69_spi_read(REG_PREAMBLELSB);

    uint8_t syncconfig = rfm69_spi_read(REG_SYNC_CONFIG);

    uint8_t syncvalue1 = rfm69_spi_read(REG_SYNCVALUE1);
    uint8_t syncvalue2 = rfm69_spi_read(REG_SYNCVALUE2);
    uint8_t syncvalue3 = rfm69_spi_read(REG_SYNCVALUE3);
    uint8_t syncvalue4 = rfm69_spi_read(REG_SYNCVALUE4);

    uint8_t packetcfg1 = rfm69_spi_read(REG_PACKETCONFIG1);
    
    uint8_t payloadlen = rfm69_spi_read(REG_PAYLOADLENGTH);

    uint8_t fifothresh = rfm69_spi_read(REG_FIFOTHRESH);

    uint8_t packetcfg2 = rfm69_spi_read(REG_PACKETCONFIG2);

    uint8_t testdagc = rfm69_spi_read(REG_TESTDAGC);



    printf("REG OPMODE      : 0x%02X   Expected : 0x04\n", opmode);
    printf("REG DATAMODUL   : 0x%02X   Expected : 0x00\n", datamodul);
    printf("REG BITRATE MSB : 0x%02X   Expected : 0x01\n", bitrate_msb);
    printf("REG BITRATE LSB : 0x%02X   Expected : 0x40\n", bitrate_lsb);
    printf("REG FDEV MSB    : 0x%02X   Expected : 0x03\n", fdev_msb);
    printf("REG FDEV LSB    : 0x%02X   Expected : 0x33\n", fdev_lsb);
    printf("REG FRF MSB     : 0x%02X   Expected : 0xE4\n", frf_msb);
    printf("REG FRF MID     : 0x%02X   Expected : 0xC0\n", frf_mid);
    printf("REG FRF LSB     : 0x%02X   Expected : 0x00\n", frf_lsb);
    printf("REG PALEVEL     : 0x%02X   Expected : 0x5F\n", palevel);
    printf("REG PARAMP      : 0x%02X   Expected : 0x09\n", paramp);
    printf("REG OCP         : 0x%02X   Expected : 0x1A\n", ocp);
    printf("REG LNA         : 0x%02X   Expected : 0b00XXX000 (unknown read only bits)\n", lna);
    printf("REG RXBW        : 0x%02X   Expected : 0x51\n", rxbw);
    printf("REG AFCBW       : 0x%02X   Expected : 0x49\n", afcbw);
    printf("REG AFCFEI      : 0x%02X   Expected : 0b000X1000 (unknown read only bit)\n", afcfei);
    printf("REG DIOMAPPING1 : 0x%02X   Expected : 0x00\n", diomap1);
    printf("REG DIOMAPPING2 : 0x%02X   Expected : 0x07\n", diomap2);
    printf("REG RSSITHRESH  : 0x%02X   Expected : 0xE4\n", rssithresh);
    printf("REG RXTIMEOUT1  : 0x%02X   Expected : 0x00\n", rxtimeout1);
    printf("REG RXTIMEOUT2  : 0x%02X   Expected : 0x00\n", rxtimeout2);
    printf("REG PREAMBLEMSB : 0x%02X   Expected : 0x00\n", preamblemsb);
    printf("REG PREAMBLELSB : 0x%02X   Expected : 0x04\n", preamblelsb);
    printf("REG SYNC_CONFIG : 0x%02X   Expected : 0x98\n", syncconfig);
    printf("REG SYNCVALUE1  : 0x%02X   Expected : 0xCA\n", syncvalue1);
    printf("REG SYNCVALUE2  : 0x%02X   Expected : 0xFE\n", syncvalue2);
    printf("REG SYNCVALUE3  : 0x%02X   Expected : 0xBA\n", syncvalue3);
    printf("REG SYNCVALUE4  : 0x%02X   Expected : 0xBE\n", syncvalue4);
    printf("REG PACKETCFG1  : 0x%02X   Expected : 0x90\n", packetcfg1);
    printf("REG PAYLOADLEN  : 0x%02X   Expected : 0x40\n", payloadlen);
    printf("REG FIFOTHRESH  : 0x%02X   Expected : 0x80\n", fifothresh);
    printf("REG PACKETCFG2  : 0x%02X   Expected : 0x32\n", packetcfg2);
    printf("REG TESTDAGC    : 0x%02X   Expected : 0x30\n", testdagc);



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
        (lna & 0xC7)        == 0x00 && // Bits 5-3 are read only (LnaCurrentGain) will most likley read 001. Masking those out
        rxbw        == 0x51 &&
        afcbw       == 0x49 &&
        (afcfei & 0xEF)     == 0x04 && // Bit 4 is read only (AFCDone), will most likley read 1 (AFC is not ongoing/has finished). Masking that out
        diomap1     == 0x00 &&
        diomap2     == 0x07 &&
        rssithresh  == 0xE4 &&
        rxtimeout1  == 0x00 &&
        rxtimeout2  == 0x00 &&
        preamblemsb == 0x00 &&
        preamblelsb == 0x04 &&
        syncconfig  == 0x98 &&
        syncvalue1  == 0xCA &&
        syncvalue2  == 0xFE &&
        syncvalue3  == 0xBA &&
        syncvalue4  == 0xBE &&
        packetcfg1  == 0x90 &&
        payloadlen  == 0x40 &&
        fifothresh  == 0x80 &&
        packetcfg2  == 0x32 &&
        testdagc    == 0x30) {
        printf("\nRadio setup SUCCESSFUL!\n \n");
    } else {
        printf("\nRadio setup FAILED\n \n");
    }
}

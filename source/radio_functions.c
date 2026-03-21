
//////////////////////////////////////
//             INCLUDES             //
//////////////////////////////////////

// ====== Standard Headers / Libraries ======
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <stdio.h> // For verify_radio_setup
#include <string.h>


// ========== User defined headers ==========
#include "radio_functions.h"
#include "peripheral_setup.h" // For GPIO pin definitions
#include "radio_registers.h" // For register definitions and bit masks



//////////////////////////////////////
//             VARIABLES            //
//////////////////////////////////////

const uint8_t SYNC_WORD[4] = {0xCA, 0xFE, 0xBA, 0xBE}; // Sync word to use for the radio




//////////////////////////////////////
//       FUNCTION DEFINITIONS       //
//////////////////////////////////////

// "reg" is the register you're writing too, and "value" is what you want to write to that register
void rfm69_spi_write(uint8_t reg, uint8_t value) {
    gpio_put(SPI_CS, 0);             // CS low to select the chip
    uint8_t buf[2] = { reg | 0x80, value }; // MSB of reg = 1 for write
    spi_write_blocking(spi0, buf, 2);
    gpio_put(SPI_CS, 1);             // CS high to finish
}

// "reg" is the register you're reading from, and the return "value" is the data read from that register
uint8_t rfm69_spi_read(uint8_t reg) {
    gpio_put(SPI_CS, 0);             // CS low to select the chip
    uint8_t out_buf[2] = { reg & 0x7F, 0x00 }; // MSB of reg = 0 for read
    uint8_t in_buf[2] = { 0 };
    spi_write_read_blocking(spi0, out_buf, in_buf, 2);
    gpio_put(SPI_CS, 1);             // CS high to finish / deselect the chip
    return in_buf[1];                // Second byte from the input buffer is the data from the register
}


// Shouldn't need to reset the radio on every boot, but this is useful for testing and ensures a known state
// This function resets all the registers to their default values
void rfm69_reset(void) {
    gpio_put(RADIO_RST, 1);
    sleep_us(100); // Datasheet specified (page 76)
    gpio_put(RADIO_RST, 0);
    sleep_ms(5); // Ready after 5ms according to datasheet
}


// Debugging function
void rfm69_check_version(void) {
    uint8_t version = rfm69_spi_read(REG_VERSION); 

    printf("RFM69 version: 0x%02X\n", version);

    if (version != 0x24) {
        printf("SPI communication failed!\n");
    } else {
        printf("SPI OK\n");
    }
}

void rfm69_setup(void) {

    // Set to standby mode (if not already) to allow writing to registers
    rfm69_spi_write(REG_OPMODE, MODE_STANDBY); 

    // Packet mode, FSK (frequency shift keying) (modulation type), no moduation shaping
    rfm69_spi_write(REG_DATAMODUL, MY_DATAMODUL); 

    // 100kbps bitrate
    rfm69_spi_write(REG_BITRATEMSB, BR_100kb_MSB); 
    rfm69_spi_write(REG_BITRATELSB, BR_100kb_LSB);

    // 50kHz frequency deviation
    rfm69_spi_write(REG_FDEVMSB, FDEV_50k_MSB);
    rfm69_spi_write(REG_FDEVLSB, FDEV_50k_LSB);

    // 915MHz frequency. Always write LSB last since freqnecy only updates on LSB write
    rfm69_spi_write(REG_FRFMSB, FRF_915_MSB); 
    rfm69_spi_write(REG_FRFMID, FRF_915_MID);
    rfm69_spi_write(REG_FRFLSB, FRF_915_LSB); 

    // PA1 on, PA2 off, output power = +13dBm (PA = Power Amplifier)
    rfm69_spi_write(REG_PALEVEL, MY_TX_POWER);

    // Set PA ramp-up time to default (40us)
    rfm69_spi_write(REG_PARAMP, MY_PARAMP);

    // OCP (over current protection) on, 95mA limit
    rfm69_spi_write(REG_OCP, MY_OCP); 

    // 50 ohm input impedance, ACG on
    rfm69_spi_write(REG_LNA, MY_LNA_RX_POWER);

    // Rx bandwidth 167kHz
    rfm69_spi_write(REG_RXBW, MY_RXBW);

    // AFC (automatic frequency correction) bandwidth 200kHz, wider than RxBw for better AFC
    rfm69_spi_write(REG_AFCBW, MY_AFCBW); 

    // Always perform AFC when entering RX
    rfm69_spi_write(REG_AFCFEI, MY_AFCFEI);

    // Set G0 IRQ to Packet Sent
    rfm69_spi_write(REG_DIOMAPPING1, MY_DIOMAPPING1_PACKETSENT);
    rfm69_spi_write(REG_DIOMAPPING2, MY_DIOMAPPING2);

    // Set RX minimum signal strength detection to -114dBm
    rfm69_spi_write(REG_RSSITHRESH, MY_RSSITHRESH);

    // Disabling RX hardware timer timeout
    rfm69_spi_write(REG_RXTIMEOUT1, MY_RXTIMEOUT1);
    rfm69_spi_write(REG_RXTIMEOUT2, MY_RXTIMEOUT2);

    // Set preamble length to 8 bytes
    rfm69_spi_write(REG_PREAMBLEMSB, MY_PREAMBLE_MSB);
    rfm69_spi_write(REG_PREAMBLELSB, MY_PREAMBLE_LSB);


    // Sync on, 4 bytes sync word, 0 bit tolerance
    rfm69_spi_write(REG_SYNC_CONFIG, MY_SYNC_CONFIG);

    // Write sync word as 0xCAFEBABE
	rfm69_spi_write(REG_SYNCVALUE1, SYNC_WORD[0]);
	rfm69_spi_write(REG_SYNCVALUE2, SYNC_WORD[1]);
	rfm69_spi_write(REG_SYNCVALUE3, SYNC_WORD[2]);
	rfm69_spi_write(REG_SYNCVALUE4, SYNC_WORD[3]);

    // Variable length packets, CRC on, discard bad packets, no address filtering
    rfm69_spi_write(REG_PACKETCONFIG1, MY_PACKETCONFIG1);

    // Maximum packet length 64 bytes
    rfm69_spi_write(REG_PAYLOADLENGTH, MY_PAYLOAD_LENGTH);

    // In TX mode: transmit if fifo not empty, threshhold = 0
    rfm69_spi_write(REG_FIFOTHRESH, MY_FIFOTHRESH); 

    // InterPacketRxDelay = 80us, auto RX restart on, AES encryption off
    rfm69_spi_write(REG_PACKETCONFIG2, MY_PACKETCONFIG2);

    // DAGC (digital automatic gain control) on, normal modulation index
    rfm69_spi_write(REG_TESTDAGC, MY_TESTDAGC);
}


void rfm69_verify_setup(void) {
    // read back and verify all configured registers are correct

    uint8_t opmode = rfm69_spi_read(REG_OPMODE); 

    uint8_t datamodul = rfm69_spi_read(REG_DATAMODUL); 

    uint8_t bitrate_msb = rfm69_spi_read(REG_BITRATEMSB); 
    uint8_t bitrate_lsb = rfm69_spi_read(REG_BITRATELSB);

    uint8_t fdev_msb = rfm69_spi_read(REG_FDEVMSB); 
    uint8_t fdev_lsb = rfm69_spi_read(REG_FDEVLSB);

    uint8_t frf_msb = rfm69_spi_read(REG_FRFMSB); 
    uint8_t frf_mid = rfm69_spi_read(REG_FRFMID);
    uint8_t frf_lsb = rfm69_spi_read(REG_FRFLSB);

    uint8_t palevel = rfm69_spi_read(REG_PALEVEL); 

    uint8_t paramp = rfm69_spi_read(REG_PARAMP); 

    uint8_t ocp = rfm69_spi_read(REG_OCP); 

    uint8_t lna = rfm69_spi_read(REG_LNA); 

    uint8_t rxbw = rfm69_spi_read(REG_RXBW); 

    uint8_t afcbw = rfm69_spi_read(REG_AFCBW); 

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
        preamblelsb == 0x08 &&
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



void rfm69_write_fifo(uint8_t *payload, uint8_t length) {
    rfm69_spi_write(REG_FIFO, length); // Writing the length of the payload (not including this byte) as the first byte
    
    for (uint8_t i = 0; i < length; i++) {
        rfm69_spi_write(REG_FIFO, payload[i]); // Writing the rest of the data
    }
}

void rfm69_verify_fifo(uint8_t *payload, uint8_t length) {
    // read back and verify - NOTE: reading FIFO is destructive
    // bytes are consumed on read, FIFO will be empty after this function
    // do not call this in normal TX flow, development verification only
    bool success = true;

    for (uint8_t i = 0; i < length; i++) {
        uint8_t readback = rfm69_spi_read(REG_FIFO);
        printf("FIFO[%d]: wrote 0x%02X  read 0x%02X\n", i, payload[i], readback);
        if (payload[i] != readback) success = false;
    }

    printf(success ? "\nFIFO verify SUCCESSFUL\n\n" : "\nFIFO verify FAILED\n\n");
}


void rfm69_set_standby(void) {
    rfm69_spi_write(REG_OPMODE, MODE_STANDBY);
}

void rfm69_set_rx(void) {
    rfm69_spi_write(REG_OPMODE, MODE_RX);
}

void rfm69_set_tx(void) {
    rfm69_spi_write(REG_OPMODE, MODE_TX);
}

void rfm69_set_G0_packet_sent(void){
    rfm69_spi_write(REG_DIOMAPPING1, MY_DIOMAPPING1_PACKETSENT);
}

void rfm69_set_G0_packet_received(void){
    rfm69_spi_write(REG_DIOMAPPING1, MY_DIOMAPPING1_PAYLOADREADY);
}


void rfm69_read_packet(uint8_t *buffer){
    uint8_t length = rfm69_spi_read(REG_FIFO);

    buffer[0] = length;

    for (uint8_t i = 0; i < length; i++) {
        buffer[i+1] = rfm69_spi_read(REG_FIFO);
    }
}
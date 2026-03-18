
//////////////////////////////////////
//             INCLUDES             //
//////////////////////////////////////

// Standard includes / hardware headers
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdio.h>
#include <string.h>

// User defined headers
#include "display_functions.h"
#include "display_font.h"



//////////////////////////////////////
//       FUNCTION DEFINITIONS       //
//////////////////////////////////////

// Bonus function in here for now
void stdio_send_ds4_outputs(struct bt_hid_state* state)
{
	printf("buttons: %04x, l: %d,%d, r: %d,%d, l2,r2: %d,%d hat: %d\n",
				state->buttons, state->lx, state->ly, state->rx, state->ry,
				state->l2, state->r2, state->hat);
}



void SSD1306_send_command(uint8_t cmd) {

    // first byte of the buffer is 0x00 for the control byte (signifying a command)
    // Bit 7	= Co (Continuation)     Co = 0 → last byte in a transmission (or just sending one command)
    //                                  Co = 1 → more bytes to follow (for sending multiple commands in one I²C write)
    // Bit 6	= D/C (Data/Command)    D/C = 0 → the next byte is a command
    //                                  D/C = 1 → the next byte is data
    // Bit 5-0	 = ignored, must be 0
    uint8_t buf[2] = {0x00, cmd};

    // i2c_write_blocking sends a byte, taking care of start and stop conditions
    // argument 1: which i2c peripheral 
    // argument 2: device adress
    // argument 3: pointer to the buffer of bytes you want to send
    // argument 4: number of bytes in the array
    // argument 5: false at the end means a stop condition will be sent
    // returns number of bytes written, should be 2
    
    int ret = i2c_write_blocking(i2c0, 0x3C, buf, 2, false);

    /* if ret does not return 2 (2 bytes written), there was an error
    if (ret != 2) {
        gpio_put(25, true);
    }
    */

}



void SSD1306_send_data(uint8_t data) {
    // see notes in send_i2c_command
    uint8_t buf[2] = {0x40, data};

    int ret = i2c_write_blocking(i2c0, 0x3C, buf, 2, false);

    /* if ret does not return 2 (2 bytes written), there was an error
    if (ret != 2) {
        gpio_put(25, true);
    }
    */
}



void SSD1306_set_col_addr(uint8_t start, uint8_t end) {
    SSD1306_send_command(0x21); // Column address command
    SSD1306_send_command(start);
    SSD1306_send_command(end);
}



void SSD1306_set_page_addr(uint8_t start, uint8_t end) {
    SSD1306_send_command(0x22); // Page address command
    SSD1306_send_command(start);
    SSD1306_send_command(end);
}



void SSD1306_clear(void) {
    // Setting window to the entire screen
    SSD1306_set_col_addr(0, 127);   
	SSD1306_set_page_addr(0, 7);

    // writting 0x00 to all locations clears the display
    for (int i = 0; i < 128 * 8; i++) { // 128 columns * 8 pages
        SSD1306_send_data(0x00);
    }
}



void SSD1306_init(void){
    
    SSD1306_send_command(0xAE); // Display OFF

    SSD1306_send_command(0xA8); // Set MUX Ratio
    SSD1306_send_command(0x3F); // Sets to 63

    SSD1306_send_command(0xD3); // Set display offset
    SSD1306_send_command(0x00); // No offset

    SSD1306_send_command(0x40); // Set display start line

    SSD1306_send_command(0xA1); // Column address mapped to 127 // Left to right

    SSD1306_send_command(0xC8); // COM output scan direction // Start top

    SSD1306_send_command(0xDA); // COM pin hardware configuration
    SSD1306_send_command(0x12); // 128x64 configuration

    SSD1306_send_command(0x81); // Contrast control
    SSD1306_send_command(0x7F);

    SSD1306_send_command(0xA4); // Entire display ON (resume RAM)

    SSD1306_send_command(0xA6); // Normal display mode

    SSD1306_send_command(0xD5); // Set oscolation frequency
    SSD1306_send_command(0x80);

    SSD1306_send_command(0x8D); // Set charge pump
    SSD1306_send_command(0x14); // Enable charge pump

    SSD1306_send_command(0x20); // Set memory addressing mode
    SSD1306_send_command(0x00); // Horizontal addressing

    SSD1306_send_command(0xAF); // Display ON
}


// Obtained from microcontrollers class
void SSD1306_display_trine_logo(void){
	SSD1306_set_col_addr(14, 113);
	SSD1306_set_page_addr(2, 5);
	for (uint16_t i = 0; i<300; i++){
		SSD1306_send_data(Trine_Logo[i]);
	}
    SSD1306_set_col_addr(14, 113);
	SSD1306_set_page_addr(5, 5);
    for (uint8_t i = 0; i<100; i++){
        SSD1306_send_data(University_Logo[i]);
    }
}


// The small characters are 5x8 pixels (one page tall)
void SSD1306_send_small_char(int character, uint8_t col, uint8_t page){
    uint8_t index = (uint8_t)character;
    SSD1306_set_col_addr(col, col+5);
    SSD1306_set_page_addr(page, page);
    for (uint8_t i = 0; i<5; i++){
        SSD1306_send_data(font_8p[index][i]);
    }
}



// The big characters are 8x16 pixels (two pages tall)
void SSD1306_send_big_char(int character, uint8_t col, uint8_t page){
    uint8_t index = (uint8_t)character;
    SSD1306_set_col_addr(col, col+7);
    SSD1306_set_page_addr(page, page+1);
    for (uint8_t i = 0; i<16; i++){
        SSD1306_send_data(font_16p[index][i]);
    }

}


// The PlayStation symbols are 16x16 pixels (two pages tall)
void SSD1306_send_PS_symbol(int character, uint8_t col, uint8_t page){
    uint8_t index = 0;

    if (character == 'X'){
        index == 0;
    } else if (character == 'S'){
        index = 1;
    } else if (character == 'O'){
        index = 2;
    } else if (character == 'T'){
        index = 3;
    } else {
        index = 4;
    }

    SSD1306_set_col_addr(col, col+15);
    SSD1306_set_page_addr(page, page+1);
    for (uint8_t i = 0; i<32; i++){
        SSD1306_send_data(PS_symbols[index][i]);
    }
}


void SSD1306_display_all_fonts(void){
    SSD1306_send_small_char('A', 0, 0);
    SSD1306_send_small_char('B', 7, 0);
    SSD1306_send_small_char('C', 14, 0);
    SSD1306_send_small_char('D', 21, 0);
    SSD1306_send_small_char('E', 28, 0);
    SSD1306_send_small_char('F', 35, 0);
    SSD1306_send_small_char('G', 42, 0);
    SSD1306_send_small_char('H', 49, 0);
    SSD1306_send_small_char('I', 56, 0);
    SSD1306_send_small_char('J', 63, 0);
    SSD1306_send_small_char('K', 70, 0);
    SSD1306_send_small_char('L', 77, 0);
    SSD1306_send_small_char('M', 84, 0);
    SSD1306_send_small_char('N', 91, 0);
    SSD1306_send_small_char('O', 98, 0);
    SSD1306_send_small_char('P', 105, 0);
    SSD1306_send_small_char('Q', 112, 0);
    SSD1306_send_small_char('R', 119, 0);
    SSD1306_send_small_char('S', 0, 1);
    SSD1306_send_small_char('T', 7, 1);
    SSD1306_send_small_char('U', 14, 1);
    SSD1306_send_small_char('V', 21, 1);
    SSD1306_send_small_char('W', 28, 1);
    SSD1306_send_small_char('X', 35, 1);
    SSD1306_send_small_char('Y', 42, 1);
    SSD1306_send_small_char('Z', 49, 1);
    SSD1306_send_small_char('0', 56, 1);
    SSD1306_send_small_char('1', 63, 1);
    SSD1306_send_small_char('2', 70, 1);
    SSD1306_send_small_char('3', 77, 1);
    SSD1306_send_small_char('4', 84, 1);
    SSD1306_send_small_char('5', 91, 1);
    SSD1306_send_small_char('6', 98, 1);
    SSD1306_send_small_char('7', 105, 1);
    SSD1306_send_small_char('8', 112, 1);
    SSD1306_send_small_char('9', 119, 1);


    SSD1306_send_big_char('A', 0, 2);
    SSD1306_send_big_char('B', 9, 2);
    SSD1306_send_big_char('C', 18, 2);
    SSD1306_send_big_char('D', 27, 2);
    SSD1306_send_big_char('E', 36, 2);
    SSD1306_send_big_char('F', 45, 2);
    SSD1306_send_big_char('G', 54, 2);
    SSD1306_send_big_char('H', 63, 2);
    SSD1306_send_big_char('I', 72, 2);
    SSD1306_send_big_char('J', 81, 2);
    SSD1306_send_big_char('K', 90, 2);
    SSD1306_send_big_char('L', 99, 2);
    SSD1306_send_big_char('M', 108, 2);
    SSD1306_send_big_char('N', 117, 2);
    SSD1306_send_big_char('O', 0, 4);
    SSD1306_send_big_char('P', 9, 4);
    SSD1306_send_big_char('Q', 18, 4);
    SSD1306_send_big_char('R', 27, 4);
    SSD1306_send_big_char('S', 36, 4);
    SSD1306_send_big_char('T', 45, 4);
    SSD1306_send_big_char('U', 54, 4);
    SSD1306_send_big_char('V', 63, 4);
    SSD1306_send_big_char('W', 72, 4);
    SSD1306_send_big_char('X', 81, 4);
    SSD1306_send_big_char('Y', 90, 4);
    SSD1306_send_big_char('Z', 99, 4);
    SSD1306_send_big_char('0', 107, 4);
    SSD1306_send_big_char('1', 114, 4);
    SSD1306_send_big_char('2', 120, 4);

    SSD1306_send_big_char('3', 0, 6);
    SSD1306_send_big_char('4', 8, 6);
    SSD1306_send_big_char('5', 16, 6);
    SSD1306_send_big_char('6', 24, 6);
    SSD1306_send_big_char('7', 32, 6);
    SSD1306_send_big_char('8', 40, 6);
    SSD1306_send_big_char(':', 54, 6);
    SSD1306_send_big_char('9', 48, 6);

    SSD1306_send_PS_symbol('X', 61, 6);
    SSD1306_send_PS_symbol('S', 78, 6);
    SSD1306_send_PS_symbol('O', 95, 6);
    SSD1306_send_PS_symbol('T', 112, 6);
}


void SSD1306_UI_setup(void){
    SSD1306_send_big_char('R', 8, 0);
    SSD1306_send_big_char('U', 17, 0);
    SSD1306_send_big_char('N', 26, 0);
    SSD1306_send_big_char('N', 35, 0);
    SSD1306_send_big_char('I', 44, 0);  
    SSD1306_send_big_char('N', 53, 0);  
    SSD1306_send_big_char('G', 62, 0);
    SSD1306_send_big_char(' ', 71, 0);
    SSD1306_send_big_char('B', 80, 0);  
    SSD1306_send_big_char('A', 89, 0);  
    SSD1306_send_big_char('C', 98, 0);  
    SSD1306_send_big_char('K', 107, 0); 
    
    SSD1306_send_big_char('R', 0, 2); 
    SSD1306_send_big_char('S', 9, 2); 
    SSD1306_send_big_char('S', 18, 2); 
    SSD1306_send_big_char('I', 27, 2); 
    SSD1306_send_big_char(':', 36, 2); 

    SSD1306_send_big_char('-', 48, 2); 
    SSD1306_send_big_char(' ', 57, 2); // RSSI TENS 57, 2
    SSD1306_send_big_char(' ', 66, 2); // RSSI ONES 66, 2
    
    SSD1306_send_big_char('d', 84, 2); 
    SSD1306_send_big_char('B', 93, 2); 
    SSD1306_send_big_char('m', 102, 2); 


    SSD1306_send_big_char('P', 0, 4); 
    SSD1306_send_big_char(' ', 9, 4); 
    SSD1306_send_big_char('L', 18, 4); 
    SSD1306_send_big_char('O', 27, 4); 
    SSD1306_send_big_char('S', 36, 4); 
    SSD1306_send_big_char('S', 45, 4); 
    SSD1306_send_big_char(':', 54, 4); 
    
    SSD1306_send_big_char(' ', 67, 4); // PKT LOSS TENS AT 67, 4
    SSD1306_send_big_char(' ', 76, 4); // PKT LOSS ONES AT 76, 4

    SSD1306_send_big_char('p', 90, 4); 
    SSD1306_send_big_char('k', 99, 4); 
    SSD1306_send_big_char('t', 108, 4);
    SSD1306_send_big_char('s', 117, 4); 
    

    SSD1306_send_big_char('B', 0, 6); 
    SSD1306_send_big_char('A', 9, 6); 
    SSD1306_send_big_char('T', 18, 6); 
    SSD1306_send_big_char('T', 27, 6); 
    SSD1306_send_big_char(':', 36, 6); 
     
    SSD1306_send_big_char(' ', 50, 6); // BATTERY TENS AT 50, 6
    SSD1306_send_big_char(' ', 59, 6); // BATTERY ONES AT 59, 6
    SSD1306_send_big_char('.', 68, 6); 
    SSD1306_send_big_char(' ', 77, 6); // BATTERY TENTH AT 77, 6

    SSD1306_send_big_char('V', 88, 6); 
    
}




void SSD1306_update(uint8_t rssi, uint8_t pkt_loss, uint8_t batt){
    
    // Variables for storing previous state, preventing unnecessary updates to the display
    static uint8_t rssi_prev; 
    static uint8_t pkt_loss_prev;
    static uint8_t batt_prev;


    // --------------- RSSI ---------------
    uint8_t rssi_dbm = rssi/2; // RSSI function according to the datasheet
    if (rssi_dbm != rssi_prev){
        // Calculations
        uint8_t rssi_tens = (rssi_dbm / 10) % 10;
        uint8_t rssi_ones = rssi_dbm % 10;
        // Update screen 
        SSD1306_send_big_char(rssi_tens, 57, 2);
        SSD1306_send_big_char(rssi_ones, 66, 2);
        // Update previous value
        rssi_prev = rssi_dbm;
    }
    

    // ------------ Packet Loss ------------
    if (pkt_loss != pkt_loss_prev){
        // Calculations
        uint8_t pkt_loss_tens = (pkt_loss / 10) % 10;
        uint8_t pkt_loss_ones = pkt_loss % 10;
        // Update screen
        if(pkt_loss_tens == 0){
            SSD1306_send_big_char(pkt_loss_ones, 72, 4);
        }
        else{
            SSD1306_send_big_char(pkt_loss_tens, 67, 4);
            SSD1306_send_big_char(pkt_loss_ones, 76, 4);
        }
        // Update previous value
        pkt_loss_prev = pkt_loss;
    }


    if (batt != batt_prev){
        // Calculations (battery voltage comes in as a 3 digit number (xyz) scaled up by 10 to represent (xy.z))
        uint8_t batt_tens  = (batt / 100) % 10;
        uint8_t batt_ones  = (batt / 10) % 10;
        uint8_t batt_tenth = batt % 10;
        // Update screen
        SSD1306_send_big_char(batt_tens, 50, 6);
        SSD1306_send_big_char(batt_ones, 59, 6); 
        SSD1306_send_big_char(batt_tenth, 77, 6); 
        // Update previous value
        batt_prev = batt;
    }
}


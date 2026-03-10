
// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2023 Brian Starkey <stark3y@gmail.com>


//////////////////////////////////////
//             INCLUDES             //
//////////////////////////////////////

// Standard Headers / Libraries
#include <stdio.h>
#include <string.h>
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "bt_hid.h"

// User defined headers
#include "peripheral_setup.h" // for all peripheral initialization
#include "display_functions.h" // for the I2C display
#include "radio_functions.h" // for the RFM69 radio functions
#include "radio_registers.h" // for the RFM69 register definitions


void rfm69_init_w_display(void){
	
	send_small_char('R', 0, 0);
	send_small_char('F', 6, 0);
	send_small_char('M', 12, 0);
	send_small_char('6', 18, 0);
	send_small_char('9', 24, 0);
	send_small_char(' ', 30, 0);
	send_small_char('L', 36, 0);
	send_small_char('O', 42, 0);
	send_small_char('G', 48, 0);
	send_small_char(':', 54, 0);

	sleep_ms(1000);

	// Configure all important registers and verify
	radio_setup();
	verify_radio_setup();
	send_small_char('S', 0, 1);
	send_small_char('E', 6, 1);
	send_small_char('T', 12, 1);
	send_small_char('U', 18, 1);
	send_small_char('P', 24, 1);
	send_small_char(' ', 30, 1);
	send_small_char(47,  36, 1);  // checkmark pt1
	send_small_char(39,  41, 1);  // checkmark pt2

	

	sleep_ms(2000);

	uint8_t payload[8] = {0x01, 0x02, 0xDE, 0xAD, 0xBE, 0xEF, 0x03, 0x04};

	// need to fill fifo in standby mode
	rfm69_set_standby(); 
	send_small_char('S', 0, 2);
	send_small_char('E', 6, 2);
	send_small_char('T', 12, 2);
	send_small_char(' ', 18, 2);
	send_small_char('M', 24, 2);
	send_small_char('O', 30, 2);
	send_small_char('D', 36, 2);
	send_small_char('E', 42, 2);
	send_small_char(' ', 48, 2);
	send_small_char('S', 54, 2);
	send_small_char('T', 60, 2);
	send_small_char('A', 66, 2);
	send_small_char('N', 72, 2);
	send_small_char('D', 78, 2);
	send_small_char('B', 84, 2);
	send_small_char('Y', 90, 2);
	send_small_char(' ', 96, 2);
	send_small_char(47,  102, 2);  // checkmark pt1
	send_small_char(39,  107, 2);  // checkmark pt2

	sleep_ms(2000);

	rfm69_write_fifo(payload, 8);
	rfm69_verify_fifo(payload, 8);

	send_small_char('F', 0, 3);
	send_small_char('I', 6, 3);
	send_small_char('F', 12, 3);
	send_small_char('O', 18, 3);
	send_small_char(' ', 24, 3);
	send_small_char('W', 30, 3);
	send_small_char('R', 36, 3);
	send_small_char('I', 42, 3);
	send_small_char('T', 48, 3);
	send_small_char('E', 54, 3);
	send_small_char('+', 60, 3);
	send_small_char('V', 66, 3);
	send_small_char('E', 72, 3);
	send_small_char('R', 78, 3);
	send_small_char('I', 84, 3);
	send_small_char('F', 90, 3);
	send_small_char('Y', 96, 3);
	send_small_char(' ', 102, 3);  
	send_small_char(47,  108, 3);  // checkmark pt1
	send_small_char(39,  113, 3);  // checkmark pt2

	sleep_ms(2000);

	rfm69_write_fifo(payload, 8);
	rfm69_set_tx();
	send_small_char('S', 0, 4);
	send_small_char('E', 6, 4);
	send_small_char('T', 12, 4);
	send_small_char(' ', 18, 4);
	send_small_char('M', 24, 4);
	send_small_char('O', 30, 4);
	send_small_char('D', 36, 4);
	send_small_char('E', 42, 4);
	send_small_char(' ', 48, 4);
	send_small_char('T', 54, 4);
	send_small_char('X', 60, 4);
	send_small_char(' ', 66, 4);  
	send_small_char(47,  72, 4);  // checkmark pt1
	send_small_char(39,  77, 4);  // checkmark pt2

	sleep_ms(10);

	uint8_t G0 = gpio_get(RADIO_IRQ);

	if (G0 == 1){
		send_small_char('G', 0, 7);
		send_small_char('0', 7, 7);
	}
	else{
		send_small_char(' ', 0, 7);
		send_small_char(' ', 7, 7);
	}

	sleep_ms(2000);

	rfm69_set_rx();
	send_small_char('S', 0, 5);
	send_small_char('E', 6, 5);
	send_small_char('T', 12, 5);
	send_small_char(' ', 18, 5);
	send_small_char('M', 24, 5);
	send_small_char('O', 30, 5);
	send_small_char('D', 36, 5);
	send_small_char('E', 42, 5);
	send_small_char(' ', 48, 5);
	send_small_char('R', 54, 5);
	send_small_char('X', 60, 5);
	send_small_char(' ', 66, 5);  
	send_small_char(47,  72, 5);  // checkmark pt1
	send_small_char(39,  77, 5);  // checkmark pt2

}






//////////////////////////////////////
//           MAIN FUNCTION          //
//////////////////////////////////////

void main(void){

	// Structure used to hold the Bluetooth controller data
	struct bt_hid_state ds4_state = {0};

	// Initializes all configured standard I/O interfaces (USB serial in our case)
	stdio_init_all();
	
	multicore_launch_core1(bt_main); // Launch the second core to run the Bluetooth HID code
	                                 // This will populate the controller state struct with the latest data from the DualShock4
	// Wait for init
	sleep_ms(1000);


	GPIO_setup(); // Setup the GPIO pins for the DIP switches and the simple cycle pin
	i2c_setup(); // Setup the I2C peripheral for the display
	spi_setup(); // Setup the SPI peripheral for the RFM69 radio

	
	// I2C Screen setup
	screen_init(); // Initialize the I2C screen with predefined commands
	clear_display();
	display_trine_logo(); // Show the Trine logo on the display at startup
	sleep_ms(5000);
	clear_display();

	radio_setup();
	verify_radio_setup();
	rfm69_set_standby();

	//uint8_t G0 = gpio_get(RADIO_IRQ);
	bool test_in = 0;
	bool test_in_prev = 0;
	uint8_t version;
	uint8_t sync_word = 0;
	uint8_t payload[8] = {0x01, 0x02, 0xDE, 0xAD, 0xBE, 0xEF, 0x03, 0x04};
	
	

	//clear_display();
	//ds4_inputs_display_setup(); // Setup the display to show the controller inputs in a nice format

	
	while (1) {

		//gpio_put(CYCLE, !gpio_get(CYCLE)); // Toggle the simple cycle pin for debugging purposes

		bt_hid_get_latest(&ds4_state); // Aquire latest Bluetooth controller state

		sleep_ms(100);

		test_in_prev = test_in;
		test_in = gpio_get(TEST);

		uint8_t dip_1 = !gpio_get(DIP1);
		uint8_t dip_2 = !gpio_get(DIP2);
		uint8_t dip_3 = !gpio_get(DIP3);
		uint8_t dip_4 = !gpio_get(DIP4);

		
		

		if (dip_1 && test_in == true && test_in_prev == false){
			send_small_char('R', 0, 1);
			send_small_char('E', 6, 1);
			send_small_char('A', 12, 1);
			send_small_char('D', 18, 1);
			send_small_char(' ', 24, 1);
			send_small_char('V', 30, 1);
			send_small_char('E', 36, 1);
			send_small_char('R', 42, 1);
			send_small_char('S', 48, 1);
			send_small_char('I', 54, 1);
			send_small_char('O', 60, 1);
			send_small_char('N', 66, 1);  
			version = rfm69_spi_read(REG_VERSION);
			send_small_char('R', 0, 2);
			send_small_char('E', 6, 2);
			send_small_char('T', 12, 2);
			send_small_char('U', 18, 2);
			send_small_char('R', 24, 2);
			send_small_char('N', 30, 2);
			send_small_char('E', 36, 2);
			send_small_char('D', 42, 2);
			send_small_char(':', 48, 2);
			send_small_char(' ', 54, 2);
			send_small_char('0', 60, 2);
			send_small_char('X', 66, 2);
			send_small_char(((version & 0xF0)>>4), 72, 2);
			send_small_char((version & 0x0F), 78, 2);
			sleep_ms(2000);
			version = 0;
			clear_display();

		}


		if (dip_2 && test_in == true && test_in_prev == false){
			sync_word = rfm69_spi_read(REG_SYNCVALUE1);
			printf("REG SYNCVALUE1  : 0x%02X\n", sync_word);
			send_small_char('C', 0, 1);
			send_small_char('U', 6, 1);
			send_small_char('R', 12, 1);
			send_small_char(' ', 18, 1);
			send_small_char('S', 24, 1);
			send_small_char('Y', 30, 1);
			send_small_char('N', 36, 1);
			send_small_char('C', 42, 1);
			send_small_char(' ', 48, 1);
			send_small_char('W', 54, 1);
			send_small_char('O', 60, 1);
			send_small_char('R', 66, 1);
			send_small_char('D', 72, 1);
			send_small_char(':', 88, 1);
			send_small_char(' ', 94, 1);
			send_small_char('0', 100, 1);
			send_small_char('X', 106, 1);
			send_small_char(((sync_word & 0xF0)>>4), 112, 1);
			send_small_char((sync_word & 0x0F), 118, 1);  


			rfm69_spi_write(REG_SYNCVALUE1, 0x29);
			send_small_char('W', 0, 2);
			send_small_char('R', 6, 2);
			send_small_char('I', 12, 2);
			send_small_char('T', 18, 2);
			send_small_char('E', 24, 2);
			send_small_char(' ', 30, 2);
			send_small_char('0', 36, 2);
			send_small_char('X', 42, 2);
			send_small_char('2', 48, 2);
			send_small_char('9', 54, 2);


			sync_word = rfm69_spi_read(REG_SYNCVALUE1);
			send_small_char('C', 0, 3);
			send_small_char('U', 6, 3);
			send_small_char('R', 12, 3);
			send_small_char(' ', 18, 3);
			send_small_char('S', 24, 3);
			send_small_char('Y', 30, 3);
			send_small_char('N', 36, 3);
			send_small_char('C', 42, 3);
			send_small_char(' ', 48, 3);
			send_small_char('W', 54, 3);
			send_small_char('O', 60, 3);
			send_small_char('R', 66, 3);
			send_small_char('D', 72, 3);
			send_small_char(':', 88, 3);
			send_small_char(' ', 94, 3);
			send_small_char('0', 100, 3);
			send_small_char('X', 106, 3);
			send_small_char(((sync_word & 0xF0)>>4), 112, 3);
			send_small_char((sync_word & 0x0F), 118, 3);
			
			sleep_ms(2000);
			clear_display();

		}

		if (dip_3 && test_in == true && test_in_prev == false){
			rfm69_write_fifo(payload, 8);

			send_small_char('G', 0, 1);
			send_small_char('I', 6, 1);
			send_small_char('V', 12, 1);
			send_small_char('E', 18, 1);
			send_small_char(' ', 24, 1);
			send_small_char('P', 30, 1);
			send_small_char('A', 36, 1);
			send_small_char('Y', 42, 1);
			send_small_char('L', 48, 1);
			send_small_char('O', 54, 1);
			send_small_char('A', 60, 1);
			send_small_char('D', 66, 1);

			rfm69_set_tx(); 

			send_small_char('T', 0, 2);
			send_small_char('X', 6, 2);
			send_small_char(' ', 12, 2);
			send_small_char('M', 18, 2);
			send_small_char('O', 24, 2);
			send_small_char('D', 30, 2);
			send_small_char('E', 36, 2);
			
			sleep_ms(2000);
			rfm69_set_standby();
			clear_display();
		}

		if (dip_4){
			rfm69_set_standby();
			rfm69_write_fifo(payload, 8);

			rfm69_set_tx(); 
			gpio_put(CYCLE, 1);

			sleep_ms(5);
			rfm69_set_rx();
			
			sleep_ms(100);
			gpio_put(CYCLE, 0);
			
		}



		
		//stdio_send_ds4_outputs(&controller_state); // print the controller outputs over usb
		
		//display_inputs(&ds4_state); // send the outputs to I2C screen
	}
}

// if(bt_hid_is_connected()) useful function to have in the future, may need to use for diconnect logic






typedef struct {
    float x;
    float y;
} dir_vect;

dir_vect dv;

void normalize_input(uint8_t raw_x, uint8_t raw_y) {
    dv.x = ((float)raw_x - 127.5f) / 127.5f;
    dv.y = ((float)raw_y - 127.5f) / 127.5f;
}

void apply_deadzone() {
    if (dv.x < 0.2f){
		dv.x = 0;
	}
	if (dv.y < 0.2f){
		dv.y = 0;
	}
}






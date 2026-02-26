
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



	
	

	//clear_display();
	//ds4_inputs_display_setup(); // Setup the display to show the controller inputs in a nice format

	
	while (1) {

		gpio_put(CYCLE, !gpio_get(CYCLE)); // Toggle the simple cycle pin for debugging purposes

		bt_hid_get_latest(&ds4_state); // Aquire latest Bluetooth controller state

		sleep_ms(200);

		G0 = gpio_get(RADIO_IRQ);

		if (G0 == 1){
			send_small_char('G', 0, 7);
			send_small_char('0', 7, 7);
		}
		else{
			send_small_char(' ', 0, 7);
			send_small_char(' ', 7, 7);
		}

		
		//stdio_send_ds4_outputs(&controller_state); // print the controller outputs over usb
		
		//display_inputs(&ds4_state); // send the outputs to I2C screen
	}
}

// if(bt_hid_is_connected()) useful function to have in the future, may need to use for diconnect logic
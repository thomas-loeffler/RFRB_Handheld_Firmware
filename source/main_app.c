
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
	

	radio_setup();
	sleep_ms(5000);
	verify_radio_setup();
	
	
	
	

	//clear_display();
	//ds4_inputs_display_setup(); // Setup the display to show the controller inputs in a nice format

	
	while (1) {

		gpio_put(CYCLE, !gpio_get(CYCLE)); // Toggle the simple cycle pin for debugging purposes

		bt_hid_get_latest(&ds4_state); // Aquire latest Bluetooth controller state

		check_version(); // Check the version register to verify SPI communication is working

		sleep_ms(500); // Sleep for a bit to avoid spamming the USB serial output and to give the radio some breathing room

		
		//stdio_send_ds4_outputs(&controller_state); // print the controller outputs over usb
		
		//display_inputs(&ds4_state); // send the outputs to I2C screen
	}
}

// if(bt_hid_is_connected()) useful function to have in the future, may need to use for diconnect logic
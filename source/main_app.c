// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2023 Brian Starkey <stark3y@gmail.com>


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






void stdio_send_ds4_outputs(struct bt_hid_state* state)
{
	printf("buttons: %04x, l: %d,%d, r: %d,%d, l2,r2: %d,%d hat: %d\n",
				state->buttons, state->lx, state->ly, state->rx, state->ry,
				state->l2, state->r2, state->hat);
}





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


	// I2C Screen setup
	i2c_setup(); // Setup the I2C peripheral for the display
	screen_init(); // Initialize the I2C screen with predefined commands
	clear_display();

	display_trine_logo(); // Show the Trine logo on the display at startup
	sleep_ms(3000);

	clear_display();
	ds4_inputs_display_setup(); // Setup the display to show the controller inputs in a nice format

	
	while (1) {

		gpio_put(SC_PIN, !gpio_get(SC_PIN)); // Toggle the simple cycle pin for debugging purposes

		bt_hid_get_latest(&ds4_state); // Aquire latest Bluetooth controller state
		
		//stdio_send_ds4_outputs(&controller_state); // print the controller outputs over usb
		
		display_inputs(&ds4_state); // send the outputs to I2C screen
	}
}

// if(bt_hid_is_connected()) useful function to have in the future
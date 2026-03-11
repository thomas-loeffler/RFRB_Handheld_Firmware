
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
	SSD1306_init(); // Initialize the I2C screen with predefined commands
	SSD1306_clear();
	SSD1306_display_trine_logo(); // Show the Trine logo on the display at startup

	sleep_ms(5000);

	SSD1306_clear();

	rfm69_setup();
	rfm69_verify_setup();
	rfm69_set_standby();

	
	uint8_t payload[8] = {0x01, 0x02, 0xDE, 0xAD, 0xBE, 0xEF, 0x03, 0x04};
	
	

	
	while (1) {

		//gpio_put(CYCLE, !gpio_get(CYCLE)); // Toggle the simple cycle pin for debugging purposes

		bt_hid_get_latest(&ds4_state); // Aquire latest Bluetooth controller state

		sleep_ms(1000);


		rfm69_write_fifo(payload, 8);
		rfm69_set_tx();
		while(!gpio_get(RADIO_IRQ)){}
		sleep_ms(10);
		rfm69_set_standby();


		

	}
}

// if(bt_hid_is_connected()) useful function to have in the future, may need to use for diconnect logic

/*
		test_in_prev = test_in;
		test_in = gpio_get(TEST);

		uint8_t dip_1 = !gpio_get(DIP1);
		uint8_t dip_2 = !gpio_get(DIP2);
		uint8_t dip_3 = !gpio_get(DIP3);
		uint8_t dip_4 = !gpio_get(DIP4);
*/



/*
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
*/






// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2023 Brian Starkey <stark3y@gmail.com>

#include <stdio.h>
#include <string.h>
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "bt_hid.h"
#include "app_version.h"
#include "peripheral_setup.h"
#include "display_functions.h"

#include "ds4_struct.h"

#include <stdint.h>







void stdio_display_ds4_outputs(struct bt_hid_state* state)
{
	printf("buttons: %04x, l: %d,%d, r: %d,%d, l2,r2: %d,%d hat: %d\n",
				state->buttons, state->lx, state->ly, state->rx, state->ry,
				state->l2, state->r2, state->hat);
}

void ds4_input_process(struct bt_hid_state* state, struct ds4* ds4_state)
{
	ds4_state->buttons1 = state->buttons & 0xFF;
	ds4_state->buttons2 = (state->buttons >> 8) & 0xFF;
	ds4_state->dpad = state->hat;

	ds4_state->l_joy_x = state->lx;
	ds4_state->l_joy_y = state->ly;
	ds4_state->r_joy_x = state->rx;
	ds4_state->r_joy_y = state->ry;

	ds4_state->l_trig = state->l2;
	ds4_state->r_trig = state->r2;
}




void main(void){

	// Structure used to hold the Bluetooth controller data
	struct bt_hid_state controller_state = {0};

	// Structure to hold the processed data
	struct ds4 ds4_state = {0};

	stdio_init_all();
	printf("Starting v%d.%d.%d\n", APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH);
	
	multicore_launch_core1(bt_main);
	// Wait for init 
	sleep_ms(1000);



	i2c_setup();
	screen_init();
	clear_display();
	GPIO_setup();

	if (gpio_get(DIP1) == 0){
		display_trine_logo();
		sleep_ms(3000);
		clear_display();
		ds4_inputs_display_setup();
	} else {
		// setting window for refresh rate test 
		set_col_addr(0, 127);   
		set_page_addr(0, 7);
	}

	
	while ( 1 ) {
		if (gpio_get(DIP1) == 0){

			gpio_put(SC_PIN, !gpio_get(SC_PIN)); // toggle the simple cycle pin for debugging purposes

			bt_hid_get_latest(&controller_state); // Aquire latest Bluetooth controller state
			
			//stdio_display_ds4_outputs(&controller_state); // print the controller outputs over usb

			//stdio_display_ds4_outputs(&controller_state); // print the controller outputs over usb
			ds4_input_process(&controller_state, &ds4_state); // put the data into a struct for use elsewhere
			
			update_ds4_input_display(&ds4_state); // send the outputs to the display

			bool dip_1 = gpio_get(DIP1);
			bool dip_2 = gpio_get(DIP2);
			bool dip_3 = gpio_get(DIP3);
			bool dip_4 = gpio_get(DIP4);
			
			if (!dip_1){
				send_small_char(1, 120, 4);
			} else {
				send_small_char(0, 120, 4);
			}

			if (!dip_2){
				send_small_char(1, 120, 5);
			} else {
				send_small_char(0, 120, 5);
			}

			if (!dip_3){
				send_small_char(1, 120, 6);
			} else {
				send_small_char(0, 120, 6);
			}

			if (!dip_4){
				send_small_char(1, 120, 7);
			} else {
				send_small_char(0, 120, 7);
			}
			
		}

		else{
			gpio_put(SC_PIN, 1);
			for (int i = 0; i < 8; i++) { // 8 pages * 128 columns
				for (int j = 0; j < 126; j = j+6){
					send_small_char('A', j, i);
				}
			}
			gpio_put(SC_PIN, 0);
			for (int i = 0; i < 8; i++) { // 8 pages * 128 columns
				for (int j = 0; j < 126; j = j+6){
					send_small_char(1, j, i);
				}
			}

		}
	}
}

// if(bt_hid_is_connected())
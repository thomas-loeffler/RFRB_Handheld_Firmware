
// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2023 Brian Starkey <stark3y@gmail.com>


//////////////////////////////////////
//             INCLUDES             //
//////////////////////////////////////

// ====== Standard Headers / Libraries ======
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/multicore.h"

// ========== User defined headers ==========
#include "bt_hid.h" // Bluetooth functionality with DS4
#include "peripheral_setup.h" // Peripheral initialization funcions
#include "display_functions.h" // SSD1306 I2C display functions 
#include "radio_functions.h" // RFM69 radio functions
#include "radio_registers.h" // RFM69 register and redister init defines
#include "task_functions.h" // Code for all the task functions
#include "helper_functions.h" // Smaller random functions that don't fit anywhere and deserve their own file


//////////////////////////////////////
//    EXTERNAL GLOBAL VARIABLES     //
//////////////////////////////////////

// Radio GO "packet received" ISR variable
extern volatile bool radio_event; 

// Structure used to hold the data from DS4
struct bt_hid_state ds4_state = {0};





//////////////////////////////////////
//           MAIN FUNCTION          //
//////////////////////////////////////


void main(void){

	// Initializes all configured standard I/O interfaces (USB serial in our case)
	stdio_init_all();

	// Launch the second core to run the Bluetooth HID code
	// This will populate the controller state struct with the latest data from the DualShock4
	multicore_launch_core1(bt_main); 

	// Wait for init
	sleep_ms(1000);


	GPIO_setup(); // Setup the GPIO pins for the DIP switches and the simple cycle pin
	i2c_setup(); // Setup the I2C peripheral for the display
	spi_setup(); // Setup the SPI peripheral for the RFM69 radio
	radio_irq_setup(); // Setup the ISR that runs on the RFM69 IRQ

	
	// I2C Screen setup
	SSD1306_init(); // Initialize the I2C screen with predefined commands
	SSD1306_clear();
	SSD1306_display_trine_logo(); 
	sleep_ms(1000);
	SSD1306_clear();
	SSD1306_UI_setup();
	
	rfm69_setup();

	init_all_queues();

	// Buffer for receiving ack messages from the robot
	uint8_t ack_buffer[4] = {0,0,0,0};

	// Variables for holding received indicators from the robot
	uint8_t rssi = 10;
	uint8_t pkt_sent = 0;
	uint8_t pkt_loss = 0;
	uint8_t pkt_rec = 0;
	uint8_t batt = 229;

	// Varaibles for 10ms loop timing
	uint64_t next_tx_time = time_us_64();
	uint64_t now = time_us_64();


	while (1) {

		// Retreiving current time
		now = time_us_64();

		// If the current time is at or after next_rx_time, then initiate a transmission
    	if (now >= next_tx_time) { 
			// Schedule next transmit EXACTLY 10ms later
			next_tx_time = now + 10000; // 10 ms in microseconds

			// If the DS4 is connected, transmit. Else, do nothing
			if(bt_hid_is_connected()){

				get_ds4_inputs();
				mechanum_driver();
				pack_and_send();
				pkt_sent += 1;

			}
		}

		// If its not time to send a transmission wait for ack from robot and display data
		else{
			if(radio_event){ // This chunk still under development
				
				// rfm69_read_packet(ack_buffer);
				// rssi = ack_buffer[1];
				// pkt_rec = ack_buffer[2];
				// batt = ack_buffer[3];
				//
				// pkt_loss = pkt_sent - pkt_rec;
				// pkt_sent = 0;

				// somehting about timing how many acks ive missed

				rssi += 1;
				pkt_loss += 1;
				batt -= 1;

				
				radio_event = false;
			}
			else{
				// Needs to be in else statement for updating even when not receiving acks
				SSD1306_UI_update(rssi, pkt_loss, batt);
			}
		}
	}
}


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
#include "task_functions.h"


extern volatile bool radio_event;

// Structure used to hold the Bluetooth controller data
struct bt_hid_state ds4_state = {0};




//////////////////////////////////////
//           MAIN FUNCTION          //
//////////////////////////////////////


void main(void){

	// Initializes all configured standard I/O interfaces (USB serial in our case)
	stdio_init_all();
	
	multicore_launch_core1(bt_main); // Launch the second core to run the Bluetooth HID code
	                                 // This will populate the controller state struct with the latest data from the DualShock4
	// Wait for init
	sleep_ms(1000);


	GPIO_setup(); // Setup the GPIO pins for the DIP switches and the simple cycle pin
	i2c_setup(); // Setup the I2C peripheral for the display
	spi_setup(); // Setup the SPI peripheral for the RFM69 radio
	radio_irq_setup();

	
	// I2C Screen setup
	SSD1306_init(); // Initialize the I2C screen with predefined commands
	SSD1306_clear();
	SSD1306_display_trine_logo(); // Show the Trine logo on the display at startup

	sleep_ms(1000);

	SSD1306_clear();
	SSD1306_UI_setup();
	

	rfm69_setup();

	init_all_queues();

	uint8_t ack_buffer[4] = {0,0,0,0};


	uint8_t rssi = 10;
	uint8_t pkt_sent = 0;
	uint8_t pkt_loss = 0;
	uint8_t pkt_rec = 0;
	uint8_t batt = 229;


	uint64_t next_tx_time = time_us_64();
	uint64_t now = time_us_64();


	while (1) {
		now = time_us_64();

    	if (now >= next_tx_time) {
			// Schedule next transmit EXACTLY 10ms later
			next_tx_time = now + 10000; // 10 ms in microseconds
			
			get_ds4_inputs();
			mechanum_driver();
			pack_and_send();
			pkt_sent += 1;

        	
		}
		else{
			if(radio_event){
				// rfm69_read_packet(ack_buffer);
				// rssi = ack_buffer[1];
				// pkt_rec = ack_buffer[2];
				// batt = ack_buffer[3];
				//
				// pkt_loss = pkt_sent - pkt_rec;
				// pkt_sent = 0;

				rssi += 1;
				pkt_loss += 1;
				batt -= 1;

				radio_event = false;
			}
			else{
				SSD1306_update(rssi, pkt_loss, batt);
			}
		}
	}
}



/*
// RECEIVER MAIN

void main(void){


	
	stdio_init_all(); // Initializes all configured standard I/O interfaces (USB serial in our case)
	
	sleep_ms(1000);


	GPIO_setup(); 		// Setup the GPIO pins for the DIP switches and the simple cycle pin
	spi_setup(); 		// Setup the SPI peripheral for the RFM69 radio
	radio_irq_setup();

	rfm69_setup();


	uint8_t buffer[9];


	
	uint32_t last_packet_time = to_ms_since_boot(get_absolute_time());  // timestamp of last received packet
	uint32_t now = 0;
	bool link_active = false;

	
	rfm69_set_G0_packet_received();
	sleep_ms(10); // needed to let the radio settle into ready state before rx
	rfm69_set_rx();
	sleep_ms(10); // let receive mode stabilise
	
	
	while (1) {

		
		if(radio_event){
			// update robot connected varaible
			// update rssi varaible
			// update battery voltage varaible
			// update packeet loss variable 
			rfm69_read_packet(buffer);
			last_packet_time = to_ms_since_boot(get_absolute_time());  // timestamp of last received packet
			link_active = true;
			//rfm69_set_rx(); // reset helpful? i dont think so
			radio_event = false;
		}


		now = to_ms_since_boot(get_absolute_time());

		// If no packet for 3 seconds
		if (now - last_packet_time > 3000) {
			link_active = false;

			// Hard reset + re-init sequence
			rfm69_reset();                // pulls RESET pin low, releases
			rfm69_setup();                // re-write all radio registers
			rfm69_set_G0_packet_received(); 
			sleep_ms(5);                  // let radio settle
			rfm69_set_rx();               // back to RX mode
			sleep_ms(5);                  // stabilize receive

			// Reset the last_packet_time to avoid repeated resets
			last_packet_time = now;
		}
		
		
	}
}
*/


















// if(bt_hid_is_connected()) useful function to have in the future, may need to use for diconnect logic

/*
		uint8_t dip_1 = !gpio_get(DIP1);
		uint8_t dip_2 = !gpio_get(DIP2);
		uint8_t dip_3 = !gpio_get(DIP3);
		uint8_t dip_4 = !gpio_get(DIP4);
*/



// heartbeat every 500ms
// transmitting rssi, battery level, packet loss?
// packet loss can be me counting how many packets i send between heartbeats, 
// and the receiver can count hpw many they received then compare the difference
//if have missed 3 or more heart beats say something

//gpio_put(CYCLE, !gpio_get(CYCLE)); // Toggle the simple cycle pin for debugging purposes

/*
if(radio_event){
	// read fifo
	// update robot connected varaible
	// update rssi varaible
	// update battery voltage varaible
	// update packeet loss variable 
	printf("Radio Event\n");
	radio_event = false;
}
*/


/* OR
// Wait for TX complete
while (!(rfm69_spi_read(REG_IRQFLAGS2) & 0x08)) {
	; // busy-wait, or optionally do other small tasks
	// maybe even add a timeout
}
*/

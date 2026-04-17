
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
#include "stdio.h"
#include "string.h"


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
	SSD1306_UI_setup2();
	
	rfm69_reset();
	rfm69_setup();
	//rfm69_set_G0_packet_sent();

	init_all_queues();

	// Variables for holding received indicators from the robot
	uint8_t pkt_sent = 0;
	
	// Varaibles for 10ms loop timing
	uint64_t next_tx_time = time_us_64();
	uint64_t next_packet_expected = time_us_64();
	uint64_t now = time_us_64();

	bool link = false; // variable describing wether the robot is connected
	bool transmit = false;
	
	while (1) {

		// Retreiving current time
		now = time_us_64();

		// If the current time is at or after next_rx_time, then initiate a transmission
    	if (now >= next_tx_time) { 
			// Schedule next transmit EXACTLY 10ms later
			next_tx_time = now + 10000; // 50 ms in microseconds

			//stdio_send_ds4_outputs(&ds4_state); // For debugging, send the DS4 inputs to USB serial

			// If the DS4 is connected and DIP1 = high, transmit. Else, do nothing
			transmit = !gpio_get(DIP1) && bt_hid_is_connected(); // active low
			if(transmit){

				get_ds4_inputs();
				mechanum_driver();
				pack_and_send();
				pkt_sent += 1;

			}
		}


		// If its not time to send a transmission wait for ack from robot and display data
		else{
			if(radio_event){ 
				process_ack();

				now = time_us_64();
				next_packet_expected = now + 1500*1000; // 1500 ms in microseconds = 3 missed packets
				SSD1306_UI_update2(transmit, link);
				int16_t pkt_sent_q = (int16_t)(pkt_sent | 0x0500); // Adding identifier so it can be distinguished when dequeueing
    			queue_try_add(&Display_q, &pkt_sent_q);
				pkt_sent = 0;
				link = true; // if we got an ack, then we know the robot is connected
				radio_event = false; // reset the radio event variable for the next packet
			}
			else{ 
				now = time_us_64();
				if (now > next_packet_expected){
					link = false;
					// rfm69_reset();
					// rfm69_setup();
					// rfm69_set_rx();
					next_packet_expected = now + 2*1000*1000;
				}
				SSD1306_UI_update2(transmit, link);
			}
		}
	}
}



/*
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
	SSD1306_UI_setup2();
	
	rfm69_setup();
	//rfm69_set_G0_packet_sent();

	init_all_queues();

	// Variables for holding received indicators from the robot
	uint8_t pkt_sent = 0;
	
	// Varaibles for 10ms loop timing
	uint64_t next_tx_time = time_us_64();
	uint64_t next_packet_expected = time_us_64();
	uint64_t now = time_us_64();

	bool link = false; // variable describing wether the robot is connected
	bool transmit = false;

	rfm69_set_rx();
	
	while (1) {

		transmit = !gpio_get(DIP1);
		while (!transmit){
			transmit = !gpio_get(DIP1);
			printf("waiting to transmit... \n");
			sleep_ms(100);
		}

		if (transmit){
			while(!radio_event){ // wait until the radio is ready to transmit, which is signaled by the IRQ when it receives a packet (which will be the ack from the robot for the previous packet)
				get_ds4_inputs();
				mechanum_driver();
				pack_and_send();
				printf("Sent packet");
				sleep_ms(50); // wait 50 ms between packets
			}
		}
			

		if(radio_event){ 
			process_ack();
			link = true;
			printf("Ack received!! \n\n\n\n\n\n\n");
			SSD1306_UI_update2(transmit, link);
			radio_event = false; // reset the radio event variable for the next packet
		}

		
	}
}
*/



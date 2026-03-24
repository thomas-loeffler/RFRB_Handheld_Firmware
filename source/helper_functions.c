
//////////////////////////////////////
//             INCLUDES             //
//////////////////////////////////////


// ====== Standard Headers / Libraries ======
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

// ========== User defined headers ==========
#include "helper_functions.h"
#include "bt_hid.h"
#include "radio_functions.h"




//////////////////////////////////////
//             VARIABLES            //
//////////////////////////////////////

// Declaring the queues in this file because this is where the init function is
queue_t Mechanum_q;
queue_t TX_q;
queue_t Display_q;




//////////////////////////////////////
//       FUNCTION DEFINITIONS       //
//////////////////////////////////////


void init_all_queues(void){
    queue_init(&Mechanum_q, sizeof(int16_t), 10);
    queue_init(&TX_q, sizeof(int16_t), 10);
    queue_init(&Display_q, sizeof(int16_t), 10);
}



uint8_t extract_ds4_lx(struct bt_hid_state* ds4_state){
    return ds4_state -> lx;
}

uint8_t extract_ds4_ly(struct bt_hid_state* ds4_state){
    return ds4_state -> ly;
}

uint8_t extract_ds4_rx(struct bt_hid_state* ds4_state){
    return ds4_state -> rx;
}



// Faster rounding than roundf()
int8_t round_and_cast(float x){
    if (x > 0) {x += 0.5f;}
    else {x -= 0.5f;}
    return (int8_t)x;
}


// For after TX, waiting unitl returning to RX
void wait_for_transmit(void){
	uint64_t start = time_us_64(); // For a timeout
	while (!(rfm69_spi_read(0x28) & 0x08)) { // Polling the "Packet Sent" Flag 0x28 = RegIRQFlags
		if ((time_us_64() - start) > 3000) { // 3 ms timeout
			break;
		}
		; // Busy wait
	}
    // Let it settle
	sleep_us(300); // Not sure why this is needed, but if I don't then the receiver misses alot of packets

}




// For testing only, determinitng resultant vector of wheels given input
void mecanum_resultant_TEST(float fl, float fr, float bl, float br){

    // Calculating the total x and y vectors imparted on the robot
    float result_x = (fl - fr - bl + br) * (COS_45);
    float result_y = (fl + fr + bl + br) * (COS_45);

    float result_rot = (fl + bl) - (fr + br);
	
    // Calculating the angle of the resulting force vector
    float angle_rad = atan2(result_x, result_y); // Swapped the arguments so 0 deg would be forward
    float angle_deg = angle_rad * (180.0f / PI);

    printf("%.1f %.1f %.1f %.1f         ", fl, fr, bl, br);
    printf("%.2f %.2f   Degrees: %.2f   Rotation: %.2f\n", result_x, result_y, angle_deg, result_rot);

}



// For early testing
void stdio_send_ds4_outputs(struct bt_hid_state* state){
    
	printf("buttons: %04x, l: %d,%d, r: %d,%d, l2,r2: %d,%d hat: %d\n",
				state->buttons, state->lx, state->ly, state->rx, state->ry,
				state->l2, state->r2, state->hat);
}


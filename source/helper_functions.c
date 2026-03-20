
//////////////////////////////////////
//             INCLUDES             //
//////////////////////////////////////


// Standard includes / hardware headers
#include "pico/stdlib.h"
#include "helper_functions.h"
#include "bt_hid.h"
#include <math.h>
#include "radio_functions.h"


#define PI 3.1416f
#define COS_45 0.70710678f
#define MOTOR_SCALING 12


/*
Wheel Roller Orientation:

   FL        FR
    \        /
     \      /
     /      \
    /        \

   BL        BR



Wheel Force Vectors

FL  ↗      ↖  FR
  ↙          ↘

  ↖          ↗
BL  ↘      ↙  BR

1  = wheel spinning towards the front of the robot
-1 = wheel spinning towards the back of the robot

*/

// 	Forward:		Backward:		Strafe Left:		Strafe Right:
//	  FL = 1		  FL = -1		  FL = -1			  FL = 1	
//	  FR = 1		  FR = -1		  FR = 1			  FR = -1
//	  BL = 1		  BL = -1		  BL = 1			  BL = -1
//	  BR = 1		  BR = -1		  BR = -1			  BR = 1


// 	Diagonal NW (forward left):		Diagonal NE (forward Right):
//	  FL = 0			  			  FL = 1
//	  FR = 1						  FR = 0
//	  BL = 1						  BL = 0
//	  BR = 0						  BR = 1


// 	Diagonal SW (backward left):	Diagonal SE (backward right):
//	  FL = -1			  			  FL = 0
//	  FR = 0						  FR = -1
//	  BL = 0						  BL = -1
//	  BR = -1						  BR = 0
	


// 	Rotate CCW:			Rotate CW:
//	  FL = -1			  FL = 1
//	  FR = 1			  FR = -1
//	  BL = -1			  BL = 1
//	  BR = 1			  BR = -1



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
	uint64_t start = time_us_64(); // for a timeout
	while (!(rfm69_spi_read(0x28) & 0x08)) { // polling the "Packet Sent" Flag 0x28 = RegIRQFlags
		if ((time_us_64() - start) > 3000) { // 3 ms timeout
			break;
		}
		; // busy wait
	}
	sleep_us(300); // let it settle not sure why this is needed but if i dont then the receiver misses alot of packets

}




// For testing only, determinitng resultant vector of wheels given input
void mecanum_resultant_TEST(float fl, float fr, float bl, float br){

    float result_x = (fl - fr - bl + br) * (COS_45);

    float result_y = (fl + fr + bl + br) * (COS_45);
	
	
    float angle_rad = atan2(result_x, result_y); // swapped the arguments so 0 deg would be forward
    float angle_deg = angle_rad * (180.0f / PI);


    //printf("%.2f %.2f   Radians: %.2f   Degrees: %.2f\n", result_x, result_y, angle_rad, angle_deg);
}


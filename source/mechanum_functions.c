
//////////////////////////////////////
//             INCLUDES             //
//////////////////////////////////////


// Standard includes / hardware headers
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define COS_45 0.70710678f


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


// this function takes approxamately 100us with no printf statements. 500 with printf
void mechanum_driver(uint8_t raw_x, uint8_t raw_y, 
                    float *fl, float *fr,
                    float *bl, float *br) {

	// Normalising raw inputs (0 - 255) to (-1 - 1)
    float norm_x = ((float)raw_x - 127.5f) / 127.5f;
    float norm_y = -1 * (((float)raw_y - 127.5f) / 127.5f); // flip so up is positive

    // Applying deadzone
    if (fabs(norm_x) < 0.1f) norm_x = 0;
    if (fabs(norm_y) < 0.1f) norm_y = 0;

	// Applying X and Y values to each motor
    *fl = norm_y + norm_x;
    *fr = norm_y - norm_x;
    *bl = norm_y - norm_x;
    *br = norm_y + norm_x;

    // Finding maximum motor value
    float max = fabs(*fl);
    if (fabs(*fr) > max) max = fabs(*fr);
    if (fabs(*bl) > max) max = fabs(*bl);
    if (fabs(*br) > max) max = fabs(*br);

    // Scaling if needed (on diagonal commands, value can exceed 1)
    if (max > 1.0f) {
        *fl /= max;
        *fr /= max;
        *bl /= max;
        *br /= max;
    }

    // Print all values for debugging
    //printf("norm_x = %.2f    norm_y = %.2f  ", norm_x, norm_y);
    //printf("FL = %.2f   FR = %.2f   BL = %.2f   BR = %.2f   ", *fl, *fr, *bl, *br);
}





void mecanum_resultant_TEST(float fl, float fr, float bl, float br){

    float result_x = (fl - fr - bl + br) * (COS_45);

    float result_y = (fl + fr + bl + br) * (COS_45);
	
	
    float angle_rad = atan2(result_x, result_y); // swapped the arguments so 0 deg would be forward
    float angle_deg = angle_rad * (180.0f / M_PI);


    //printf("%.2f %.2f   Radians: %.2f   Degrees: %.2f\n", result_x, result_y, angle_rad, angle_deg);
}


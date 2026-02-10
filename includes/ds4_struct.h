
#include <stdint.h>

struct ds4 {
	uint8_t buttons1;
    /* 
       bit 0 = square
       bit 1 = cross
       bit 2 = circle
       bit 3 = triangle
       bit 4 = L1
       bit 5 = R1
       bit 6 = L2 (digital)
       bit 7 = R2 (digital)
    */

	uint8_t buttons2;
    /*
       bit 0 = share
       bit 1 = options
       bit 2 = PS
       bit 3 = touchpad
       bit 4 = L3
       bit 5 = R3
       bits 6–7 unused
    */
    
	uint8_t dpad;
    /*
       bit 0 = up
       bit 1 = down
       bit 2 = left
       bit 3 = right
       bits 4–7 unused
    */
    
    // Joysticks
    uint8_t l_joy_x;     
    uint8_t l_joy_y;     
    uint8_t r_joy_x;    
    uint8_t r_joy_y;    

    // Triggers
    uint8_t l_trig;        // 0 .. 255
    uint8_t r_trig;        // 0 .. 255

    uint8_t battery;
};
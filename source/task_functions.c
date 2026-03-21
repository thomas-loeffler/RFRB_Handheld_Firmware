
//////////////////////////////////////
//             INCLUDES             //
//////////////////////////////////////

// ====== Standard Headers / Libraries ======
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "math.h"

// ========== User defined headers ==========
#include "task_functions.h"
#include "helper_functions.h"
#include "radio_functions.h"





//////////////////////////////////////
//       FUNCTION DEFINITIONS       //
//////////////////////////////////////

void get_ds4_inputs(){

    // Init variables for joystick values
    uint8_t raw_lx;
	uint8_t raw_ly;
    uint8_t raw_rx;

    // Aquire latest Bluetooth controller state
    bt_hid_get_latest(&ds4_state); 

    raw_lx = extract_ds4_lx(&ds4_state); // grab the left joytick X value from the struct
    raw_ly = extract_ds4_ly(&ds4_state); // grab the left joytick Y value from the struct

    raw_rx = extract_ds4_rx(&ds4_state); // grab the left joytick Y value from the struct

    // Adding identifiers to the variables so they can be distinguished when dequeueing
    int16_t lx = (int16_t)(raw_lx | 0x0100); 
    int16_t ly = (int16_t)(raw_ly | 0x0200);
    int16_t rx = (int16_t)(raw_rx | 0x0300);

    // Enqueueing for the mechanum math function
    queue_try_add(&Mechanum_q, &lx);
    queue_try_add(&Mechanum_q, &ly);
    queue_try_add(&Mechanum_q, &rx);
}





void mechanum_driver(void) {

    // Init varaibles to be received from input acquisition
    int16_t data;
    uint8_t raw_lx = 128; // Init to resting position incase dequeing fails
    uint8_t raw_ly = 128;
    uint8_t raw_rx = 128;

    // Dequeue inputs until queue is empty
    for (uint8_t i = 0; i<10; i++){
        if(queue_is_empty(&Mechanum_q)){
            break;
        }

        queue_try_remove(&Mechanum_q, &data);

        if((data & 0xFF00) == 0x0100){ // 1 is the ID for left joystick x
            raw_lx = (uint8_t)(data & 0x00FF);
        }
        if((data & 0xFF00) == 0x0200){ // 2 is the ID for left joystick y
            raw_ly = (uint8_t)(data & 0x00FF);
        }
        if((data & 0xFF00) == 0x0300){ // 3 is the ID for right joystick x
            raw_rx = (uint8_t)(data & 0x00FF);
        }
    }

    // Init variables for motor commands
    float fl;
	float fr;
    float bl;
	float br;

	// Normalising raw inputs (0 - 255) to (-1 - 1)
    float norm_x = ((float)raw_lx - 127.5f) / 127.5f;
    float norm_y = -1 * (((float)raw_ly - 127.5f) / 127.5f); // Flip sign so up is positive
    float norm_t = ((float)raw_rx - 127.5f) / 127.5f; // T for turn

    // Applying deadzone
    if (fabs(norm_x) < 0.1f) norm_x = 0;
    if (fabs(norm_y) < 0.1f) norm_y = 0;
    if (fabs(norm_t) < 0.1f) norm_t = 0;

    // Damping the rotational component so our running back doesnt turn into a helicopter
    norm_t *= ROT_DAMP_FACT;

    // Early exit to skip all the calulations if the controller is at rest
    if (norm_x == 0 && norm_y == 0 && norm_t == 0) {
        int16_t fl_q = (int16_t)(0x0100); 
        int16_t fr_q = (int16_t)(0x0200); 
        int16_t bl_q = (int16_t)(0x0300);
        int16_t br_q = (int16_t)(0x0400);

        // Sending straight up 0s since we know the variables are 0
        queue_try_add(&TX_q, &fl_q);
        queue_try_add(&TX_q, &fr_q);
        queue_try_add(&TX_q, &bl_q);
        queue_try_add(&TX_q, &br_q);
        return;
    }
    
    // Applying X, Y, and Turn values to each motor
    fl = norm_y + norm_x + norm_t;
    fr = norm_y - norm_x - norm_t;
    bl = norm_y - norm_x + norm_t;
    br = norm_y + norm_x - norm_t;
    

    // Finding maximum motor value
    float max = fabs(fl);
    if (fabs(fr) > max) max = fabs(fr);
    if (fabs(bl) > max) max = fabs(bl);
    if (fabs(br) > max) max = fabs(br);
    
    // Normalize all motors by the largest motor value so no motor exceeds 1.
    // This is necessary because diagonal and combined inputs can push individual
    // motors above 1, while cardinal directions never exceed 1. Dividing by the
    // max preserves the ratio between motors (direction) while keeping all
    // values within range.
    if (max > 0.0f) {  // guard against division by zero when all inputs are 0
        fl /= max;
        fr /= max;
        bl /= max;
        br /= max;
    }

    // Then re-scale by the actual joystick magnitude so half-stick = half speed
    // float magnitude = sqrtf(norm_x * norm_x + norm_y * norm_y);
    float magnitude = sqrtf(norm_x * norm_x + norm_y * norm_y + norm_t * norm_t); // with turn

    if (magnitude > 1.0f) magnitude = 1.0f;  // clamp to 1 (corners of joystick range)

    fl *= magnitude;
    fr *= magnitude;
    bl *= magnitude;
    br *= magnitude;
    

    // Scaling up to the max value, rounding to nearest whole number then casting to an integer
    int8_t fl_scaled = round_and_cast(fl * MOTOR_SCALING); 
    int8_t fr_scaled = round_and_cast(fr * MOTOR_SCALING); 
    int8_t bl_scaled = round_and_cast(bl * MOTOR_SCALING); 
    int8_t br_scaled = round_and_cast(br * MOTOR_SCALING); 

    // mecanum_resultant_TEST(fl * MOTOR_SCALING, fr * MOTOR_SCALING, bl * MOTOR_SCALING, br * MOTOR_SCALING);    

    // Adding identifiers to the variables so they can be distinguished when dequeueing
    int16_t fl_q = (int16_t)(((uint8_t)fl_scaled) | 0x0100); 
    int16_t fr_q = (int16_t)(((uint8_t)fr_scaled) | 0x0200); 
    int16_t bl_q = (int16_t)(((uint8_t)bl_scaled) | 0x0300);
    int16_t br_q = (int16_t)(((uint8_t)br_scaled) | 0x0400);

    // Enqueueing for the transmitting function
    queue_try_add(&TX_q, &fl_q);
    queue_try_add(&TX_q, &fr_q);
    queue_try_add(&TX_q, &bl_q);
    queue_try_add(&TX_q, &br_q);
}



void pack_and_send(void){

    int16_t data;
    // Init all motor commands to 0 just for safety
    int8_t fl = 0; 
    int8_t fr = 0; 
    int8_t bl = 0; 
    int8_t br = 0;

    // Dequeue inputs until queue is empty
    for (uint8_t i = 0; i<10; i++){
        if(queue_is_empty(&TX_q)){
            break;
        }

        queue_try_remove(&TX_q, &data);

        if((data & 0xFF00) == 0x0100){
            fl = (uint8_t)(data & 0x00FF);
        }
        else if((data & 0xFF00) == 0x0200){
            fr = (uint8_t)(data & 0x00FF);
        }
        else if((data & 0xFF00) == 0x0300){
            bl = (uint8_t)(data & 0x00FF);
        }
        else if((data & 0xFF00) == 0x0400){
            br = (uint8_t)(data & 0x00FF);
        }
    }

    uint8_t payload[4];

    // Casting to a uint for SPI and radio unit. Need to interpret correctly on receiving end
    payload[0] = (uint8_t)fl; 
    payload[1] = (uint8_t)fr;
    payload[2] = (uint8_t)bl;
    payload[3] = (uint8_t)br;

    // Sending payload then switching back to RX
    rfm69_set_standby();
    rfm69_write_fifo(payload, 4);
    rfm69_set_tx();
    wait_for_transmit();
    rfm69_set_rx();

}

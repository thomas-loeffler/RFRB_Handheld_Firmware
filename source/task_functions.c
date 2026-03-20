
//////////////////////////////////////
//             INCLUDES             //
//////////////////////////////////////

// Standard includes / hardware headers
#include "pico/stdlib.h"
#include "math.h"

// User defined headers
#include "task_functions.h"
#include "helper_functions.h"
#include "radio_functions.h"







//////////////////////////////////////
//       FUNCTION DEFINITIONS       //
//////////////////////////////////////

void get_ds4_inputs(){

    uint8_t raw_lx;
	uint8_t raw_ly;
    uint8_t raw_rx;

	float fl;
	float fr;
    float bl;
	float br;


    bt_hid_get_latest(&ds4_state); // Aquire latest Bluetooth controller state

    raw_lx = extract_ds4_lx(&ds4_state); // grab the left joytick X value from the struct
    raw_ly = extract_ds4_ly(&ds4_state); // grab the left joytick Y value from the struct

    raw_rx = extract_ds4_rx(&ds4_state); // grab the left joytick Y value from the struct


    int16_t lx = (int16_t)(raw_lx | 0x0100); // top 8 bits are id for what data it is
    int16_t ly = (int16_t)(raw_ly | 0x0200); // top 8 bits are id for what data it is
    int16_t rx = (int16_t)(raw_rx | 0x0300); // top 8 bits are id for what data it is
    

    enqueue(&Mechanum_q, lx);
    enqueue(&Mechanum_q, ly);
    enqueue(&Mechanum_q, rx);

}





void mechanum_driver(void) {

    int16_t data;
    uint8_t raw_lx;
    uint8_t raw_ly;
    uint8_t raw_rx;

    for (uint8_t i = 0; i<10; i++){
        dequeue(&Mechanum_q, &data);
        if((data & 0xFF00) == 0x0100){
            raw_lx = (uint8_t)(data & 0x00FF);
        }
        if((data & 0xFF00) == 0x0200){
            raw_ly = (uint8_t)(data & 0x00FF);
        }
        if((data & 0xFF00) == 0x0300){
            raw_rx = (uint8_t)(data & 0x00FF);
        }
    }


    float fl;
	float fr;
    float bl;
	float br;


	// Normalising raw inputs (0 - 255) to (-1 - 1)
    float norm_x = ((float)raw_lx - 127.5f) / 127.5f;
    float norm_y = -1 * (((float)raw_ly - 127.5f) / 127.5f); // flip so up is positive

    // Applying deadzone
    if (fabs(norm_x) < 0.1f) norm_x = 0;
    if (fabs(norm_y) < 0.1f) norm_y = 0;

	// Applying X and Y values to each motor
    fl = norm_y + norm_x;
    fr = norm_y - norm_x;
    bl = norm_y - norm_x;
    br = norm_y + norm_x;

    // Finding maximum motor value
    float max = fabs(fl);
    if (fabs(fr) > max) max = fabs(fr);
    if (fabs(bl) > max) max = fabs(bl);
    if (fabs(br) > max) max = fabs(br);

    // Scaling if needed (on diagonal commands, value can exceed 1)
    if (max > 1.0f) {
        fl /= max;
        fr /= max;
        bl /= max;
        br /= max;
    }

    // Rounding to nearest whole number then casting to an integer
    int8_t fl_scaled = round_and_cast(fl * MOTOR_SCALING); 
    int8_t fr_scaled = round_and_cast(fr * MOTOR_SCALING); 
    int8_t bl_scaled = round_and_cast(bl * MOTOR_SCALING); 
    int8_t br_scaled = round_and_cast(br * MOTOR_SCALING); 


    int16_t fl_q = (int16_t)(((uint8_t)fl_scaled) | 0x0100); // top 8 bits are id for what data it is
    int16_t fr_q = (int16_t)(((uint8_t)fr_scaled) | 0x0200); // top 8 bits are id for what data it is
    int16_t bl_q = (int16_t)(((uint8_t)bl_scaled) | 0x0300); // top 8 bits are id for what data it is
    int16_t br_q = (int16_t)(((uint8_t)br_scaled) | 0x0400); // top 8 bits are id for what data it is

    enqueue(&TX_q, fl_q);
    enqueue(&TX_q, fr_q);
    enqueue(&TX_q, bl_q);
    enqueue(&TX_q, br_q);

  
}



void pack_and_send(void){

    int16_t data;
    int8_t fl;
    int8_t fr; 
    int8_t bl; 
    int8_t br;

    for (uint8_t i = 0; i<10; i++){
        bool n_empty = dequeue(&TX_q, &data);
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

        if(!n_empty){
            break;
        }
    }

    uint8_t payload[4];

    // Casting to a uint for SPI and radio unit. Need to interpret correctly on receiving end
    payload[0] = (uint8_t)fl; 
    payload[1] = (uint8_t)fr;
    payload[2] = (uint8_t)bl;
    payload[3] = (uint8_t)br;

    rfm69_set_standby();
    rfm69_write_fifo(payload, 4);
    rfm69_set_tx();
    wait_for_transmit();
    rfm69_set_rx();

}

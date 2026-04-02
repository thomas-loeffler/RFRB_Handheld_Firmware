
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
#include "display_functions.h"
#include "peripheral_setup.h"
#include "stdio.h"
#include "string.h"





//////////////////////////////////////
//       FUNCTION DEFINITIONS       //
//////////////////////////////////////

void get_ds4_inputs(void){

    // Init variables for joystick values
    uint8_t raw_lx;
	uint8_t raw_ly;
    uint8_t raw_rx;

    // Aquire latest Bluetooth controller state
    bt_hid_get_latest(&ds4_state); 

    raw_lx = extract_ds4_lx(&ds4_state); // grab the left joytick X value from the struct
    raw_ly = extract_ds4_ly(&ds4_state); // grab the left joytick Y value from the struct

    raw_rx = extract_ds4_rx(&ds4_state); // grab the right joytick X value from the struct

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
    float norm_t = -1 * ((float)raw_rx - 127.5f) / 127.5f; // T for turn

    // Applying deadzone
    if (fabs(norm_x) < 0.1f) norm_x = 0;
    if (fabs(norm_y) < 0.1f) norm_y = 0;
    if (fabs(norm_t) < 0.1f) norm_t = 0;

    // Damping the rotational component so our running back doesnt turn into a helicopter
    norm_t *= ROT_DAMP_FACT;

    uint8_t gear = get_gear(&ds4_state); // Get the current gear, updating on bumper presses

    uint8_t motor_scaling; // the factor in which the motor outputs are sclaed by. Instead of 0->1, 0->motor_scaling

    // Changing the motor scaling value based on the current gear
    switch(gear){
        case 1: motor_scaling = 5; 
                break;
        case 2: motor_scaling = 10;
                break;
        case 3: motor_scaling = 20;
                break;
        case 4: motor_scaling = 30;
                break;
        default: motor_scaling = 5;
                break;
    }

    // Enqueueing the motor scaling varaible for the display function
    int16_t ms_q = (int16_t)(motor_scaling | 0x0400); // Adding identifier so it can be distinguished when dequeueing
    queue_try_add(&Display_q, &ms_q);



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
    int8_t fl_scaled = round_and_cast(fl * motor_scaling);
    int8_t fr_scaled = round_and_cast(fr * motor_scaling); 
    int8_t bl_scaled = round_and_cast(bl * motor_scaling); 
    int8_t br_scaled = round_and_cast(br * motor_scaling); 

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

    static uint16_t buttons_prev;

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

    uint8_t payload[5];

    // Casting to a uint for SPI and radio unit. Need to interpret correctly on receiving end
    payload[0] = (uint8_t)fl; 
    payload[1] = (uint8_t)fr;
    payload[2] = (uint8_t)bl;
    payload[3] = (uint8_t)br;


    bool motors_idle = (payload[0] | payload[1] | payload[2] | payload[3]) == 0;
    bool ps_just_pressed = (ds4_state.buttons & 0x2000) && !(buttons_prev & 0x2000);

    if (motors_idle && ps_just_pressed) { // if all motors are 0 and the PS button was just pressed, send a special packet to indicate velocity mode
        payload[4] = VELOCITY_MODE;
    }
    else{
        payload[4] = 0x00; 
    }

    buttons_prev = ds4_state.buttons; // For detecting changes in button state

    // Sending payload then switching back to RX
    rfm69_set_standby();
    rfm69_write_fifo(payload, 5);
    rfm69_set_tx();
    wait_for_transmit();
    // maybe add something in here to indicate on the screen that it is transmitting?
    rfm69_set_rx();

}







// Updating the monitored values
void SSD1306_UI_update1(uint8_t pkt_sent, bool transmit, bool link){

    static bool DS4_conn = false; // init to false

    int16_t data; // For dequeueing
    uint8_t pkt_rec = 0; // Ack data
    uint8_t rssi = 0; // Ack data
    uint8_t batt = 0; // Ack data
    static uint8_t gear = 0; // For displaying current gear. Static so it doesnt reinit to 0 every loop, latching the value until a new value comes in
	

    // Variables for storing previous state, preventing unnecessary updates to the display
    static uint8_t rssi_prev; 
    static uint8_t pkt_loss_prev;
    static uint8_t batt_prev;
    static uint8_t gear_prev;
    static bool link_displayed = true;



    // Updating the bluetooth icon that designates if the DS4 is connected
    if(bt_hid_is_connected() && !DS4_conn){
        SSD1306_send_big_char('*', 120, 0); // if the DS4 just connected, update display with BT icon
        DS4_conn = true;
    }
    else if (!bt_hid_is_connected() && DS4_conn){
        SSD1306_send_big_char(' ', 120, 0); // if the DS4 just disconnected, clear BT icon
        DS4_conn = false;
    }

    if(link && !link_displayed){
        SSD1306_send_big_char(20, 110, 6);
	    SSD1306_send_big_char(21, 118, 6);
        link_displayed = true;
    }
    else if(!link && link_displayed){
        SSD1306_send_big_char(' ', 110, 6); // clear rf symbol
        SSD1306_send_big_char(' ', 118, 6);

        SSD1306_send_big_char('-', 57, 2); // RSSI
        SSD1306_send_big_char('-', 66, 2);

        SSD1306_send_big_char('-', 67, 4); // Packet Loss
        SSD1306_send_big_char('-', 76, 4);

        SSD1306_send_big_char('-', 50, 6); // Battery
        SSD1306_send_big_char('-', 59, 6); 
        SSD1306_send_big_char('-', 77, 6); 

        link_displayed = false;
    }





    // Dequeue inputs until queue is empty
    for (uint8_t i = 0; i<10; i++){
        // If there is nothing to dequeue, then there is nothing to update. get out
        if(queue_is_empty(&Display_q)){
            break;
        }

        queue_try_remove(&Display_q, &data);

        if((data & 0xFF00) == 0x0100){
            rssi = (uint8_t)(data & 0x00FF);
        }
        else if((data & 0xFF00) == 0x0200){
            pkt_rec = (uint8_t)(data & 0x00FF);
        }
        else if((data & 0xFF00) == 0x0300){
            batt = (uint8_t)(data & 0x00FF);
        }
        else if((data & 0xFF00) == 0x0400){
            gear = (uint8_t)(data & 0x00FF);
            
        }
    }


    


    // --------------- RSSI ---------------
    uint8_t rssi_dbm = rssi/2; // RSSI function according to the datasheet
    if (rssi_dbm != rssi_prev){
        // Calculations
        uint8_t rssi_tens = (rssi_dbm / 10) % 10;
        uint8_t rssi_ones = rssi_dbm % 10;
        // Update screen 
        SSD1306_send_big_char(rssi_tens, 57, 2);
        SSD1306_send_big_char(rssi_ones, 66, 2);
        // Update previous value
        rssi_prev = rssi_dbm;
    }
    

    // ------------ Packet Loss ------------
    uint8_t pkt_loss = pkt_sent - pkt_rec;

    if ((pkt_loss != pkt_loss_prev) && link){
        // Calculations
        uint8_t pkt_loss_tens = (pkt_loss / 10) % 10;
        uint8_t pkt_loss_ones = pkt_loss % 10;
        // Update screen
        if(pkt_loss_tens == 0){ // if value is only one digit, display it centered without the tens digit
            SSD1306_send_big_char(pkt_loss_ones, 72, 4);
        }
        else{
            SSD1306_send_big_char(pkt_loss_tens, 67, 4);
            SSD1306_send_big_char(pkt_loss_ones, 76, 4);
        }
        // Update previous value
        pkt_loss_prev = pkt_loss;
    }

    // ------------ Battery ------------
    if (batt != batt_prev){
        // Calculations (battery voltage comes in as a 3 digit number (xyz) scaled up by 10, to represent (xy.z))
        uint8_t batt_tens  = (batt / 100) % 10;
        uint8_t batt_ones  = (batt / 10) % 10;
        uint8_t batt_tenth = batt % 10;
        // Update screen
        SSD1306_send_big_char(batt_tens, 50, 6);
        SSD1306_send_big_char(batt_ones, 59, 6); 
        SSD1306_send_big_char(batt_tenth, 77, 6); 
        // Update previous value
        batt_prev = batt;
    }

    // ------------ Gear ------------
    if (gear != gear_prev){
        // Update screen
        SSD1306_send_big_char(gear, 115, 2);
        // Update previous value
        gear_prev = gear;
    }


}








// Updating the monitored values
void SSD1306_UI_update2(uint8_t pkt_sent, bool transmit, bool link){

    static bool DS4_conn = false; // init to false

    int16_t data; // For dequeueing
    uint8_t pkt_rec = 0; // Ack data
    uint8_t rssi = 0; // Ack data
    uint8_t batt = 0; // Ack data
    static uint8_t gear = 1; // For displaying current gear. Static so it doesnt reinit to 0 every loop, latching the value until a new value comes in
	

    // Variables for storing previous state, preventing unnecessary updates to the display
    static uint8_t rssi_prev; 
    static uint8_t pkt_loss_prev;
    static uint8_t batt_prev;
    static uint8_t gear_prev = 1;
    static bool link_displayed = true;
    static bool transmit_displayed = false;



    // Updating the bluetooth icon that designates if the DS4 is connected
    if(bt_hid_is_connected() && !DS4_conn){
        SSD1306_send_big_char('*', 105, 3); // if the DS4 just connected, update display with BT icon
        DS4_conn = true;
    }
    else if (!bt_hid_is_connected() && DS4_conn){
        SSD1306_send_big_char(' ', 105, 3); // if the DS4 just disconnected, clear BT icon
        DS4_conn = false;
    }


     if(transmit && !transmit_displayed){
        SSD1306_send_big_char(22, 90, 6); // display controller rf symbol
        SSD1306_send_big_char(23, 98, 6);

        SSD1306_send_big_char(gear, 50, 6); 
        transmit_displayed = true;
    }
    else if (!transmit && transmit_displayed){
        SSD1306_send_big_char(' ', 90, 6); // clear controller rf symbol
        SSD1306_send_big_char(' ', 98, 6);

        SSD1306_send_big_char('-', 50, 6); // clear gear value

        transmit_displayed = false;
    }


    if(link && !link_displayed){
        SSD1306_send_big_char(24, 110, 6); // display robot rf symbol
	    SSD1306_send_big_char(25, 118, 6);
        link_displayed = true;
    }
    else if(!link && link_displayed){ // if there is no more link, clear the values

        SSD1306_send_big_char(' ', 110, 6); // clear robot rf symbol
	    SSD1306_send_big_char(' ', 118, 6);

        SSD1306_send_small_char('-', 42, 3); // RSSI
        SSD1306_send_small_char('-', 48, 3);

        SSD1306_send_small_char('-', 48, 4); // Packet Loss
        SSD1306_send_small_char('-', 54, 4);

        SSD1306_send_small_char('-', 36, 5); // Battery
        SSD1306_send_small_char('-', 42, 5); 
        SSD1306_send_small_char('-', 54, 5); 

        SSD1306_send_big_char('-', 50, 6); // Gear

        link_displayed = false;
    }






    // Dequeue inputs until queue is empty
    for (uint8_t i = 0; i<10; i++){
        // If there is nothing to dequeue, then there is nothing to update. get out
        if(queue_is_empty(&Display_q)){
            break;
        }

        queue_try_remove(&Display_q, &data);

        if((data & 0xFF00) == 0x0100){
            rssi = (uint8_t)(data & 0x00FF);
        }
        else if((data & 0xFF00) == 0x0200){
            pkt_rec = (uint8_t)(data & 0x00FF);
        }
        else if((data & 0xFF00) == 0x0300){
            batt = (uint8_t)(data & 0x00FF);
        }
        else if((data & 0xFF00) == 0x0400){
            gear = (uint8_t)(data & 0x00FF);
            
        }
    }


    


    // --------------- RSSI ---------------
    uint8_t rssi_dbm = rssi/2; // RSSI function according to the datasheet
    if (rssi_dbm != rssi_prev){
        // Calculations
        uint8_t rssi_tens = (rssi_dbm / 10) % 10;
        uint8_t rssi_ones = rssi_dbm % 10;
        // Update screen 
        SSD1306_send_small_char(rssi_tens, 42, 3);
        SSD1306_send_small_char(rssi_ones, 48, 3);
        // Update previous value
        rssi_prev = rssi_dbm;
    }
    

    // ------------ Packet Loss ------------
    uint8_t pkt_loss = pkt_sent - pkt_rec;

    if ((pkt_loss != pkt_loss_prev) && link){
        // Calculations
        uint8_t pkt_loss_tens = pkt_loss / 10;
        uint8_t pkt_loss_ones = pkt_loss % 10;
        // Update screen
        if(pkt_loss_tens == 0){ // if value is only one digit, display it centered without the tens digit
            SSD1306_send_small_char(' ', 48, 4);
            SSD1306_send_small_char(' ', 54, 4);
            SSD1306_send_small_char(pkt_loss_ones, 51, 4);
        }
        else{
            SSD1306_send_small_char(' ', 51, 4);
            SSD1306_send_small_char(pkt_loss_tens, 48, 4);
            SSD1306_send_small_char(pkt_loss_ones, 54, 4);
        }
        // Update previous value
        pkt_loss_prev = pkt_loss;
    }

    // ------------ Battery ------------
    if (batt != batt_prev){
        // Calculations (battery voltage comes in as a 3 digit number (xyz) scaled up by 10, to represent (xy.z))
        uint8_t batt_tens  = (batt / 100) % 10;
        uint8_t batt_ones  = (batt / 10) % 10;
        uint8_t batt_tenth = batt % 10;
        // Update screen
        SSD1306_send_big_char(batt_tens, 36, 5);
        SSD1306_send_big_char(batt_ones, 42, 5); 
        SSD1306_send_big_char(batt_tenth, 54, 5); 
        // Update previous value
        batt_prev = batt;
    }

    // ------------ Gear ------------
    if (gear != gear_prev){

        uint8_t gear_tens = gear / 10;
        uint8_t gear_ones = gear % 10;
        // Update screen
        if(gear_tens == 0){ // if value is only one digit, display it centered without the tens digit
            SSD1306_send_big_char(' ', 46, 6); // 46 81
            SSD1306_send_big_char(' ', 54, 6); // 54 90
            SSD1306_send_big_char(gear_ones, 50, 6); // 50 85
        }
        else{
            SSD1306_send_big_char(' ', 50, 6); // 50 85
            SSD1306_send_big_char(gear_tens, 46, 6); // 46 81
            SSD1306_send_big_char(gear_ones, 54, 6); // 54 90
        }
        
        // Update previous value
        gear_prev = gear;
    }

}













void process_ack(void){


    // Buffer for receiving ack messages from the robot
	uint8_t ack_buffer[4] = {0,0,0,0};

    // Reading the message and filling our buffer
    rfm69_read_packet(ack_buffer);
    
    // Extracting the data
    uint8_t rssi = ack_buffer[1];
    uint8_t pkt_rec = ack_buffer[2];
    uint8_t batt = ack_buffer[3];

    // Adding identifiers to the variables so they can be distinguished when dequeueing
    int16_t rssi_data = (int16_t)((rssi) | 0x0100); 
    int16_t pkt_rec_data = (int16_t)((pkt_rec) | 0x0200); 
    int16_t batt_data = (int16_t)((batt) | 0x0300);

    // Enqueueing for the display function
    queue_try_add(&Display_q, &rssi_data);
    queue_try_add(&Display_q, &pkt_rec_data);
    queue_try_add(&Display_q, &batt_data);
}

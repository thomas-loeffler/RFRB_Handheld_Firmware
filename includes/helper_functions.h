//////////////////////////////////////
//             INCLUDES             //
//////////////////////////////////////

#include <stdint.h>
#include "bt_hid.h"
#include "pico/util/queue.h"



//////////////////////////////////////
//    GLOBAL VARIABLE DECLARATION   //
//////////////////////////////////////

extern queue_t Mechanum_q;
extern queue_t TX_q;
extern queue_t Display_q;



//////////////////////////////////////
//             DEFINES              //
//////////////////////////////////////

#define PI 3.1416f
#define COS_45 0.70710678f
#define MOTOR_SCALING 12
#define VELOCITY_MODE 0x07


//////////////////////////////////////
//       FUNCTION PROTOTYPES        //
//////////////////////////////////////

void init_all_queues(void);

uint8_t extract_ds4_lx(struct bt_hid_state* ds4_state);
uint8_t extract_ds4_ly(struct bt_hid_state* ds4_state);
uint8_t extract_ds4_rx(struct bt_hid_state* ds4_state);

uint8_t get_gear(struct bt_hid_state* ds4_state);

int8_t round_and_cast(float x);

void wait_for_transmit(void);

void mecanum_resultant_TEST(float fl, float fr, float bl, float br);

void stdio_send_ds4_outputs(struct bt_hid_state* state);
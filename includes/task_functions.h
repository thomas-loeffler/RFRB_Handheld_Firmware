

//////////////////////////////////////
//             INCLUDES             //
//////////////////////////////////////

#include <stdint.h>
#include "bt_hid.h" // For externs
#include "pico/util/queue.h" // For externs



//////////////////////////////////////
//    GLOBAL VARIABLE DECLARATION   //
//////////////////////////////////////

extern struct bt_hid_state ds4_state;



//////////////////////////////////////
//              DEFINES             //
//////////////////////////////////////


#define ROT_DAMP_FACT 0.5f
#define DISCONNECT_ERROR 0x04FF


//////////////////////////////////////
//       FUNCTION PROTOTYPES        //
//////////////////////////////////////

void get_ds4_inputs(void);

void mechanum_driver(void);

void pack_and_send(void);

void SSD1306_UI_update1(uint8_t pkt_sent, bool transmit, bool link);
void SSD1306_UI_update2(uint8_t pkt_sent, bool transmit, bool link);

void process_ack(void);
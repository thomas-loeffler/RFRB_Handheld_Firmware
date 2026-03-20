//////////////////////////////////////
//             INCLUDES             //
//////////////////////////////////////

#include <stdint.h>
#include "bt_hid.h"




//////////////////////////////////////
//       FUNCTION PROTOTYPES        //
//////////////////////////////////////

uint8_t extract_ds4_lx(struct bt_hid_state* ds4_state);

uint8_t extract_ds4_ly(struct bt_hid_state* ds4_state);

uint8_t extract_ds4_rx(struct bt_hid_state* ds4_state);

int8_t round_and_cast(float x);

void wait_for_transmit(void);

void mecanum_resultant_TEST(float fl, float fr, float bl, float br);
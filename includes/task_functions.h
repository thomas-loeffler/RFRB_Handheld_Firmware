

//////////////////////////////////////
//             INCLUDES             //
//////////////////////////////////////

#include <stdint.h>
#include "bt_hid.h" // for externs
#include "queue.h"



//////////////////////////////////////
//    EXTERNAL GLOBAL VARIABLES     //
//////////////////////////////////////

extern struct bt_hid_state ds4_state;
extern Queue Mechanum_q;		
extern Queue TX_q;




#define MOTOR_SCALING 12


//////////////////////////////////////
//       FUNCTION PROTOTYPES        //
//////////////////////////////////////

void get_ds4_inputs(void);

void mechanum_driver(void);

void pack_and_send(void);
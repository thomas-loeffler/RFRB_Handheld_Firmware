//////////////////////////////////////
//             INCLUDES             //
//////////////////////////////////////

#include <stdint.h>




//////////////////////////////////////
//       FUNCTION PROTOTYPES        //
//////////////////////////////////////



void mechanum_driver(uint8_t raw_x, uint8_t raw_y, 
                    float *fl, float *fr,
                    float *bl, float *br);

int8_t round_and_cast(float x);


void packet_packing(float fl, float fr, float bl, float br, uint8_t *payload);

void mecanum_resultant_TEST(float fl, float fr, float bl, float br);
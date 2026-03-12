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

void mecanum_resultant_TEST(float fl, float fr, float bl, float br);
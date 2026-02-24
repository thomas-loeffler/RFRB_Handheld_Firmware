
//////////////////////////////////////
//             INCLUDES             //
//////////////////////////////////////

#include <stdint.h>



extern const uint8_t SYNC_WORD[4]; // Sync word to use for the radio





//////////////////////////////////////
//       FUNCTION PROTOTYPES        //
//////////////////////////////////////

void rfm69_spi_write(uint8_t reg, uint8_t value);
uint8_t rfm69_spi_read(uint8_t reg);
void radio_reset(void);
void check_version(void);
void radio_setup(void);
void verify_radio_setup(void);

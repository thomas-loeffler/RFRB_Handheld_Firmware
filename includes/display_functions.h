
//////////////////////////////////////
//             INCLUDES             //
//////////////////////////////////////

#include <stdint.h>
#include "bt_hid.h" // so the header knows what type struct bt_hid_state is


//////////////////////////////////////
//            VARIABLES             //
//////////////////////////////////////

extern struct ds4 ds4_state;



//////////////////////////////////////
//       FUNCTION PROTOTYPES        //
//////////////////////////////////////

void send_i2c_command(uint8_t cmd);
void send_i2c_data(uint8_t data);
void set_col_addr(uint8_t start, uint8_t end);
void set_page_addr(uint8_t start, uint8_t end);
void send_small_char(int character, uint8_t col, uint8_t page);
void clear_display(void);
void screen_init(void);
void display_trine_logo(void);
void send_big_char(int character, uint8_t col, uint8_t page);
void send_PS_symbol(int character, uint8_t col, uint8_t page);
void display_all_fonts(void);
void ds4_inputs_display_setup(void);
void display_inputs(struct bt_hid_state* ds4_state);






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

void SSD1306_send_command(uint8_t cmd);
void SSD1306_send_data(uint8_t data);
void SSD1306_set_col_addr(uint8_t start, uint8_t end);
void SSD1306_set_page_addr(uint8_t start, uint8_t end);
void SSD1306_send_small_char(int character, uint8_t col, uint8_t page);
void SSD1306_clear(void);
void SSD1306_init(void);
void SSD1306_display_trine_logo(void);
void SSD1306_send_big_char(int character, uint8_t col, uint8_t page);
void SSD1306_send_PS_symbol(int character, uint8_t col, uint8_t page);
void SSD1306_display_all_fonts(void);
void SSD1306_UI_setup(void);
void SSD1306_display_inputs(struct bt_hid_state* ds4_state);





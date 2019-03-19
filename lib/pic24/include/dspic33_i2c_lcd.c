/*
 * File:   dspic33_i2c_lcd.c
 * Author: wizerdwolf
 *
 * Created on September 11, 2018, 4:29 AM
 */

/*
 1602 LCD documentation - this should be similar to other sizes,
 assuming a fairly generic serial LCD...
 
    pins
 VSS    
 VDD    
 V0     
 RS     register select
 R/W    read/write
 E      
 D0-D7  
 A      
 K      
 
    other terms
 IR     instruction register
 DR     data register
  
 
 
 RS R/W operation
 0  0   IR write as an internal operation (display clear, etc.) 
 0  1   Read busy flag (DB7) and address counter (DB0 to DB7)
 1  0   Write data to DDRAM or CG RAM (DR to DDRAM or CGRAM) 
 1  1   Read data from DDRAM or CG RAM (DDRAM or CGRAM to DR) 
 
  
 
 */



#include "dspic33_i2c_lcd.h"

enum COMMAND {
    lcd_clear,
    lcd_return,
    lcd_entry,
    lcd_display,
    lcd_shift,
    lcd_function,
    lcd_cgram,
    lcd_ddram,
    lcd_busy,
    lcd_write,
    lcd_read
};

typedef struct LCD_IN {
    uint8_t rs : 1;     // 1: data, 0: instruction
    uint8_t rw : 1;     // 1: read, 0: write
    uint8_t e : 1;      // 1, 1->0: enable; I assume we can keep held high?
                        // if we CAN keep it high, no need to include in struct
    uint8_t id : 1;     // direction of cursor; 1: inc, 0: dec
    uint8_t s : 1;      // enable shift of display; 1: on, 0: off
    uint8_t data : 8;   // RB7-0
}LCD_IN;

typedef struct MESSAGE {
    uint8_t high;
    uint8_t low;
}MESSAGE;


//void build_message(LCD_IN inst, MESSAGE msg);
//void build_instruction(COMMAND com, uint8_t data, LCD_IN inst);

void clear(I2C_LCD lcd);
void return_home(I2C_LCD lcd);
void entry_mode_set(I2C_LCD lcd, uint8_t id, uint8_t s);
void display_control(I2C_LCD lcd, uint8_t d, uint8_t c, uint8_t b);
void cursor_display_shift(I2C_LCD lcd, uint8_t sc, uint8_t rl);
void function_set(I2C_LCD lcd, uint8_t dl, uint8_t n, uint8_t f);
void set_cgram_addr(I2C_LCD lcd, uint8_t acg);
void set_ddram_addr(I2C_LCD lcd, uint8_t add);
void read_busy_flag_and_addr(I2C_LCD lcd, uint8_t *bf, uint8_t *ac);
void write_data_to_ram(I2C_LCD lcd, uint8_t data);
void read_data_from_ram(I2C_LCD lcd, uint8_t *data);

void wait_for_lcd(I2C_LCD lcd);
void lcd_printf(I2C_LCD lcd, const char *format, ...);

// Used to block further operations until lcd is no longer busy
void wait_for_lcd(I2C_LCD lcd)
{
    uint8_t busy;
    uint8_t ac;
    do {
        read_busy_flag_and_addr(lcd, &busy, &ac);
    } while(busy>0);
}

// Same syntax as fprintf(), except with type I2C_LCD instead of a file id
void lcd_printf(I2C_LCD lcd, const char *format, ...)
{
    uint8_t size = sizeof(format);
    char *data = malloc(size);
    // insert optional variables into format string to fill data
    
    wait_for_lcd(lcd);
    // for char in data
        // write_data_to_ram(lcd, char);
    
    free(data);
}
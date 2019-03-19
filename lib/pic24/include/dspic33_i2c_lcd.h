/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

#ifndef DSPIC33_I2C_LCD_H
#define	DSPIC33_I2C_LCD_H

#include <xc.h> // include processor files - each processor file is guarded.  

#include "pic24_i2c.h"

typedef struct I2C_LCD {
    uint8_t addr;
    uint8_t rows;
    uint8_t cols;
}I2C_LCD;


void i2c_lcd_init(I2C_LCD lcd);
void i2c_lcd_backlight(I2C_LCD lcd, uint8_t enable);

void i2c_lcd_clear(I2C_LCD lcd);

uint8_t i2c_lcd_setcursor(I2C_LCD lcd, uint8_t loc);
uint8_t i2c_lcd_findcursor(I2C_LCD lcd);

void i2c_lcd_replace(I2C_LCD lcd, uint8_t loc, char c);
void i2c_lcd_insert(I2C_LCD lcd, uint8_t loc, char c);
void i2c_lcd_remove(I2C_LCD lcd, uint8_t loc);

void i2c_lcd_printf(I2C_LCD lcd,const char *format, ...);


uint8_t i2c_lcd_busy(I2C_LCD lcd);  // returns true if busy


#endif	/* DSPIC33_I2C_LCD_H */


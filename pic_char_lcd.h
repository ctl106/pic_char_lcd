/* 

 File:		pic_char_lcd.h
 Author:	Champagne Lewis

 Comment:	

*/

/*
 This software is released under GNU GPL Version 3

 You should have received a copy of the GNU General Public License along with
 this program (README.md).  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef PIC_CHAR_LCD_H
#define	PIC_CHAR_LCD_H

#include <xc.h>


/*		CONFIGURATION FLAGS		*/
// entry mode
#define LCD_DEC 		0x00
#define LCD_INC			0x01
#define LCD_NOSHIFT		0x00
#define LCD_SHIFT		0x02
// display      Probably no need...
#define LCD_NODISPLAY	0x00
#define LCD_DISPLAY		0x04
#define LCD_NOCURSOR	0x00
#define LCD_CURSOR		0x08
#define LCD_NOBLINK		0x00
#define LCD_BLINK		0x10
// function set
#define LCD_4BIT		0x00
#define LCD_8BIT		0x20
#define LCD_FONT_5x8	0x00
#define LCD_FONT_5x11	0x40
// back light set   Probably no need...
#define LCD_NOBACKLIGHT	0x00
#define LCD_BACKLIGHT	0x80

// some defaults
#define LCD_DEFAULT_I2C	LCD_INC 	\
						| LCD_DISPLAY	\
						| LCD_4BIT		\
						| LCD_FONT_5x8	\
						| LCD_BACKLIGHT


// from 0x20 - 0x7D, char encoding is ascii, which represents most use cases
#define LCD_RIGHT_ARROW	0x7E
#define LCD_LEFT_ARROW	0x7F


#ifdef __cplusplus
extern "C"
{
#endif


typedef enum lcd_interface
{
	lcd_i2c1=1,
	lcd_i2c2=2,
	lcd_i2c=3	// don't set to this value; only for bitmasking
	/*
	lcd_spi,
	lcd_gpio
	*/
} lcd_interface;

typedef struct lcd_map
{
	uint8_t d0;
	uint8_t d1;
	uint8_t d2;
	uint8_t d3;
	uint8_t d4;
	uint8_t d5;
	uint8_t d6;
	uint8_t d7;
	uint8_t rs;
	uint8_t rw;
	uint8_t e;
	uint8_t v0;	// pretty sure this is on the i2c expander...
} lcd_map;

typedef struct lcd
{
	lcd_interface interface;
	uint8_t address;
	lcd_map map;
	
	uint8_t lines;
	uint8_t columns;
	uint8_t config;	// flags corresponding to various LCD settings

	// user can read these, but not directly modify
	size_t max_addr;
	uint8_t data;
	uint8_t rs;
	uint8_t rw;
	
	// to be used as a masking value rather than data; DO NOT modify
	uint8_t e;
	uint8_t v0;
} lcd;


// new interface using FILE objects
FILE *lcd_open(lcd *dev);


// old interface not using FILE objects
/*
 Arguably I could have had lcd_putc() take/return and lcd_getc() return a char
 instead of an int as well as repositioned "lcd *dev" before "int character" in
 the arg list for lcd_putc() to be more consistent with lcd_printf() and
 lcd_vprintf(), but I'm trying to keep the interface as consistent with the
 stdio.h functions that they mirror as I can. This way, the developer using the
 library shouldn't have to guess at usage or fuss over documentation too much if
 they already know how to use similar functions in stdio.h.
*/

// Display control functions
void	lcd_clear(lcd *dev);
uint8_t	lcd_current_addr(lcd *dev);
void	lcd_home(lcd *dev);
int		lcd_init(lcd *dev);
int		lcd_move_cursor(lcd *dev, uint8_t row, uint8_t col);
int		lcd_set_addr(lcd *dev, uint8_t addr);

int		lcd_is_backlight(lcd *dev);
void	lcd_set_backlight(lcd *dev, int status);
int		lcd_is_blink(lcd *dev);
void	lcd_set_blink(lcd *dev, int status);
int		lcd_is_cursor(lcd *dev);
void	lcd_set_cursor(lcd *dev, int status);
int		lcd_is_display(lcd *dev);
void	lcd_set_display(lcd *dev, int status);

// System call-like functions
size_t	lcd_read(lcd *dev, void *buf, size_t cnt);
void 	lcd_read_byte(lcd *dev, uint8_t *data);
int		lcd_seek(lcd *dev, int offset, int whence);
size_t	lcd_write(lcd *dev, void *buf, size_t cnt);
void	lcd_write_byte(lcd *dev, uint8_t data);

// Helper functions
int		lcd_is_addr_valid(lcd *dev, uint8_t addr);
char	lcd_create_char(lcd *dev, uint8_t addr, uint8_t bitmap[8]);

#ifdef __cplusplus
}
#endif

#endif	// PIC_CHAR_LCD_H


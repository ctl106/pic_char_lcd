#include <string.h>

#include "pic24_all.h"

#include "pic_char_lcd.h"

int main()
{
    configBasic(HELLO_MSG);
    
	lcd *dev = malloc(sizeof(lcd));
	dev->interface = lcd_i2c1;
	dev->address = 0x27;	// need to double check...
	// let dev->map automatically be set to default
	dev->lines = 2;
	dev->columns = 16;
	dev->config = LCD_DEFAULT_I2C;

	lcd_init(dev);

	char test[] = "test";
	size_t test_s = strlen(test);
	
	lcd_write(dev, test, test_s-1);
	
	while(1)
        IDLE();
}

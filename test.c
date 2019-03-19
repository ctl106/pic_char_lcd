#include "pic_char_lcd.h"

int main()
{
	lcd *dev;
	dev->interface = lcd_i2c1;
	dev->address = 0x27;	// need to double check...
	// let dev->map automatically be set to default
	dev->lines = 2;
	dev->columns = 16;
	dev->config = LCD_DEFAULT_I2C;

	lcd_init(dev);

	size_t test_s = 5;
	char test[test_s] = "test";
	lcd_write(dev, test, test_s-1);
}

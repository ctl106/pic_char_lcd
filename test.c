#include "pic24_all.h"
#include "pic_char_lcd.h"


int main(void) {
printf("Config clock, led, and I2C\n");
    configClock();
  configHeartbeat();
  configDefaultUART(DEFAULT_BAUDRATE);
  printResetCause();       //print statement about what caused reset
  outString(HELLO_MSG);
    configI2C1(100);            // kHz

printf("Config lcd\n");
    lcd r_dev;
	lcd *dev = &r_dev;//(lcd*)malloc(26);
if (!dev) printf("malloc failed :/\n");
	dev->interface = lcd_i2c1;
printf("sizeof(lcd): %d\tdev: %x\tinterface: %d\n", sizeof(lcd), dev, dev->interface);
	dev->address = 0x27<<1;	// need to double check...
	// let dev->map automatically be set to default
	dev->lines = 2;
	dev->columns = 16;
	dev->config = LCD_DEFAULT_I2C | LCD_CURSOR | LCD_BLINK;

printf("Init lcd\n");
	lcd_init(dev);
    
//    lcd_clear(dev);
//    lcd_home(dev);
printf("Write char:\t't'\n");
    lcd_write_byte(dev, 't');

printf("Enter main loop\n");
size_t loop = 0;
  while (1) {
      loop++;
  }
}


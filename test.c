#include "pic24_all.h"
#include "pic_char_lcd.h"


void config_led1() {
// Setup the RB15 pin to drive the LED
// Our normal procedure is to wire up the RB15 LED as opendrain.
// however, some chips do NOT support OD on RB15, so we need to
// first determine if the chip supports OD on RB15.  We will do
// that by seeing if the device header file has define the OD enable
// bit for RB15.  Unfortunately, Microchip has used two different
// names for this bit over the years, so we must check for both.
// If that SFR bit is not defined, then setup a normal CMOS digital
// output on RB15
#if (defined(_ODCB15) || defined(_ODB15))
  CONFIG_RB15_AS_DIG_OUTPUT();
  ENABLE_RB15_OPENDRAIN();
#else
  CONFIG_RB15_AS_DIG_OUTPUT();
#endif
}

// _LATB15 is the port register for RB15.
#define LED1 (_LATB15)

int main(void) {
  configClock();
  config_led1();
    configI2C1(400);            //configure I2C for 400 KHz
    
	lcd *dev = malloc(sizeof(lcd));
	dev->interface = lcd_i2c1;
	dev->address = 0x27;	// need to double check...
	// let dev->map automatically be set to default
	dev->lines = 2;
	dev->columns = 16;
	dev->config = LCD_DEFAULT_I2C | LCD_CURSOR | LCD_BLINK;

	lcd_init(dev);
    
    lcd_clear(dev);
    lcd_home(dev);
    lcd_write_byte(dev, 't');

	/*char test[] = "test";
	size_t test_s = strlen(test);
	
	lcd_write(dev, test, test_s-1);*/
    
  LED1 = 0;
  while (1) {
    // Delay long enough to see LED blink.
    DELAY_MS(250);
    // Toggle the LED.
    LED1 = !LED1;
  } // end while (1)
}


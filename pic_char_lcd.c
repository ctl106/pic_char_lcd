#include <xc.h>
//#include <libpic30.h>	// __delay_us()
#include <stdarg.h>		// va_arg
#include <stdio.h>		// vsnprintf()

// #ifdef macro to determine library to include
//#include "lib/pic24/include/pic24_i2c.h"
//#include "lib/pic24/include/pic24_delay.h"
#include "pic24_all.h"

// #endif

#include "pic_char_lcd.h"


typedef struct message
{
	uint8_t part1;
	uint8_t part2;
} message;


// timing constants
const unsigned long lcd_setup_time1		= 4500;
const unsigned long lcd_setup_time2		= 150;
const unsigned long lcd_enable_time1	= 1;
const unsigned long lcd_enable_time2	= 100;


// ANSI escape sequence(s)
static const char ansi_csi = 0x9b;	// control sequence introducer "ESC [""
static const char ansi_delimiter = ';';
static const char ansi_cup = 'H';	// cursor position


// LCD commands
static void clear_display(lcd *dev);
static void return_home(lcd *dev);
static void entry_mode_set(lcd *dev, uint8_t dir, uint8_t shift);
static void disp_on_off(lcd *dev, uint8_t disp, uint8_t cursor, uint8_t blink);
static void cursor_or_disp_shift(lcd *dev, uint8_t select, uint8_t direction);
static void function_set(lcd *dev, uint8_t mode, uint8_t lines, uint8_t font);
static void set_cgram_addr(lcd *dev, uint8_t addr);
static void set_ddram_addr(lcd *dev, uint8_t addr);
static void read_busy_addr(lcd *dev, uint8_t *busy, uint8_t *addr);
static void write_to_ram(lcd *dev, uint8_t data);
static void read_from_ram(lcd *dev, uint8_t *data);

// formatting functions
static int		at_eof(lcd *dev);
static int		at_eol(lcd *dev);
static uint8_t	current_line(lcd *dev);
static void		newline(lcd *dev);

// helper functions
static void command(lcd *dev);	// generic low-level interface to LCD
static void command_4bit(lcd *dev);	// 4bit interface to LCD
static void	default_i2c_map(lcd_map map);
static void	init_4bit(lcd *dev);
static void	init_8bit(lcd *dev);
static int	is_busy(lcd *dev);
static int	is_map_valid(uint8_t mode, lcd_map map);
static void	map_message(lcd *dev, message msg);	// for non-GPIO interfaces
static void	unmap_message(lcd *dev, message msg);
static void	newline(lcd *dev);
static void	read_4bit(lcd *dev, uint8_t *data);
static void	write_4bit(lcd *dev, uint8_t data);
static void	reset_values(lcd *dev);	// set data, rs, and rw to 0
static void	send_byte(lcd *dev, uint8_t data);
static void	set_v0(lcd *dev, int status);


/*
 some notes on common DDRAM addressing based on display size

 start		end			range

 1 line
 0x00 - 0	0x4F - 79	80

 2 line
 0x00 - 0	0x27 - 39	40
 0x40 - 64	0x67 - 103	40

 4 line
 0x00 - 0	0x13 - 19	20
 0x40		0x53		20
 0x14		0x27		20
 0x54		0x67		20
*/


// high level  API

void lcd_clear(lcd *dev)
{
	while(is_busy(dev));
	clear_display(dev);
}

char lcd_create_char(lcd *dev, uint8_t addr, uint8_t bitmap[8])
{
	if(addr > 0x07)
		return -1;
	
	// back up ddram addr
	uint8_t backup_addr = lcd_current_addr(dev);
	
	// what we're actually here for: storing the bitmap
	set_cgram_addr(dev, addr<<3);
	int i=0;
	for(i=0; i<8; i++) {
		while(is_busy(dev));
		write_to_ram(dev, bitmap[i]);
	}
	
	// restore ddram addr
	lcd_set_addr(dev, backup_addr);
	
	return (char)addr;
}

uint8_t lcd_current_addr(lcd *dev)
{
	uint8_t pos;
	while(is_busy(dev));
	read_busy_addr(dev, NULL, &pos);
	return pos;
}

int lcd_init(lcd *dev)
{
	/*
	documentation appears to say you can't
	check busy till after init is complete.
	*/
	
	// generate bitmasks for enable and backlight
	dev->e = 0x01 << dev->map.e;
	dev->v0 = 0x01 << dev->map.v0;
	
	// determine size of ddram based on rows x cols
	
	if(!is_map_valid(dev->config&LCD_8BIT, dev->map)) {
		if(dev->interface&lcd_i2c)
			default_i2c_map(dev->map);
		// if(dev->interface & lcd_spi)
	}
	
	if(dev->config&LCD_8BIT)
		init_8bit(dev);
	else
		init_4bit(dev);
	
	function_set(
				dev,
				dev->config&LCD_8BIT,
				dev->lines,
				dev->config&LCD_FONT_5x11
				);
				
	disp_on_off(
				dev,
				dev->config&LCD_DISPLAY,
				dev->config&LCD_CURSOR,
				dev->config&LCD_BLINK
				);
				
	clear_display(dev);
				
	entry_mode_set(
				dev,
				dev->config&LCD_INC,
				dev->config&LCD_NOSHIFT
				);
				
	return 0;
}

int lcd_is_addr_valid(lcd *dev, uint8_t addr)
{
	int status = 1;
	
	if (	(dev->lines == 1 && addr > 0x4F)				||
			(dev->lines == 2 && addr > 0x27 && addr < 0x40)	||
			(dev->lines == 2 && addr > 0x67)				||
			(dev->lines == 4 && addr > 0x27 && addr < 0x40)	||
			(dev->lines == 4 && addr > 0x67)				)
		status = 0;
	
	return status;
}

int lcd_is_backlight(lcd *dev)
{
	return dev->config & LCD_BACKLIGHT;
}

int lcd_is_blink(lcd *dev)
{
	return dev->config & LCD_BLINK;
}

int lcd_is_cursor(lcd *dev)
{
	return dev->config & LCD_CURSOR;
}

int lcd_is_display(lcd *dev)
{
	return dev->config & LCD_DISPLAY;
}

int lcd_move_cursor(lcd *dev, uint8_t row, uint8_t col)
{
	uint8_t addr = 0x00;
	int status = 0;
	
	switch(dev->lines)
	{
		case 1:
			if (row != 0 || col > 0x4F)
				status = -1;
			
		case 2:
			if (col > 0x27)
				status = -1;
			else {
				switch(row)
				{
					case 0:
						addr = 0x00;
						break;
					case 1:
						addr = 0x40;
						break;
					default:
						status = -1;
				}
			}
			break;
			
		case 4:
			if (col > 0x13)
				status = -1;
			else {
				switch(row)
				{
					case 0:
						addr = 0x00;
						break;
					case 1:
						addr = 0x40;
						break;
					case 2:
						addr = 0x14;
						break;
					case 4:
						addr = 0x54;
						break;
					default:
						status = -1;
				}
			}
			break;
			
		default:
			status = -1;
	}
	
	if (!status){
		addr += col;
		status = lcd_set_addr(dev, addr);
	}
	
	return status;
}

size_t lcd_read(lcd *dev, void *buf, size_t cnt)
{
	uint8_t total;
	for(total = 0; total < cnt && at_eof(dev); total++)
		lcd_read_byte(dev, ((uint8_t*)buf)+total);
	return total;
}

void lcd_read_byte(lcd *dev, uint8_t *data)
{
	//int status = 0;
	while(is_busy(dev));
	read_from_ram(dev, data);
	//return status;
}

int lcd_seek(lcd *dev, int offset, int whence)	// NOT DONE; need a more elegant solution...
{
	uint8_t pos;
	uint8_t addr = 0;
	int status = 0;
	int jump = 1;
	
	switch(whence)	// determine starting point
	{
		case SEEK_SET:
			pos = 0;
		case SEEK_CUR:
			pos = lcd_current_addr(dev);
			if (offset == 0)
				jump = 0;
			break;
		case SEEK_END:
			pos = dev->max_addr + 1;
			break;
		default:
			status = -1;
			jump = 0;
	}
	
	if (status >= 0) {
		if (offset > 80)	// cut down offset to make calculation easier
			offset %= 80;
		else while (offset < 0)
			offset += 80;

		uint8_t div, rem;	// fit the pos+offset to the DDRAM address space
		switch(dev->lines)
		{
			case 1:		// 1 line mode has 1 continuous block of ram
				addr = offset + pos;
				if (addr > 0x4F)
					addr -= 0x4F;
				break;

			case 2:		// 2 line mode has 2 separated blocks of ram
				div = offset / 40;
				rem = offset % 40;
				if (div % 2)
					pos = (pos <= 0x27) ? pos+0x40 : pos-0x40;
				addr = pos + rem;
				break;

			case 4:		// 4 lines has 2, but the lines are mixed up, so treat as 4
				div = offset / 20;
				rem = offset % 20;
				switch(div)
				{
					case 1:
						if (pos <= 0x13)					pos += 0x40;
						else if (pos>=0x40 && pos<=0x53)	pos -= 0x40 - 0x14;
						else if (pos>=0x14 && pos<=0x27)	pos += 0x54 - 0x14;
						else								pos -= 0x54;
						break;
					case 2:
						if (pos <= 0x13)					pos += 0x14;
						else if (pos>=0x40 && pos<=0x53)	pos += 0x54 - 0x40;
						else if (pos>=0x14 && pos<=0x27)	pos -= 0x14;
						else								pos -= 0x54 - 0x40;
						break;
					case 3:
						if (pos <= 0x13)					pos += 0x54;
						else if (pos>=0x40 && pos<=0x53)	pos -= 0x40;
						else if (pos>=0x14 && pos<=0x27)	pos += 0x40 - 0x14;
						else								pos -= 0x50 - 0x14;
						break;
				}
				addr = pos + rem;
				break;

			default:
				status = -1;
				jump = 0;
		}

		if (jump) {
			status = lcd_set_addr(dev, addr);
			if (status == 0)
				status = addr;
		}
	}
	
	return status;
}

int lcd_set_addr(lcd *dev, uint8_t addr)
{
	int status = 0;
	
	if (lcd_is_addr_valid(dev, addr)) {
		while(is_busy(dev));
		set_ddram_addr(dev, addr);
	} else
		status = -1;
	
	return status;
}

void lcd_set_backlight(lcd *dev, int status)
{
	while(is_busy(dev));
	if(status)
		dev->config |= LCD_BACKLIGHT;
	else
		dev->config &= ~LCD_BACKLIGHT;
	set_v0(dev, status);
}

void lcd_set_blink(lcd *dev, int status)
{
	uint8_t display, cursor;
	display = dev->config | LCD_DISPLAY;
	cursor = dev->config | LCD_CURSOR;
	
	if (status)
		dev->config |= LCD_BLINK;
	else
		dev->config &= ~LCD_BLINK;
	
	disp_on_off(dev, display, cursor, (uint8_t)status);
}

void lcd_set_cursor(lcd *dev, int status)
{
	uint8_t blink, display;
	blink = dev->config | LCD_BLINK;
	display = dev->config | LCD_DISPLAY;
	
	if (status)
		dev->config |= LCD_CURSOR;
	else
		dev->config &= ~LCD_CURSOR;
	
	disp_on_off(dev, display, (uint8_t)status, blink);
}

void lcd_set_display(lcd *dev, int status)
{
	uint8_t blink, cursor;
	blink = dev->config | LCD_BLINK;
	cursor = dev->config | LCD_CURSOR;
	
	if (status)
		dev->config |= LCD_DISPLAY;
	else
		dev->config &= ~LCD_DISPLAY;
	
	disp_on_off(dev, (uint8_t)status, cursor, blink);
}

size_t lcd_write(lcd *dev, void *buf, size_t cnt)
{
	size_t total;
	uint8_t data;
	
	for(total = 0; total < cnt; total ++) {
		data = ((char*)buf)[total];
		if (data == '\n')
			newline(dev);
		else {
			lcd_write_byte(dev, data);
		}
	}
	
	return total;
}

void lcd_write_byte(lcd *dev, uint8_t data)
{
	// a fix for the fact that the DDRAM for 4 line displays go: 1, 3, 2, 4
	int line = -1;
	if (at_eol(dev)) {
		line = current_line(dev) + 1;
		if (line >= dev->lines)
			line %= dev->lines;
	}
	
	while(is_busy(dev));
	write_to_ram(dev, data);
	
	if (line >= 0)
		lcd_move_cursor(dev, line, 0);
}


// LCD commands

static void clear_display(lcd *dev)
{
	reset_values(dev);
	dev->data = 0x01;
	command(dev);
}

static void return_home(lcd *dev)
{
	reset_values(dev);
	dev->data = 0x02;
	command(dev);
}

static void entry_mode_set(lcd *dev, uint8_t dir, uint8_t shift)
{
	reset_values(dev);
	dev->data = 0x04;
	
	if(dir)
		dev->data |= 0x02;
	if(shift)
		dev->data |= 0x01;
	
	command(dev);
}

static void disp_on_off(lcd *dev, uint8_t disp, uint8_t cursor, uint8_t blink)
{
	reset_values(dev);
	dev->data = 0x08;
	
	if(disp)
		dev->data |= 0x04;
	if(cursor)
		dev->data |= 0x02;
	if(blink)
		dev->data |= 0x01;
	
	command(dev);
}

static void cursor_or_disp_shift(lcd *dev, uint8_t select, uint8_t direction)
{
	reset_values(dev);
	dev->data = 0x10;
	
	if(select)
		dev->data |= 0x08;
	if(direction)
		dev->data |= 0x04;
	
	command(dev);
}

static void function_set(lcd *dev, uint8_t mode, uint8_t lines, uint8_t font)
{
	reset_values(dev);
	dev->data = 0x20;
	
	if(mode)
		dev->data |= 0x10;
	if(lines)
		dev->data |= 0x08;
	if(font)
		dev->data |= 0x04;
	
	command(dev);
}

static void set_cgram_addr(lcd *dev, uint8_t addr)
{
	reset_values(dev);
	dev->data = 0x40 | (0x3F & addr);
	command(dev);
}

static void set_ddram_addr(lcd *dev, uint8_t addr)
{
	reset_values(dev);
	dev->data = 0x80 | (0x7F & addr);
	command(dev);
}

static void read_busy_addr(lcd *dev, uint8_t *busy, uint8_t *addr)
{
	dev->data = 0xFF;	// pins must float for LCD to drive them
	dev->rs = 0;	// instruction register selected
	dev->rw = 1;
	
	command(dev);
	
	if(dev->data & 0x80)
		*busy = 0xFF;
	*addr = dev->data & 0x7F;
}

static void write_to_ram(lcd *dev, uint8_t data)
{
	reset_values(dev);
	dev->data = data;
	dev->rs = 1; // data register selected
	command(dev);
}

static void read_from_ram(lcd *dev, uint8_t *data)
{
	dev->data = 0xFF;	// pins must float for LCD to drive them
	dev->rs = 1;	// data register selected
	dev->rw = 1;
	
	command(dev);
	*data = dev->data;
}


// formatting functions

static int at_eof(lcd *dev)
{
	uint8_t pos;
	pos = lcd_current_addr(dev);
	int output = (pos == dev->max_addr);
	return output;
}

static int at_eol(lcd *dev)
{
	int output = 0;
	uint8_t pos;
	pos = lcd_current_addr(dev);
	
	switch (dev->lines)
	{
		case 1:
			if (pos == 0x45)
				output = 1;
			break;
		case 2:
			if (pos == 0x27 || pos == 0x67)
				output = 1;
			break;
		case 4:
			if (pos == 0x13 || pos == 0x53 || pos == 0x27 || pos == 0x67)
				output = 1;
			break;
	}
	
	return output;
}

static uint8_t current_line(lcd *dev)
{
	uint8_t pos, line;
	pos = lcd_current_addr(dev);
	
	switch(dev->lines)
	{
		case 1:
			line = 0;
			break;
			
		case 2:
			if (pos <= 0x27)
				line = 0;
			else
				line = 1;
			break;
			
		case 4:
			if (pos <= 0x13)
				line = 0;
			else if (pos >= 0x40 && pos <= 0x53)
				line = 1;
			else if (pos >= 0x14 && pos <= 0x27)
				line = 2;
			else
				line = 3;
			break;
			
		default:
			line = 0;
	}
	
	return line;
}

static void newline(lcd *dev)
{
	uint8_t line = current_line(dev) + 1;
	if (line >= dev->lines)
		line %= dev->lines;	
	lcd_move_cursor(dev, line, 0);
}


// helper functions

static void command(lcd *dev)
{
	if(dev->config&LCD_8BIT)
		return;
	else
		command_4bit(dev);
}

static void command_4bit(lcd *dev)
{
	message msg;
	map_message(dev, msg);
	
	write_4bit(dev, msg.part1);
	read_4bit(dev, &msg.part1);
	write_4bit(dev, msg.part2);
	read_4bit(dev, &msg.part2);
	
	unmap_message(dev, msg);
}

static void default_i2c_map(lcd_map map)
{
	map.rs = 0;
	map.rw = 1;
	map.e = 2;
	map.v0 = 3;
	map.d4 = 4;
	map.d5 = 5;
	map.d6 = 6;
	map.d7 = 7;
	
	// unused in 4 data bit i2c mode
	map.d0 = 0xFF;
	map.d1 = 0xFF;
	map.d2 = 0xFF;
	map.d3 = 0xFF;
}

static void init_4bit(lcd *dev)
{
	message msg;
	reset_values(dev);
	
	// start in 8 bit mode, x3 bursts
	dev->data = 0x20 | LCD_8BIT;
	map_message(dev, msg);
	
	write_4bit(dev, msg.part1);
	DELAY_US(lcd_setup_time1);
	
	write_4bit(dev, msg.part1);
	DELAY_US(lcd_setup_time1);
	
	write_4bit(dev, msg.part1);
	DELAY_US(lcd_setup_time2);
	
	// aaand finally set to 4bit mode
	dev->data = 0x20 | LCD_4BIT;
	map_message(dev, msg);
	write_4bit(dev, msg.part1);
}

static void init_8bit(lcd *dev)
{
	function_set(dev, LCD_8BIT, 0, 0);
	DELAY_US(lcd_setup_time1);
	
	function_set(dev, LCD_8BIT, 0, 0);
	DELAY_US(lcd_setup_time2);
	
	function_set(dev, LCD_8BIT, 0, 0);
}

int is_busy(lcd *dev)
{
	uint8_t busy;
	read_busy_addr(dev, &busy, NULL);
	return (int)busy;
}

static int is_map_valid(uint8_t mode, lcd_map map)
{
	if(mode == LCD_8BIT) {
		return 0;	// GPIO only, so individually check against a list of pins
	
	} else {	// we need pins 0-7 assigned
		size_t assignments[8] = {0};
		assignments[map.d4] += 1;
		assignments[map.d5] += 1;
		assignments[map.d6] += 1;
		assignments[map.d7] += 1;
		assignments[map.rs] += 1;
		assignments[map.rw] += 1;
		assignments[map.e] += 1;
		assignments[map.v0] += 1;
		
		int i;
		for(i=0; i<8; i++) if(assignments[i] != 1) return 0;
		
		return 1;
	}
}

static void map_message(lcd *dev, message msg)
{
	msg.part1 = 0x00;
	msg.part2 = 0x00;
	
	if(dev->data & 0x80)
		msg.part1 |= 0x01 << dev->map.d7;
	if(dev->data & 0x40)
		msg.part1 |= 0x01 << dev->map.d6;
	if(dev->data & 0x20)
		msg.part1 |= 0x01 << dev->map.d5;
	if(dev->data & 0x10)
		msg.part1 |= 0x01 << dev->map.d4;
	if(dev->data & 0x08)
		msg.part2 |= 0x01 << dev->map.d7;
	if(dev->data & 0x04)
		msg.part2 |= 0x01 << dev->map.d6;
	if(dev->data & 0x02)
		msg.part2 |= 0x01 << dev->map.d5;
	if(dev->data & 0x01)
		msg.part2 |= 0x01 << dev->map.d4;
	
	if(dev->rs) {
		msg.part1 |= 0x01 << dev->map.rs;
		msg.part2 |= 0x01 << dev->map.rs;
	}
	if(dev->rw) {
		msg.part1 |= 0x01 << dev->map.rw;
		msg.part2 |= 0x01 << dev->map.rw;
	}
	
	if(dev->config & LCD_BACKLIGHT) {
		msg.part1 |= 0x01 << dev->map.v0;
		msg.part2 |= 0x01 << dev->map.v0;
	}
}

static void unmap_message(lcd *dev, message msg)
{
	dev->data = 0x00;
	dev->data |= ((msg.part1 >> dev->map.d7) & 0x01) << 7;
	dev->data |= ((msg.part1 >> dev->map.d6) & 0x01) << 6;
	dev->data |= ((msg.part1 >> dev->map.d5) & 0x01) << 5;
	dev->data |= ((msg.part1 >> dev->map.d4) & 0x01) << 4;
	dev->data |= ((msg.part2 >> dev->map.d7) & 0x01) << 3;
	dev->data |= ((msg.part2 >> dev->map.d6) & 0x01) << 2;
	dev->data |= ((msg.part2 >> dev->map.d5) & 0x01) << 1;
	dev->data |= ((msg.part2 >> dev->map.d4) & 0x01);
	
	/*	these aren't needed when reading in values...
	dev->rs = (msg.part1 >> dev->map.rs) & 0x01;
	dev->rw = (msg.part1 >> dev->map.rw) & 0x01;
	*/
}

static void read_4bit(lcd *dev, uint8_t *data)
{
	if(dev->interface == lcd_i2c1)
		read1I2C1(dev->address, data);
	else if(dev->interface == lcd_i2c2)
		read1I2C2(dev->address, data);
}

static void write_4bit(lcd *dev, uint8_t data)
{
	data &= ~dev->e;
	send_byte(dev, data);
	DELAY_US(lcd_enable_time1);
	
	data |= dev->e;
	send_byte(dev, data);
	DELAY_US(lcd_enable_time1);
	
	data &= ~dev->e;
	send_byte(dev, data);
	DELAY_US(lcd_enable_time2);
}

static void reset_values(lcd *dev)
{
	dev->data = 0x00;
	dev->rs = 0;
	dev->rw = 0;
}

static void send_byte(lcd *dev, uint8_t data)
{
	if(dev->interface == lcd_i2c1)
		write1I2C1(dev->address, data);
	else if(dev->interface == lcd_i2c2)
		write1I2C2(dev->address, data);
}

static void set_v0(lcd *dev, int status)
{
	uint8_t data;
	read_4bit(dev, &data);
	if(status)
		data |= dev->v0;
	else
		data &= ~dev->v0;
	send_byte(dev, data);	// doesn't toggle e like write_4bit; this is desired
}


/*
 let's try to hack out a libc interface to allow fopen(), fprintf(), etc...
 Bit uncomfortable with this because I don't know how fopen() works, and if it
 can find different definitions of open() to use. If only one open() can be
 recognized by fopen() in a given hex, then this and pic24_stdio_uart.h
 would not be both usable at the same time...
*/ 
/*
// ripped the following chunk from pic24_stdio_uart.h
// These definitions are for libc placement and compatibility
#define _LIBC_FUNCTION __attribute__((__weak__, __section__(".libc")))
#define SUCCESS 0
#define FAIL -1
#define CHAR_ACCESS 0x4000
#define DATA_ACCESS 0x8000  //this flag assumed if CHAR_ACCESS not set
#define READ_ACCESS 0x0
#define WRITE_ACCESS 0x1
#define READ_WRITE_ACCESS 0x2
#define ACCESS_RW_MASK 0x3 // bits set to 0 or 2 for read and 1 or 2 for write
#define ACCESS_SET_OPEN DATA_ACCESS // expedient - reuse bit position to indicate open


// libc interface

int _LIBC_FUNCTION	open(const char *name, int access, int mode);
int _LIBC_FUNCTION	read(int handle, void *buffer, unsigned int len);
int _LIBC_FUNCTION	write(int handle, void *buffer, unsigned int len);
int _LIBC_FUNCTION	close(int handle);
long _LIBC_FUNCTION	lseek(int handle, long offset, int origin);


// helpers to libc interface
int add_lcd(lcd *dev);

// libc interface

int _LIBC_FUNCTION open(const char *name, int access, int mode)
{
	return FAIL;
}

int _LIBC_FUNCTION read(int handle, void *buffer, unsigned int len)
{
	return FAIL;
}

int _LIBC_FUNCTION write(int handle, void *buffer, unsigned int len)
{
	return FAIL;
}

int _LIBC_FUNCTION close(int handle)
{
	// function required by libc, but has no useful function for pic_char_lcd
	// we are not buffering the data to be sent
	return SUCCESS;
}

long _LIBC_FUNCTION lseek(int handle, long offset, int origin)
{
	// unlike pic24_stdio_uart, this is useful for LCD DDRAM, which is file-like
	return FAIL;
}

// helpers to libc interface

*/


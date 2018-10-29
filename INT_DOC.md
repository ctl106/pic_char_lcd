### Interface
###### `pic_char_lcd.h`

It is important to stress that in early development, this API is volatile and
should not be relied upon. After the basic API is finished, it should be stable.

#### Methods

------
##### `lcd_clear`
````c
void lcd_clear(lcd *dev);
````
###### Behavior
Clears the contents of the display of `dev` and resets the cursor position
to the first address in DDRAM.

------
##### `lcd_create`
````c
char lcd_create_char(lcd *dev, uint8_t addr, uint8_t bitmap[8]);
````
###### Behavior
Writes the `bitmap` to the CGRAM location specified by `addr` on `dev`.

`bitmap` is a pixel representation of a character in a 5x8 font that, once
created on the device, can be displayed on `dev`.

`addr` is one of the CGRAM addresses in the range of `0x00`-`0x07`

###### Returns
On success, `addr` is returned as a `char` so that it can be used when creating
future strings to write to DDRAM. Once the returned `char` is written to ddram,
`dev` will render the `bitmap` onto the display.

###### Notes
The optional include `pic_char_lcd_cg.h` contains macro definitions for various
custom characters. These macros can either be used as an example for how
to create `bitmap`s to be written to `dev`, OR they can themselves be used as
`bitmap` to be written to `dev`.

------
##### `lcd_current_addr`
````c
uint8_t lcd_current_addr(lcd *dev);
````

###### Behavior
Queries `dev` for the current address in DDRAM.

###### Returns
Returns the current DDRAM address as a `uint8_t`.

------
##### `lcd_init`
````c
int lcd_init(lcd *dev);
````

###### Behavior
Performs the initial setup of `dev`. This MUST be performed before the device
can be used and any other operations can be performed.

###### Returns
A `0` is returned upon success and non-`0` on failure.

------
##### `lcd_is_addr_valid`
````c
int lcd_is_addr_valid(lcd *dev, uint8_t addr);
````

###### Behavior
Evaluates whether or not `addr` is a valid DDRAM address for the given `dev`,
as determined by the line mode and size of display.

###### Returns
Returns `0` for False if `addr` is not valid and non-`0` for True if `addr` is valid.

------
##### `lcd_move_cursor`
````c
int lcd_move_cursor(lcd *dev, uint8_t row, uint8_t col);
````

###### Behavior
Sets the cursor of `dev` to the row `row` and column `col`.

###### Returns
Returns `0` on success and non-`0` on failure.
Possible causes of failure are invalid `row` and `col` parameters.

------
##### `lcd_read`
````c
size_t lcd_read(lcd *dev, void *buf, size_t cnt);
````
Should behaive similar to the `READ(2)` system call.
http://man7.org/linux/man-pages/man2/read.2.html

CANDIDATE FOR REMOVAL as a read operation from an output device is fairly
pointless. So far, this is only included for the sake of completion and providing
full-support for the LCD's function set.

###### Behavior
Attempts to read `cnt` bytes from `dev`, starting at the current location of the
cursor, and storing the results into `buf`. The cursor will shift according to the
`Entry Mode` flags set in config.

###### Returns
Returns the total number of bytes stored into `buf` as type `size_t`. It is not an
error for the returned value to be less than `cnt` as reading stops at the end of
DDRAM.

------
##### `lcd_read_byte`
````c
void lcd_read_byte(lcd *dev, uint8_t *data);
````

CANDIDATE FOR REMOVAL as a read operation from an output device is fairly
pointless. So far, this is only included for the sake of completion and providing
full-support for the LCD's function set.

###### Behavior
Reads a single byte from `dev` at the current location of the cursor, and stores
the result into `data`. The cursor will shift according to the `Entry Mode` flags
set in config. 

------
##### `lcd_seek`
````c
int lcd_seek(lcd *dev, int offset, int whence);
````
Should behaive similar to the `LSEEK(2)` system call.
No support currently planned for `SEEK_DATA` or `SEEK_HOLE`.
http://man7.org/linux/man-pages/man2/lseek.2.html

###### Behavior
Repositions the `dev`'s cursor to `offset` relative to `whence`.

Supported values for `whence`:

| Value | Behavior |
| --- | --- |
| `SEEK_SET` | Start at the beginning of DDRAM |
| `SEEK_CUR` | Start at the current location of the cursor |
| `SEEK_END` | Start t the end of DDRAM |

###### Returns
Returns the new address of the cursor, a `uint8_t` value, type-cast as an `int`
on success, and returns <`0` on failure.

------
##### `lcd_set_addr`
````c
int lcd_set_addr(lcd *dev, uint8_t addr);
````

###### Behavior
Sets the `dev`'s cursor to the DDRAM address `addr`.

###### Returns
Returns `0` on success and non-`0` on failure.
A possible cause of failure is passing an invalid `addr` value.

------
##### `lcd_write`
````c
size_t lcd_write(lcd *dev, void *buf, size_t cnt);
````
Should behaive similar to the `WRITE(2)` system call.
http://man7.org/linux/man-pages/man2/write.2.html

###### Behavior
Writes up to `cnt` bytes from `buf` to `dev`'s display starting at the current
cursor location. The cursor will shift according to the `Entry Mode` flags set in
config.

Like `WRITE(2)`, writing WILL NOT stop on `\0`. Furthermore, since the ascii
value of `\0` is `0x00`, this WILL refer to the first custom character address,
and will result in the bitmap at that address being displayed.

The current implementation has the diplay go to the beginning of the next row
upon reaching a `\n` character so that data-entry will be more familiar
to writting data to text files, `stdout`, and `stderr`.

###### Returns
Returns the number of bytes as `size_t`written to the display.

###### Notes
I have yet to find in documentation if the cursor stops at the last address in
DDRAM when writing, or if it loops to the very first address. As a result, behavior
is currently undefined when attempting to write would cause the cursor to reach
or exceed the last address in `dev`'s DDRAM.

Support for `\t` may eventually come as well. Alternatively, `\t` may not be
implemented due to how small these displays are as is as well as the added
complexity.

The ANSI escape sequence for the "control sequence intruder" may be
implemented to allow reposition the cursor one or more times within a call
to `lcd_write`. Since this escape sequence requires a row and column value,
it would be translated into a function call to `lcd_move_cursor`.

------
##### `lcd_write_byte`
````c
void lcd_write_byte(lcd *dev, uint8_t chr);
````

###### Behavior
Writes `char` to `dev`'s display at the current cursor location. . The cursor will
shift according to the `Entry Mode` flags set in config.

Unlike `lcd_write`, no escape characters are substituted for another behavior.
Any single byte supplied as `char` will be written as a literal value.

------

#### Types

------
##### `lcd_interface`
````c
typedef enum lcd_interface lcd_interface;
````

The `lcd_interface` value is used to indicate which hardware interface is being
used to communicate with a given LCD

| Value | Interface |
| --- | --- |
| `lcd_i2c1` | I2C bus #1 |
| `lcd_i2c2` | I2C bus #2 |
| `lcd_i2c` | DO NOT SET THIS VALUE; indicates either I2C bus |
| `lcd_spi` | NOT IMPLEMENTED; SPI bus |
| `lcd_gpio` | NOT IMPLEMENTED; GPIO pins of the host PIC |

------
##### `lcd_map`
````c
typedef struct lcd_map lcd_map;
````
A structure that indicates which pins on the given `lcd_interface` corresponds
to the control and data pins on the connected LCD.

| Member | Pin |
| --- | --- |
| `uint8_t d0` | data bit 0; located at 0b 0000 0001 |
| `uint8_t d1` | data bit 1; located at 0b 0000 0010 |
| `uint8_t d2` | data bit 2; located at 0b 0000 0100 |
| `uint8_t d3` | data bit 3; located at 0b 0000 1000 |
| `uint8_t d4` | data bit 4; located at 0b 0001 0000 |
| `uint8_t d5` | data bit 5; located at 0b 0010 0000 |
| `uint8_t d6` | data bit 6; located at 0b 0100 0000 |
| `uint8_t d7` | data bit 7; located at 0b 1000 0000 |
| `uint8_t rs` | register select |
| `uint8_t rw` | read/write |
| `uint8_t e` | enable |
| `uint8_t v0` | backlight |

###### Notes
I have not yet confirmed that `v0` actually is connected to the I2C I/O expander,
but I'm pretty sure there is. Otherwise, one of the 8 bits on the expander is
entirely wasted.

------
##### `lcd`
````c
typedef struct lcd lcd;
````
A structure that directly represents no more than 1 connected LCD.

###### Members to set before calling `lcd_init`
| Member | Represents |
| --- | --- |
| `lcd_interface interface` | Which interface the LCD is on |
| `uint8_t address` | The address of the device (1) |
| `lcd_map map` | Links `interface` pins to the LCD pins |
| `uint8_t lines` | How many rows on the LCD display are to be used |
| `uint8_t columns` | How many columns the LCD display has |
| `uint8_t config` | `Config Flags` to specify the settings on the LCD |

###### Remaining members (2)
| Members | Represents |
| --- | --- |
| `size_t max_addr` | The last address in DDRAM |
| `uint8_t data` | A temporary container representing the values of `d0`-`d7` |
| `uint8_t rs` | A temporary container representing the value of `rs` |
| `uint8_t rw` | A temporary container representing the value of `rw` |
| `uint8_t e` | Bitmask used by the low-level interface to select `e` |
| `uint8_t v0` | Bitmask used by the low-level interface to select `v0` |

###### Notes
1. `address` is ignored when `interface` is set as `lcd_gpio`.
Additionally, other members might need to be added if `interface` is `lcd_spi`.

2. The members under this heading should NOT be modified or set by user code
but rather is for the internal operation of this library. Additionally the presence
and current definition of these members should not be relied upon.

------

#### Config Flags
These are to be bit-wise ORed together when setting the config of the LCD.

##### Entry Mode

| Flag | Behavior |
| --- | --- |
| `LCD_DEC` | Decrement RAM address on write |
| `LCD_INC` | Increment RAM address on write |
| `LCD_NOSHIFT` | Do not shift display on write |
| `LCD_SHIFT` | Shift display on write, keeping the cursor stationary |

##### Function Set

| Flag | Behavior |
| --- | --- |
| `LCD_4bit` | Enable 4 data bit mode |
| `LCD_8bit` | Enable 8 data bit mode |
| `LCD_FONT_5x8` | Enables 5x8 fonts; needed for 2 line mode |
| `LCD_FONT_5x11` | Enables 5x11 fonts; 1 or 2 line mode |

##### Default Configs

| Flag | Behavior |
| --- | --- |
| `LCD_DEFAULT_I2C` | Sets commonly needed bits for I2C usage |
| `LCD_DEFAULT_SPI` | NOT IMPLEMENTED |
| `LCD_DEFAULT_GPIO` | NOT IMPLEMENTED |

------

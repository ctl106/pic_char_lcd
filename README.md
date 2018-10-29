# pic_char_lcd
###### A driver library for generic character LCDs targetted at Pic MCUs
------


### Purpose

pic_char_lcd is a driver library targetted at the Microchip Pic family of
microcontrollers allowing easy control over generic character LCDs.

Currently support for LCDs with I2C I/O expander backpacks is underway, but
support is planned also for SPI backpacks as well as direct control from the
GPIO pins of the host device.

Currently it is being developed against 16 bit MCUs, specifically the
dsPIC33EP128GP502, but the finished product should be usable amongst a wide
assortment of Microchip MCUs.

### Goals

1. Specify the basic API
2. Write tests against API
3. Create overridden I2C commands for testbenching
4. [done] Implement low-level LCD commands
5. [done] Implement I2C interface
6. Implement basic API
7. Create overridden SPI commands and SPI interface
8. Create GPIO commands that can be overridden and GPIO interface

### Setup

Given that this library is not in a working state yet, I cannot give a complete
setup instructions.

When a working version is complete, it may be ideal to generate a precompiled
library file against the target device to place in a common location for future
projects to include and utilize.

While developing this library, or as an alternative to compiling a library
file, this library should be cloned and the header locations added to the the
search path of the project utilizing this library.

### Testing the Library

A testbench is to be written for this library against the public API. All
commands for peripheral devices (I2C, SPI, etc) are to be overridden so that
functionality can be easily tested via simulation in the MPLAB X IDE so that
a physical device with connected peripherals is not required. A macro should be
defined that will allow the test code to run on physical hardware instead of
only relying on the simulated device to test the functionality.

------

### Usage Examples

Usage examples will likely be developed alongside the test suite.

------

### Author(s)

  * Champagne Lewis - champagne.t.lewis@gmail.com

### License
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.


See the LICENSE.md file for details.

### Acknowledgements

Existing code was taken from several different sources, and I have tried to
document this in the locatons where they exist.

  * Microchip Pic 24 Support Library
My former ECE professors at Mississippi State have created a support library
for the 16 bit family of Microchip Pic MCUs that is publically available. It
is a very useful resource I extensively use when developing for the Pic.
Unfortunately this is not available as a GitHub project, so I have copied
the necessary files directly into my project. This library is unlikely to see
significant change, so this is probably a safe move.
https://courses.ece.msstate.edu/ece3724/main_pic24/docs/index.html

  * .gitignore for MPLAB X projects
For the time being, I'm using a .gitignore for this project created by
Chris Hofstaedtler specifically for MPLAB X projects.
https://gist.github.com/zeha/5999375

  * Adafruit_LiquidCrystal
While I AM NOT making any efforts to replicate the API of this project, it
is important to note I did throuroughly read the source code of this project
to suppliment the knowledge I gleaned from the documentation I read, and it
served as inspiration for this project. This version of the
Adafruit_LiquidCrystal library was forked and further developed by Tony Dicola
of Adafruit Industries.
https://github.com/adafruit/Adafruit_LiquidCrystal


/* 

 File:		pic_char_lcd_cg.h
 Author:		Champagne Lewis

 Comment:	This header contains macros for the bitmaps of common and example 
			custom characters to be written to the CGRAM of a character LCD.
			This header is purely optional and not "#include"ed in the .c file.

*/

#ifndef PIC_CHAR_LCD_CG_H
#define	PIC_CHAR_LCD_CG_H

/*
 up arrow

 0x00	000 00000
 0x04	000 00?00
 0x0E	000 0???0
 0x15	000 ?0?0?
 0x04	000 00?00
 0x04	000 00?00
 0x00	000 00000
 0x00	000 00000	// generally reserved for the cursor
*/
#define LCD_UP_ARROW_CG {0x00, 0x04, 0x0E, 0x15, 0x04, 0x04, 0x00, 0x00}

/*
 down arrow

 0x00	000 00000
 0x04	000 00?00
 0x04	000 00?00
 0x15	000 ?0?0?
 0x0E	000 0???0
 0x04	000 00?00
 0x00	000 00000
 0x00	000 00000	// generally reserved for the cursor
*/
#define LCD_DOWN_ARROW_CG {0x00, 0x04, 0x04, 0x15, 0x0E, 0x04, 0x00, 0x00}

/*
 bluetooth logo

 0x04	000 00?00
 0x06	000 00??0
 0x15	000 ?0?0?
 0x0E	000 0???0
 0x15	000 ?0?0?
 0x06	000 00??0
 0x04	000 00?00
 0x00	000 00000	// generally reserved for the cursor
*/
#define LCD_BLUETOOTH_CG {0x04, 0x06, 0x15, 0x0E, 0x15, 0x06, 0x04, 0x00}

/*
 It is recommendable to use LCD_BATTERY_CG(percent) to generate the correct
 corresponding character bitmap instead of directly calling LCD_BATTERY_X_CG.
 Then, a regular interrupt can re-write the CGRAM slot containing the battery
 meter with the current correct character, meaning only one address needs to be
 filled in, and the other addresses can be used for other characters.
*/

/*
 battery meter 0

 0x0E	000 0???0
 0x1B	000 ??0??
 0x11	000 ?000?
 0x11	000 ?000?
 0x11	000 ?000?
 0x11	000 ?000?
 0x1F	000 ?????
 0x00	000 00000	// generally reserved for the cursor
*/
#define LCD_BATTERY_0_CG {0x0E, 0x1B, 0x11, 0x11, 0x11, 0x11, 0x1F, 0x00}

/*
 battery meter 1

 0x0E	000 0???0
 0x1B	000 ??0??
 0x11	000 ?000?
 0x11	000 ?000?
 0x11	000 ?000?
 0x1F	000 ?????
 0x1F	000 ?????
 0x00	000 00000	// generally reserved for the cursor
*/
#define LCD_BATTERY_1_CG {0x0E, 0x1B, 0x11, 0x11, 0x11, 0x1F, 0x1F, 0x00}

/*
 battery meter 2

 0x0E	000 0???0
 0x1B	000 ??0??
 0x11	000 ?000?
 0x11	000 ?000?
 0x1F	000 ?????
 0x1F	000 ?????
 0x1F	000 ?????
 0x00	000 00000	// generally reserved for the cursor
*/
#define LCD_BATTERY_2_CG {0x0E, 0x1B, 0x11, 0x11, 0x1F, 0x1F, 0x1F, 0x00}

/*
 battery meter 3

 0x0E	000 0???0
 0x1B	000 ??0??
 0x11	000 ?000?
 0x1F	000 ?????
 0x1F	000 ?????
 0x1F	000 ?????
 0x1F	000 ?????
 0x00	000 00000	// generally reserved for the cursor
*/
#define LCD_BATTERY_3_CG {0x0E, 0x1B, 0x11, 0x1F, 0x1F, 0x1F, 0x1F, 0x00}

/*
 battery meter 4

 0x0E	000 0???0
 0x1B	000 ??0??
 0x1F	000 ?????
 0x1F	000 ?????
 0x1F	000 ?????
 0x1F	000 ?????
 0x1F	000 ?????
 0x00	000 00000	// generally reserved for the cursor
*/
#define LCD_BATTERY_4_CG {0x0E, 0x1B, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00}

/*
 battery meter 5

 0x0E	000 0???0
 0x1F	000 ?????
 0x1F	000 ?????
 0x1F	000 ?????
 0x1F	000 ?????
 0x1F	000 ?????
 0x1F	000 ?????
 0x00	000 00000	// generally reserved for the cursor
*/
#define LCD_BATTERY_5_CG {0x0E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00}

#define LCD_BATTERY_CG(percent)												\
			(	((percent) <= 10)					?	LCD_BATTERY_0_CG	\
			:	((percent) > 10 && (percent) <= 30)	?	LCD_BATTERY_1_CG	\
			:	((percent) > 30 && (percent) <= 50) ?	LCD_BATTERY_2_CG	\
			:	((percent) > 50 && (percent) <= 70) ?	LCD_BATTERY_3_CG	\
			:	((percent) > 70 && (percent) <= 90) ?	LCD_BATTERY_4_CG	\
			:											LCD_BATTERY_5_CG	)

#endif	// PIC_CHAR_LCD_CG_H


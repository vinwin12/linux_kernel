
#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "lib.h"
#include "i8259.h"
#include "terminal.h"
#include "paging.h"

#define NUM_SCAN_CODES 		256
#define KEYBOARD_PORT 		0x60
#define KEYBOARD_INT_NUM 	1

/* Actions realted to special keypress */
#define CAPS_LOCK 			0x3A
#define LEFT_SHIFT_PRESS 	0x2A
#define LEFT_SHIFT_RELEASE 	0xAA
#define RIGHT_SHIFT_PRESS 	0x36
#define RIGHT_SHIFT_RELEASE	0xB6
#define CTRL_PRESS 			0x1D
#define CTRL_RELEASE 		0x9D
#define ALT_PRESS       0x38
#define ALT_RELEASE     0xB8
#define BACKSPACE_PRESS		0x0E

#define LETTER_CASE_CHANGE 	32 	 /* add 32 to make letters upper/lower case */
#define KEY_PRESS_MAX 		0x58 /* all values above 0x58 are key releases */
#define CLEAR				0xFF /* Used for CTRL + L */

/* Keypress Ascii Values */
#define BACKSPACE 			8
#define ESC 				27
#define ENTER 				10
#define SPACE 				32
#define TAB 				9
#define F1          0x3B
#define F2          0x3C
#define F3          0x3D

/* Used before Demo, since our laptops required Fn key for F#'s */
#define ONE         0x02
#define TWO         0x03
#define THREE       0x04

/* keyboard initializer */
void keyboard_init();

/* Keyboard interrupt handler */
void keyboard_handler();

/* Converts scan code from keyboard to ASCII value */
unsigned char scan_to_ASCII(unsigned char code);

#endif

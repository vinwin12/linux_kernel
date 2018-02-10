/*
	terminal.h



	This file is for everything related to the terminal.
*/

#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "lib.h"
#include "keyboard.h"
#include "system_calls.h"

#define NUM_TERMINALS 3

#define BUFFER_LENGTH 		128
#define PRINT_LENGTH 		127
#define LAST_PRINTED 		126
#define SCAN_START 			14 	//height
#define SCAN_END 			15 	//of cursor

#define MASK 				0xFF
#define SHIFT_8 			8
#define VGA_PORT_1 			0x3D4
#define VGA_PORT_2 			0x3D5
#define SELECT_REG 			0x0A
#define SELECT_REG_2 		0x0B
#define CURSOR_OFF 			0x20
#define VIDEO_WIDTH 		80
#define PORT2_MASK 			0xC0
#define VGA_PORT_CURSOR 	0x3E0
#define CURSOR_MASK 		0xE0

#define EMPTY 				0
#define SELECT_X 			0x0F
#define SELECT_Y 			0x0E
#define CURR_POSITON 		1
#define PREV_POSITON 		2
#define DECREASE_TWO_UPDATE 2

/* either 0, 1 or 2 depending on which terminal is visible */
int visible_terminal;

/* terminal structure */
typedef struct terminal_t {
  unsigned char io_buffer[BUFFER_LENGTH]; /* buffer used for read/write from/to terminal */
  int length;                             /* length of buffer */
  volatile int commit_flag;               /* terminal read / buffer flag */
  int line_flag;                          /* determines which vertical line for screen position */
  int term_write_flag;                    /* determines horizontal position to prevent backspacing (if write() is called) */
  int screen_x;                           /* screen's x position */
  int screen_y;                           /* screen's y position */
  int current_process;                    /* process ID of the current process in this terminal */
  int has_been_launched;                  /* 1 if base shell has been FULLY opened, 0 if not */
  int is_visible;                         /* flag which determines if this is the visible terminal */
  volatile int rtc_flag;                  /* flag for each terminal's RTC */
  uint32_t vidmem_addr;                   /* pointer to terminal-specific vid mem page */
  uint32_t user_vidmem_addr;              /* address used specifically for the vidmap() function */
} terminal_t;

/* array of our terminal structures */
terminal_t terminals[NUM_TERMINALS];

/* Initializes the 3 terminal structures */
void terminal_init();

/* terminal-specific open syscall */
int32_t terminal_open(const uint8_t* filename);

/* terminal-specific close syscall */
int32_t terminal_close(int32_t fd);

/* terminal-specific read syscall */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

/* terminal-specific write syscall */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

/* enables cursor */
void enable_cursor();

/* disable cursor */
void disable_cursor();

/* update cursor position */
void update_cursor(int term_num);

/* clears screen, puts the cursor at the top */
void clear_screen(int term_num);

/* clears buffer */
void clear_buffer(int term_num);

/* moves display up one line to make room for a new emppty line */
void scroll_up(int term_num);

/* deletes a character from the input buffer */
void backspace(int term_num);

/*helper function that interacts with buffer if a key is pressed*/
void handle_buffer(unsigned char input, int term_num);

#endif

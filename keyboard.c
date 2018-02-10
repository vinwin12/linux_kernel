/*
	keyboard.c

	Created by: Thomas Korenchan
*/
#include "keyboard.h"
#include "system_calls.h"

/*
	array of characters mapped by index to scan codes
	SHIFT INACTIVE
*/
unsigned char scan_codes_no_shift[NUM_SCAN_CODES] = {
	0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'0', '-', '=', 0, 0, 'q', 'w', 'e', 'r', 't',
	'y', 'u', 'i', 'o', 'p', '[', ']', ENTER, 0, 'a', 's',
	'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
	'\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
	0, '*', 0, SPACE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2',
	'3', '0', '.', 0, 0, 0, 0, 0
};

/*
	array of characters mapped by index to scan codes
	SHIFT ACTIVE
*/
unsigned char scan_codes_shift[NUM_SCAN_CODES] = {
	0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(',
	')', '_', '+', 0, 0, 'Q', 'W', 'E', 'R', 'T',
	'Y', 'U', 'I', 'O', 'P', '{', '}', ENTER, 0, 'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0,
	'|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
	0, '*', 0, SPACE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2',
	'3', '0', '.', 0, 0, 0, 0, 0
};

/* special key flags */
static int control_flag = 0;
static int shift_flag = 0;
static int caps_lock_flag = 0; // NOTE: only matters for letters
static int alt_flag = 0;


/*
	keyboard_init()

	Description: Initializes keyboard use
	Inputs: None
	Outputs: None
	Side Effects: enables Keyboard IRQs
*/
void keyboard_init()
{
	/* enable (unmask) IRQ1 for keyboard interrupts */
	enable_irq(KEYBOARD_INT_NUM);
}

/*
	keyboard_handler()

	Description: handler for a keyboard interrupt
	Inputs: None
	Outputs: None
	Side Effects: handles Keyboard interrupt, adds char to
								terminal-specific buffer, and	prints a character to the screen
*/
void keyboard_handler()
{
	send_eoi(KEYBOARD_INT_NUM);

	cli();

	/* get scan code and convert to ASCII */
	unsigned char ascii = inb(KEYBOARD_PORT);
	ascii = scan_to_ASCII(ascii);

	/* print it to the screen */
	if( (ascii != 0 && ascii != BACKSPACE) && (terminals[visible_terminal].length < PRINT_LENGTH || ascii == ENTER) )
		putc(ascii, visible_terminal);
  /* send ascii value to buffer for handling the buffer */
  handle_buffer(ascii, visible_terminal);

	/* update cursor */
	update_cursor(visible_terminal);

	sti();
}

/*
	scan_to_ASCII()

	Description: takes a scan code input and
				 converts it to that scan code's
				 respective ASCII value
	Inputs: code - scan code from keyboard port
	Outputs: ascii value to be printed or scan
					 code value in special cases
	Side Effects: none
*/
unsigned char scan_to_ASCII(unsigned char code)
{
	unsigned char val;

	/* set flags/process special keys */
	switch(code)
	{
		case CAPS_LOCK:
			if(caps_lock_flag)
				caps_lock_flag = 0;
			else
				caps_lock_flag = 1;
			break;

		case LEFT_SHIFT_PRESS:
			shift_flag = 1; break;

		case RIGHT_SHIFT_PRESS:
			shift_flag = 1; break;

		case LEFT_SHIFT_RELEASE:
			shift_flag = 0; break;

		case RIGHT_SHIFT_RELEASE:
			shift_flag = 0; break;

		case CTRL_PRESS:
			control_flag = 1; break;

		case CTRL_RELEASE:
			control_flag = 0; break;

		case ALT_PRESS:
			alt_flag = 1; break;

		case ALT_RELEASE:
			alt_flag = 0; break;

		case BACKSPACE_PRESS:
			backspace(visible_terminal); return BACKSPACE;

		default:
			break;
	}

	/* Check if key is being released */
	if( code > KEY_PRESS_MAX )
		return 0;


	/* Determine if shift is being pressed, access array accordingly */
	if(shift_flag)
		val = scan_codes_shift[(int)(code)];
	else
		val = scan_codes_no_shift[(int)(code)];

	/* CTRL + L clears the screen, puts cursor at top */
	if( control_flag && (val == 'L' || val == 'l') )
	{
		clear_screen(visible_terminal);
		//printf("391OS> ");
		return 0;
	}

	if( alt_flag && (code == F1) )
	{
		int prev_term = visible_terminal;
		visible_terminal = 0;
		int curr_term = visible_terminal;
		terminals[prev_term].is_visible = 0;
		terminals[curr_term].is_visible = 1;
		/* point Terminal 1's video memory page to physical video memory */
		display_terminal(curr_term, prev_term);
		update_cursor(visible_terminal);
		sti();
		return 0;
	}

	if( alt_flag && (code == F2) )
	{
		int prev_term = visible_terminal;
		visible_terminal = 1;
		int curr_term = visible_terminal;
		terminals[prev_term].is_visible = 0;
		terminals[curr_term].is_visible = 1;
		/* point Terminal 2's video memory page to physical video memory */
		display_terminal(curr_term, prev_term);
		update_cursor(visible_terminal);
		if( terminals[1].has_been_launched == 0 )
		{
			sti();
			/* launch Terminal 2's base shell */
			if( execute((uint8_t*)"shell") == 0 )
			{
					/* execute failed due to max processes reached, reset has_been_launched and curr_idx */
					terminals[1].has_been_launched = 0;
					curr_idx = restore_curr_idx;
			}
		}
		sti();
		return 0;
	}

	if( alt_flag && (code == F3) )
	{
		int prev_term = visible_terminal;
		visible_terminal = 2;
		int curr_term = visible_terminal;
		terminals[prev_term].is_visible = 0;
		terminals[curr_term].is_visible = 1;
		/* point Terminal 3's video memory page to physical video memory */
		display_terminal(curr_term, prev_term);
		update_cursor(visible_terminal);
		if( terminals[2].has_been_launched == 0 )
		{
			sti();
			/* launch Terminal 3's base shell */
			if( execute((uint8_t*)"shell") == 0 )
			{
					/* execute failed due to max processes reached, reset has_been_launched and curr_idx */
					terminals[2].has_been_launched = 0;
					curr_idx = restore_curr_idx;
			}
		}
		sti();
		return 0;
	}

	/*
		Check if ASCII value is alphabetical,
		NOTE: Caps Lock only affects letters
	*/
	if(caps_lock_flag)
	{
		/* shift is active, switch to lower case */
		if( val >= 'A' && val <= 'Z' )
			val += LETTER_CASE_CHANGE;
		/* shift is inactive, convert to uppercase */
		else if( val >= 'a' && val <= 'z' )
			val -= LETTER_CASE_CHANGE;
	}

	/* scan_to_ASCII produced a return-able value */
	return val;
}

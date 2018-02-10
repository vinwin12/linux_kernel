/*
		terminal.c


*/

#include "terminal.h"

/*
	terminal_init()

	Description: initializes all 3 terminal structures
	Inputs: None
	Outputs: None
	Side Effects: None
*/
void terminal_init()
{
	int i;
	for( i = 0; i < NUM_TERMINALS; i++ )
	{
			clear_buffer(i);
			terminals[i].length = 0;
			terminals[i].commit_flag = 0;
			terminals[i].line_flag = 0;
			terminals[i].term_write_flag = 0;
			terminals[i].screen_x = 0;
			terminals[i].screen_y = 0;
			terminals[i].current_process = -1;
      terminals[i].has_been_launched = 0;
			terminals[i].is_visible = 0;
			terminals[i].rtc_flag = 0;
	}
	terminals[0].vidmem_addr = TERM_1_VIDEO;
	terminals[1].vidmem_addr = TERM_2_VIDEO;
	terminals[2].vidmem_addr = TERM_3_VIDEO;

	terminals[0].user_vidmem_addr = TERM_1_USER_VID;
	terminals[1].user_vidmem_addr = TERM_2_USER_VID;
	terminals[2].user_vidmem_addr = TERM_3_USER_VID;

	clear_screen(0);
	clear_screen(1);
	clear_screen(2);

  /* prepare for the execution of Terminal 1's shell */
  visible_terminal = 0;
	terminals[0].has_been_launched = 1;
	terminals[0].is_visible = 1;
  display_terminal(visible_terminal, -1);
}

/*
	terminal_open()

	Description: opens/initializes a new terminal
	Inputs: pointer to filename (DOES NOT USE)
	Outputs: should always return 0
*/
int32_t terminal_open(const uint8_t* filename)
{
	return 0;
}

/*
	terminal_close()

	Description: clears terminal-specific stuff
	Inputs: file descriptor name (DOES NOT USE)
	Outputs: should always return 0
*/
int32_t terminal_close(int32_t fd)
{
	return 0;
}

/*
	terminal_read()

	Description: reads from keyboard buffer into parameter buffer
	Inputs: file descriptor, pointer to a buffer, and number of bytes to copy
	Outputs: number of bytes read, 0 for failure
	Side Effects: fills buf
*/
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes)
{
	int i;
	int32_t ret_val;
	unsigned char* ptr = (unsigned char*)buf;

	cli();
	pcb_t * pcb = get_PCB_from_stack();
	int term_num = pcb->terminal_number;
	sti();

	/* loop until the keyboard buffer is ready */
	while(1)
	{
		cli();
		if( terminals[term_num].commit_flag )
				break;
		sti();
	}

	/* reset flag */
	terminals[term_num].commit_flag = 0;

	/* Fail cases */
	if( buf == NULL || nbytes <= 0)
		return -1;

	ret_val = 0;
	i = 0;
	while( terminals[term_num].io_buffer[i] != ENTER && (i < nbytes) )
	{
		ptr[i] = terminals[term_num].io_buffer[i];
		ret_val++;
		i++;
	}
	if( i == nbytes )
	{
		ptr[i - 1] = ENTER;
	}
	else // newline terminated the while loop
	{
		/* add the newline character at the end */
		ptr[i] = ENTER;
	}
	ret_val++;

	/* clear buffer */
	clear_buffer(term_num);

	/* return number of bytes read */
	return ret_val;
}

/*
	terminal_write()

	Description: writes to screen from the paramater buffer
	Inputs: file descriptor, buffer pointer, number of bytes to print
	Outputs: number of bytes written, or -1 for failure
*/
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes)
{
	cli();

	pcb_t * current_pcb = get_PCB_from_stack();

	int i = 0;
	unsigned char* ptr = (unsigned char*)buf;

	/* check if buffer pointer is NULL */
	if( buf == NULL )
		return -1;

	/* print buffer to screen */
	for( i = 0; i < nbytes; i++ )
		putc( (unsigned char)(ptr[i]), current_pcb->terminal_number );

	/* since write() is being called, we want to prevent horizontal backspacing */
	terminals[current_pcb->terminal_number].term_write_flag = terminals[current_pcb->terminal_number].screen_x;

	update_cursor(visible_terminal);

	sti();

	return 0;
}

/*
	enable_cursor()

	Description: enables cursor
	Inputs: None
	Outputs: None
	Side Effects: Enables cursor
    Inspiration: OSDev


*/
void enable_cursor()
{
    uint8_t cursor_start = SCAN_START;
    uint8_t cursor_end   = SCAN_END;

    outb(SELECT_REG,VGA_PORT_1);
    outb((inb(VGA_PORT_2) & PORT2_MASK) | cursor_start,VGA_PORT_2);

    outb(SELECT_REG_2,VGA_PORT_1);
    outb((inb(VGA_PORT_CURSOR) & CURSOR_MASK) | cursor_end, VGA_PORT_2);

		/* place cursor at initial terminal's current screen location */
		update_cursor(0);
}

/*
	disable_cursor()

	Description: disables cursor
	Inputs: None
	Outputs: None
	Side Effects: disables cursor
    Inspiration: OSDev

*/
void disable_cursor()
{
    outb(SELECT_REG,VGA_PORT_1);
    outb(CURSOR_OFF,VGA_PORT_2);
}

/*
    update_cursor()

	Description: updates cursor
	Inputs: term_num = terminal number
	Outputs: None
	Side Effects: disables cursor
    Inspiration: OSDev
 */
void update_cursor(int term_num)
{
		int x = terminals[term_num].screen_x;
		int y = terminals[term_num].screen_y;

    uint16_t  pos = (y * VIDEO_WIDTH + x);

		outb(SELECT_X,VGA_PORT_1);
	  outb((uint8_t) (pos & MASK), VGA_PORT_2);
	  outb(SELECT_Y,VGA_PORT_1);
	  outb((uint8_t) ((pos >> SHIFT_8) & MASK) , VGA_PORT_2);
}

/*
	clear_screen()

	Description: Clears the screen, sets cursor back to top
	Inputs: terminal number
	Outputs: None
	Side Effects: Clears screen, resets cursor
*/
void clear_screen(int term_num)
{
	/* clears screen/buffer, resets x and y, updates cursor */
	clear(term_num);
	clear_buffer(term_num);
	terminals[term_num].screen_x = 0;
	terminals[term_num].screen_y = 0;
	terminals[term_num].line_flag = 0;
	terminals[term_num].term_write_flag = 0;
	update_cursor(term_num);
}

/*
	clear_buffer()

	Description: wipes buffer data
	Inputs: terminal number
	Outputs: None
	Side Effects: resets index pointer to 0
*/
void clear_buffer(int term_num)
{
	int i;
	terminals[term_num].length = 0;
	terminals[term_num].commit_flag = 0;

	/* initialize buffer entries to 0 */
	for( i = 0; i < BUFFER_LENGTH; i++ )
		terminals[term_num].io_buffer[i] = 0;
}

/*
	scroll_up()

	Description: Scrolls video memory up by one "window view" line
	Inputs: terminal number
	Outputs: None
	Side Effects: scrolls up one line
	Inspiration: OSDev.org/Text_UI
*/
void scroll_up(int term_num)
{
	int i;
	int num_bytes;

	uint8_t * virt_addr = (uint8_t*)terminals[term_num].vidmem_addr;

	/*
		General Note: we multiply everything
		by 2 since we are shifting both the character
		AND its color (though the color is always the same)
	*/
	num_bytes = (NUM_COLS * (NUM_ROWS - 1) * 2);
	/*
		copy all data written on screen (except the
		very top line, since that will be erased by
		scrolling) and move it "back" one logical line
	*/
	memcpy(virt_addr, virt_addr + (NUM_COLS * 2), num_bytes);

	/* "erase" the bottom most line */
	for( i = 0; i < NUM_COLS; i++ )
	{
	    *(uint8_t *)(virt_addr + num_bytes + (i << 1)) = ' ';
	    *(uint8_t *)(virt_addr + num_bytes + (i << 1) + 1) = ATTRIB;
	}
	/* reset x and y coords */
	terminals[term_num].screen_x = 0;
	terminals[term_num].screen_y = (NUM_ROWS - 1);
}

/*
	backspace()

	Description: deletes a character from the screen
	Inputs: terminal number
	Outputs: None
	Side Effects: deletes a character from the
				  input buffer, updates screen
*/
void backspace(int term_num)
{
	uint8_t * virt_addr = (uint8_t*)terminals[term_num].vidmem_addr;

	/*
		cases:
		1) top left corner, just return
		2) far left side, need to go up one line and be on the far right
		3) anywhere else
	*/
	if( terminals[term_num].screen_y == terminals[term_num].line_flag && terminals[term_num].screen_x == terminals[term_num].term_write_flag )
		return;

	/* first check if position is on the far left side */
	if( terminals[term_num].screen_x == 0 )
	{
		/* top left corner (or on the line just after the last committed one, just return */
		if( (terminals[term_num].screen_y == 0) || (terminals[term_num].screen_y == terminals[term_num].line_flag) )
		{
			return;
		}
		/* go up one line and move on the far right */
		else
		{
			terminals[term_num].screen_x = (NUM_COLS - 1);
			terminals[term_num].screen_y -= 1;
		}
	}
	/* just need to move left one */
	else
	{
		terminals[term_num].screen_x -= 1;
	}

	/* update video memory */
	*(uint8_t *)(virt_addr + ((NUM_COLS * terminals[term_num].screen_y + terminals[term_num].screen_x) << 1)) = ' ';
  *(uint8_t *)(virt_addr + ((NUM_COLS * terminals[term_num].screen_y + terminals[term_num].screen_x) << 1) + 1) = ATTRIB;
	return;
}

/*
	handle_buffer()

	Description: This is a helper function that is called when a key is pressed and may/may not interact with buffer
	Inputs: input = ascii value to put/interact with buffer
					term_num = terminal number
	Outputs: None
	Side Effects: Either removes a character from buffer, adds a character, or does nothing if buffer is filled.
*/


void handle_buffer(unsigned char input, int term_num)
{
		int len = terminals[term_num].length;
    /* boundary checks (if commit flag = 1, buffer is no longer altered) */
    if ( (input == BACKSPACE && (terminals[term_num].length == EMPTY)) || (input == EMPTY) || terminals[term_num].commit_flag )
    {
        return;
    }
    /* backspace has been pressed, delete a char from the buffer */
    if( input == BACKSPACE )
    {
    	/* don't have to worry about length being 0 */
    	/* erase the newline char, place it 1 position previous */
			terminals[term_num].io_buffer[len] = EMPTY;
			terminals[term_num].io_buffer[len - 1] = ENTER;
			terminals[term_num].length--;
			return;
    }
    /* backspace has not been pressed, add a char to the buffer */
    else
    {
    	if( input == ENTER )
    	{
    		/* buffer is done, append a newline char and commit it */
    		terminals[term_num].io_buffer[len] = ENTER;
    		/* terminal read is ready to process the buffer */
    		terminals[term_num].commit_flag = 1;
    		//clear_buffer();
    		return;
    	}
    	else if( len < PRINT_LENGTH )
    	{
    		/* add char to buffer */
				terminals[term_num].io_buffer[len] = input;
				terminals[term_num].io_buffer[len + 1] = ENTER;
				terminals[term_num].length++;
        return;
    	}
    }
}

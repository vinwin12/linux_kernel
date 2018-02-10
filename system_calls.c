/*

	system_calls.c



*/
#include "system_calls.h"
#include "scheduler.h"

/* bitmap array which tells if a process id (the array index) is free or not */
int processes[MAX_NUM_PROCS] = {0, 0, 0, 0, 0, 0};

/*
	File Operations Tables

	These 5 tables specify which read/write/open/close is to
	be used with the respective prototype, depending on the
	file type it is associated with:

	1) stdin
	2) stdout
	3) rtc file type
	4) directory file type
	5) other file
*/
file_op stdin_fops = {terminal_read, failure, terminal_open, terminal_close};

file_op stdout_fops = {failure, terminal_write, terminal_open, terminal_close};

file_op rtc_fops = {rtc_read, rtc_write, rtc_open, rtc_close};

file_op dir_fops = {directory_read, directory_write, directory_open, directory_close};

file_op file_fops = {file_read, file_write, file_open, file_close};

file_op fail_fops = {failure, failure, failure, failure};

/*
	failure()

	Description: used for the stdin and stdout
				 file operations, since they don't
				 use either read() or write()
	Inputs: None
	Outputs: None
	Side Effects: None
*/
int32_t failure()
{
	return -1;
}

/*
	check_exec()

	Description: checks if the command attempting to execute requires
							 arguments or not. If it does not and its arg buffer
							 is not empty, execution should fail
	Inputs: cmd - buffer which holds the name of the executable
					args - pointer to args buffer
					n - number of bytes being compared
	Outputs: 0 for success, -1 for failure

*/
int32_t check_exec(uint8_t* cmd, int8_t* args, int n)
{
	int8_t* temp = (int8_t*)cmd;

	if( (strncmp(temp, (int8_t*)"cat", n) != 0) &&
			(strncmp(temp, (int8_t*)"grep", n) != 0) &&
			(strncmp(temp, (int8_t*)"sigtest", n) != 0) &&
			args[0] != NULL )
	{
		return -1;
	}
	else
		return 0;
}

/*

	get_PCB_from_stack()

	Description: gets the pointer to the active process's
							 PCB by ANDing the current ESP with a mask
	Inputs: None
	Outputs: pcb_t pointer
	Side Effects: None

*/
pcb_t * get_PCB_from_stack()
{
	pcb_t * temp;
	asm volatile("												\n\
	andl %%esp, %%edx											\n\
	"
	: "=d" (temp)
	: "d"(PCB_MASK)
	: "cc"
	);

	return temp;
}

/*

	find_free_process()

	Description: finds a free process ID
							 for a new PCB
	Inputs: none
	Outputs: process ID of a free PCB
	Side Effects: -1 for failure, index of free process ID for success

*/
int find_free_process()
{
	int ret = -1;
	int i;
	for( i = 0; i < MAX_NUM_PROCS; i++ )
	{
		/* check each index until a free one is found */
		if( processes[i] == FREE )
		{
				ret = i;
				processes[i] = BUSY;
				break;
		}
	}

	return ret;
}

/*
	execute()

	Description: loads and executes a new program
	Inputs: command: string of the executable name and its args
	Outputs: -1 for failure, 256 for fail due to exception, or 0-255
			 depending on halt() return val
	Side Effects: sets up and executes a new program
*/
int32_t execute(const uint8_t* command)
{
	/*
		Local Variables
	*/
	uint8_t file_name[MAX_BUFFER_LENGTH];																	// command/file name string
	int8_t arguments[MAX_BUFFER_LENGTH];																	// arguments after the file name
	uint8_t valid_check[FOUR_BYTES];																			// return buf from file_read()
	uint8_t ELF_check[FOUR_BYTES] = {MAGIC, MAGIC_E, MAGIC_L, MAGIC_F};		// buffer to check against valid_check
	uint8_t entry_point[FOUR_BYTES];																			// 4 bytes to determine entry point in a file
	uint32_t point_of_entry = 0;																					// actual address of executable's first instruction
	int done_parsing = 0;																									// flag if we end parsing early

	cli();

	/* if this Terminal is launched for first time, take appropriate actions */
	if( terminals[visible_terminal].has_been_launched == 0 )
	{
			terminals[visible_terminal].has_been_launched = 1;
			/* set curr_idx (scheduling) to the new terminal */
			restore_curr_idx = curr_idx;
			curr_idx = visible_terminal;
	}


	/* as a precaution, initialize all indices of file_name, args and valid_check to 0 */
	int x;
	for( x = 0; x < MAX_BUFFER_LENGTH; x++ )
	{
		file_name[x] = 0;
		arguments[x] = 0;
		if( x < FOUR_BYTES )
		{
			valid_check[x] = 0;
		}
	}

	/*
		Parse Arguments

		- parse out the command string and its arg strings
			- ex. "cd /workdir" = "cd" + "/workdir"
		- take consideration of trailing spaces

	*/

	/* check for null pointer */
	if( command == NULL )
		return -1;

	/* parse file name */
	int i = 0;
	int j = 0;
	/* failure conditions: newline, done early, or out of bounds */
	while( command[i] != NULL && !done_parsing	 )
	{
		if( command[i] != SPACE )
		{
				file_name[j] = command[i];
				/* check if next char is a space, if so we are done parsing */
				if( command[i + 1] == SPACE )
				{
					done_parsing = TRUE;
				}
				j++;
		}
		i++;
	}

	/* parse arguments */
	j = 0;
	/* failure conditions: newline or out of bounds */
	while( command[i] != NULL )
	{
		if( command[i] != SPACE)
		{
				arguments[j] = command[i];
				/* check if next char is a space, if so wait for next arg */
				// if( command[i + 1] != NULL && command[i + 1] == SPACE )
				// {
				// 	j++;
				// 	arguments[j] = SPACE;
				// }
				j++;
		}
		i++;
	}

	/* NULL-terminate the arguments string */
	arguments[j] = NULL;

	/* Add-on: check what Terminal we are in by typing "term" */
	if( strncmp((int8_t*)file_name, (int8_t*)"term", FOUR_BYTES) == 0 )
	{
		printf("Terminal Number: %d\n", (visible_terminal + 1));
		return 0;
	}



	int count = 0;
	while( file_name[count] != NULL )
		count++;

	/* check if command does not require arguments but has them anyway (garbage values) */
	if( check_exec(file_name, arguments, count) != 0 )
		return -1;

	/*

		Check file validity

	*/

	/* grab dentry for the program image, if it exists */
	dentry_t prog_img;
	if( read_dentry_by_name(file_name, &prog_img) == FAILED )
        return -1;
	/* check if file exists in the filesystem */
	if( read_data(prog_img.inode, 0, valid_check, FOUR_BYTES) == FAILED )
        return -1;
	/* check first 4 bytes to see if it is an executable file */
	if( strncmp((int8_t*)valid_check, (int8_t*)ELF_check, FOUR_BYTES) != 0 )
        return -1;

	/* local process ID variable */
	int process = find_free_process();

	/* check if we can support another process */
	if( process == -1 )
	{
		printf("Max number of processes reached.\n");
		return 0;
	}

	/*

		Set up Paging (Mapping)

	*/

	/* set the mapping from virtual to physical memory FOR THIS PARTICULAR PROCESS */
	map_task(MB128, MB8 + (process * MB4));

	/*

		Load the File into Memory and Determine Entry Point

	*/

	/* read from the filesystem and copy the program image into physical memory */
	if( read_data(prog_img.inode, 0, (uint8_t*)PROG_IMG_ADDR, MB4) == -1 )
	{
		processes[process] = FREE;
		return -1;
	}

	/* Determine entry point (4 bytes) into the file */
	if( read_data(prog_img.inode, FILE_ENTRY_OFFSET, entry_point, FOUR_BYTES) == -1 )
	{
		processes[process] = FREE;
		return -1;
	}

	/* set up the 32-bit address of the first instruction in the program image */
	point_of_entry = (entry_point[BYTE3] << SHIFT_24) + (entry_point[BYTE2] << SHIFT_16) + (entry_point[BYTE1] << SHIFT_8) + entry_point[BYTE0];

	/*

		Create the Task's PCB and Open its FDs


	*/

	// create a PCB for the current process
	pcb_t* current_pcb = (pcb_t *)(MB8 - (KB8 * (process + 1)));
	current_pcb->process_id = process;
	current_pcb->terminal_number = visible_terminal;

	// if this is the very first process (per terminal)
	if( terminals[current_pcb->terminal_number].current_process == -1 ) {
		current_pcb->parent = NULL;
	}
	// find the parent PCB
	else {
		current_pcb->parent = (pcb_t *)(MB8 - (KB8 * (terminals[current_pcb->terminal_number].current_process + 1)));

		/* retrieve parent PCB's kernel EBP and kernel ESP values  */
		asm volatile("			\n\
		movl %%esp, %0 			\n\
		movl %%ebp, %1 			\n\
		"
		: "=g"(current_pcb->parent_esp), "=g"(current_pcb->parent_ebp)
		);
	}

	/* update the current process number for the active terminal */
	terminals[current_pcb->terminal_number].current_process = current_pcb->process_id;

	// opening process file descriptor
	for(i = 0; i < FD_ARRAY_SIZE; i++) {

		if(i == 0) {
			//setting stdin
			current_pcb->fd_array[i].flags = BUSY;
			current_pcb->fd_array[i].f_op = stdin_fops;
		}
		else if( i == 1) {
			//setting stdout
			current_pcb->fd_array[i].flags = BUSY;
			current_pcb->fd_array[i].f_op = stdout_fops;
		}
		else {
			current_pcb->fd_array[i].flags = FREE;
			current_pcb->fd_array[i].f_op = fail_fops;
		}
		current_pcb->fd_array[i].file_position = FILE_POS_EMPTY_FD;
		current_pcb->fd_array[i].inode_num = FAIL_INODE_NUM;

	}

	/*

		Setup IRET context and Perform Context Switch

	*/

	/* set up the Task State Segment */
	tss.ss0 = KERNEL_DS;																		// Kernel Data Segment
	tss.esp0 = (MB8 - (KB8 * current_pcb->process_id) - 4);	// bottom of this process' kernel stack

	/*
		set up IRET context (artificial stack) and call IRET

		Inspiration: http://jamesmolloy.co.uk/tutorial_html/10.-User%20Mode.html

		Note about setting IF: it is better to set IF here, rather
							   than call STI, since apparently it
							   can cause a General Protection Fault
	*/

	// copy arguments buffer into pcb arg_buf
	strcpy(current_pcb->arg_buf, arguments);

	sti();

	asm volatile("																					\n\
	cli 																										\n\
	movw $0x2B, %%ax 																				\n\
	movw %%ax, %%ds 																				\n\
	pushl %%eax 		# push USER_DS													\n\
	movl %0, %%eax 																					\n\
	pushl %%eax 		# push bottom of user page							\n\
	pushfl																									\n\
	popl %%eax 																							\n\
	orl $0x200, %%eax 	# set IF in EFLAGS (sti)						\n\
	pushl %%eax 		# push EFLAGS 													\n\
	movl %1, %%eax 																					\n\
	pushl %%eax 		# push USER_CS													\n\
	movl %2, %%eax 																					\n\
	pushl %%eax 		# push address of first instruction			\n\
	iret 																										\n\
	execute_return:		# this is where halt will return to 	\n\
		leave																									\n\
		ret 																									\n\
	"
	:
	: "r"(USR_LVL_STACK_START), "r"(USER_CS), "r"(point_of_entry)
	: "eax"
	);

	/* EXECUTE SHOULD NEVER REACH THIS POINT. THIS IS CONSIDERED A FAILURE */

	return 0;
}

/*
	halt()

	Description: halts the execution of the currently executing process
	Inputs: status: integer value to be returned in execute()
	Outputs: -1 for failure
	Side Effects: jumps back to kernel space from user space
*/
int32_t halt(uint8_t status)
{
	cli();

	/* get the current process's PCB */
	pcb_t * current_pcb = get_PCB_from_stack();

	/* if this is the base shell, reset and execute shell again */
	if( current_pcb->parent == NULL )
	{
		processes[current_pcb->process_id] = FREE;
		terminals[current_pcb->terminal_number].current_process = -1;
		clear_screen(current_pcb->terminal_number);
		current_pcb->terminal_number = -1;
		sti();
		execute((uint8_t*)"shell");
	}



	/* update active terminal's current process */
	terminals[current_pcb->terminal_number].current_process = current_pcb->parent->process_id;

	/*

		Close relevant FDs

	*/
	int i;
	for( i = FD_OFFSET; i < FD_ARRAY_SIZE; i++ )
	{
		if( current_pcb->fd_array[i].flags == BUSY )
			close(i);
	}

	/*

		Restore Parent Paging (Mapping)

	*/
	map_task(MB128, (MB8 + (current_pcb->parent->process_id * MB4)));

	/*

		Jump to execute() return

		Note: jumping because this function should technically never return

	*/

	/* set esp back to the parent process */
	//tss.ss0 = KERNEL_DS;
	/* THIS FIXED THE STACK OVERFLOW BUG */
	tss.esp0 = MB8 - (KB8 * current_pcb->parent->process_id) - 4; // = current_pcb->parent_esp
	processes[current_pcb->process_id] = FREE;
	current_pcb->terminal_number = -1;

	sti();

	/*
		pass of the parent process's Kernel SP and BP to restore execution
		to the parent process. EAX contains uint32_t status, which is the
		return value to execute()
	*/

	/* if halting by exception, return 256 to execute to squash the user-level program */
	if( status == 255 )
	{
		int32_t exception_status = 256;
		asm volatile("												\n\
		# move ret val into EAX and return  	\n\
		mov %0, %%eax													\n\
		mov %1, %%esp													\n\
		mov %2, %%ebp													\n\
		# jump back to execute 								\n\
		jmp execute_return 										\n\
		"
		:
		: "r"((int32_t)(exception_status & STATUS_MASK)), "r"(current_pcb->parent_esp), "r"(current_pcb->parent_ebp)
		: "eax"
		);
	}
	else
	{
		asm volatile("												\n\
		# move ret val into EAX and return  	\n\
		mov %0, %%eax													\n\
		mov %1, %%esp													\n\
		mov %2, %%ebp													\n\
		# jump back to execute 								\n\
		jmp execute_return 										\n\
		"
		:
		: "r"((int32_t)(status & STATUS_MASK)), "r"(current_pcb->parent_esp), "r"(current_pcb->parent_ebp)
		: "eax"
		);
	}

	/* SHOULD NEVER REACH THIS POINT */
	return 0;
}

/*
	read()

	Description: passes parameters to file-specific read()
	Inputs: fd: file descriptor number
			buf: address to read data into
			nbytes: number of bytes to read
	Outputs: 0 for success, -1 for failure
*/
int32_t read(int32_t fd, void* buf, int32_t nbytes)
{
	/* check if fd index is valid */
	if( fd >= FD_ARRAY_SIZE || fd < MIN_FD_NUM )
		return -1;

	/* determine which process is executing */
	pcb_t * pcb = get_PCB_from_stack();

	/* check if the file has been opened */
	if( pcb->fd_array[fd].flags == FREE )
		return -1;

	/* call the correct read function */
	return pcb->fd_array[fd].f_op.read(fd, buf, nbytes);
}

/*
	write()

	Description: passes parameters to file-specific write()
	Inputs: fd: file descriptior #
			buf: pointer to buffer which is being written to screen
			nbytes: num bytes written to screen
	Outputs: the return value of the file-specific write()
	Side Effects: writes a buffer to the screen
*/
int32_t write(int32_t fd, const void* buf, int32_t nbytes)
{
	/* check if fd index is valid */
	if( fd >= FD_ARRAY_SIZE || fd < MIN_FD_NUM )
		return -1;

	/* determine which process is executing */
	pcb_t * pcb = get_PCB_from_stack();

	/* check if the file has been opened */
	if( pcb->fd_array[fd].flags == FREE )
		return -1;

	/* call the correct write function */
	return pcb->fd_array[fd].f_op.write(fd, buf, nbytes);
}

/*
	open()

	Description: opens a new spot in the file descriptor array
	Inputs: filename string
	Outputs: fd for success, -1 for failure
	Side Effects: opens/fills an fd entry in the PCB's fd_array
*/
int32_t open(const uint8_t* filename)
{
	dentry_t dentry;
	int i;
	int fd = TEMP_VALUE;
	pcb_t * pcb = get_PCB_from_stack();
	for(i = FD_OFFSET; i < FD_ARRAY_SIZE; i++)
	{
		if(pcb->fd_array[i].flags == FREE)
		{
			fd = i;
			break;
		}
	}

	if(fd == TEMP_VALUE)
		return -1;

	if(read_dentry_by_name(filename, &dentry) == -1)
		return -1;

	switch(dentry.filetype)
	{
		case RTC_DENTRY_VAL:
			pcb->fd_array[fd].f_op = rtc_fops;
			pcb->fd_array[fd].inode_num = PRESET_INODE_NUM;
			break;

		case DIR_DENTRY_VAL:
			pcb->fd_array[fd].f_op = dir_fops;
			pcb->fd_array[fd].inode_num = PRESET_INODE_NUM;
			break;

		case FILE_DENTRY_VAL:
			pcb->fd_array[fd].f_op = file_fops;
			pcb->fd_array[fd].inode_num = dentry.inode;
			break;
		default:
			//should never reach this
			return FAILED;
	}

	pcb->fd_array[fd].file_position = FILE_POS_EMPTY_FD;
	pcb->fd_array[fd].flags = BUSY;

	if( pcb->fd_array[fd].f_op.open(filename) == -1 )
		return -1;

	return fd;
}

/*
	close()

	Description: The closes the specified file descriptor and makes it available for return from later calls to open.
	Inputs: Takes in the file descriptor.
	Outputs: Returns either success or failure.
	Side Effects: Sets the flag of the file descriptor to usable.
*/
int32_t close(int32_t fd)
{
    pcb_t * pcb = get_PCB_from_stack();


    // error check the fd that will be closed. User should not close 0 and 1 or try to
    // close a closed file descriptor already. Also check for a number < 0 or > 7, which is invalid.

    if(fd < MIN_FD_NUM || fd > MAX_FD_NUM || fd == FD_IN || fd == FD_OUT || pcb->fd_array[fd].flags == FREE){
        return -1;
    }

    pcb->fd_array[fd].flags = FREE;

	if( pcb->fd_array[fd].f_op.close(fd) != 0 )
		return -1;

	return 0;


}

/*
	getargs()

	Description: Checks buf, copies arguments into user level buffer
	Inputs: buffer that holds the arguments, bytes to copy
	Outputs: Returns either success or failure
	Side Effects:
*/
int32_t getargs(uint8_t* buf, int32_t nbytes)
{
	if( buf == NULL )
		return -1;

	pcb_t * pcb = get_PCB_from_stack();

	/* check if there are any arguments */
	if (pcb->arg_buf[0] == 0)
		return -1;

	/* check if nbytes is out of bounds */
	if (nbytes < 0)
		return -1;

	if(nbytes >= MAX_BUFFER_LENGTH)
		nbytes = MAX_BUFFER_LENGTH - 1;

	strncpy((int8_t*)buf, (int8_t*)pcb->arg_buf, nbytes);

	/* check if args + NULL char fit into the buffer */
	if (pcb->arg_buf[MAX_BUFFER_LENGTH - 1] != 0)
		return -1;

	return 0;
}

/*
	vidmap()

	Description: Maps vidmem into terminal-specific user space (pre-set virtual address)
	Inputs: Takes in pointer to user video memory
	Outputs: Returns either failure or virtual address

	Note: The address that is used for mapping (132MB) doesn't *really* matter,
		  as long as it isn't equal to anywhere in the Kernel Page, User Page or Video Memory.
		  We use 132MB here since that is directly right after the User page in virtual memory.
*/
int32_t vidmap(uint8_t** screen_start)
{
	if ((uint32_t)screen_start < MB128 || (uint32_t)screen_start > (MB128 + MB4))
	{
		//printf("Address of screen_start is not within 128 - 132 MB (i.e. user page)\n");
		return -1;
	}

	pcb_t * current_pcb = get_PCB_from_stack();

	uint32_t terminal_user_vid = terminals[current_pcb->terminal_number].user_vidmem_addr;

	/* Set up page mapping (pointing) so user can safely access video memory */
	map_vidmem(terminal_user_vid, VIDMEM_START_ADDR);
	*screen_start = ((uint8_t*)terminal_user_vid);
	return terminal_user_vid;
}

/*
	set_handler()

	Description: Not implemented yet.
	Inputs: signum, handler_address
	Outputs: -1 for failure
	Side Effects: none
*/
int32_t set_handler(int32_t signum, void* handler_address)
{
	return -1;
}

/*
	sigreturn()

	Description: Not implemented yet
	Inputs: none
	Outputs: -1 for failure
	Side Effects: none
*/
int32_t sigreturn(void)
{
	return -1;
}

/*

	system_calls.h

*/

#ifndef _SYSTEM_CALLS_H
#define _SYSTEM_CALLS_H

#include "lib.h"
#include "x86_desc.h"
#include "file_system.h"
#include "paging.h"
#include "terminal.h"
#include "rtc.h"
#include "int_handler.h"
#include "scheduler.h"


#define MAX_BUFFER_LENGTH 	     1024
#define MAX_ARGUMENT_LENGTH      128
#define NEWLINE 			           10
#define SPACE 				           32
#define FOUR_BYTES			         4
#define FIVE_BYTES 			         5
#define FD_ARRAY_SIZE		         8
#define FILE_ENTRY_OFFSET	       24

#define MAGIC 				           0x7F
#define MAGIC_E				           0x45
#define MAGIC_L				           0x4C
#define MAGIC_F 			           0x46

#define PCB_MASK 			           0xFFFFE000 // from Docs/Piazza
#define USR_LVL_STACK_START	     0x83FFFFC  // 132MB - 4

#define MB128 				           0x8000000
#define MB8 				             0x0800000
#define MB4 				             0x0400000
#define KB8					             0x0002000

#define PROG_IMG_ADDR 		       0x8048000
#define MAX_NUM_PROCS		         6

#define BUSY 				             1
#define FREE				             0

#define MAX_FD_NUM 			         7
#define MIN_FD_NUM			         0
#define FD_IN				             0
#define FD_OUT				           1
#define PCB_OFFSET			         1
#define TRUE                     1
#define FAILED                  -1
#define BYTE3                    3
#define SHIFT_24                 24
#define BYTE2                    2
#define SHIFT_16                 16
#define BYTE1                    1
#define SHIFT_8                  8
#define BYTE0                    0
#define FAIL_INODE_NUM          -1
#define FILE_POS_EMPTY_FD        0
#define FD_OFFSET                2
#define RESET_PROCESS_ID        -1
#define STATUS_MASK              0xFF
#define NUM_PROCESS_OFFSET 	     1
#define TEMP_VALUE              -1
#define RTC_DENTRY_VAL           0
#define DIR_DENTRY_VAL           1
#define FILE_DENTRY_VAL          2
#define PRESET_INODE_NUM         0


extern int processes[MAX_NUM_PROCS];

/* basic failure function (returns -1) */
int32_t failure();

/* obtains the active process's PCB */
extern struct pcb_t * get_PCB_from_stack();

/* function which finds a free process ID */
int find_free_process();

/* checks if executable has args but shouldn't */
int32_t check_exec(uint8_t* cmd, int8_t* args, int n);

/*

	The System Calls

*/
int32_t execute(const uint8_t* command);
int32_t halt(uint8_t status);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
int32_t open(const uint8_t* filename);
int32_t close(int32_t fd);
int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn(void);

/*


	File Operation Structure

	This table contains the prototypes for the read,
	write, open and close system calls. Specifically,
	it containts function pointers, so that they can
	be mapped to the more specific calls for the different
	file types (i.e. terminal, RTC, etc)
*/
typedef struct file_op {
 	int32_t (*read) (int32_t fd, void* buf, int32_t nbytes);
	int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
	int32_t (*open) (const uint8_t* filename);
	int32_t (*close)(int32_t fd);
} file_op;
/*


	File Descriptor structure


*/
typedef struct fd_t {
	file_op f_op; 			//pointer to a file operations table
	int32_t inode_num; 		//inode for file
	uint32_t file_position; //where in the file we are
	uint32_t flags;
} fd_t;

/*

	Process Control Block structure

*/
typedef struct pcb_t {
	fd_t fd_array[FD_ARRAY_SIZE];        // process-specific file descriptor array
	struct pcb_t * parent; 					     // pointer to a process's parent process
	uint32_t process_id;                 // process number, ranges from 0 to MAX_NUM_PROCS
	uint32_t parent_process_id;          // parent process number, ranges from 0 to MAX_NUM_PROCS
	uint32_t parent_esp;                 // process kernel stack pointer
	uint32_t parent_ebp;                 // process kernel base pointer
	int8_t arg_buf[MAX_BUFFER_LENGTH];   // holds the process's arguments
  int terminal_number;                 // terminal number of this process (either 0,1,2)
  uint32_t esp;						             // holds the current process's esp for scheduling
	uint32_t ebp;						             // holds the current process's ebp for scheduling
	int status;
} pcb_t;

#endif

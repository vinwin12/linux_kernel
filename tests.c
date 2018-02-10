#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "paging.h"
#include "rtc.h"
#include "terminal.h"
#include "file_system.h"
#include "system_calls.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


// /* Checkpoint 1 tests */

// /* IDT Test - Example
//  *
//  * Asserts that first 10 IDT entries are not NULL
//  * Inputs: None
//  * Outputs: PASS/FAIL
//  * Side Effects: None
//  * Coverage: Load IDT, IDT definition
//  * Files: x86_desc.h/S
//  */
// int idt_test(){
// 	TEST_HEADER;
// 	int i;
// 	int result = PASS;
// 	for (i = 0; i < 10; i++){
// 		if ((idt[i].offset_15_00 == NULL) &&
// 			(idt[i].offset_31_16 == NULL)){
// 			assertion_failure();
// 			result = FAIL;
// 		}
// 	}

// 	return result;
// }

// /*memory_bounds_test
// * dereferences memory outside of vid/kernmem causing a fault
// * Input: none
// * Output: Fail if end reached
// * Side Effects: page fault
// */
// int memory_bounds_test()
// {
// 	TEST_HEADER;
// 	int*x;
// 	int y;
// 	x = (int*)(VIDMEM_START_ADDR-1);
// 	y = *x;
// 	return FAIL;

// }

// /*paging_test
// * dereference values in vidmem, kernal, then page fault
// * 	by accessing memory outside of those two ranges
// * Input: none
// * Output: pass if end is reached
// * Side Effect: none
// * Coverage: all page tests except null dereference
// */
// int paging_test()
// {
// 	TEST_HEADER;
// 	int* x;
// 	int y;
// 	x = (int*)VIDMEM_START_ADDR;
// 	y = *x;
// 	printf("First vidmem value: %x\n", y);
// 	x = (int*)KERNEL_START_ADDR;
// 	y = *x;
// 	printf("First kernel value: %x\n", y);
// 	return PASS;
// }

// /*dereference_null_test
// *	dereference a null pointer to test if paging is working
// * Input: none
// *	Output: Fail if end is reached
// * Side Effects: page fault, will freeze FOR NOW (checkpoint 1)
// */
// int dereference_null_test()
// {
// 	TEST_HEADER;
// 	int* x = NULL;
// 	*x = 5;
// 	return FAIL;
// }

// /*loop_test
// *	demonstrates that our OS can boot then enter a while(1) loop
// *	Input: none
// * Output: Fail if end is reached
// *	Side Effects: program will freeze here (infinite loop)
// */
// int loop_test()
// {
// 	TEST_HEADER;
// 	while(1){}
// 	return FAIL;
// }

// /* Checkpoint 2 tests */

// int rtc_write_test(int32_t freq){
// 	TEST_HEADER;
// 	int32_t buf[1];
// 	buf[0] = freq;
// 	if( rtc_write(buf) == -1 )
// 	{
// 		printf("Frequency not a power of 2\n");
// 		return FAIL;
// 	}
// 	return PASS;

// }

// int terminal_write_test(int * str, uint32_t bytes){
// 	TEST_HEADER;
// 	if( -1 == terminal_write(str, bytes) )
// 		return FAIL;
// 	else
// 		return PASS;

// }

// //used to test file system
// uint8_t test_buf[FOUR_K];

// /*ls_test
// * used to prove that the dentries are properly set up
// * Input: none
// * Output: pass/FAIL
// * Side Effects: performs a makeshift ls
// */
// int ls_test()
// {
// 	TEST_HEADER;
// 	clear_screen();
// 	if(directory_read(test_buf))
// 		return FAIL;
// 	puts((int8_t*)test_buf);
// 	return PASS;
// }


// /*file_read_test
// * proves the files in the fs are accessible by name
// * Input: file - name of file you wish to accessible
// * Output: pass/fail
// * Side Effects: prints file contents to screen
// */
// int file_read_test(int8_t* file)
// {
// 	TEST_HEADER;
// 	clear_screen();
// 	if(file_read((uint8_t*)file, 0, test_buf, FOUR_K) < 0)
// 		return FAIL;
// 	puts((int8_t*)test_buf);
// 	return PASS;
// }

// /*file_read_by_index
// * proves the files in the fs are accessible by index
// * Input: file - name of file you wish to accessible
// * Output: pass/fail
// * Side Effects: prints file contents to screen
// */
// int file_read_by_index(uint32_t index)
// {
// 	TEST_HEADER;
// 	clear_screen();
// 	dentry_t temp_dentry;
//   	if(read_dentry_by_index(index, &temp_dentry))
//     	return FAIL;
//   	if(read_data(temp_dentry.inode, 0, test_buf, FOUR_K)<0)
// 		return FAIL;
// 	puts((int8_t*)test_buf);
// 	return PASS;
// }

// /*	execute_test_1
//  *
//  *	tests if file name is parsed correctly, and if the
//  *	filesystem read / vailidity works correctly
//  */
// int execute_test_1(uint8_t * buf)
// {
// 	TEST_HEADER;
// 	if( 0 == execute(buf) )
// 		return PASS;
// 	else
// 		return FAIL;
// }


/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	/*checkpoint 1 tests
	* Start with no tests commented out.
	* idt_test checks values in idt to be filled
	* paging_test checks if you can access vid and kern mem
	* loop_test reaches while(1) loop. Comment once proven.
	* memory_bounds_test dereferences outside of kern/vidmem causing fault. Comment once proven.
	* dereference_null_test does just that. Comment once proven.
	*/
	//TEST_OUTPUT("idt_test", idt_test());
	//TEST_OUTPUT("memory_bounds_test", memory_bounds_test());
	//TEST_OUTPUT("paging_test", paging_test());
	//TEST_OUTPUT("dereference_null_test", dereference_null_test());
	//TEST_OUTPUT("loop_test", loop_test());

	/*

		Checkpoint 2 Tests

	*/

	/* Run 1 */
	//TEST_OUTPUT("rtc_write_test", rtc_write_test(8));
	/* Run 2 */
	//TEST_OUTPUT("rtc_write_test", rtc_write_test(128));
	/* Run 3 */
	//TEST_OUTPUT("rtc_write_test", rtc_write_test(1024));
	/* Run 4 */
	//TEST_OUTPUT("rtc_write_test: 2048", rtc_write_test(2048));
	//TEST_OUTPUT("rtc_write_test: -1", rtc_write_test(-1));
	//TEST_OUTPUT("rtc_write_test: 23", rtc_write_test(23));
	/* Run 5 */
	/* TEST STRINGS: edit for terminal write tests */
	//int string1[128] = {'V', 'i', 'n', ' ', 's', 'm', 'e', 'l', 'l', 's', '!'};
	//int string2[128] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 'X', '\n'};
	//TEST_OUTPUT("terminal_write_test", terminal_write_test(string1, 11));
	//printf("\n");
	//TEST_OUTPUT("terminal_write_test", terminal_write_test(string2, 11));
	/* Run 6 */
	//TEST_OUTPUT("file_read_test", file_read_test("frame0.txt"));
	/* Run 7 */
	//TEST_OUTPUT("file_read_by_index", file_read_by_index(8));
	/* Run 8 */
	//TEST_OUTPUT("ls_test", ls_test());

	/*

		Checkpoint 3 Tests

	*/
	// /* tests if file name works */
	// uint8_t buffer1[1023] = {'l', 's', '\n'};
	// /* tests if file name works w/ trailing spaces */
	// uint8_t buffer2[1023] = {' ', ' ', ' ', 's', 'h', 'e', 'l', 'l', ' ', ' ', '\n'};
	// /* tests if invalid file name fails */
	// uint8_t buffer3[1023] = {'c', 'h', 'r', 'i', 's', ':', ')', ' ', ' ', ' ', '\n'};
	// /* tests if file name and 1 arg works */
	// uint8_t buffer4[1023] = {' ', 's', 'h', 'e', 'l', 'l', ' ', ' ', '-', 'c', ' ', '\n'};
	//  tests if file name and multiple args works
	// uint8_t buffer5[1023] = {' ', 's', 'h', 'e', 'l', 'l', ' ', ' ', '-', 'c', ' ', ' ', ' ', '-', 'f', '\n'};
	// /* tests buffer of max size, with file name at the very end */
	// uint8_t buffer6[1023];
	// int i;
	// for( i = 0; i < 1023; i++ )
	// {
	// 	buffer6[i] = ' ';
	// }
	// buffer6[696] = 's';
	// buffer6[697] = 'h';
	// buffer6[698] = 'e';
	// buffer6[699] = 'l';
	// buffer6[700] = 'l';
	// buffer6[1021] = '-';
	// buffer6[1022] = 'c';


	// TEST_OUTPUT("execute_test_1", execute_test_1(buffer6));

	/*

		Checkpoint 4 tests

	*/

	/* vidmap non-user-level failure test */
	/*
	uint8_t* vid_map_base;

	if( vidmap(&vid_map_base) == -1 )
		printf("Address of vid_map_base is not within 128 - 132 MB (i.e. user page)\n");
	else
	{
		printf(vid_map_base);
	}
	*/
}

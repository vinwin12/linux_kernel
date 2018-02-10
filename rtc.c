/*
    rtc.c

*/

#include "rtc.h"
#include "keyboard.h"
#define HZ  0xF //2
#define USR_LIMIT_HZ 1024
#define RTC_PORT 0x70
#define CMOS_PORT 0x71
#define A_REGISTER 0x8A
#define B_REGISTER 0x8B
#define C_REGISTER 0x8C
#define ENABLE_PERIODIC_INT 0x40
#define RTC_INT_NUM 8



/*
	rtc_init()

	Description: initializes rtc
	Inputs: None
	Outputs: None
	Side Effects: Enables rtc IRQs, writes to RTC ports
    Inspiration: OSDev
    TODO: Check whether values passed in Register A and B should deal with specific bit or all bits.

*/
void RTC_init(){

	char prev;

    /*marks start of critical section*/
    cli();//Masking all interrupts excluding NMI

    //writing to Register A to specify the Hz

    outb(A_REGISTER,RTC_PORT);  //specifying what register
    prev = inb(CMOS_PORT);
    outb(A_REGISTER,RTC_PORT);  //specifying what register
    outb(prev | HZ,CMOS_PORT);  //writing Hz = 2 in Register A

    //writing to Register B

    outb(B_REGISTER,RTC_PORT);                  //specifying what register
    prev = inb(CMOS_PORT);                 		//reading the previous value of Register B to make sure we write the same value besides Bit 6
    outb(B_REGISTER,RTC_PORT);                  //Accessing Register B
    outb(prev | ENABLE_PERIODIC_INT,CMOS_PORT); //write to bit 6 in Register B to enable periodic Interrupts

    sti();//unmasking all interrupts excluding NMI
    /*marks end of critical section*/

    enable_irq(RTC_INT_NUM);
}

/*
	RTC_handler()

	Description: handler for an RTC interrupt
	Inputs: None
	Outputs: None
	Side Effects: handles RTC interrupt
*/
void RTC_handler()
{
  cli();

	int i;
  for( i = 0; i < 3; i++ )
	{
		terminals[i].rtc_flag = ACTIVE;
	}
	outb(C_REGISTER, RTC_PORT);
	// From OSDev: don't care about what's in Reg C
	inb(CMOS_PORT);

  sti();

	send_eoi(RTC_INT_NUM);
}




/*
	rtc_open()

	Description: initializes rtc
	Inputs: pointer to filename
	Outputs: should always return 0
	Side Effects: None

*/



int32_t rtc_open(const uint8_t* filename) {

    return SUCCESS;
}



/*
	rtc_close()

	Description: closes rtc
	Inputs: file descriptor
	Outputs: 0
    Side Effects: None

*/

int32_t rtc_close(int32_t fd) {
        //does nothing for now, will change when virtualizing RTC happens
    return SUCCESS;
}


/*
	rtc_read()

	Description: Halts until next interrupt
	Inputs: file descriptor, buffer, nbytes
	Outputs: 0
	Side Effects: Resets flag to 0

*/


int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes) {

		pcb_t * current_pcb = get_PCB_from_stack();

    //spins
    while(!terminals[current_pcb->terminal_number].rtc_flag); //should block until the next interrupt


    //reset flag
    terminals[current_pcb->terminal_number].rtc_flag = RESET;

    return SUCCESS;
}



/*
	rtc_write()

	Description: Changes the RTC frequency
	Inputs: file descriptor, buffer, nbytes
	Outputs: 0 for success, -1 for failure
	Side Effects: writes to RTC ports
  Inspiration: OSDev

*/


int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes) {

    //make sure nbytes is equal to 4, since the Documentation requires it
    if(nbytes != RTC_WRITE_SUCCESS || (int32_t)buf == NULL )
        return FAILURE;

    //receving frequency from position 0 of buf
    int32_t* temp_ptr = (int32_t*)buf;
    int32_t frequency = temp_ptr[0];

    uint8_t prev;
    uint8_t value;

    //check whether it's in bounds and a power of 2

    //must execute first
    if(!(frequency >= 2 && frequency <= 1024)){
        return FAILURE;
    }

    switch (frequency) {

        case F_2:
            value = HEX_2;
            break;
        case F_4:
            value = HEX_4;
            break;
        case F_8:
            value = HEX_8;
            break;
        case F_16:
            value = HEX_16;
            break;
        case F_32:
            value = HEX_32;
            break;
        case F_64:
            value = HEX_64;
            break;
        case F_128:
            value = HEX_128;
            break;
        case F_256:
            value = HEX_256;
            break;
        case F_512:
            value = HEX_512;
            break;
        case F_1024:
            value = HEX_1024;
            break;
        default:        //not a power of TWO
            return FAILURE;

    }


    //put frequency(in proper bits) into Register A


    cli();//Masking all interrupts excluding NMI

    //writing to Register A to specify the Hz

    outb(A_REGISTER,RTC_PORT);  //specifying what register
    prev = inb(CMOS_PORT);
    outb(A_REGISTER,RTC_PORT);  //specifying what register
    value = value | (prev & CLEAR_REG); //clearing the last 4 bits of Reg. A
    outb(value,CMOS_PORT);  //writing frequency into Register A

    sti();


    return RTC_WRITE_SUCCESS; //returns 4, the number of bytes written
}

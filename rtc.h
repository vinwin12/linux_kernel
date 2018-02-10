
#ifndef _RTC_H
#define _RTC_H

#include "lib.h"
#include "i8259.h"
#include "system_calls.h"

/* RTC relevant constants */
#define HZ  0xF //2
#define USR_LIMIT_HZ 1024
#define RTC_PORT 0x70
#define CMOS_PORT 0x71
#define A_REGISTER 0x8A
#define B_REGISTER 0x8B
#define C_REGISTER 0x8C
#define ENABLE_PERIODIC_INT 0x40

#define RTC_INT_NUM 8
#define RESET 0
#define ACTIVE 1
#define RTC_WRITE_SUCCESS 4
#define SUCCESS 0
#define FAILURE -1



//for frequency check and HEX value
#define F_2 2
#define HEX_2 0xF
#define F_4 4
#define HEX_4 0xE
#define F_8 8
#define HEX_8 0xD
#define F_16 16
#define HEX_16 0xC
#define F_32 32
#define HEX_32 0xB
#define F_64 64
#define HEX_64 0xA
#define F_128 128
#define HEX_128 0x9
#define F_256 256
#define HEX_256 0x8
#define F_512 512
#define HEX_512 0x7
#define F_1024 1024
#define HEX_1024 0x6
#define CLEAR_REG 0xF0


/* Initializes the RTC */
void RTC_init();

/* RTC interrupt handler */
void RTC_handler();

/*initializes RTC frequency to 2HZ*/
int32_t rtc_open(const uint8_t* filename);

/*closes RTC, does nothing at the moment*/
int32_t rtc_close(int32_t fd);

/*blocks until the next interrupt*/
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

/*writes a frequency into the register*/
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

#endif

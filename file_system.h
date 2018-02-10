#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "lib.h"
#include "types.h"
#include "x86_desc.h"
#include "system_calls.h"

#define MAX_DENTRIES  63
#define STATS_RESERVED  52
#define STATS_SIZE 64
#define FILENAME_SIZE  32
#define DENTRY_RESERVED  24
#define ONE_K  1024
#define FOUR_K  4096
// first half of boot block is statistics
typedef struct
{
	uint32_t num_dentries;
	uint32_t num_inodes;
	uint32_t num_datablocks;
	uint8_t  reserved[STATS_RESERVED];
} fs_statistics_t;

// directory entry, 63 of these in boot block
typedef struct
{
  int8_t filename[FILENAME_SIZE];
  uint32_t filetype;
  uint32_t inode;
  uint8_t reserved[DENTRY_RESERVED];
} dentry_t;

// inode, length in bytes
typedef struct
{
  uint32_t length;
  uint32_t datablocks[ONE_K - 1];
} inode_t;

// three main file system access functions from Apendix A
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

//file system startup/shutdown
int32_t file_system_init(uint32_t start_addr);
int32_t file_system_close(void);

//read, write, open, close for directory and files
int32_t directory_open(const uint8_t* filename);
int32_t directory_close(int32_t fd);
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes);
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t file_open(const uint8_t* filename);
int32_t file_close(int32_t fd);
int32_t file_read(int32_t fd, void* buf, int32_t nbytes);
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);

#endif

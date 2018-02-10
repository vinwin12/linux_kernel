/*
    file_system.c

*/

#include "file_system.h"

// used to prevent double open, 1 = open
uint32_t fs_open_flag;

// boot block pointer, need for init
uint32_t boot_block;

// the fs stats in the boot block (64B)
fs_statistics_t fs_stats;

// dentry array, can be 63 long
dentry_t *dentries;

// inode array,
inode_t *inodes;

// datablocks start address
uint32_t datablocks_start;

// counts the file_name reads for ls
uint32_t ls_helper = 0;

/*read_dentry_by_name
* finds a dentry by name and copies the info into the dentry param
* Input: fname - name of entry to find
*        dentry - where to copy the data to
* Output: -1 fail, 0 success
* Side Effects: none
*
*/
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
{
  int i;
  int32_t str_length = strlen((int8_t*)fname);

  //check valid input
  if(fname == NULL || dentry == NULL || str_length == 0 || str_length > FILENAME_SIZE)
    return -1;

  for(i = 0; i < MAX_DENTRIES; i++)
  {
    if(str_length==strlen(dentries[i].filename) ||
        (str_length == FILENAME_SIZE &&
          strlen(dentries[i].filename) >= FILENAME_SIZE))
      if(!strncmp((int8_t*)fname, dentries[i].filename, FILENAME_SIZE))
      {
        // copy all params into dentry, success
        strcpy(dentry->filename, dentries[i].filename);
				dentry->filetype = dentries[i].filetype;
				dentry->inode = dentries[i].inode;
				return 0;
      }
  }
  // no such dentry exists
  return -1;
}

/*read_dentry_by_index
* finds a dentry by index and copies the info into the dentry param
* Input: index - dentry to copy
*        dentry - where to copy the data to
* Output: -1 fail, 0 success
* Side Effects: none
*/
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
{
  if(index >= MAX_DENTRIES || dentry == NULL)
    return -1;

  // copy all params into dentry, success
	strcpy(dentry->filename, dentries[index].filename);
	dentry->filetype = dentries[index].filetype;
	dentry->inode = dentries[index].inode;

	return 0;
}

/*read_data
* reads length bytes of data (maximum) into buf from inode
* Input: inode - inode of file being read
         offset - where in file to start read (in bytes)
         buf - where to read the data into
         length - number of bytes to read
* Output: -1 fail, 0 end of file reached, or num_bytes_read
* Side Effects: none
uint8_t *read_addr;
*/
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length)
{
  uint32_t cur_datablock;
  uint32_t block_offset;
  uint32_t num_bytes_read;
  uint8_t *read_addr;

  //check valid buf
  if(buf == NULL)
    return -1;

  // check valid inode #
  if(inode >= fs_stats.num_inodes)
    return -1;

  // check valid offset
  if(offset > inodes[inode].length)
    return -1;

  // check if at end of file already
  if(offset == inodes[inode].length)
    return 0;

  // get cur_datablock, check valid
  cur_datablock = offset / FOUR_K;
	if(inodes[inode].datablocks[cur_datablock] >= fs_stats.num_datablocks)
		return -1;

  // where in block to start
  block_offset = offset % FOUR_K;

  // where to actually read from
  read_addr = (uint8_t *)(datablocks_start +
    (inodes[inode].datablocks[cur_datablock]) * FOUR_K + block_offset);

  // start copying data
  num_bytes_read = 0;
  while(num_bytes_read < length)
  {
    if(block_offset >= FOUR_K)
    {
      // reset block_offset, inc/check cur_datablock
      block_offset = 0;
      cur_datablock++;
      if(inodes[inode].datablocks[cur_datablock] >= fs_stats.num_datablocks)
        return -1;
      read_addr = (uint8_t*)(datablocks_start +
        (inodes[inode].datablocks[cur_datablock]) * FOUR_K);
    }
    // check for end of file
    if(num_bytes_read + offset >= inodes[inode].length)
      return num_bytes_read;
    // do the copy, inc counters
    buf[num_bytes_read] = *read_addr;
    block_offset++;
    num_bytes_read++;
    read_addr++;
  }
  return num_bytes_read;
}

/*file_system_open
* initializes all variables if the file system isn't already open
* Input: start_addr - address the fs starts at
* Output: 0 - success, -1 - failure
* Side Effects: file system is now initialized
*/
int32_t file_system_init(uint32_t start_addr)
{
  if(fs_open_flag == 1)
    return -1;
  //set start addr of dentries, inodes, and the data blocks
  boot_block = start_addr;
  memcpy(&fs_stats, (void*)boot_block, STATS_SIZE);
  dentries = (dentry_t*)(boot_block + STATS_SIZE);
  inodes = (inode_t *)(boot_block + FOUR_K);
  datablocks_start = boot_block + (fs_stats.num_inodes+1)*FOUR_K;
  //mark fs as open
  fs_open_flag = 1;
  return 0;
}

/*file_system_close
* sets the fs_open_flag to 0
* Input: none
* Output: 0 success, -1 fail
* Side Effects: none
*/
int32_t file_system_close(void)
{
  if(fs_open_flag == 0)
    return -1;
  fs_open_flag = 0;
  return 0;
}

/*directory_open
* does nothing
* Input: filename string
* Output: 0 = success
* Side Effects: none
*/
int32_t directory_open(const uint8_t* filename)
{
  return 0;
}

/*directory_close
* does nothing
* Input: file descriptor number
* Output: 0 = success
* Sides Effects: none
*/
int32_t directory_close(int32_t fd)
{
  return 0;
}

/*directory_read
* Performs dir-specific read() sys call
* Input: fd: file descriptor number
         buf: address to read data into (SHOULD BE DIRECTORY NAME)
         nbytes: number of bytes to read
* Output: 0 - success, -1= fail
* Side Effects: prints to screen
*/
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes)
{
    int i = 0;

    /* check if we have finished prnting all of the file names in the directory */
    if(ls_helper < fs_stats.num_dentries)
    {
        /* copy the file name into the buffer */
        while( (dentries[ls_helper].filename[i] != NULL) && (i < FILENAME_SIZE) )
        {
          ((int8_t*)(buf))[i] = dentries[ls_helper].filename[i];
          i++;
        }
        ls_helper++;
        return i;
    }
    else
    {
        ls_helper = 0;
        return 0;
    }
}


/*directory_write
* does nothing
* Input: fd, buf, nbytes (not used)
* Output: -1 always (read only)
* Side Effects: none
*/
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes)
{
  return -1;
}

/*file_open
* does nothing
* Input: filename (not used)
* Output: 0 = success
* Side Effects: none
*/
int32_t file_open(const uint8_t* filename)
{
  return 0;
}

/*file_close
* does nothing
* Input: fd (not used)
* Output: 0 = success
* Side Effects: none
*/
int32_t file_close(int32_t fd)
{
  return 0;
}

/*file_read
* Perform the fs-specific read() system call
* Input: fd: file descriptor array number
         buf: address to read data into
         nbytes: number of bytes to read
* Output:-1 = fail, 0 = end of file, positive number = bytes read
* Side Effects: reads nbytes of data into a buffer
*/
int32_t file_read(int32_t fd, void* buf, int32_t nbytes)
{
  uint32_t inode;
  uint32_t offset;
  int f_pos_update;

  /* get PCB for current process */
  pcb_t * pcb = get_PCB_from_stack();

  /* obtain the inode number and offset from the PCB */
  inode = (uint32_t)pcb->fd_array[fd].inode_num;
  offset = pcb->fd_array[fd].file_position;

  /* read the data into the buf */
  f_pos_update = read_data(inode, offset, (uint8_t*)buf, nbytes);

  /* read failed */
  if( f_pos_update == -1 )
    return -1;

  /* update file position within the PCB */
  pcb->fd_array[fd].file_position += f_pos_update;

  return f_pos_update;
}

/*file_write
* does nothing
* Input: fd, buf, nbytes (not used)
* Output: -1 always (read only)
* Side Effect: none
*/
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes)
{
  return -1;
}

#ifndef FILE_H
#define FILE_H

#include <stdbool.h>
#include <stdint.h>
#include "pipe.h"
// NDIRECT Macro In disk.h
#ifndef NDIRECT
#define NDIRECT 12
#endif
struct inode_s;
// File Descriptor
typedef struct file_s {
	enum { FD_FILE, FD_INODE, FD_PIPE } type;
	int ref_count;  // Number Of Soft (Symbolic) Links
	struct inode_s* inode; // Underlying inode_s (FD_INODE) 	
	struct pipe_s*  pipe;  // Underlying pipe_s (FD_PIPE)
} file_s;
// Underlying Index Node
typedef struct inode_s {
	uint16_t id; 
	bool valid;		     // Tracks Whether Or Not This Inode Has Been Read
	uint8_t mode; 		     // Read / Write / Execute Permissions
	uint16_t size;		     // Number Of Blocks Allocated
	int ref_count; 		     // Number Of Hard Links
	uint32_t address[NDIRECT+1]; // A Number Of Direct Pointers To Disk Addresses, And One Indirect Pointer
} inode_s;

#endif

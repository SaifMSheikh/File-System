#ifndef DISK_H
#define DISK_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define BLOCK_SIZE (4 * 1024) // Size Of Each Logical Segment Of The Disk
// Maximum File Direct & Indirect Pointers
#define NDIRECT 12
#define NINDIRECT (BLOCK_SIZE / sizeof(uint16_t))
#define MAX_FILE NDIRECT + NINDIRECT
// Disk Superblock (File Header)
typedef struct superblock_s {
	uint32_t size;        // Total Number Of Blocks
	uint32_t data_size;   // Number Of Data Blocks
	uint32_t inode_size;  // Number Of Inode Blocks
	uint32_t data_start;  // Data Starting Block
	uint32_t inode_start; // Inode Starting Block
} superblock_s;
// Logical Representation Of A Disk
typedef struct disk_s {
	superblock_s info;
	const char* file_name; // "Disk" File Path
	uint8_t* mem_start;    // Start Of Allocated Memory Block
	size_t mem_size;       // Size Of Allocated Memory Block
	FILE* file;	       // "Disk" File Descriptor
} disk_s;
// Public Disk Interface
bool is_valid(const disk_s*,const char*);
bool disk_open(disk_s*,const char*);
bool disk_close(disk_s*);
// Protected Disk Interface
superblock_s _disk_info(const disk_s*);
uint8_t* _disk_data_get(const disk_s*,const uint16_t);
uint16_t _disk_data_alloc(const disk_s*);
bool _disk_data_free(const disk_s*,const uint16_t);
#endif

#ifndef DISK_H
#define DISK_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define BLOCK_SIZE (4 * 1024) // Size Of Each Logical Segment Of The Disk
#define DIR_SIZE 14
// Maximum File Direct & Indirect Pointers
#define NDIRECT 12
#define NINDIRECT (BLOCK_SIZE / 4)
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
// On-Disk Directory Structure
typedef struct dirent_s {
	// Total Size Will Be (DIR_SIZE * 8) + 16 = 16 * 16 bytes When DIR_SIZE = 14
	uint16_t inode;
	char name[DIR_SIZE];
} dirent_s;
// On-Disk Inode Structure
typedef struct dnode_s {
	uint16_t type;
	uint32_t size;
	uint32_t address[NDIRECT+1];
} dnode_s;
#define IPB (BLOCK_SIZE/sizeof(dnode_s)) // Inodes Per Block
#define IBLOCK(_inum_,_superblock_) ((_inum_)/IPB+_superblock_.inode_start) // Block Containing Inode #_inum_
bool is_valid(const disk_s*,const char*);
bool disk_open(disk_s*,const char*);
bool disk_close(disk_s*);
superblock_s disk_info(const disk_s*);

#endif

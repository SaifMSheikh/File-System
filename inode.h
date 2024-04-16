#ifndef INODE_H
#define INODE_H

#include "disk.h"
struct dnode_s;
#define I_FREE 0
#define I_FILE 1
#define I_DIRE 2
// Logical Index Node Structure
typedef struct inode_s {
	struct dnode_s* info;
	disk_s*  dev;
	uint16_t inum;
	bool     valid;
} inode_s;
// On-Disk Inode Structure
typedef struct dnode_s {
	uint8_t  type; // 0 - Free / 1 - I_FILE / 2 - I_DIR
	uint16_t size; // Number Of Blocks
	uint8_t  ref_count;
	uint16_t addr[NDIRECT+1];
} dnode_s;
#define IPB (BLOCK_SIZE/sizeof(dnode_s)) // Inodes Per Block
#define IMAX(_superblock_) ((_superblock_).inode_size*IPB)
#define IBLOCK(_inum_,_superblock_) ((_inum_)/IPB+_superblock_.inode_start) // Block Containing Inode #_inum_
// DNode Operations
dnode_s*       _disk_inode(const disk_s*,const uint16_t);
bool           _disk_inode_free(const disk_s*,const uint16_t);
const uint16_t _disk_inode_alloc(const disk_s*);
// Inode Operations
inode_s _inode_get(disk_s*,const uint16_t);
inode_s _inode_create(disk_s*,const uint8_t);
bool    _inode_destroy(inode_s*);

#endif

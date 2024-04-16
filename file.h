#ifndef FILE_H
#define FILE_H

#include "inode.h"
#define NDIRENT (BLOCK_SIZE/sizeof(dirent_s))
#define DIRENT_NAME_LEN 14 // Maximum # Of Characters Allowed For Dirent Name
// Logical File Descriptor Structure
typedef struct file_s {
	uint16_t inum;
	uint8_t ref_count;
	bool readable;
	bool writable;
} file_s;
// On-Disk Directory Structure
typedef struct dirent_s {
	uint16_t inum;
	char name[DIRENT_NAME_LEN];
} dirent_s;
// Directory Operations
uint16_t dir_lookup(inode_s*,char*);
uint16_t dir_create(inode_s*,char*);
void     dir_print(const inode_s*,const bool);
void    _dir_print(const inode_s*,const bool,const uint8_t);

#endif

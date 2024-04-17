#ifndef FILE_H
#define FILE_H

#include "inode.h"
// Directory Entry Utilities
#define NDIRENT (BLOCK_SIZE/sizeof(dirent_s))
#define DIRENT_NAME_LEN 14 // Maximum # Of Characters Allowed For Dirent Name
// File Permissions (Open Modes)
#define FILE_READABLE_BIT 0
#define FILE_WRITABLE_BIT 1
#define FILE_EXECUTABLE_BIT 2 // This Does Nothing
// Logical File Descriptor Structure
typedef struct file_s {
	uint16_t inum;
	uint8_t mode;
	bool valid;
} file_s;
// On-Disk Directory Structure
typedef struct dirent_s {
	uint16_t inum;
	char name[DIRENT_NAME_LEN];
} dirent_s;
// Public Interface
uint16_t dir_lookup(const inode_s*,const char*);
void     dir_print(const inode_s*);
inode_s  file_create(const inode_s*,const char*,const uint8_t type);
bool     file_delete(const inode_s*,const char*);

#endif

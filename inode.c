#include "inode.h"
// Inode Types
#define I_FREE 0
#define I_FILE 1
#define I_DIRE 2
// Get Inode Address From Disk
dnode_s* _disk_inode(const disk_s* disk, const uint16_t inum) { 
	char* err_str;
	if (!is_valid(disk,err_str)) {
		printf("%s",err_str);
		return NULL;
	}
	return (dnode_s*)&disk->mem_start[IBLOCK(inum,disk->info)+(inum*sizeof(dnode_s))]; 
}
const uint16_t _disk_inode_alloc(disk_s* disk) {
	// Validate Inputs
	if (!is_valid(disk,NULL))
		return -1;
	// Query Inode Bitmap For Free Space
	uint16_t inum = 0;
	for (uint8_t* imap = &disk->mem_start[BLOCK_SIZE]; inum < IMAX(disk->info);++inum) {
		if (!imap[inum/8]&(1<<(inum%8))) {
			// Found Free Inode
			imap[inum/8]|=(1<<(inum%8));
			return inum;
		}
	}
	// No Free Inodes
	printf("No Free Inodes");
	return -1;
}
bool _disk_inode_free(disk_s* disk,const uint16_t inum) {
	// Validate Inputs
	if (!is_valid(disk,NULL)) 
		return false;
	dnode_s* node = _disk_inode(disk,inum);
	if (!node->type)
		return false;
	// Update Inode Bitmap
	uint8_t* imap = &disk->mem_start[BLOCK_SIZE];
	imap[inum/8]&=~(1<<(inum%8));
	// Update Data Bitmap For All Data Blocks
	uint8_t* dmap = &disk->mem_start[BLOCK_SIZE*2];
	for (int i = 0;i<NDIRECT;++i)
		if (node->addr[i]>=0)
			dmap[node->addr[i]/8]&=~(1<<(node->addr[i]%8));
}

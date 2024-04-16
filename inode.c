#include "inode.h"
#include "file.h"
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
	return (dnode_s*)&(disk->mem_start[IBLOCK(inum,disk->info)+(inum*sizeof(dnode_s))]); 
}
const uint16_t _disk_inode_alloc(const disk_s* disk) {
	// Validate Inputs
	char* err_str;
	if (!is_valid(disk,err_str)) {
		printf("%s",err_str);
		return IMAX(disk->info);
	}
	// Query Inode Bitmap For Free Space
	uint8_t* imap = &(disk->mem_start[BLOCK_SIZE]);
	for (uint16_t inum = 0; inum < IMAX(disk->info);++inum) {
		if (!(imap[inum/8]&(1<<(inum%8)))) {
			// Found Free Inode
			imap[inum/8]|=(1<<(inum%8));
			// Set Default State
			dnode_s* node=_disk_inode(disk,inum);
			node->ref_count=0;
			node->type=0;
			node->size=0;
			return inum;
		}
	}
	// No Free Inodes
	printf("No Free Inodes\n");
	return IMAX(disk->info);
}
bool _disk_inode_free(const disk_s* disk,const uint16_t inum) {
	// Validate Inputs
	char* err_str;
	if (!is_valid(disk,err_str)) {
		printf("%s",err_str);
		return false;
	}
	if (inum >= IMAX(disk->info)) {
		printf("Invalid Index\n");
		return false;
	}
	// Update Inode Bitmap
	uint8_t* imap = &(disk->mem_start[BLOCK_SIZE]);
	imap[inum/8]&=~(1<<(inum%8));
	// Update Data Bitmap For All Data Blocks
	uint8_t* dmap = &(disk->mem_start[BLOCK_SIZE*2]);
	dnode_s* node = _disk_inode(disk,inum);
	for (int i = 0;i<(node->size>NDIRECT?node->size:NDIRECT);++i)
		if (node->addr[i]>=0)
			dmap[node->addr[i]/8]&=~(1<<(node->addr[i]%8));
	// Free Indirect Data
	if (node->size>NDIRECT) { 
		int size=node->size-NDIRECT;
		uint16_t* entry=(uint16_t*)&(disk->mem_start[BLOCK_SIZE*(disk->info.data_start+node->addr[NDIRECT])]);
		for (int i=0;(i<size)&&(i<NINDIRECT);++i)
			dmap[entry[i]/8]&=~(1<<(entry[i]%8));
	}
	return true;
}
// Get Allocated Inode
inode_s _inode_get(disk_s* disk,const uint16_t inum) {
	// Populate Inode Struct
	inode_s node;
	node.valid=false;
	node.inum=inum;
	node.dev=disk;
	// Get Disk-Allocated Node
	node.info=_disk_inode(disk,inum);
	// Validate Structure
	if (node.inum<IMAX(disk->info))
		node.valid=true;
	return node;
}
// Allocate New Inode
inode_s _inode_create(disk_s* disk,const uint8_t type) {
	inode_s node;
	node.valid=false;
	// Validate Input
	char* err_str;
	if (!is_valid(disk,err_str)) {
		printf("%s",err_str);
		return node;;
	}
	// Allocate Disk Space & Populate Inode Structure
	node.dev=disk;
	node.inum=_disk_inode_alloc(disk);
	if (node.inum>=IMAX(disk->info))
		return node;
	node.info=_disk_inode(disk,node.inum);
	if (node.info==NULL)
		return node;
	node.info->type=type;
	if (node.info->type==I_DIRE) {
		node.info->addr[0]=_disk_data_alloc(disk);
		if (node.info->addr[0]>=disk->info.data_size)
			return node;
	}
	// Validate Inode Struct
	node.valid=true;
	return node;
}
// Release Inode
bool _inode_destroy(inode_s* inode) {
	// Validate Input
	if (!inode->valid) {
		printf("Invalid Inode\n");
		return false;
	}
	inode->valid=false;
	// Free Data
	switch (inode->info->type) {
		case I_FILE: {
			// Direct Data
			for (int i=0;(i<inode->info->size)&&(i<NDIRECT);++i)
				_disk_data_free(inode->dev,i);
			// Indirect Data
			int size=(int)inode->info->size-NDIRECT;
			uint16_t* entry=(uint16_t*)_disk_data_get(inode->dev,inode->info->addr[NDIRECT]);
			for (int i=0;(i<size)&&(i<NINDIRECT);++i)	
				_disk_data_free(inode->dev,i);
			break;
		}
		case I_DIRE: {
			// Indirect Data
			dirent_s* entry=(dirent_s*)_disk_data_get(inode->dev,inode->info->addr[0]);
			for (int i=0;(i<inode->info->size)&&(i<NDIRENT);++i) {
				inode_s entry_node=_inode_get(inode->dev,entry[i].inum);
				if (!_inode_destroy(&entry_node))
					return false;
			}
			break;
		}
	};
	// Free Disk Space
	return _disk_inode_free(inode->dev,inode->inum);
}

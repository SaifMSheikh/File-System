#include "disk.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
// Check Whether Disk Struct Is In A Valid State
bool is_valid(const disk_s* disk, const char* err_str) {
	err_str = "";
	if (disk->mem_start == NULL)
		strcat("Invalid Memory Pointer\n",err_str);
	if (disk->mem_size <= 0l)
		strcat("Invalid Memory Size\n",err_str);
	if (disk->file == NULL)
		strcat("Invalid File Descriptor\n",err_str);
	return strcmp(err_str,"") == 0;
}
// Read Disk File Header (Superblock)
superblock_s _disk_info(const disk_s* disk) {
	printf("Reading Filesystem Data...");
	superblock_s superblock;
	const uint32_t* header = (const uint32_t*)disk->mem_start;
	superblock.size = *(header++);
	superblock.data_size = *(header++);
	superblock.inode_size = *(header++);
	superblock.data_start = *(header++);
	superblock.inode_start = *header;
	printf("Success\n");
#ifndef NDEBUG
	printf("Data:\n");
	printf("\tFile System Size : %d (%d Bytes)\n",superblock.size,superblock.size*BLOCK_SIZE);
	printf("\tData Blocks : %d (%d Bytes)\n",superblock.data_size,superblock.data_size*BLOCK_SIZE);
	printf("\tMeta-Data (Inode) Blocks : %d (%d Bytes)\n",superblock.inode_size,superblock.inode_size*BLOCK_SIZE);
	printf("\tData Start : Block %d\n",superblock.data_start);
	printf("\tInode Start : Block %d\n",superblock.inode_start);
#endif
	return superblock;
}
// Initialize Disk Struct Into Valid State
bool disk_open(disk_s* disk, const char* file_name) {
	// Open Disk File
	disk->file = fopen(file_name,"r+");
	if (disk->file == NULL) {
		printf("Failed To Open %s",file_name);
		return false;
	}
	disk->file_name=file_name;
	// Get File Size
	fseek(disk->file,0,SEEK_END);
	disk->mem_size = ftell(disk->file);
	disk->mem_size -= disk->mem_size % BLOCK_SIZE;
	rewind(disk->file);
#ifndef NDEBUG
	printf("Allocating %ld Bytes... ",disk->mem_size);
#endif
	// Load Disk File Into Memory
	disk->mem_start = (uint8_t*) malloc(disk->mem_size);
	uint32_t ret = fread((void*)disk->mem_start,sizeof(uint8_t),disk->mem_size,disk->file);
	if (ret <= 0) {
#ifndef NDEBUG
		printf("Failed\n");
#endif
		if    (ferror(disk->file)) printf("Error Reading %s\n",disk->file_name);
		else if (feof(disk->file)) printf("Reached End Of File\n");
		free(disk->mem_start);
		fclose(disk->file);
		return false;
	}
#ifndef NDEBUG
	printf("Success.\n");
	printf("Allocated %ld Blocks (%db Each)\n",disk->mem_size/BLOCK_SIZE,BLOCK_SIZE);
#endif
	// Validate Disk State
	const char* check;
	if (!is_valid(disk,check)) {
		printf("Disk Struct Invalid: \n%s",check);
		return false;
	}
	// Populate Superblock Struct
	disk->info = _disk_info(disk);
	return true;
}
// Cleanup Disk Struct (Becomes Invalid)
bool disk_close(disk_s* disk) {
	// Validate Disk State
	const char* check;
	if (!is_valid(disk,check)) { 
		printf("Disk Struct Invalid: \n%s",check);
		return false;
	}
#ifndef NDEBUG
	printf("Copying State To Disk...");
#endif
	// Copy Data To File
	rewind(disk->file);
	if (fwrite((void*)disk->mem_start,sizeof(uint8_t),disk->mem_size,disk->file) < disk->mem_size) {
#ifndef NDEBUG
		printf("Failed\n");
#endif
		free(disk->mem_start);
		fclose(disk->file);
		return false;
	}
#ifndef NDEBUG
	printf("Success\n");
#endif  
	// Cleanup
	free(disk->mem_start);
	fclose(disk->file);
}
// Get Data Block From Memory
uint8_t* _disk_data_get(const disk_s* disk,const uint16_t bnum) {
	// Validate Inputs
	char* err_str;
	if (!is_valid(disk,err_str)) {
		printf("%s",err_str);
		return NULL;
	}
	if (bnum>=disk->info.data_size) {
		printf("Invalid Block Number\n");
		return NULL;
	}
	// Get Data
	return &disk->mem_start[BLOCK_SIZE*(disk->info.data_start+bnum)];
}
// Allocate Data Block
uint16_t _disk_data_alloc(const disk_s* disk) {
	// Validate Input
	char* err_str;
	if (!is_valid(disk,err_str)) {
		printf("%s",err_str);
		return disk->info.data_size;
	}
	// Query Data Bitmap For Free Block
	uint8_t* dmap=&disk->mem_start[BLOCK_SIZE*2];
	for (int bnum=0;bnum<disk->info.data_size;++bnum) {
		if (!(dmap[bnum/8]&(1<<(bnum%8)))) {
			dmap[bnum/8]|=(1<<(bnum%8));
			return bnum;
		}
	}
}
// Free Allocated Data Block
bool _disk_data_free(const disk_s* disk,const uint16_t bnum) {
	// Validate Input
	char* err_str;
	if (!is_valid(disk,err_str)) {
		printf("%s",err_str);
		return false;
	}
	if (bnum>=disk->info.data_size) {
		printf("Invalid Block Number\n");
		return false;
	}
	// Update Data Bitmap
	uint8_t* dmap=&disk->mem_start[BLOCK_SIZE*2];
	dmap[bnum/8]&=~(1<<(bnum%8));
	return true;
}

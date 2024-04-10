#include "disk.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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

bool open_disk(disk_s* disk, const char* file_name) {
	// Open Disk File
	disk->file = fopen(file_name,"r+");
	if (disk->file == NULL) {
		printf("Failed To Open %s",disk->file_name);
		return false;
	}
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
		if    (ferror(disk->file)) printf("Error Reading %s\n",disk->file_name);
		else if (feof(disk->file)) printf("Reached End Of File\n");
		free(disk->mem_start);
		fclose(disk->file);
		return false;
	}
#ifndef NDEBUG
	printf("Success.\n");
	printf("Allocated %ld Blocks Of %d Each\n",disk->mem_size/BLOCK_SIZE,BLOCK_SIZE);
#endif
	// Validate Disk State
	const char* check;
	if (!is_valid(disk,check)) {
		printf("Disk Struct Invalid: \n%s",check);
		return false;
	}
	return true;
}

bool close_disk(disk_s* disk) {
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
		printf("Failed\n");
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

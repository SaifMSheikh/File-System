#include "disk.h"
#include "inode.h"
#include "file.h"
#include <string.h>
// Test Utility Functions
bool test_reset(disk_s* disk) {
	printf("Freeing %ld Inodes:\n",IMAX(disk->info));
	for (int i=0;i<IMAX(disk->info);++i) {
		printf("Freeing Inode %d...",i);
		if (_disk_inode_free(disk,i))
			printf("Success\n");
		else printf("Failed\n");
	}
	printf("Freeing %d Data Blocks:\n",disk->info.data_size);
	for (int i=0;i<disk->info.data_size;++i) {
		printf("Freeing Data Block %d...",i);
		if (_disk_data_free(disk,i))
			printf("Success\n");
		else printf("Failed\n");
	}
	printf("Allocating Root Directory...");
	inode_s root=_inode_create(disk,I_DIRE);
	if (!root.valid) {
		printf("Failed\n");
		return false;
	}
	printf("Inode %u\n",root.inum);
	return true;
}

void test_print_imap_index(const disk_s* disk,const uint16_t inum) {
	// Validate Inputs
	char* err_str;
	if (!is_valid(disk,err_str)) {
		printf("%s",err_str); 
		return;
	}
	// Print Bitmap Index
	uint8_t* imap=&disk->mem_start[BLOCK_SIZE];
	printf("%d",(int)(imap[inum/8]&(1<<(inum%8))));
}

int main(const int argc, const char* argv[]) {
	// Validate Arguments
	if (argc != 2) {
		printf("Valid Usage : ./[executable name] [disk file name]\n");
		return 1;
	}
	// Setup
	disk_s disk;
	if (!disk_open(&disk,argv[1])) 
		return 1;
	// Testing
	if (!test_reset(&disk))
		return 1;
	printf("Bitmap Index 0: ");
	test_print_imap_index(&disk,0);
	printf("\n");
	inode_s root=_inode_get(&disk,0);
	printf("Creating File \"Saif\"...");
	uint16_t inum=file_create(&root,"Saif",I_FILE);
	file_s saif=file_open(&root,"Saif",FILE_READABLE_BIT|FILE_WRITABLE_BIT);
	if (!saif.valid) {
		printf("Failed\n");
		return 1;
	}
	printf("Allocated Node %u\n",inum);
	printf("ROOT DIRECTORY:");
	dir_print(&root);
	char* buffer="Hello, World!\n";
	file_write(&saif,(uint8_t*)buffer,strlen(buffer));
	char out[255];
	file_seek(&saif,0);
	file_read(&saif,(uint8_t*)out,15);
	printf("%s",out);
	printf("Closing File...");
	if (file_close(&saif))
		printf("Success\n");
	else printf("Failed\n");
	printf("Deleting File...");
	if (file_delete(&root,"Saif"))
		printf("Success\n");
	else printf("Failed\n");
	// Cleanup
	if (disk_close(&disk))
		return 1;

	return 0;
}

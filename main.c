#include "disk.h"
#include "inode.h"
#include "file.h"

void test_reset(disk_s* disk) {
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
	inode_s root=_inode_get(&disk,0);
	dir_print(&root,false);
//	
//	for (int i = 0; i < 10; ++i) {
//		printf("Allocating Inode...");
//		inode_s inode = _inode_create(&disk);
//		if (!inode.valid) printf("%hu\n",inode.inum);
//		else printf("Failed\n");
//	}
	// Cleanup
	if (disk_close(&disk))
		return 1;

	return 0;
}

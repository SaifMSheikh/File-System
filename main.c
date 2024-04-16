#include "disk.h"
#include "inode.h"
#include "file.h"

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
	return true;
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
	inode_s root=_inode_get(&disk,0);
	dir_create(&root,"Saif");
	dir_print(&root);
	dir_destroy(&root,"Saif");
	dir_print(&root);
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

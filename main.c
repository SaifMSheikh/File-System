#include "disk.h"
#include "inode.h"
#include "file.h"

void test_reset(disk_s* disk) {
	printf("Freeing %ld Inodes:\n",IMAX(disk->info));
	for (int i=0;i<IMAX(disk->info);++i) {
		printf("Freeing Inode %d...",i);
		if (_disk_inode_free(disk,i))
			printf("Success\n");
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
	//test_reset(&disk);
	printf("Creating Directory...");
	inode_s root=_inode_create(&disk);
	root.info->type=I_DIRE;
	root.info->size=0;
	uint16_t inum=dir_create(&root,"Saif");
	if (inum>=IMAX(disk.info))
		printf("Failed\n");
	else printf("Success\n");
//	printf("Creating Directory...");
//	inode_s root=_inode_get(&disk,2);
//	root.info->type=I_DIRE;
//	uint16_t dir_index=dir_create(&root,"Saif");
//	if (dir_index>=IMAX(disk.info))
//		printf("Failed\n");
//	else printf("Allocated Inode %u",dir_index);
//	for (int i = 0; i < IMAX(disk.info); ++i) {
//		printf("Freeing Inode %d...",i);
//		if (_disk_inode_free(&disk,i))
//			printf("Success\n");
//		else printf("Failed\n");
//	}
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

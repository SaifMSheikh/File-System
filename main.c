#include "disk.h"
#include "inode.h"

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
	for (int i = 0; i < IMAX(disk.info); ++i) {
		printf("Freeing Inode %d...",i);
		if (_disk_inode_free(&disk,i))
			printf("Success\n");
		else printf("Failed\n");
	}
//	for (int i = 0; i < 10; ++i) {
//		printf("Allocating Inode...");
//		uint16_t inum = _disk_inode_alloc(&disk);
//		if (inum!=IMAX(disk.info)) printf("%hu\n",inum);
//		else printf("Failed\n");
//	}
	// Cleanup
	if (disk_close(&disk))
		return 1;

	return 0;
}

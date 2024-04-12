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
	// Cleanup
	if (disk_close(&disk))
		return 1;

	return 0;
}

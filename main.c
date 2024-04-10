#include "disk.h"

int main(const int argc, const char* argv[]) {
	// Validate Arguments
	if (argc != 2) {
		printf("Valid Usage : ./[executable name] [disk file name]\n");
		return 1;
	}
	// Setup
	disk_s disk;
	if (!open_disk(&disk,argv[1])) 
		return 1;
	// Testing
	// Cleanup
	if (close_disk(&disk))
		return 1;

	return 0;
}

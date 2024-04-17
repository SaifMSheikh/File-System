#include "filesystem.hpp"
#include <iostream>

int main(int argc,char* argv[]) {
	// Validate Arguments
	if (argc!=2) {
		std::cout<<"Valid Usage : ./[Executable File] [Disk File]";
		return 1;
	}
	// Testing
	FileSystem fs(argv[1]);
	fs.ls_dir();
	fs.ls_dir();
	// End
	return 0;
}

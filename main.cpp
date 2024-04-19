#include "filesystem.hpp"
#include "shell.hpp"
#include <iostream>

int main(int argc,char* argv[]) {
	// Validate Arguments
	if (argc!=2) {
		std::cout<<"Valid Usage : ./[Executable File] [Disk File]";
		return 1;
	}
	
	// Testing
	// FileSystem fs(argv[1]);
	// fs.ls_dir();
	// fs.ls_dir();

	// Shell Testing
	Shell shell(argv[1]);
	shell.Run();

	// End
	return 0;
}

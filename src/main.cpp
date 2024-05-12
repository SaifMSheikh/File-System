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
//	fs.create_file("a");
//	auto a=fs.open("a",FILE_READABLE_BIT|FILE_WRITABLE_BIT);
//	a.write("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
	fs.ls_dir();
	// End
	return 0;
}

#include "filesystem.hpp"
// disk_s RAII
FileSystem::FileSystem(const std::string& disk_file_name) {
	// Setup disk_s struct
	if (!disk_open(&m_disk,(const char*)disk_file_name.c_str())) 
		delete this;
}
FileSystem::~FileSystem() {
	// Save disk_s To File
	disk_close(&m_disk);
}

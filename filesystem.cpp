#include "filesystem.hpp"
// disk_s RAII
FileSystem::FileSystem(const std::string& disk_file_name) {
	// Setup disk_s struct
	if (!disk_open(&m_disk,(const char*)disk_file_name.c_str())) 
		delete this;
	// Get Root Directory
	uint8_t* imap=&m_disk.mem_start[BLOCK_SIZE];
	for (int i=0;i<IMAX(m_disk.info);++i) {
		if (imap[i/8]&(1<<(i%8))) {
			m_dir=_inode_get(&m_disk,i);
			return;
		}
	}
	// If Not Found, Allocate New Root Directory
	printf("Failed To Find Root Directory\nAllocating New Root...");
	m_dir=_inode_create(&m_disk,I_DIRE);
}
FileSystem::~FileSystem() {
	// Save disk_s To File
	disk_close(&m_disk);
}
// Directory Operations
void FileSystem::ls_dir() 
{ dir_print(&m_dir); }
void FileSystem::mk_dir(const std::string& path) 
{ file_create(&m_dir,path.c_str(),I_DIRE); }
void FileSystem::ch_dir(const std::string& path) { 
	inode_s dir=_inode_get(m_dir.dev,dir_lookup(&m_dir,path.c_str()));
	if (dir.valid)
		m_dir=dir;
}
// File Operations
void FileSystem::delete_file(const std::string& path) 
{ file_delete(&m_dir,path.c_str()); }

#include "filesystem.hpp"
#include <cstring>
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
void FileSystem::create_file(const std::string& path) 
{ file_create(&m_dir,path.c_str(),I_FILE); }
void FileSystem::delete_file(const std::string& path) 
{ file_delete(&m_dir,path.c_str()); }
void FileSystem::move_file(const std::string& src_path,const std::string& dst_path) { 
	// Get Source File Handle
	if (dir_lookup(&m_dir,src_path.c_str())>=IMAX(m_disk.info)) {
		printf("Cannot Find Source File\n");
		return;
	}
	file_s src=file_open(&m_dir,src_path.c_str(),FILE_READABLE_BIT|FILE_WRITABLE_BIT);
	// Update Destination Parent Directory
	{	// Destination
		char parent_path[255]="";
		char target_name[255]="";
		for (int i=dst_path.length();i>=0;--i) {
			if (dst_path[i]=='/') {
				strncpy(parent_path,dst_path.c_str(),i);
				strncpy(target_name,dst_path.c_str()+i+1,dst_path.length()-i+1);
				break;
			}
		}
		printf("Destination : %s | %s\n",parent_path,target_name);
		inode_s parent_dir;
		inode_s old_entry;
		if (!strcmp(parent_path,"")) {
			parent_dir=m_dir;
			strncpy(target_name,dst_path.c_str(),dst_path.length());;
		}
		else parent_dir=_inode_get(&m_disk,dir_lookup(&m_dir,parent_path));
		dirent_s* entry=(dirent_s*)_disk_data_get(&m_disk,parent_dir.info->addr[0]);
		for (int i=0;i<parent_dir.info->size;++i) {
			if (!strcmp(entry[i].name,target_name)) {
				inode_s old_node=_inode_get(&m_disk,entry[i].inum);
				_inode_destroy(&old_node);
				entry[i].inum=src.node.inum;
				strncpy(entry[i].name,target_name,DIRENT_NAME_LEN);
				goto source_delete;
			}
		}
		// If Not Found, Add New Entry
		entry[parent_dir.info->size].inum=src.node.inum;
		strncpy(entry[parent_dir.info->size].name,target_name,DIRENT_NAME_LEN);
		parent_dir.info->size++;
	}
	{	// Source
		source_delete:
		char parent_path[255]="";
		char target_name[255]="";
		for (int i=src_path.length();i>=0;--i) {
			if (src_path[i]=='/') {
				strncpy(parent_path,src_path.c_str(),i);
				strncpy(target_name,src_path.c_str()+i+1,src_path.length()-i+1);
				break;
			}
		}
		printf("Source : %s | %s\n",parent_path,target_name);
		inode_s parent_dir;
		if (!strcmp(parent_path,"")) {
			parent_dir=m_dir;
			strncpy(target_name,src_path.c_str(),src_path.length());
		}
		else parent_dir=_inode_get(&m_disk,dir_lookup(&m_dir,parent_path));
		dirent_s* entry=(dirent_s*)_disk_data_get(&m_disk,parent_dir.info->addr[0]);
		for (int i=0;i<parent_dir.info->size;++i) {
			if (entry[i].inum==src.node.inum) {
				for (int j=i+1;j<parent_dir.info->size;++j)
					memmove(&entry[i],&entry[j],sizeof(dirent_s));
				parent_dir.info->size--;
				break;
			}
		}
	}
	file_close(&src);
}
// File Handle Interface
File FileSystem::open(const std::string& path,const uint8_t& mode) 
{ return File(file_open(&m_dir,path.c_str(),mode)); }
File::File(file_s file) : m_file(file) { end=0; }
File::~File() { file_close(&m_file); }
void File::write(const std::string& buffer) { 
	// Write To End
	uint32_t temp_iter=file_tell(&m_file);
	file_seek(&m_file,end);
	end=file_write(&m_file,(uint8_t*)buffer.c_str(),buffer.length()); 
	file_seek(&m_file,temp_iter);
}
void File::write(const std::string& buffer,const uint8_t& position) {
	// Write To Specific Position
	file_seek(&m_file,position);
	file_write(&m_file,(uint8_t*)buffer.c_str(),buffer.length());
	if (m_file.iter>end)
		end=file_tell(&m_file);
}
std::string File::read() {
	// Read All Contents
	uint32_t temp_iter=file_tell(&m_file);
	int size=0;
	while (read(size,1)!="\0") { size++; }
	file_seek(&m_file,0);
	std::string buffer=read(0,size);
	file_seek(&m_file,temp_iter);
	return buffer;
}
std::string File::read(const uint8_t& start,const uint8_t& size) {
	// Read From Offset
	file_seek(&m_file,start);
	uint8_t buffer[size];
	file_read(&m_file,buffer,size);
	return std::string((char*)buffer);
}

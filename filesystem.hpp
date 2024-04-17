#include "disk.h"
#include "inode.h"
#include "file.h"

#include <string>

class File;

class FileSystem {
	disk_s  m_disk;
	inode_s m_dir;
public: // Disk RAII
	FileSystem(const std::string& disk_file_name="disk.dat");
       ~FileSystem();
public:	// Target Interface
// File Interface
//	File open(const std::string&);
//	void close(const std::string&);
	void create_file(const std::string&);
	void delete_file(const std::string&);
	void move_file(const std::string&,const std::string&);
// Directory Interface
	void ls_dir();
	void mk_dir(const std::string&);
	void ch_dir(const std::string&);
};

//class File{
//	file_s m_file;
//public: // File RAII
//	File(file_s);
//       ~File();
//public: // Read / Write Interface
//	void write(const std::string&);
//	void write(const std::string&,const size_t&);
//	std::string read(const size_t&,const size_t&);
//};

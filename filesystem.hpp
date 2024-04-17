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
//	bool create_file(const std::string&);
//	bool delete_file(const std::string&);
//	bool move(const std::string&,const std::string&);
//	bool mk_dir(const std::string&);
//	bool ch_dir(const std::string&);
//	File open(const std::string&);
//	bool close(const std::string&);
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

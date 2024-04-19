#include "filesystem.hpp"

class Shell{
private:
    FileSystem fs;
public:
    Shell(char* disk_file_name);
    void Run();
};
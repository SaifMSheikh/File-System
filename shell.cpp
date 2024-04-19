#include "shell.hpp"
#include <iostream>
#include <string>
#include <sstream>

Shell::Shell(char *disk_file_name) : fs(disk_file_name) {}

void Shell::Run()
{
    std::string command;
    std::cout << "\n\nWelcome to the File System! Type 'help' for a list of commands.\n";

    while (true)
    {
        std::cout << "\n$ ";
        std::getline(std::cin, command);

        if (command == "help")
        {
            std::cout << "Available commands:\n"
                         "  mkdir <directory_name>\tCreate a new directory\n"
                         "  cd <directory_name>\t\tChange current directory\n"
                         "  ls \t\t\tList contents of current directory\n"
                         "  create <file_name>\t\tCreate a new file\n"
                         "  rm <file_or_directory_name>\t\tDelete a file or directory\n"
                         "  write <flag> [optional position] <file_name> <text to write>\tWrite text to a file\n"
                         "  read <file_name> [start] [size]\tRead text from a file\n"
                         "  mv <source> <destination>\tMove a file or directory\n"
                         "  command <command>\t\t\t Utility to run a command on the underlying shell"
                         "  exit\t\t\t\tExit the program\n";
        }
        else if (command == "exit")
        {
            break;
        }
        else
        {
            std::istringstream iss(command);
            std::string cmd;
            iss >> cmd;

            if (cmd == "mkdir")
            {
                std::string dirName;
                iss >> dirName;
                fs.mk_dir(dirName);
            }
            if (cmd == "command")
            {
                std::string cmd;
                std::getline(iss, cmd);
                system(cmd.c_str());
            }
            else if (cmd == "cd")
            {
                std::string dirName;
                iss >> dirName;
                fs.ch_dir(dirName);
            }
            else if (cmd == "ls")
            {
                fs.ls_dir();
            }
            else if (cmd == "create")
            {
                std::string fileName;
                iss >> fileName;
                fs.create_file(fileName);
            }
            else if (cmd == "rm")
            {
                std::string name;
                iss >> name;
                fs.delete_file(name);
            }
            else if (cmd == "write")
            {
                std::string fileName, text, flag;
                int pos = -1;
                iss >> flag;

                if (flag != "-a" && flag != "-t" && flag != "-p")
                {
                    std::cout << "Invalid flag.\n";
                    continue;
                }

                if (flag == "-p")
                {
                    iss >> pos;
                }

                iss >> fileName;
                std::getline(iss, text);
                if (text.empty())
                {
                    std::cout << "Invalid command. Usage: write <flag> [optional position] <file_name> <text to write>\n";
                    continue;
                }

                File file = fs.open(fileName, FILE_WRITABLE_BIT | FILE_READABLE_BIT);
                std::cout << "DEBUG: File name \"" << fileName << "\"" << std::endl;
                if (flag == "-t")
                {
                    if (file.is_valid())
                    {
                        auto text = file.read();
                        fs.delete_file(fileName);
                        fs.create_file(fileName);
                        std::cout << text << std::endl;
                        file.write(text);
                    }
                }
                else if (flag == "-p")
                    file.write(text, pos);
                else // append
                {
                    std::string data(file.read());
                    data.append(text);
                    file.write(data);
                }
            }
            else if (cmd == "read")
            {
                std::string fileName;
                int start = 0, size = -1;
                iss >> fileName >> start >> size;
                File file = fs.open(fileName, FILE_READABLE_BIT);
                if (!file.is_valid())
                    continue;
                if (size == -1)
                {
                    std::cout << file.read();
                }
                else
                {
                    std::cout << file.read(start, size);
                }
            }
            else if (cmd == "mv")
            {
                std::string source, destination;
                iss >> source >> destination;
                fs.move_file(source, destination);
            }
            else
            {
                std::cout << "Unknown command. Type 'help' for a list of commands.\n";
            }
        }
    }
}
#include <iostream>
#include <cstdio>
#include <string>
#include "filesystem.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <fileSystem.data> <operation> [parameters]" << std::endl;
        return 1;
    }

    std::string file_system_name = argv[1];
    std::string operation = argv[2];

    FileSystem fs(file_system_name);

    if (operation == "dir") {
        if (argc != 4) {
            std::cerr << "Usage: " << argv[0] << " <fileSystem.data> dir <path>" << std::endl;
            return 1;
        }
        std::string path = argv[3];
        fs.dir(argv[3]);
    } else if (operation == "mkdir") {
        if (argc != 4) {
            std::cerr << "Usage: " << argv[0] << " <fileSystem.data> mkdir <path>" << std::endl;
            return 1;
        }
        fs.mkdir(argv[3]);
    } else if (operation == "rmdir") {
        if (argc != 4) {
            std::cerr << "Usage: " << argv[0] << " <fileSystem.data> rmdir <path>" << std::endl;
            return 1;
        }
        fs.rmdir(argv[3]);
    } else if (operation == "dumpe2fs") {
        if (argc != 3) {
            std::cerr << "Usage: " << argv[0] << " <fileSystem.data> dumpe2fs" << std::endl;
            return 1;
        }
        fs.dumpe2fs();
    } else if (operation == "write") {
        if (argc != 5) {
            std::cerr << "Usage: " << argv[0] << " <fileSystem.data> write <path> <linux_file>" << std::endl;
            return 1;
        }
        fs.write(argv[3], argv[4]);
    } else if (operation == "read") {
        if (argc != 5) {
            std::cerr << "Usage: " << argv[0] << " <fileSystem.data> read <path> <linux_file>" << std::endl;
            return 1;
        }
        fs.read(argv[3], argv[4]);
    } else if (operation == "del") {
        if (argc != 4) {
            std::cerr << "Usage: " << argv[0] << " <fileSystem.data> del <path>" << std::endl;
            return 1;
        }
        fs.del(argv[3]);
    } else if (operation == "chmod") {
        if (argc != 5) {
            std::cerr << "Usage: " << argv[0] << " <fileSystem.data> chmod <path> <permissions>" << std::endl;
            return 1;
        }
        fs.fs_chmod(argv[3], argv[4]);
    } else if (operation == "addpw") {
        if (argc != 5) {
            std::cerr << "Usage: " << argv[0] << " <fileSystem.data> addpw <path> <password>" << std::endl;
            return 1;
        }
        fs.addpw(argv[3],argv[4]);
    }


    fs.save_filesystem(file_system_name);

}
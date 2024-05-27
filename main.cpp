#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include <cstring>
#include "filesystem.h"

using namespace std;

/*
    Main Function that creats the file system.
*/
int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <block size in KB> <file system name>" << std::endl;
        return 1;
    }

    double block_size_kb = std::stod(argv[1]);
    std::string file_system_name = argv[2];

    if (block_size_kb != 0.5 && block_size_kb != 1.0) {
        std::cerr << "Block size must be either 0.5 KB or 1 KB." << std::endl;
        return 1;
    }

    uint32_t block_size = static_cast<uint32_t>(block_size_kb * 1024);
    uint32_t total_blocks;
    uint32_t max_file_system_size;

    if (block_size_kb == 0.5) {
        max_file_system_size = MAX_FILE_SYSTEM_SIZE_512;
        total_blocks = max_file_system_size / block_size;
    } else {
        max_file_system_size = MAX_FILE_SYSTEM_SIZE_1024;
        total_blocks = max_file_system_size / block_size;
    }

    FileSystem fs(total_blocks, block_size);
    fs.save_filesystem(file_system_name);

    std::cout << "File system created successfully: " << file_system_name << std::endl;

    return 0;
}
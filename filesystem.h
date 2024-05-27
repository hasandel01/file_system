#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <ctime>
#include <vector>
#include "directoryentry.h"
#include <iostream>

struct Superblock {
    uint32_t total_blocks;
    uint32_t fat_start;
    uint32_t root_dir_start;
    uint32_t block_size;
};


struct DiskBlock {
    uint32_t block_number;           // Block number for identification
    std::vector<char> data;          // Data stored in this block
};

class FileSystem {

    private:
        Superblock superblock;
        std::vector<uint16_t> fat;
        std::vector<DiskBlock> blocks;
        void load_filesystem(const std::string& filename);
        DirectoryEntry root_directory;
        void write_directory(std::ofstream& ofs, const DirectoryEntry& directory);
        void read_directory(std::ifstream& ifs, DirectoryEntry& directory);

    public:

        FileSystem(uint32_t total_blocks, uint32_t block_size);
        FileSystem(const std::string& file_name);

        void save_filesystem(const std::string& filename);

        void dir(const std::string& path);
        void ls_directory(const DirectoryEntry& entry);
        DirectoryEntry* findDirectory(const std::string& path);
        bool is_directory(const DirectoryEntry& entry);
        void allocateBlocksForFile(DirectoryEntry& entry, uint32_t file_size);
        void deallocateBlocksForFile(const DirectoryEntry& entry);
        uint16_t findNextFreeBlock();
        void calculateDirectorySize(DirectoryEntry& directory);
        void setPermissionsFromLinuxFile(DirectoryEntry& entry, const std::string& linux_file);

        void apply_file_metadata(const std::string& path, const DirectoryEntry& entry);
        void set_file_metadata(const std::string& path, DirectoryEntry& entry);
        void mkdir(const std::string& path);
        void rmdir(const std::string& path);
        void dumpe2fs();
        uint32_t countFiles(const DirectoryEntry& directory);
        uint32_t countDirectories(const DirectoryEntry& directory);
        void listOccupiedBlocks(const DirectoryEntry& directory);
        void write(const std::string& path, const std::string& linux_file);
        void read(const std::string& path, const std::string& linux_file);
        void del(const std::string& path);;
        void fs_chmod(const std::string& path, const std::string& permissions);
        void addpw(const std::string& path, const std::string& password);
        bool checkPassword(const DirectoryEntry& entry);

};

#endif
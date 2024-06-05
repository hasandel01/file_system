#ifndef DIRECTORYENTRY_H
#define DIRECTORYENTRY_H

#include <ctime>
#include <string>
#include <vector>
#include <cstring>

const int MAX_FILE_SYSTEM_SIZE_512 = 2 * 1024 * 1024; // 2 MB for 0.5 KB blocks (Figure 4.1)
const int MAX_FILE_SYSTEM_SIZE_1024 = 4 * 1024 * 1024; // 4 MB for 1 KB blocks (Figure 4.1)
const int MAX_FILENAME_LENGTH = 255;
const uint16_t FAT_FREE = 0xFFFF; // Representing a free block in the FAT
const uint16_t FAT_USED = 0xFFFE; // Representing a used block in the FAT
const uint16_t FAT_EOC = 0xFFFD;  // End of Chain marker
const uint8_t ATTR_DIRECTORY = 0x10; // 0b00010000


struct Permissions {
    bool read;
    bool write;
};


class DirectoryEntry {

    private: 
        std::string filename;
        uint32_t size; // File size in bytes
        Permissions permissions;
        std::time_t creation_time;
        std::time_t modification_time;
        std::string password; // Password for file protection, if any
        uint16_t start_block; // Start block in FAT
        uint8_t attribute;

    public:

        std::vector<DirectoryEntry> children; // Only used if it's a directory

        DirectoryEntry(int size = 0) {
            this->size = size;
            permissions = {true, true};
            creation_time = std::time(nullptr);
            modification_time = std::time(nullptr);
            start_block = 0;
            attribute = 0;
        }

        std::string getFilename() const { return filename; }
        void setFilename(const std::string& new_filename) { filename = new_filename; }

        uint32_t getSize() const { return size; }
        void setSize(uint32_t new_size) { size = new_size; }

        Permissions getPermissions() const { return permissions; }
        void setPermissions(Permissions new_permissions) { permissions = new_permissions; }

        std::time_t getCreationTime() const { return creation_time; }
        void setCreationTime(std::time_t new_creation_time) { creation_time = new_creation_time; }

        std::time_t getModificationTime() const { return modification_time; }
        void setModificationTime(std::time_t new_modification_time) { modification_time = new_modification_time; }

        std::string getPassword() const { return password; }
        void setPassword(const std::string& new_password) { password = new_password; }

        uint16_t getStartBlock() const { return start_block; }
        void setStartBlock(uint16_t new_start_block) { start_block = new_start_block; }

        uint16_t getAttribute() const { return attribute; }
        void  setAttribute(uint16_t attribute) { this->attribute  = attribute; }

};


#endif
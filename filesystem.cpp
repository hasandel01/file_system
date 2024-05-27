#include "filesystem.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <sys/stat.h> // For file permissions
#include "utility.h"
#include <fcntl.h>
#include <utime.h>

FileSystem::FileSystem(uint32_t total_blocks, uint32_t block_size) {
    superblock.total_blocks = total_blocks;
    superblock.block_size = block_size;
    superblock.fat_start = sizeof(Superblock);
    superblock.root_dir_start = superblock.fat_start + (total_blocks * sizeof(uint16_t));
    fat.resize(total_blocks, 0);
    std::fill(fat.begin(), fat.end(), FAT_FREE);

    // Resize the vector of blocks to hold 'total_blocks' DiskBlock objects
    blocks.resize(total_blocks);

    // Initialize each DiskBlock with its block number and empty data space
    for (uint32_t i = 0; i < total_blocks; ++i) {
        blocks[i].block_number = i;
        // Initialize data vector with empty space (1KB in this case)
        blocks[i].data.resize(superblock.block_size, '\0');
    }

    // Initialize root directory
    root_directory.setFilename("/");
    root_directory.setSize(0);
    root_directory.setPermissions({true, true});
    root_directory.setCreationTime(std::time(nullptr));
    root_directory.setModificationTime(std::time(nullptr));
    root_directory.setStartBlock(superblock.root_dir_start);
    root_directory.setAttribute(ATTR_DIRECTORY);

}

FileSystem::FileSystem(const std::string& file_name) {
    load_filesystem(file_name);
}

void FileSystem::save_filesystem(const std::string& filename) {

    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs.is_open()) {
        throw std::runtime_error("Failed to open file for saving filesystem");
    }

    // Save the superblock
    ofs.write(reinterpret_cast<const char*>(&superblock), sizeof(superblock));

    // Save the FAT
    uint32_t fat_size = fat.size();
    ofs.write(reinterpret_cast<const char*>(&fat_size), sizeof(fat_size));
    ofs.write(reinterpret_cast<const char*>(fat.data()), fat_size * sizeof(uint16_t));

    // Save the root directory and its children recursively
    write_directory(ofs, root_directory);

    // Save disk blocks
    for (const DiskBlock& block : blocks) {
        ofs.write(reinterpret_cast<const char*>(&block), sizeof(DiskBlock));
        // Write block data
        ofs.write(block.data.data(), superblock.block_size);
    }

    ofs.close();
}

void FileSystem::write_directory(std::ofstream& ofs, const DirectoryEntry& directory) {
    // Save filename length and content
    uint32_t filename_length = directory.getFilename().size();
    ofs.write(reinterpret_cast<const char*>(&filename_length), sizeof(filename_length));
    ofs.write(directory.getFilename().c_str(), filename_length);

    // Save size
    uint32_t size = directory.getSize();
    ofs.write(reinterpret_cast<const char*>(&size), sizeof(size));

    // Save permissions
    Permissions permissions = directory.getPermissions();
    ofs.write(reinterpret_cast<const char*>(&permissions), sizeof(permissions));

    // Save creation time
    std::time_t creation_time = directory.getCreationTime();
    ofs.write(reinterpret_cast<const char*>(&creation_time), sizeof(creation_time));

    // Save modification time
    std::time_t modification_time = directory.getModificationTime();
    ofs.write(reinterpret_cast<const char*>(&modification_time), sizeof(modification_time));

    // Save password length and content
    uint32_t password_length = directory.getPassword().size();
    ofs.write(reinterpret_cast<const char*>(&password_length), sizeof(password_length));
    ofs.write(directory.getPassword().c_str(), password_length);

    // Save start block
    uint16_t start_block = directory.getStartBlock();
    ofs.write(reinterpret_cast<const char*>(&start_block), sizeof(start_block));

    // Save attribute
    uint8_t attribute = directory.getAttribute();
    ofs.write(reinterpret_cast<const char*>(&attribute), sizeof(attribute));

    // Save the number of children
    uint32_t num_children = directory.children.size();
    ofs.write(reinterpret_cast<const char*>(&num_children), sizeof(num_children));

    // Recursively save each child
    for (const auto& child : directory.children) {
        write_directory(ofs, child);
    }
}


void FileSystem::load_filesystem(const std::string& filename) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs.is_open()) {
        throw std::runtime_error("Failed to open file for loading filesystem");
    }

    // Load the superblock
    ifs.read(reinterpret_cast<char*>(&superblock), sizeof(superblock));

    // Load the FAT
    uint32_t fat_size;
    ifs.read(reinterpret_cast<char*>(&fat_size), sizeof(fat_size));
    fat.resize(fat_size);
    ifs.read(reinterpret_cast<char*>(fat.data()), fat_size * sizeof(uint16_t));

    // Load the root directory and its children recursively
    read_directory(ifs, root_directory);

    // Load disk blocks
    blocks.resize(superblock.total_blocks);
    for (size_t i = 0; i < superblock.total_blocks; ++i) {
        ifs.read(reinterpret_cast<char*>(&blocks[i].block_number), sizeof(uint32_t));
        // Read block data
        blocks[i].data.resize(superblock.block_size);
        ifs.read(blocks[i].data.data(), superblock.block_size);
    }

    ifs.close();
}


void FileSystem::read_directory(std::ifstream& ifs, DirectoryEntry& directory) {
    // Load filename length and content
    uint32_t filename_length;
    ifs.read(reinterpret_cast<char*>(&filename_length), sizeof(filename_length));
    std::string filename(filename_length, '\0');
    ifs.read(&filename[0], filename_length);
    directory.setFilename(filename);

    // Load size
    uint32_t size;
    ifs.read(reinterpret_cast<char*>(&size), sizeof(size));
    directory.setSize(size);

    // Load permissions
    Permissions permissions;
    ifs.read(reinterpret_cast<char*>(&permissions), sizeof(permissions));
    directory.setPermissions(permissions);

    // Load creation time
    std::time_t creation_time;
    ifs.read(reinterpret_cast<char*>(&creation_time), sizeof(creation_time));
    directory.setCreationTime(creation_time);

    // Load modification time
    std::time_t modification_time;
    ifs.read(reinterpret_cast<char*>(&modification_time), sizeof(modification_time));
    directory.setModificationTime(modification_time);

    // Load password length and content
    uint32_t password_length;
    ifs.read(reinterpret_cast<char*>(&password_length), sizeof(password_length));
    std::string password(password_length, '\0');
    ifs.read(&password[0], password_length);
    directory.setPassword(password);

    // Load start block
    uint16_t start_block;
    ifs.read(reinterpret_cast<char*>(&start_block), sizeof(start_block));
    directory.setStartBlock(start_block);

    // Load attribute
    uint8_t attribute;
    ifs.read(reinterpret_cast<char*>(&attribute), sizeof(attribute));
    directory.setAttribute(attribute);

    // Load the number of children
    uint32_t num_children;
    ifs.read(reinterpret_cast<char*>(&num_children), sizeof(num_children));
    
    // Recursively load each child
    directory.children.resize(num_children);
    for (auto& child : directory.children) {
        read_directory(ifs, child);
    }
}

/*
OPERATIONS
*/

void FileSystem::dir(const std::string& path) {
    
    // Check if path is the root directory
    if (path == "/") {
        std::cout << "Directory listing for root directory:" << std::endl;
        ls_directory(root_directory);
        return;
    }

    // Find the directory entry corresponding to the specified path
    DirectoryEntry* directory = findDirectory(path);
    if (directory == nullptr) {
        std::cerr << "Directory not found: " << path << std::endl;
        return;
    }

    // Check if the directory entry is a directory
    if (is_directory(*directory)) {
        std::cout << "Directory listing for " << path << ":" << std::endl;
        ls_directory(*directory);
    } else {
        std::cerr << "Not a directory: " << path << std::endl;
    }
}

// Assume entry is a DirectoryEntry structure
bool FileSystem::is_directory(const DirectoryEntry& entry) {
    // Check if the directory attribute bit is set
    return (entry.getAttribute() & ATTR_DIRECTORY);
}



void FileSystem::ls_directory(const DirectoryEntry& directory) {
    // Output directory header
    std::cout << std::left << std::setw(20) << "Name";
    std::cout << std::setw(10) << "Size";
    std::cout << std::setw(10) << "Perm";
    std::cout << std::setw(30) << "Creation Time";
    std::cout << std::setw(30) << "Mod Time";
    std::cout << std::endl;

    std::cout << directory.children.size() << std::endl;

    // Output directory contents
    for (const auto& entry : directory.children) {
        std::cout << std::left << std::setw(20) << entry.getFilename();
        std::cout << std::setw(10) << entry.getSize();
        std::cout << std::setw(1) << ((entry.getAttribute() == ATTR_DIRECTORY) ? "D" : "-");
        std::cout << std::setw(1) << (entry.getPermissions().read ? "R" : "-");
        std::cout << std::setw(9) << (entry.getPermissions().write ? "W" : "-");

        std::time_t creation_time = entry.getCreationTime();
        std::string creation_time_str = std::ctime(&creation_time); 
        creation_time_str = creation_time_str.substr(0, creation_time_str.length() - 1); // Remove newline character

        std::time_t modification_time = entry.getModificationTime();
        std::string modification_time_str = std::ctime(&modification_time); 
        modification_time_str = modification_time_str.substr(0, modification_time_str.length() - 1); // Remove newline character
        
        std::cout << std::setw(30) << creation_time_str;
        std::cout << std::setw(30) << modification_time_str;
        std::cout << std::endl;
    }
}


DirectoryEntry* FileSystem::findDirectory(const std::string& path) {
    
    DirectoryEntry* current_directory = &root_directory;

    // Split the path into components
    std::stringstream ss(path);
    std::string component;
    while (std::getline(ss, component, '/')) {
        if (component.empty()) continue;

        bool found = false;
        for (DirectoryEntry& entry : current_directory->children) {
            if (entry.getFilename() == component && (entry.getAttribute() & ATTR_DIRECTORY)) {
                current_directory = &entry;
                found = true;
                break;
            }
        }

        // If the directory doesn't exist, return nullptr
        if (!found) {
            return nullptr;
        }
    }

    return current_directory;
}



void FileSystem::mkdir(const std::string& path) {
    // Parse the input path to extract the directory path and new directory name
    std::string directory_path = extract_directory_path(path);
    std::string dir_name = extract_filename(path);

    // Find or create the parent directory
    DirectoryEntry* parent_directory = findDirectory(directory_path);
    if (parent_directory == nullptr) {
        std::cerr << "Failed to find the parent directory: " << directory_path << std::endl;
        return;
    }


    // Check if the directory already exists in the parent directory
    for (const DirectoryEntry& entry : parent_directory->children) {
        if (entry.getFilename() == dir_name) {
            std::cerr << "Directory already exists: " << path << std::endl;
            return;
        }
    }

    // Create a new directory entry for the new directory
    DirectoryEntry new_directory;
    new_directory.setFilename(dir_name);
    new_directory.setPermissions({true, true}); // Default permissions: read/write
    new_directory.setCreationTime(std::time(nullptr));
    new_directory.setModificationTime(std::time(nullptr));
    new_directory.setAttribute(ATTR_DIRECTORY);

    // Directories don't need data blocks, so we don't allocate blocks for them
    new_directory.setSize(0);
    new_directory.setStartBlock(FAT_EOC);

    // Add the new directory entry to the parent directory's children
    parent_directory->children.push_back(new_directory); // Move new_directory into the vector
}


void FileSystem::rmdir(const std::string& path) {
    // Find the parent directory of the directory to be removed
    std::string parentPath = extract_directory_path(path);
    std::string dirName = extract_filename(path);
    DirectoryEntry* parentDirectory = findDirectory(parentPath);
    if (parentDirectory == nullptr) {
        std::cerr << "Parent directory not found: " << parentPath << std::endl;
        return;
    }

    // Find and remove the directory from the parent directory's children
    auto it = parentDirectory->children.begin();
    while (it != parentDirectory->children.end()) {
        if (it->getFilename() == dirName) {
            // Check if the entry is a directory
            if (it->getAttribute() & ATTR_DIRECTORY) {
                parentDirectory->children.erase(it);
                std::cout << "Directory removed: " << path << std::endl;
                return;
            } else {
                std::cerr << "Error: The specified path points to a file, not a directory." << std::endl;
                return;
            }
        }
        ++it;
    }

    // If directory not found
    std::cerr << "Directory not found: " << path << std::endl;
}


void FileSystem::dumpe2fs() {
    // Print basic filesystem information
    std::cout << "Filesystem Information:" << std::endl;
    std::cout << "Block Count: " << superblock.total_blocks << std::endl;
    std::cout << "Block Size: " << superblock.block_size << " bytes" << std::endl;

    // Count free blocks
    uint32_t free_blocks = 0;
    for (uint16_t block : fat) {
        if (block == FAT_FREE) {
            free_blocks++;
        }
    }
    std::cout << "Free Blocks: " << free_blocks << std::endl;

    // Count number of files and directories
    uint32_t num_files = countFiles(root_directory);
    uint32_t num_directories = countDirectories(root_directory);
    std::cout << "Number of Files: " << num_files << std::endl;
    std::cout << "Number of Directories: " << num_directories << std::endl;

    // List occupied blocks and corresponding filenames
    std::cout << "Occupied Blocks:" << std::endl;
    listOccupiedBlocks(root_directory);
}

// Helper function to count files recursively
uint32_t FileSystem::countFiles(const DirectoryEntry& directory) {
    uint32_t count = 0;
    for (const auto& entry : directory.children) {
        if (!is_directory(entry)) {
            count++;
        } else {
            count += countFiles(entry);
        }
    }
    return count;
}

// Helper function to count directories recursively
uint32_t FileSystem::countDirectories(const DirectoryEntry& directory) {
    uint32_t count = 1; // Count the current directory itself
    for (const auto& entry : directory.children) {
        if (is_directory(entry)) {
            count += countDirectories(entry);
        }
    }
    return count;
}

// Helper function to list occupied blocks and corresponding filenames recursively
void FileSystem::listOccupiedBlocks(const DirectoryEntry& directory) {
    for (const auto& entry : directory.children) {
        if (!is_directory(entry)) {
            std::cout << "Block: " << entry.getStartBlock() << ", Filename: " << entry.getFilename() << std::endl;
        } else {
            listOccupiedBlocks(entry);
        }
    }
}

void FileSystem::allocateBlocksForFile(DirectoryEntry& entry, uint32_t file_size) {
    // Calculate the number of blocks needed for the file
    uint32_t num_blocks_needed = (file_size + superblock.block_size - 1) / superblock.block_size;

    // Find the next free block and assign it as the start block
    uint16_t start_block = findNextFreeBlock();
    if (start_block == FAT_FREE) {
        std::cerr << "Error: Insufficient free blocks to allocate for file." << std::endl;
        return;
    }
    entry.setStartBlock(start_block);

    // Allocate the remaining blocks for the file
    uint16_t prev_block = start_block;
    for (uint32_t i = 1; i < num_blocks_needed; ++i) {  // Change <= to <
        uint16_t free_block = findNextFreeBlock();
        if (free_block == FAT_FREE) {
            std::cerr << "Error: Insufficient free blocks to allocate for file." << std::endl;
            return;
        }
        fat[prev_block] = free_block;
        prev_block = free_block;
    }

    // Mark the last block as the end of the chain
    fat[prev_block] = FAT_EOC;

    /*
    // Debug output for FAT entries related to this file
    uint16_t current_block = start_block;
    while (current_block != FAT_EOC && current_block != FAT_FREE) {
        std::cout << "Block " << current_block << " -> " << fat[current_block] << std::endl;
        current_block = fat[current_block];
    }
    */

}

uint16_t FileSystem::findNextFreeBlock() {
    for (uint16_t i = 1; i < fat.size(); ++i) { // Start from 1 to avoid using block 0
        if (fat[i] == FAT_FREE) {
            fat[i] = FAT_USED; // Mark the block as used
            return i;
        }
    }
    return FAT_FREE; // Indicate no free block found
}


void FileSystem::deallocateBlocksForFile(const DirectoryEntry& entry) {
    uint16_t block = entry.getStartBlock();
    while (block != FAT_EOC && block != 0) {
        uint16_t next_block = fat[block];
        // Optionally clear the block data (to prevent residual data issues)
        // Overwrite the block data with null characters
        std::fill(blocks[block].data.begin(), blocks[block].data.end(), '\0');
        fat[block] = FAT_FREE; // Mark the block as free
        block = next_block;
    }
}

void FileSystem::calculateDirectorySize(DirectoryEntry& directory) {
    uint32_t totalSize = 0;

    for (const auto& entry : directory.children) {
        if (!is_directory(entry)) 
            // Add the size of the file
            totalSize += entry.getSize();
        }

    directory.setSize(totalSize);
}


void FileSystem::write(const std::string& path, const std::string& linux_file) {
    // Parse the provided path to determine the directory where the new file should be created
    std::string parent_directory_path = extract_directory_path(path);
    DirectoryEntry* parent_directory = findDirectory(parent_directory_path);

    if (!parent_directory) {
        std::cerr << "Error: Directory does not exist." << std::endl;
        return;
    }

    // Check if a file with the same name already exists in the parent directory
    std::string new_file_name = extract_filename(path);
    for (const auto& child : parent_directory->children) {
        if (child.getFilename() == new_file_name) {
            std::cerr << "Error: File with the same name already exists in the directory." << std::endl;
            return;
        }
    }

    // Open the Linux file
    std::ifstream linux_ifs(linux_file, std::ios::binary | std::ios::ate);
    if (!linux_ifs.is_open()) {
        std::cerr << "Error: Unable to open Linux file." << std::endl;
        return;
    }
    // Get the size of the Linux file
    std::streamsize linux_file_size = linux_ifs.tellg();
    linux_ifs.seekg(0); // Move the file pointer back to the beginning

    // Create a new DirectoryEntry for the file
    DirectoryEntry new_file;
    new_file.setFilename(extract_filename(path));
    new_file.setSize(static_cast<uint32_t>(linux_file_size));

    // Copy Linux file permissions to the new file
    setPermissionsFromLinuxFile(new_file, linux_file);
    set_file_metadata(linux_file,new_file);

    // Allocate blocks for the new file
    allocateBlocksForFile(new_file, new_file.getSize());

    // Write the contents of the Linux file into the blocks allocated for the new file
    uint32_t remaining_bytes = new_file.getSize();
    uint32_t current_block = new_file.getStartBlock();
    
    while (remaining_bytes > 0 && current_block != FAT_EOC){
        DiskBlock& block = blocks[current_block];
        uint32_t bytes_to_write = std::min(remaining_bytes, superblock.block_size);
        linux_ifs.read(block.data.data(), bytes_to_write);
        remaining_bytes -= bytes_to_write;
        current_block = fat[current_block];
    };

    // Close the Linux file
    linux_ifs.close();

    // Add the new file to the parent directory
    parent_directory->children.push_back(new_file);

    calculateDirectorySize(*parent_directory);

}

void FileSystem::setPermissionsFromLinuxFile(DirectoryEntry& entry, const std::string& linux_file) {
    struct stat linux_stat;
    if (stat(linux_file.c_str(), &linux_stat) != 0) {
        std::cerr << "Error: Unable to get file stat for Linux file." << std::endl;
        return;
    }

    // Extract permissions from Linux file stat
    mode_t permissions = linux_stat.st_mode & 0777; // Extract permission bits from stat

    // Set permissions for the new file based on the read and write bits of the Linux file
    entry.setPermissions({(permissions & S_IRUSR) != 0, (permissions & S_IWUSR) != 0});
}


void FileSystem::set_file_metadata(const std::string& path, DirectoryEntry& entry) {
    struct stat file_stat;
    if (stat(path.c_str(), &file_stat) == 0) {
        // Set permissions
        Permissions perms;
        perms.read = (file_stat.st_mode & S_IRUSR) != 0;
        perms.write = (file_stat.st_mode & S_IWUSR) != 0;
        entry.setPermissions(perms);

        // Set creation and modification times
        entry.setCreationTime(file_stat.st_ctime);
        entry.setModificationTime(file_stat.st_mtime);
    } else {
        std::cerr << "Error: Unable to retrieve metadata for " << path << std::endl;
    }
}

void FileSystem::apply_file_metadata(const std::string& path, const DirectoryEntry& entry) {
    // Set permissions
    Permissions perms = entry.getPermissions();
    mode_t mode = 0;
    if (perms.read) {
        mode |= S_IRUSR;
    }
    if (perms.write) {
        mode |= S_IWUSR;
    }

    // Apply permissions using chmod
    if (chmod(path.c_str(), mode) != 0) {
        std::cerr << "Error: Unable to set permissions for " << path << std::endl;
    }

    // Set creation and modification times
    struct utimbuf new_times;
    new_times.actime = entry.getCreationTime(); // Access time
    new_times.modtime = entry.getModificationTime(); // Modification time

    // Apply times using utime
    if (utime(path.c_str(), &new_times) != 0) {
        std::cerr << "Error: Unable to set times for " << path << std::endl;
    }
}

void FileSystem::read(const std::string& path, const std::string& linux_file) {
    // Extract the parent directory path and the file name
    std::string parent_path = extract_directory_path(path);
    std::string file_name = extract_filename(path);

    // Find the parent directory
    DirectoryEntry* parent_directory = findDirectory(parent_path);
    if (parent_directory == nullptr) {
        std::cerr << "Error: Parent directory not found: " << parent_path << std::endl;
        return;
    }

    // Find the file within the parent directory
    DirectoryEntry* entry = nullptr;
    for (auto& child : parent_directory->children) {
        if (child.getFilename() == file_name && !(is_directory(child))) {
            entry = &child;
            break;
        }
    }

    if (entry == nullptr) {
        std::cerr << "Error: File not found: " << file_name << std::endl;
        return;
    }

    if (!checkPassword(*entry)) {
        std::cerr << "Error: Incorrect password." << std::endl;
        return;
    }
    
    if(!(entry->getPermissions().read)) {
        std::cerr << "Error: File do not have a permission for reading: " << file_name << std::endl;
        return;
    }

    // Open the Linux file for writing
    std::ofstream ofs(linux_file, std::ios::out);
    if (!ofs) {
        std::cerr << "Error: Unable to open Linux file for writing" << std::endl;
        return;
    }


    // Read the file contents from the blocks and write them to the Linux file
    uint32_t remaining_bytes = entry->getSize();
    uint16_t current_block = entry->getStartBlock();


    while (remaining_bytes > 0 && current_block != FAT_EOC){
        const DiskBlock& block = blocks[current_block];
        uint32_t bytes_to_read = std::min(remaining_bytes, superblock.block_size);
        ofs.write(block.data.data(), bytes_to_read);
        remaining_bytes -= bytes_to_read;
        current_block = fat[current_block];
    }

    ofs.close();


    // Set the file metadata (permissions, creation time, modification time)
    apply_file_metadata(linux_file, *entry);

}


void FileSystem::del(const std::string& path) {
    // Extract the parent directory path and the file name
    std::string parentDirectoryPath = extract_directory_path(path);
    std::string fileName = extract_filename(path);

    // Find the parent directory
    DirectoryEntry* parentDirectory = findDirectory(parentDirectoryPath);
    if (!parentDirectory) {
        std::cerr << "Error: Parent directory not found." << std::endl;
        return;
    }

    // Iterate through the children of the parent directory to find the file
        auto it = parentDirectory->children.begin();
        while (it != parentDirectory->children.end()) {
        if (it->getFilename() == fileName) {
            // Check if the entry is a file (not a directory)
            if (!(it->getAttribute() & ATTR_DIRECTORY)) {

                if (!checkPassword(*it)) {
                    std::cerr << "Error: Incorrect password." << std::endl;
                    return;
                }
                // Deallocate blocks occupied by the file
                deallocateBlocksForFile(*it);
                // Remove the file entry from the parent directory's list of children
                it = parentDirectory->children.erase(it);
                std::cout << "File deleted successfully." << std::endl;
                return;
            } else {
                std::cerr << "Error: The specified path points to a directory, not a file." << std::endl;
                return;
            }
        }
        ++it;
        }

        std::cerr << "Error: File not found in the specified directory." << std::endl;
}


void FileSystem::fs_chmod(const std::string& path, const std::string& permissions) {
    // Extract the parent directory path and the file name
    std::string parentDirectoryPath = extract_directory_path(path);
    std::string fileName = extract_filename(path);

    // Find the parent directory
    DirectoryEntry* parentDirectory = findDirectory(parentDirectoryPath);
    if (!parentDirectory) {
        std::cerr << "Error: Parent directory not found." << std::endl;
        return;
    }

    // Find the file in the parent directory
    DirectoryEntry* fileEntry = nullptr;
    for (auto& child : parentDirectory->children) {
        if (child.getFilename() == fileName) {
            fileEntry = &child;
            break;
        }
    }

    if (!fileEntry) {
        std::cerr << "Error: File not found in the specified directory." << std::endl;
        return;
    }

    if (!checkPassword(*fileEntry)) {
        std::cerr << "Error: Incorrect password." << std::endl;
        return;
    }

    // Modify the permissions of the file
    Permissions currentPermissions = fileEntry->getPermissions();

    if (permissions == "+r") {
        currentPermissions.read = true;
    } else if (permissions == "+w") {
        currentPermissions.write = true;
    } else if (permissions == "-r") {
        currentPermissions.read = false;
    } else if (permissions == "-w") {
        currentPermissions.write = false;
    } else if (permissions == "+rw" || permissions == "+wr") {
        currentPermissions.read = true;
        currentPermissions.write = true;
    } else if (permissions == "-rw" || permissions == "-wr") {
        currentPermissions.read = false;
        currentPermissions.write = false;
    } else {
        std::cerr << "Error: Invalid permissions string." << std::endl;
        return;
    }

    fileEntry->setPermissions(currentPermissions);
    fileEntry->setModificationTime(std::time(nullptr));
}


    
void FileSystem::addpw(const std::string& path, const std::string& password) {
    // Extract the parent directory path and the file name
    std::string parentDirectoryPath = extract_directory_path(path);
    std::string fileName = extract_filename(path);

    // Find the parent directory
    DirectoryEntry* parentDirectory = findDirectory(parentDirectoryPath);
    if (!parentDirectory) {
        std::cerr << "Error: Parent directory not found." << std::endl;
        return;
    }

    // Find the file in the parent directory
    DirectoryEntry* fileEntry = nullptr;
    for (auto& child : parentDirectory->children) {
        if (child.getFilename() == fileName) {
            fileEntry = &child;
            break;
        }
    }

    if (!checkPassword(*fileEntry)) {
        std::cerr << "Error: Incorrect password." << std::endl;
        return;
    }


    if (!fileEntry) {
        std::cerr << "Error: File not found in the specified directory." << std::endl;
        return;
    }

    fileEntry->setPassword(password);
    fileEntry->setModificationTime(std::time(nullptr));
         
}

bool FileSystem::checkPassword(const DirectoryEntry& entry) {
    std::string storedPassword = entry.getPassword();
    if (storedPassword.empty()) {
        // No password set, no need to check
        return true;
    }

    std::string inputPassword;
    std::cout << "Enter password for " << entry.getFilename() << ": ";
    std::cin >> inputPassword;

    return storedPassword == inputPassword;
}


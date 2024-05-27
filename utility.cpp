#include "utility.h"

std::string extract_filename(const std::string& path) {
    size_t last_slash_pos = path.find_last_of('/');
    if (last_slash_pos == std::string::npos) {
        return path;
    }
    return path.substr(last_slash_pos + 1);
}

std::string extract_directory_path(const std::string& path) {
    if (path == "/") {
        return "/";
    }

    size_t last_slash_pos = path.find_last_of('/');
    if (last_slash_pos == std::string::npos) {
        return ""; // Return empty string for paths without slashes
    }
    return path.substr(0, last_slash_pos);
}


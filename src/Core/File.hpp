#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace Izo {

class File {
public:
    static std::string read_all_text(const std::string& path);

    /* TODO: Implement more file functions */
    static std::vector<uint8_t> read_all_bytes(const std::string& path);

    static bool write_all_text(const std::string& path, const std::string& content);
    static bool write_all_bytes(const std::string& path, const std::vector<uint8_t>& data);

    static bool exists(const std::string& path);
    static bool remove(const std::string& path);
    static bool copy(const std::string& source, const std::string& destination);
    static bool move(const std::string& source, const std::string& destination);

    static size_t get_size(const std::string& path);

    static bool create_directory(const std::string& path);
    static bool directory_exists(const std::string& path);
    static bool remove_directory(const std::string& path);
    static std::vector<std::string> list_directory(const std::string& path);

    static std::string get_extension(const std::string& path);
    static std::string get_filename(const std::string& path);
    static std::string get_directory_name(const std::string& path);
    static std::string combine_paths(const std::string& path1, const std::string& path2);
};

}

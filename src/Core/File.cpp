#include "Core/File.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <system_error>
#include "Debug/Logger.hpp"

namespace Izo {

std::string File::read_all_text(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return "";
    std::stringstream buffer;
    buffer << f.rdbuf();
    return buffer.str();
}

std::vector<uint8_t> File::read_all_bytes(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f.is_open()) return {};
    const std::ifstream::pos_type size = f.tellg();
    if (size <= 0) return {};
    std::vector<uint8_t> data(static_cast<size_t>(size));
    f.seekg(0, std::ios::beg);
    f.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()));
    if (!f) return {};
    return data;
}

bool File::write_all_text(const std::string& path, const std::string& content) {
    std::ofstream f(path);
    if (!f.is_open()) return false;
    f.write(content.data(), static_cast<std::streamsize>(content.size()));
    return static_cast<bool>(f);
}

bool File::write_all_bytes(const std::string& path, const std::vector<uint8_t>& data) {
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open()) return false;
    if (!data.empty())
        f.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    return static_cast<bool>(f);
}

bool File::exists(const std::string &path) {
    bool found = std::filesystem::exists(path);

    if (found)
        LogDebug("Checking if {} exists: Found!", path);
    else
        LogDebug("Checking if {} exists: Not Found!", path);

    return found;
}

bool File::is_directory(const std::string& path) {
    return std::filesystem::is_directory(path);
}

bool File::remove(const std::string& path) {
    std::error_code ec;
    return std::filesystem::remove(path, ec);
}

bool File::copy(const std::string& source, const std::string& destination) {
    std::error_code ec;
    return std::filesystem::copy_file(source, destination, std::filesystem::copy_options::overwrite_existing, ec);
}

bool File::move(const std::string& source, const std::string& destination) {
    std::error_code ec;
    std::filesystem::rename(source, destination, ec);
    return !ec;
}

size_t File::get_size(const std::string& path) {
    std::error_code ec;
    const auto size = std::filesystem::file_size(path, ec);
    if (ec) return 0;
    return static_cast<size_t>(size);
}

bool File::create_directory(const std::string& path) {
    std::error_code ec;
    return std::filesystem::create_directories(path, ec);
}

bool File::remove_directory(const std::string& path) {
    std::error_code ec;
    std::filesystem::remove_all(path, ec);
    return !ec;
}

std::vector<std::string> File::list_directory(const std::string& path) {
    std::vector<std::string> entries;
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(path, ec)) {
        if (ec) break;
        entries.push_back(entry.path().string());
    }
    return entries;
}

std::string File::get_extension(const std::string& path) {
    return std::filesystem::path(path).extension().string();
}

std::string File::get_filename(const std::string& path) {
    return std::filesystem::path(path).filename().string();
}

std::string File::get_directory_name(const std::string& path) {
    return std::filesystem::path(path).parent_path().string();
}

std::string File::combine_paths(const std::string& path1, const std::string& path2) {
    return (std::filesystem::path(path1) / std::filesystem::path(path2)).string();
}

}

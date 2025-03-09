#include "FileSystem.h"

namespace logger = SKSE::log;

namespace fs {
    std::vector<std::filesystem::path> GetAllFilesInDirectory(const std::filesystem::path& dir_path) {
        std::vector<std::filesystem::path> files;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(dir_path)) {
            if (std::filesystem::is_regular_file(entry)) {
                files.push_back(entry.path());
            }
        }
        return files;
    }
    
    std::string GetFileContents(const std::filesystem::path& filePath) {
        if (!std::filesystem::exists(filePath)) {

        }

        std::ifstream file_stream(filePath, std::ios::in | std::ios::binary);
        if (!file_stream.is_open()) {
            logger::error("Could not open file[{}]", filePath.string());
            return "";
        }
        std::stringstream string_stream;
        string_stream << file_stream.rdbuf();
        return string_stream.str();
    }
}
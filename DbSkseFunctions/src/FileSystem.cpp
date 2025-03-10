#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
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

//replace LogAndMessage with logger:: functions ==================================================================================
    enum logLevel { trace, debug, info, warn, error, critical };
    enum debugLevel { notification, messageBox }; 

    std::vector<std::string> logLevelsVec = {
        ", trace",
        ", debug",
        ", info",
        ", warn",
        ", error",
        ", critical"
    };

    std::pair<int, size_t> GetLogLevelForLogAndMessageFunc(std::string& sFunc) {
        auto size = logLevelsVec.size();
        for (int i = 0; i < size; i++) {
            size_t iStart = sFunc.find(logLevelsVec[i]);
            if (iStart != std::string::npos) {
                return { i, iStart };
            }
        }
        return { size, 0 };
    }

    void ReplaceLogAndMessage(std::string& fileContents) {
        auto npos = std::string::npos;

        std::string stdFmt = "std::format(";
        auto stdFmtSize = stdFmt.size();

        std::string sLogAndMsg = "gfuncs::LogAndMessage";
        auto sLogAndMsgSize = sLogAndMsg.size();

        size_t iStart = fileContents.find(sLogAndMsg);
        while (iStart != npos) {
            size_t iEnd = fileContents.find(";", iStart);
            if (iEnd != npos) {
                std::string sFunc = fileContents.substr(iStart, (iEnd - iStart));
                size_t fStart = sFunc.find(stdFmt);
                if (fStart != npos) {
                    size_t fEnd = sFunc.find(")", fStart);
                    if (fEnd != npos) {
                        sFunc.erase(fEnd, 1);
                    }
                    sFunc.erase(fStart, stdFmtSize);
                }
                std::string sFuncStart = "logger::trace";

                auto lvl = GetLogLevelForLogAndMessageFunc(sFunc);
                //lvl.first is the log level found, .second is the index found in the sFunc string.

                switch (lvl.first) {
                case trace: {
                    sFunc.erase(lvl.second, logLevelsVec[trace].size());
                    break;
                }
                case debug: {
                    sFuncStart = "logger::debug";
                    sFunc.erase(lvl.second, logLevelsVec[debug].size());
                    break;
                }
                case info: {
                    sFuncStart = "logger::info";
                    sFunc.erase(lvl.second, logLevelsVec[info].size());
                    break;
                }
                case warn: {
                    sFuncStart = "logger::warn";
                    sFunc.erase(lvl.second, logLevelsVec[warn].size());
                    break;
                }
                case error: {
                    sFuncStart = "logger::error";
                    sFunc.erase(lvl.second, logLevelsVec[error].size());
                    break;
                }
                case critical: {
                    sFuncStart = "logger::critical";
                    sFunc.erase(lvl.second, logLevelsVec[critical].size());
                    break;
                }
                }
                std::string sNewFunc = sFuncStart + sFunc.substr(sLogAndMsgSize);
                std::string contentsA = fileContents.substr(0, iStart);
                std::string contentsB = fileContents.substr(iEnd);
                fileContents = contentsA + sNewFunc + contentsB;
            }
            else {
                fileContents.erase(iStart, sLogAndMsgSize);
            }
            iStart = fileContents.find(sLogAndMsg);
        }
    }

    void ReplaceLogAndMessageInFile(std::filesystem::path filePath) {
        if (!std::filesystem::exists(filePath)) {
            logger::error("file[{}] not found", filePath.generic_string());
            return;
        }

        std::string fileContents = GetFileContents(filePath);
        ReplaceLogAndMessage(fileContents);

        std::ofstream outputFile(filePath, std::ios::binary);
        if (!outputFile.is_open()) {
            logger::error("Error opening file[{}]", filePath.generic_string());
            return;
        }

        outputFile << fileContents;
    } 

    void ReplaceLogAndMessageInFolder(std::filesystem::path folderPath) {
        auto paths = GetAllFilesInDirectory(folderPath);
        for (auto& path : paths) {
            ReplaceLogAndMessageInFile(path);
        }
    }

    void ReplaceLogAndMessageFuncs() {
        std::filesystem::path folderPath = std::filesystem::current_path();
        folderPath.append("Data");
        folderPath.append("CPPFilesToReplaceLogAndMessageFunc");
        fs::ReplaceLogAndMessageInFolder(folderPath);
        logger::critical("folderPath[{}]", folderPath.generic_string());
    }

}
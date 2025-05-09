#pragma once

namespace fs {
	std::vector<std::filesystem::path> GetAllFilesInDirectory(const std::filesystem::path& dir_path);

	std::string GetFileContents(const std::filesystem::path& filePath);

	void ReplaceLogAndMessageInFile(std::filesystem::path filePath);

	void ReplaceLogAndMessageInFolder(std::filesystem::path folderPath);

	void ReplaceLogAndMessageFuncs();
}


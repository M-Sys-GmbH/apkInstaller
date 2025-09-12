#ifndef APKINSTALLER_HPP
#define APKINSTALLER_HPP

#include <string>
#include <vector>
#include <filesystem>

namespace apkInstaller {

	namespace fs = std::filesystem;

	void printHelp(const std::string& progName);

	bool handleFileOption(const std::string& filePath);
	bool handleDirectoryOption(const std::string& dirPath);

	std::vector<fs::path> findApkFiles(const std::string& dirPath);
	fs::path selectApkFile(const std::vector<fs::path>& apkFiles);

	bool checkAdbInstalled();
	std::vector<std::string> getAdbDevices();
	bool installApk(const fs::path& apkFile);

} // namespace apkInstaller

#endif // APKINSTALLER_HPP

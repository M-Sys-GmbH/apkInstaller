#include "apkInstaller.h"

#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <array>

using FileCloser = int(*)(FILE*);

namespace apkInstaller {

    static std::string toLower(const std::string& s) {
        std::string result = s;
        std::ranges::transform(result, result.begin(), ::tolower);
        return result;
    }

    void printHelp(const std::string& progName) {
        std::cout << "Usage:\n"
            << "  " << progName << " -f <apk-file>\n"
            << "  " << progName << " -d <directory>\n\n"
            << "Options:\n"
            << "  -f <apk-file>    Path to a single APK file to install\n"
            << "  -d <directory>   Path to a directory which contains one or more APKs\n\n"
            << "Note:\n"
            << "  This tool is an apkInstaller. You must provide either the path to an APK\n"
            << "  file with -f, or the path to a directory with -d where at least one APK\n"
            << "  file is located.\n";
    }

    bool handleFileOption(const std::string& filePath) {
        fs::path file(filePath);

        if (!fs::exists(file)) {
            std::cerr << "Error: File does not exist: " << file << "\n";
            return false;
        }

        if (toLower(file.extension().string()) != ".apk") {
            std::cerr << "Error: Not an APK file: " << file << "\n";
            return false;
        }

        std::cout << "Using APK file: " << file << "\n";
        return installApk(file);
    }

    bool handleDirectoryOption(const std::string& dirPath) {
        if (!fs::is_directory(dirPath)) {
            std::cerr << "Error: Not a directory: " << dirPath << "\n";
            return false;
        }

        auto apkFiles = findApkFiles(dirPath);
        if (apkFiles.empty()) {
            std::cerr << "Error: No APK files found in directory: " << dirPath << "\n";
            return false;
        }

        fs::path chosenApk = selectApkFile(apkFiles);
        if (chosenApk.empty()) {
            std::cerr << "No APK selected.\n";
            return false;
        }

        std::cout << "Using APK file: " << chosenApk << "\n";
        return installApk(chosenApk);
    }

    std::vector<fs::path> findApkFiles(const std::string& dirPath) {
        std::vector<fs::path> apkFiles;

        for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
            if (entry.is_regular_file() &&
                toLower(entry.path().extension().string()) == ".apk") {
                apkFiles.push_back(entry.path());
            }
        }

        return apkFiles;
    }

    fs::path selectApkFile(const std::vector<fs::path>& apkFiles) {
        if (apkFiles.size() == 1) {
            return apkFiles.front();
        }

        // Prefer "signed" APK if present
        for (const auto& file : apkFiles) {
            if (toLower(file.filename().string()).find("signed") != std::string::npos) {
                return file;
            }
        }

        std::cout << "Multiple APK files found:\n";
        for (size_t i = 0; i < apkFiles.size(); ++i) {
            std::cout << "  [" << i + 1 << "] " << apkFiles[i] << "\n";
        }
        std::cout << "Select APK to use (1-" << apkFiles.size() << "): " << std::flush;

        size_t choice = 0;
        std::cin >> choice;

        if (choice == 0 || choice > apkFiles.size()) {
            std::cerr << "Invalid choice.\n";
            return {};
        }

        return apkFiles[choice - 1];
    }

    bool checkAdbInstalled() {
        int ret = 0;

#ifdef _WIN32
        ret = std::system("adb version >nul 2>&1");
        if (ret != 0) {
            fs::path localAdb = fs::current_path() / "adb.exe";
            if (fs::exists(localAdb) && fs::is_regular_file(localAdb)) {
                std::string cmd = "\"" + localAdb.string() + "\" version >nul 2>&1";
                ret = std::system(cmd.c_str());
            }
        }
#else
        ret = std::system("adb version >/dev/null 2>&1");
#endif

        if (ret != 0) {
            std::cerr << "Error: adb is not installed or not in PATH.\n"
                << "Please install Android Platform Tools.\n"
                << "  - Linux: sudo apt install adb\n"
                << "  - macOS: brew install android-platform-tools\n"
                << "  - Windows: Install from https://developer.android.com/studio/releases/platform-tools\n";
            return false;
        }

        return true;
    }

    std::vector<std::string> getAdbDevices() {
        std::vector<std::string> devices;
        std::array<char, 128> buffer{};
        std::string result;

#ifdef _WIN32
        std::unique_ptr<FILE, FileCloser> pipe(_popen("adb devices", "r"), _pclose);
#else
        std::unique_ptr<FILE, FileCloser> pipe(popen("adb devices", "r"), pclose);
#endif

        if (!pipe) {
            std::cerr << "Error: Failed to run adb devices.\n";
            return devices;
        }

        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }

        std::istringstream iss(result);
        std::string line;
        while (std::getline(iss, line)) {
            if (line.find("device") != std::string::npos && line.find("List") == std::string::npos) {
                std::string serial = line.substr(0, line.find('\t'));
                devices.push_back(serial);
            }
        }

        return devices;
    }

    bool installApk(const fs::path& apkFile) {
        if (!checkAdbInstalled()) {
            return false;
        }

        auto devices = getAdbDevices();
        if (devices.empty()) {
            std::cerr << "Error: No adb devices found.\n";
            return false;
        }

        std::string targetDevice;
        if (devices.size() == 1) {
            targetDevice = devices.front();
        }
        else {
            std::cout << "Multiple devices detected:\n";
            for (size_t i = 0; i < devices.size(); ++i) {
                std::cout << "  [" << i + 1 << "] " << devices[i] << "\n";
            }
            std::cout << "Select device (1-" << devices.size() << "): " << std::flush;

            size_t choice = 0;
            std::cin >> choice;

            if (choice == 0 || choice > devices.size()) {
                std::cerr << "Invalid choice.\n";
                return false;
            }
            targetDevice = devices[choice - 1];
        }

        std::string command = "adb -s \"" + targetDevice + "\" install \"" + apkFile.string() + "\"";
        std::cout << "Running: " << command << "\n";
        int ret = std::system(command.c_str());
        return (ret == 0);
    }

} // namespace apkInstaller

// ---------------- MAIN ----------------

int main(int argc, char* argv[]) {
    using namespace apkInstaller;

    if (argc != 3) {
        printHelp(argv[0]);
        return 1;
    }

    std::string option = argv[1];
    std::string path = argv[2];

    bool success = false;
    if (option == "-f") {
        success = handleFileOption(path);
    }
    else if (option == "-d") {
        success = handleDirectoryOption(path);
    }
    else {
        printHelp(argv[0]);
        return 2;
    }

    return success ? 0 : 3;
}

# apkInstaller

**apkInstaller** is a cross-platform C++20 command-line tool that simplifies installing Android APKs onto connected devices.  
It validates APK files, automatically discovers them in directories, and installs them via `adb` on the selected device.

## Features
- Accepts either:
  - `-f <apk-file>` → direct path to a single APK  
  - `-d <directory>` → recursively searches for APKs in a directory
- Case-insensitive `.apk` detection
- Interactive selection if multiple APKs or devices are present
- Checks for `adb` and provides installation hints if missing
- Cross-platform (Linux, macOS, Windows)

## Requirements
- **C++20 compiler** (GCC ≥ 10, Clang ≥ 10, MSVC ≥ 2019)
- **Android Debug Bridge (adb)** installed and available in `PATH`

## Installation
Clone the repository and build with CMake:

```bash
git clone https://github.com/<your-org-or-user>/apkInstaller.git
cd apkInstaller
mkdir build && cd build
cmake ..
make
````

On Windows, use CMake with MSVC or MinGW.

## Usage

```bash
apkInstaller -f myapp.apk
apkInstaller -d ./build/outputs/apk
```

### Examples

* Install a specific APK:

  ```bash
  apkInstaller -f /path/to/app.apk
  ```
* Install from a directory (recursively finds APKs):

  ```bash
  apkInstaller -d ./apps/
  ```

If multiple APKs are found, the tool:

1. Chooses the signed APK if available.
2. Otherwise, prompts the user to select.

If multiple devices are connected, the user selects the target device interactively.

## Notes

* To install APKs, make sure USB debugging is enabled on your Android device.
* If `adb` is missing, follow the installation instructions from the tool’s output:

  * Linux: `sudo apt install adb`
  * macOS: `brew install android-platform-tools`
  * Windows: [Download Platform Tools](https://developer.android.com/studio/releases/platform-tools)


# Add source to this project's executable.
add_executable (apkInstaller "apkInstaller.cpp" "apkInstaller.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET apkInstaller PROPERTY CXX_STANDARD 20)
endif()

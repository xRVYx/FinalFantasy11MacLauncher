cmake_minimum_required(VERSION 3.21)
project(FFXIMacLauncher VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Homebrew Qt hint; adjust if Qt is installed elsewhere.
list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/qt6" "/opt/homebrew/opt/qt6")

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

add_executable(FFXIMacLauncher MACOSX_BUNDLE
    src/app/main.cpp
    src/core/LoadConfig.cpp
    src/core/SaveConfig.cpp
    src/core/StartDownloadFile.cpp
    src/core/ComputeSha256.cpp
    src/core/FetchContentLength.cpp
    src/core/LoadDownloadManifest.cpp
    src/core/SaveDownloadManifest.cpp
    src/core/LogPath.cpp
    src/core/AppendLog.cpp
    src/core/ExtractInstallParts.cpp
    src/core/LaunchGame.cpp
    src/core/RunCompatProcess.cpp
    src/ui/CreateMainWindow.cpp
    src/ui/CreateLogViewerDialog.cpp
    src/ui/CreateChecksumDialog.cpp
)

target_include_directories(FFXIMacLauncher PRIVATE src)
target_link_libraries(FFXIMacLauncher PRIVATE Qt6::Widgets Qt6::Network)

set_target_properties(FFXIMacLauncher PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.ffximaclauncher"
    MACOSX_BUNDLE_BUNDLE_NAME "FFXI Mac Launcher"
)

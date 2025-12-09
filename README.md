# FFXI Mac Launcher (fan project)

Community-built launcher to help Final Fantasy XI fans download and prepare the Windows installer parts on macOS. Downloads are fetched from the official Square Enix URLs; this tool just streamlines the process.

> Status: Work in progress. The launcher is not fully functional yet.

## Features
- Guided download of all installer parts with resume support
- Progress bars for current file and overall batch
- Basic checksum tracking to avoid re-downloading valid files
- Optional extraction step once downloads finish (requires `7za` or `unar`)
- Simple log viewer for troubleshooting

## Requirements
- macOS with Qt 6 (e.g. `brew install qt`)
- CMake 3.24+ and a build tool (Ninja or Unix Makefiles)
- Optional: `7za` (p7zip) or `unar` in `PATH` for extraction

## Build
```bash
brew install cmake ninja qt    # if not already installed
cmake -S . -B build -G "Ninja" -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build
```
If you prefer Makefiles, drop the `-G "Ninja"` argument.

## Run
```bash
./build/FFXIMacLauncher.app/Contents/MacOS/FFXIMacLauncher
```
Downloads and the manifest are stored under the app data directory (typically `~/Library/Application Support/FFXIMacLauncher/downloads`).

## Disclaimer
This is a fan-made, open-source project. It is not affiliated with, endorsed by, or supported by Square Enix. FINAL FANTASY XI and related trademarks are owned by Square Enix.

## Contributing
Issues and pull requests are welcome. Please keep contributions focused on improving the macOS player experience while respecting Square Enix’s rights and distribution terms.

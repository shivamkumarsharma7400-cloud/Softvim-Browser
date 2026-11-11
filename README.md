# Softvim Browser

A lightweight, premium Chromium-based browser for Windows, built with C++ and the Chromium Embedded Framework (CEF).

Softvim Browser is designed to be elegant, minimal, and calm — inspired by Safari's design philosophy while feeling native to Windows.

![Status](https://img.shields.io/badge/status-pre--alpha-orange)
![Platform](https://img.shields.io/badge/platform-Windows%20x64-blue)
![CEF](https://img.shields.io/badge/engine-Chromium%20144-green)

---

## Features (Roadmap)

- [x] **Milestone 1** — CEF browser window with Chromium rendering
- [ ] **Milestone 2** — Custom Safari-inspired toolbar with address bar
- [ ] **Milestone 3** — Tab management
- [ ] **Milestone 4** — Bookmarks, downloads, history, settings
- [ ] **Milestone 5** — Private browsing, themes, keyboard shortcuts
- [ ] **Milestone 6** — Installer and release

## Building

### Prerequisites

- **Visual Studio 2022** (Community or Build Tools) with C++ desktop workload
- **CMake 3.21+**
- **Python 3.9–3.11** (required by CEF build scripts)
- **Git**

> **No compiler?** All builds also run automatically on GitHub Actions. Push your changes and download the built artifacts.

### Build Steps

```powershell
# Clone the repository
git clone https://github.com/softvim/browser.git
cd browser

# Create build directory
mkdir build
cd build

# Configure (CMake will download CEF automatically — ~300MB, takes a few minutes)
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release ..

# Build
cmake --build . --config Release -j 4

# Run
./bin/Release/SoftvimBrowser.exe
```

### GitHub Actions (Cloud Build)

Every push to `main` triggers an automated build. Download the latest build:

1. Go to the **Actions** tab on GitHub
2. Click the latest successful workflow run
3. Download the **SoftvimBrowser-windows-x64** artifact

## Architecture

```
SoftvimBrowser.exe (main process)
├── CEF Browser Process — Manages windows and navigation
├── CEF Renderer Process — Renders web pages (sandboxed)
├── CEF GPU Process — Hardware-accelerated graphics
└── CEF Utility Processes — Network, audio, etc.
```

The browser uses CEF's **Alloy runtime mode**, which gives us full control over the user interface. The toolbar, tabs, and panels are rendered as HTML/CSS inside a dedicated CEF browser view — this is what enables the premium, Safari-inspired aesthetic.

## Design Philosophy

- **Elegant**: Every pixel is intentional
- **Minimal**: Show only what's needed
- **Calm**: No visual noise or distractions
- **Native**: Respects Windows conventions
- **Fast**: Lightweight interface, maximum content space

## Project Structure

```
Browser/
├── CMakeLists.txt          # Root build configuration
├── cmake/                  # CMake helpers (CEF download)
├── src/                    # C++ source code
│   ├── main.cc             # Entry point
│   ├── app.h/cc            # CEF application setup
│   ├── browser_window.h/cc # Window management
│   ├── client_handler.h/cc # Browser event handling
│   └── resource_util.h/cc  # Resource helpers
├── resources/              # UI assets and Windows resources
│   ├── ui/                 # HTML/CSS/JS interface
│   └── softvim.rc          # Windows resource script
└── .github/workflows/      # CI/CD pipeline
```

## License

Copyright © 2024 Softvim Solutions. All rights reserved.


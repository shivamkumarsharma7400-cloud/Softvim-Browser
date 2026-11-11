# Softvim Browser

Softvim Browser is a lightweight, custom Chromium-powered Windows web browser developed by Softvim Solutions. It uses C++ and the Chromium Embedded Framework (CEF 144) in Alloy runtime mode, coupled with a minimal, elegant desktop user interface.

## Highlights & Features

- **Custom Dual-View Architecture**: A dedicated native Win32 window hosting a top HTML/CSS/JS control chrome and isolated content browser instances below.
- **Safari-Inspired Visual Aesthetic**: Centered, rounded search/address bar, compact tab bar, thin-line iconography, and light/dark theme toggle.
- **Multi-Tab Management**: Tab switching, new tab creation, and smooth cleanup without page reloads.
- **Private Browsing Mode**: Isolated in-memory request contexts that prevent cookies, local cache, and history from persisting to disk.
- **Persistent Local Storage**: Bookmarks, browsing history, and download activity tracking stored securely in `%LOCALAPPDATA%`.
- **Keyboard Shortcuts**: Complete desktop shortcuts (`Ctrl+T`, `Ctrl+W`, `Ctrl+L`, `Ctrl+R`, `Ctrl+Shift+P`, `Ctrl+B`, `Ctrl+H`, `Ctrl+J`, and more).
- **Automated Cloud CI/CD**: Pre-configured GitHub Actions build workflow for compiling Windows x64 binaries without needing a local MSVC compiler installation.

---

## Directory Structure

```text
Browser/
├── .github/workflows/
│   └── build.yml             # GitHub Actions CI build & packaging pipeline
├── cmake/
│   └── DownloadCEF.cmake     # Automatic CEF 144 binary distribution downloader
├── resources/
│   ├── installer.iss         # Inno Setup Windows installer script
│   ├── softvim.ico           # Application icon
│   ├── softvim.rc            # Windows executable resource file
│   └── ui/                   # Browser UI chrome (HTML5 / CSS3 / JavaScript)
│       ├── index.html        # Main toolbar & sidebar structure
│       ├── scripts/app.js    # Browser frontend controller & IPC client
│       └── styles/           # Theme, toolbar, and sidebar stylesheets
├── src/                      # C++ browser implementation
│   ├── CMakeLists.txt        # Build definitions & target settings
│   ├── main.cc               # WinMain entry point & DPI enablement
│   ├── app.h / app.cc        # CEF application lifecycle handlers
│   ├── browser_window.h / cc # Dual-view Win32 window manager
│   ├── client_handler.h / cc # Web content client callbacks
│   ├── ui_handler.h / cc     # UI chrome CEF client & IPC message router
│   ├── tab_manager.h / cc    # Multi-tab lifecycle & Private Browsing manager
│   ├── storage_manager.h / cc# Bookmarks, history, and downloads persistence
│   └── resource_util.h / cc  # Resource resolution & URI helpers
├── CMakeLists.txt            # Top-level project CMake configuration
└── LICENSE                   # MIT License
```

---

## How to Build

### Option A: Cloud Builds via GitHub Actions (Recommended)

1. Connect your local git repository to a GitHub remote:
   ```powershell
   git remote add origin https://github.com/<your-username>/<your-repo>.git
   git push -u origin main
   ```
2. In GitHub, open the **Actions** tab.
3. The build workflow will automatically configure CMake, download the official CEF binary distribution, compile `SoftvimBrowser.exe`, and generate the downloadable zip artifact.

### Option B: Local Compilation on Windows

Requirements:
- Visual Studio 2022 (with "Desktop development with C++" workload)
- CMake 3.21+
- Python 3.9–3.11

Commands:
```powershell
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release -j 4
```
The compiled output will be generated inside `build/bin/Release/`.

---

## Keyboard Shortcuts

| Shortcut | Description |
| :--- | :--- |
| `Ctrl + T` | Open a new tab |
| `Ctrl + Shift + P` | Open a new Private Browsing tab |
| `Ctrl + W` | Close the active tab |
| `Ctrl + L` | Focus and highlight the address bar |
| `Ctrl + R` / `F5` | Reload the active page |
| `Alt + Left` | Go back in history |
| `Alt + Right` | Go forward in history |
| `Ctrl + B` | Toggle Bookmarks sidebar |
| `Ctrl + H` | Toggle History sidebar |
| `Ctrl + J` | Toggle Downloads sidebar |
| `Ctrl + D` | Bookmark the current page |
| `Escape` | Dismiss active sidebar |

---

## License

Softvim Browser is licensed under the [MIT License](LICENSE).

// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// browser_window.cc — Main browser window implementation.

#include "browser_window.h"

#include <dwmapi.h>

#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "resource_util.h"

// Static members
BrowserWindow* BrowserWindow::instance_ = nullptr;
const wchar_t BrowserWindow::kWindowClass[] = L"SoftvimBrowserWindow";
const wchar_t BrowserWindow::kWindowTitle[] = L"Softvim Browser";

BrowserWindow::BrowserWindow() = default;
BrowserWindow::~BrowserWindow() = default;

// static
void BrowserWindow::Create() {
  if (instance_) {
    return;  // Already created
  }

  instance_ = new BrowserWindow();

  if (!instance_->CreateNativeWindow()) {
    delete instance_;
    instance_ = nullptr;
    return;
  }

  instance_->CreateBrowser();
}

// static
BrowserWindow* BrowserWindow::GetInstance() {
  return instance_;
}

CefRefPtr<CefBrowser> BrowserWindow::GetBrowser() const {
  if (client_handler_) {
    return client_handler_->GetBrowser();
  }
  return nullptr;
}

bool BrowserWindow::CreateNativeWindow() {
  // Register window class
  WNDCLASSEXW wcex = {};
  wcex.cbSize = sizeof(WNDCLASSEXW);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.hInstance = GetModuleHandle(nullptr);
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  // Use a dark background brush to avoid white flash on startup
  wcex.hbrBackground = CreateSolidBrush(RGB(0x1C, 0x1C, 0x1E));
  wcex.lpszClassName = kWindowClass;

  // Load our application icon from resources, fall back to default
  wcex.hIcon = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(101));
  if (!wcex.hIcon) {
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
  }
  wcex.hIconSm = wcex.hIcon;

  if (!RegisterClassExW(&wcex)) {
    return false;
  }

  // Calculate centered window position
  int screenWidth = GetSystemMetrics(SM_CXSCREEN);
  int screenHeight = GetSystemMetrics(SM_CYSCREEN);
  int x = (screenWidth - kDefaultWidth) / 2;
  int y = (screenHeight - kDefaultHeight) / 2;

  // Create the window
  hwnd_ = CreateWindowExW(
      0,                    // Extended style
      kWindowClass,         // Window class
      kWindowTitle,         // Title
      WS_OVERLAPPEDWINDOW,  // Standard window with all controls
      x, y,                 // Position (centered)
      kDefaultWidth,        // Width
      kDefaultHeight,       // Height
      nullptr,              // Parent
      nullptr,              // Menu
      GetModuleHandle(nullptr),
      nullptr               // Additional data
  );

  if (!hwnd_) {
    return false;
  }

  // Enable dark mode title bar on Windows 10/11
  BOOL useDarkMode = TRUE;
  // DWMWA_USE_IMMERSIVE_DARK_MODE = 20
  DwmSetWindowAttribute(hwnd_, 20, &useDarkMode, sizeof(useDarkMode));

  ShowWindow(hwnd_, SW_SHOW);
  UpdateWindow(hwnd_);

  return true;
}

void BrowserWindow::CreateBrowser() {
  // Set up the browser to fill the entire client area
  CefWindowInfo window_info;
  RECT rect;
  GetClientRect(hwnd_, &rect);
  window_info.SetAsChild(hwnd_, rect);

  // Browser settings — mostly defaults for now
  CefBrowserSettings browser_settings;
  browser_settings.background_color = CefColorSetARGB(255, 0x1C, 0x1C, 0x1E);

  // Create the client handler
  client_handler_ = new ClientHandler();

  // Navigate to the start page
  // For Milestone 1, we open a simple welcome page.
  // Later, this will be our custom new-tab page.
  std::string start_url = "https://www.google.com";

  // Create the browser
  CefBrowserHost::CreateBrowser(
      window_info,
      client_handler_,
      start_url,
      browser_settings,
      nullptr,  // extra_info
      nullptr   // request_context
  );
}

// static
LRESULT CALLBACK BrowserWindow::WndProc(
    HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

  BrowserWindow* window = GetInstance();

  switch (message) {
    case WM_SIZE:
      if (window) {
        window->OnSize();
      }
      return 0;

    case WM_CLOSE:
      if (window) {
        window->OnClose();
      }
      return 0;

    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;

    // Handle erase background to prevent flicker
    case WM_ERASEBKGND:
      return 1;

    default:
      break;
  }

  return DefWindowProcW(hwnd, message, wParam, lParam);
}

void BrowserWindow::OnSize() {
  if (!hwnd_ || !client_handler_) {
    return;
  }

  CefRefPtr<CefBrowser> browser = client_handler_->GetBrowser();
  if (!browser) {
    return;
  }

  // Resize the browser to fill the entire client area
  RECT rect;
  GetClientRect(hwnd_, &rect);

  HWND browser_hwnd = browser->GetHost()->GetWindowHandle();
  if (browser_hwnd) {
    SetWindowPos(browser_hwnd, nullptr,
                 rect.left, rect.top,
                 rect.right - rect.left,
                 rect.bottom - rect.top,
                 SWP_NOZORDER);
  }
}

void BrowserWindow::OnClose() {
  if (client_handler_) {
    CefRefPtr<CefBrowser> browser = client_handler_->GetBrowser();
    if (browser) {
      // Request the browser to close. This triggers the CefLifeSpanHandler
      // callbacks which will handle cleanup.
      browser->GetHost()->CloseBrowser(false);
      return;  // Don't destroy the window yet — wait for browser to close
    }
  }

  DestroyWindow(hwnd_);
}


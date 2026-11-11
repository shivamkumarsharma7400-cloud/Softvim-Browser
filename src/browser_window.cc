// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// browser_window.cc — Implementation of dual-view browser window.

#include "browser_window.h"

#include <dwmapi.h>

#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "resource_util.h"

BrowserWindow* BrowserWindow::instance_ = nullptr;
const wchar_t BrowserWindow::kWindowClass[] = L"SoftvimBrowserWindow";
const wchar_t BrowserWindow::kWindowTitle[] = L"Softvim Browser";

BrowserWindow::BrowserWindow() = default;
BrowserWindow::~BrowserWindow() = default;

void BrowserWindow::Create() {
  if (instance_) return;

  instance_ = new BrowserWindow();
  if (!instance_->CreateNativeWindow()) {
    delete instance_;
    instance_ = nullptr;
    return;
  }

  instance_->CreateBrowsers();
}

BrowserWindow* BrowserWindow::GetInstance() {
  return instance_;
}

CefRefPtr<CefBrowser> BrowserWindow::GetContentBrowser() const {
  return content_handler_ ? content_handler_->GetBrowser() : nullptr;
}

CefRefPtr<CefBrowser> BrowserWindow::GetUiBrowser() const {
  return ui_handler_ ? ui_handler_->GetBrowser() : nullptr;
}

void BrowserWindow::NavigateContent(const std::string& url) {
  CefRefPtr<CefBrowser> browser = GetContentBrowser();
  if (browser && browser->GetMainFrame()) {
    browser->GetMainFrame()->LoadURL(url);
  }
}

void BrowserWindow::GoBackContent() {
  CefRefPtr<CefBrowser> browser = GetContentBrowser();
  if (browser && browser->CanGoBack()) {
    browser->GoBack();
  }
}

void BrowserWindow::GoForwardContent() {
  CefRefPtr<CefBrowser> browser = GetContentBrowser();
  if (browser && browser->CanGoForward()) {
    browser->GoForward();
  }
}

void BrowserWindow::ReloadContent() {
  CefRefPtr<CefBrowser> browser = GetContentBrowser();
  if (browser) {
    browser->Reload();
  }
}

void BrowserWindow::NotifyNavigationState(const std::string& url,
                                          const std::string& title,
                                          bool canGoBack,
                                          bool canGoForward) {
  if (ui_handler_) {
    ui_handler_->UpdateBrowserState(url, title, canGoBack, canGoForward);
  }
}

bool BrowserWindow::CreateNativeWindow() {
  WNDCLASSEXW wcex = {};
  wcex.cbSize = sizeof(WNDCLASSEXW);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.hInstance = GetModuleHandle(nullptr);
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = CreateSolidBrush(RGB(0x1C, 0x1C, 0x1E));
  wcex.lpszClassName = kWindowClass;

  wcex.hIcon = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(101));
  if (!wcex.hIcon) {
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
  }
  wcex.hIconSm = wcex.hIcon;

  if (!RegisterClassExW(&wcex)) {
    return false;
  }

  int screenWidth = GetSystemMetrics(SM_CXSCREEN);
  int screenHeight = GetSystemMetrics(SM_CYSCREEN);
  int x = (screenWidth - kDefaultWidth) / 2;
  int y = (screenHeight - kDefaultHeight) / 2;

  hwnd_ = CreateWindowExW(
      0,
      kWindowClass,
      kWindowTitle,
      WS_OVERLAPPEDWINDOW,
      x, y,
      kDefaultWidth,
      kDefaultHeight,
      nullptr,
      nullptr,
      GetModuleHandle(nullptr),
      nullptr
  );

  if (!hwnd_) {
    return false;
  }

  // Windows 11/10 Dark Title Bar
  BOOL useDarkMode = TRUE;
  DwmSetWindowAttribute(hwnd_, 20, &useDarkMode, sizeof(useDarkMode));

  ShowWindow(hwnd_, SW_SHOW);
  UpdateWindow(hwnd_);

  return true;
}

void BrowserWindow::CreateBrowsers() {
  RECT rect;
  GetClientRect(hwnd_, &rect);
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;

  // 1. Create UI Chrome Browser View (Top docking)
  ui_handler_ = new UiHandler();
  CefWindowInfo uiWindowInfo;
  RECT uiRect = {0, 0, width, kUiChromeHeight};
  uiWindowInfo.SetAsChild(hwnd_, uiRect);

  CefBrowserSettings uiSettings;
  uiSettings.background_color = CefColorSetARGB(255, 0x1C, 0x1C, 0x1E);

  std::string uiUrl = "file:///" + ResourceUtil::GetUIResourcePath("ui\\index.html");
  // Replace backslashes with forward slashes for valid URL
  for (char& c : uiUrl) {
    if (c == '\\') c = '/';
  }

  CefBrowserHost::CreateBrowser(
      uiWindowInfo,
      ui_handler_,
      uiUrl,
      uiSettings,
      nullptr,
      nullptr
  );

  // 2. Create Content Browser View (Remaining window space)
  content_handler_ = new ClientHandler();
  CefWindowInfo contentWindowInfo;
  RECT contentRect = {0, kUiChromeHeight, width, height};
  contentWindowInfo.SetAsChild(hwnd_, contentRect);

  CefBrowserSettings contentSettings;
  contentSettings.background_color = CefColorSetARGB(255, 0xFF, 0xFF, 0xFF);

  std::string startUrl = "https://www.google.com";

  CefBrowserHost::CreateBrowser(
      contentWindowInfo,
      content_handler_,
      startUrl,
      contentSettings,
      nullptr,
      nullptr
  );
}

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

    case WM_ERASEBKGND:
      return 1;

    default:
      break;
  }

  return DefWindowProcW(hwnd, message, wParam, lParam);
}

void BrowserWindow::OnSize() {
  if (!hwnd_) return;

  RECT rect;
  GetClientRect(hwnd_, &rect);
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;

  // Resize UI Chrome View
  if (ui_handler_ && ui_handler_->GetBrowser()) {
    HWND uiHwnd = ui_handler_->GetBrowser()->GetHost()->GetWindowHandle();
    if (uiHwnd) {
      SetWindowPos(uiHwnd, nullptr, 0, 0, width, kUiChromeHeight, SWP_NOZORDER);
    }
  }

  // Resize Content View
  if (content_handler_ && content_handler_->GetBrowser()) {
    HWND contentHwnd = content_handler_->GetBrowser()->GetHost()->GetWindowHandle();
    if (contentHwnd) {
      SetWindowPos(contentHwnd, nullptr,
                   0, kUiChromeHeight,
                   width, max(0, height - kUiChromeHeight),
                   SWP_NOZORDER);
    }
  }
}

void BrowserWindow::OnClose() {
  if (content_handler_ && content_handler_->GetBrowser()) {
    content_handler_->GetBrowser()->GetHost()->CloseBrowser(false);
  }
  if (ui_handler_ && ui_handler_->GetBrowser()) {
    ui_handler_->GetBrowser()->GetHost()->CloseBrowser(false);
  }
  DestroyWindow(hwnd_);
}

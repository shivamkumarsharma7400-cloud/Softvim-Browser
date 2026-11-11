// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// browser_window.cc — Multi-tab browser window with Private Browsing & Shortcuts.

#include "browser_window.h"

#include <dwmapi.h>
#include <algorithm>

#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/internal/cef_types_wrappers.h"
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

CefRefPtr<CefBrowser> BrowserWindow::GetUiBrowser() const {
  return ui_handler_ ? ui_handler_->GetBrowser() : nullptr;
}

int BrowserWindow::CreateTab(const std::string& url, bool is_private) {
  if (tab_manager_) {
    int tab_id = tab_manager_->CreateTab(url, is_private);
    if (ui_handler_) {
      ui_handler_->OnTabCreated(tab_id, is_private ? "Private Tab" : "New Tab", url);
    }
    return tab_id;
  }
  return -1;
}

bool BrowserWindow::CloseTab(int tab_id) {
  if (tab_manager_) {
    bool ok = tab_manager_->CloseTab(tab_id);
    if (ok && ui_handler_) {
      ui_handler_->OnTabClosed(tab_id, tab_manager_->GetActiveTabId());
    }
    return ok;
  }
  return false;
}

bool BrowserWindow::SwitchToTab(int tab_id) {
  if (tab_manager_) {
    bool ok = tab_manager_->SwitchToTab(tab_id);
    if (ok && ui_handler_) {
      TabInfo* tab = tab_manager_->GetTab(tab_id);
      if (tab) {
        bool canGoBack = tab->browser ? tab->browser->CanGoBack() : false;
        bool canGoForward = tab->browser ? tab->browser->CanGoForward() : false;
        ui_handler_->OnTabSwitched(tab_id, tab->url, tab->title, canGoBack, canGoForward);
      }
    }
    return ok;
  }
  return false;
}

void BrowserWindow::NavigateActiveTab(const std::string& url) {
  if (tab_manager_) tab_manager_->NavigateActiveTab(url);
}

void BrowserWindow::GoBackActiveTab() {
  if (tab_manager_) tab_manager_->GoBackActiveTab();
}

void BrowserWindow::GoForwardActiveTab() {
  if (tab_manager_) tab_manager_->GoForwardActiveTab();
}

void BrowserWindow::ReloadActiveTab() {
  if (tab_manager_) tab_manager_->ReloadActiveTab();
}

void BrowserWindow::NotifyNavigationState(int tab_id,
                                          const std::string& url,
                                          const std::string& title,
                                          bool canGoBack,
                                          bool canGoForward) {
  if (ui_handler_) {
    ui_handler_->UpdateBrowserState(tab_id, url, title, canGoBack, canGoForward);
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

  ui_handler_ = new UiHandler();
  CefWindowInfo uiWindowInfo;
  CefRect uiBounds(0, 0, width, kUiChromeHeight);
  uiWindowInfo.SetAsChild(hwnd_, uiBounds);

  CefBrowserSettings uiSettings;
  uiSettings.background_color = CefColorSetARGB(255, 0x1C, 0x1C, 0x1E);

  std::string uiUrl = "file:///" + ResourceUtil::GetUIResourcePath("ui\\index.html");
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

  tab_manager_ = std::make_unique<TabManager>(hwnd_, kUiChromeHeight);
  tab_manager_->CreateTab("https://www.google.com");
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

  if (ui_handler_ && ui_handler_->GetBrowser()) {
    HWND uiHwnd = ui_handler_->GetBrowser()->GetHost()->GetWindowHandle();
    if (uiHwnd) {
      SetWindowPos(uiHwnd, nullptr, 0, 0, width, kUiChromeHeight, SWP_NOZORDER);
    }
  }

  if (tab_manager_) {
    tab_manager_->ResizeActiveTab(width, height);
  }
}

void BrowserWindow::OnClose() {
  if (ui_handler_ && ui_handler_->GetBrowser()) {
    ui_handler_->GetBrowser()->GetHost()->CloseBrowser(false);
  }
  DestroyWindow(hwnd_);
}

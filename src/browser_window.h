// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// browser_window.h — Main browser window with Private Browsing & Keyboard Shortcuts.

#ifndef SOFTVIM_BROWSER_WINDOW_H_
#define SOFTVIM_BROWSER_WINDOW_H_

#include <windows.h>
#include <string>

#include "include/cef_browser.h"
#include "tab_manager.h"
#include "ui_handler.h"

class BrowserWindow {
 public:
  static void Create();
  static BrowserWindow* GetInstance();

  HWND GetHwnd() const { return hwnd_; }

  CefRefPtr<CefBrowser> GetUiBrowser() const;

  // Tab management actions invoked from UI
  int CreateTab(const std::string& url = "https://www.google.com", bool is_private = false);
  bool CloseTab(int tab_id);
  bool SwitchToTab(int tab_id);

  // Navigation actions on active tab
  void NavigateActiveTab(const std::string& url);
  void GoBackActiveTab();
  void GoForwardActiveTab();
  void ReloadActiveTab();

  // Navigation state synchronization
  void NotifyNavigationState(int tab_id,
                             const std::string& url,
                             const std::string& title,
                             bool canGoBack,
                             bool canGoForward);

 private:
  BrowserWindow();
  ~BrowserWindow();

  bool CreateNativeWindow();
  void CreateBrowsers();

  static LRESULT CALLBACK WndProc(HWND hwnd, UINT message,
                                  WPARAM wParam, LPARAM lParam);

  void OnSize();
  void OnClose();

  HWND hwnd_ = nullptr;
  CefRefPtr<UiHandler> ui_handler_;
  std::unique_ptr<TabManager> tab_manager_;

  static BrowserWindow* instance_;

  static const wchar_t kWindowClass[];
  static const wchar_t kWindowTitle[];

  static const int kDefaultWidth = 1280;
  static const int kDefaultHeight = 850;
  static const int kUiChromeHeight = 86;
};

#endif  // SOFTVIM_BROWSER_WINDOW_H_

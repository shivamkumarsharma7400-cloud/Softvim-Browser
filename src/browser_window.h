// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// browser_window.h — Main browser window with dual-view architecture.

#ifndef SOFTVIM_BROWSER_WINDOW_H_
#define SOFTVIM_BROWSER_WINDOW_H_

#include <windows.h>
#include <string>

#include "include/cef_browser.h"
#include "client_handler.h"
#include "ui_handler.h"

class BrowserWindow {
 public:
  static void Create();
  static BrowserWindow* GetInstance();

  HWND GetHwnd() const { return hwnd_; }

  CefRefPtr<CefBrowser> GetContentBrowser() const;
  CefRefPtr<CefBrowser> GetUiBrowser() const;

  // Actions invoked from UI Chrome
  void NavigateContent(const std::string& url);
  void GoBackContent();
  void GoForwardContent();
  void ReloadContent();

  // Called by Content ClientHandler to sync with UI Chrome
  void NotifyNavigationState(const std::string& url,
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
  CefRefPtr<ClientHandler> content_handler_;

  static BrowserWindow* instance_;

  static const wchar_t kWindowClass[];
  static const wchar_t kWindowTitle[];

  static const int kDefaultWidth = 1280;
  static const int kDefaultHeight = 850;
  static const int kUiChromeHeight = 86; // Height for Tab bar (38px) + Toolbar (48px)
};

#endif  // SOFTVIM_BROWSER_WINDOW_H_

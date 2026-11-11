// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// browser_window.h — Main browser window management.
//
// BrowserWindow is the native Win32 window that hosts our CEF browser.
// For Milestone 1, it creates a single browser view showing a webpage.
// Later milestones will add the dual-view architecture (UI chrome + content).

#ifndef SOFTVIM_BROWSER_WINDOW_H_
#define SOFTVIM_BROWSER_WINDOW_H_

#include <windows.h>
#include <string>

#include "include/cef_browser.h"
#include "client_handler.h"

class BrowserWindow {
 public:
  // Creates and shows the main browser window.
  static void Create();

  // Returns the singleton window instance.
  static BrowserWindow* GetInstance();

  // Returns the native window handle.
  HWND GetHwnd() const { return hwnd_; }

  // Returns the main browser.
  CefRefPtr<CefBrowser> GetBrowser() const;

 private:
  BrowserWindow();
  ~BrowserWindow();

  // Creates the native Win32 window.
  bool CreateNativeWindow();

  // Creates the CEF browser inside the window.
  void CreateBrowser();

  // Win32 window procedure.
  static LRESULT CALLBACK WndProc(HWND hwnd, UINT message,
                                  WPARAM wParam, LPARAM lParam);

  // Handle window resize — adjust browser to fill the window.
  void OnSize();

  // Handle window close — shut down CEF.
  void OnClose();

  HWND hwnd_ = nullptr;
  CefRefPtr<ClientHandler> client_handler_;

  static BrowserWindow* instance_;

  // Window class name and title
  static const wchar_t kWindowClass[];
  static const wchar_t kWindowTitle[];

  // Default window dimensions
  static const int kDefaultWidth = 1280;
  static const int kDefaultHeight = 800;
};

#endif  // SOFTVIM_BROWSER_WINDOW_H_


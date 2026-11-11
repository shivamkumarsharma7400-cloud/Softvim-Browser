// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// tab_manager.h — Multi-tab lifecycle and view management.

#ifndef SOFTVIM_TAB_MANAGER_H_
#define SOFTVIM_TAB_MANAGER_H_

#include <windows.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "include/cef_browser.h"
#include "client_handler.h"

struct TabInfo {
  int id;
  std::string url;
  std::string title;
  CefRefPtr<CefBrowser> browser;
  CefRefPtr<ClientHandler> handler;
  bool is_active;
};

class TabManager {
 public:
  TabManager(HWND parent_hwnd, int top_offset);
  ~TabManager();

  // Tab actions
  int CreateTab(const std::string& url = "https://www.google.com");
  bool CloseTab(int tab_id);
  bool SwitchToTab(int tab_id);

  // Tab state queries
  TabInfo* GetTab(int tab_id);
  TabInfo* GetActiveTab();
  int GetActiveTabId() const { return active_tab_id_; }
  const std::map<int, std::unique_ptr<TabInfo>>& GetAllTabs() const { return tabs_; }

  // Sizing & layout
  void ResizeActiveTab(int width, int height);

  // Navigation helpers targeting the active tab
  void NavigateActiveTab(const std::string& url);
  void GoBackActiveTab();
  void GoForwardActiveTab();
  void ReloadActiveTab();

 private:
  HWND parent_hwnd_;
  int top_offset_;
  int next_tab_id_ = 1;
  int active_tab_id_ = -1;
  std::map<int, std::unique_ptr<TabInfo>> tabs_;
};

#endif  // SOFTVIM_TAB_MANAGER_H_

// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// tab_manager.h — Multi-tab lifecycle and view management with Private Browsing support.

#ifndef SOFTVIM_TAB_MANAGER_H_
#define SOFTVIM_TAB_MANAGER_H_

#include <windows.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "include/cef_browser.h"
#include "include/cef_request_context.h"
#include "include/cef_request_context_handler.h"
#include "client_handler.h"

struct TabInfo {
  int id;
  std::string url;
  std::string title;
  CefRefPtr<CefBrowser> browser;
  CefRefPtr<ClientHandler> handler;
  bool is_active;
  bool is_private;
};

class TabManager {
 public:
  TabManager(HWND parent_hwnd, int top_offset);
  ~TabManager();

  // Tab actions (with private browsing flag)
  int CreateTab(const std::string& url = "", bool is_private = false);
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

  // Dedicated in-memory request context for private/incognito tabs
  CefRefPtr<CefRequestContext> private_request_context_;
};

#endif  // SOFTVIM_TAB_MANAGER_H_

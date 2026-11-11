// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// tab_manager.cc — Multi-tab lifecycle and view management implementation.

#include "tab_manager.h"
#include <algorithm>

TabManager::TabManager(HWND parent_hwnd, int top_offset)
    : parent_hwnd_(parent_hwnd), top_offset_(top_offset) {}

TabManager::~TabManager() {
  tabs_.clear();
}

int TabManager::CreateTab(const std::string& url) {
  int id = next_tab_id_++;
  auto tab = std::make_unique<TabInfo>();
  tab->id = id;
  tab->url = url;
  tab->title = "New Tab";
  tab->handler = new ClientHandler();
  tab->is_active = false;

  RECT rect;
  GetClientRect(parent_hwnd_, &rect);
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;

  CefWindowInfo windowInfo;
  RECT tabRect = {0, top_offset_, width, height};
  windowInfo.SetAsChild(parent_hwnd_, tabRect);

  CefBrowserSettings settings;
  settings.background_color = CefColorSetARGB(255, 0xFF, 0xFF, 0xFF);

  CefBrowserHost::CreateBrowser(
      windowInfo,
      tab->handler,
      url,
      settings,
      nullptr,
      nullptr
  );

  tabs_[id] = std::move(tab);

  // If this is the first tab, make it active
  if (active_tab_id_ == -1) {
    SwitchToTab(id);
  } else {
    // Hide new tab until switched
    TabInfo* created = GetTab(id);
    if (created && created->handler && created->handler->GetBrowser()) {
      HWND tabHwnd = created->handler->GetBrowser()->GetHost()->GetWindowHandle();
      if (tabHwnd) ShowWindow(tabHwnd, SW_HIDE);
    }
  }

  return id;
}

bool TabManager::CloseTab(int tab_id) {
  auto it = tabs_.find(tab_id);
  if (it == tabs_.end()) return false;

  // Close CEF browser instance
  CefRefPtr<CefBrowser> browser = it->second->handler ? it->second->handler->GetBrowser() : nullptr;
  if (browser) {
    browser->GetHost()->CloseBrowser(true);
  }

  bool was_active = (active_tab_id_ == tab_id);
  tabs_.erase(it);

  // Switch to another existing tab if the closed one was active
  if (was_active) {
    if (!tabs_.empty()) {
      SwitchToTab(tabs_.begin()->first);
    } else {
      active_tab_id_ = -1;
    }
  }

  return true;
}

bool TabManager::SwitchToTab(int tab_id) {
  auto it = tabs_.find(tab_id);
  if (it == tabs_.end()) return false;

  // Hide current active tab
  if (active_tab_id_ != -1) {
    auto currentIt = tabs_.find(active_tab_id_);
    if (currentIt != tabs_.end()) {
      currentIt->second->is_active = false;
      CefRefPtr<CefBrowser> curBrowser = currentIt->second->handler ? currentIt->second->handler->GetBrowser() : nullptr;
      if (curBrowser) {
        HWND curHwnd = curBrowser->GetHost()->GetWindowHandle();
        if (curHwnd) ShowWindow(curHwnd, SW_HIDE);
      }
    }
  }

  // Show selected tab
  active_tab_id_ = tab_id;
  it->second->is_active = true;

  RECT rect;
  GetClientRect(parent_hwnd_, &rect);
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;

  CefRefPtr<CefBrowser> nextBrowser = it->second->handler ? it->second->handler->GetBrowser() : nullptr;
  if (nextBrowser) {
    HWND nextHwnd = nextBrowser->GetHost()->GetWindowHandle();
    if (nextHwnd) {
      SetWindowPos(nextHwnd, nullptr, 0, top_offset_, width, (std::max)(0, height - top_offset_), SWP_SHOWWINDOW);
      ShowWindow(nextHwnd, SW_SHOW);
      SetFocus(nextHwnd);
    }
  }

  return true;
}

TabInfo* TabManager::GetTab(int tab_id) {
  auto it = tabs_.find(tab_id);
  return (it != tabs_.end()) ? it->second.get() : nullptr;
}

TabInfo* TabManager::GetActiveTab() {
  return GetTab(active_tab_id_);
}

void TabManager::ResizeActiveTab(int width, int height) {
  TabInfo* active = GetActiveTab();
  if (active && active->handler && active->handler->GetBrowser()) {
    HWND tabHwnd = active->handler->GetBrowser()->GetHost()->GetWindowHandle();
    if (tabHwnd) {
      SetWindowPos(tabHwnd, nullptr, 0, top_offset_, width, (std::max)(0, height - top_offset_), SWP_NOZORDER);
    }
  }
}

void TabManager::NavigateActiveTab(const std::string& url) {
  TabInfo* active = GetActiveTab();
  if (active && active->handler && active->handler->GetBrowser()) {
    CefRefPtr<CefFrame> frame = active->handler->GetBrowser()->GetMainFrame();
    if (frame) frame->LoadURL(url);
  }
}

void TabManager::GoBackActiveTab() {
  TabInfo* active = GetActiveTab();
  if (active && active->handler && active->handler->GetBrowser()) {
    if (active->handler->GetBrowser()->CanGoBack()) {
      active->handler->GetBrowser()->GoBack();
    }
  }
}

void TabManager::GoForwardActiveTab() {
  TabInfo* active = GetActiveTab();
  if (active && active->handler && active->handler->GetBrowser()) {
    if (active->handler->GetBrowser()->CanGoForward()) {
      active->handler->GetBrowser()->GoForward();
    }
  }
}

void TabManager::ReloadActiveTab() {
  TabInfo* active = GetActiveTab();
  if (active && active->handler && active->handler->GetBrowser()) {
    active->handler->GetBrowser()->Reload();
  }
}

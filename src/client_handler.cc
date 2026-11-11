// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// client_handler.cc — CefClient implementation.

#include "client_handler.h"
#include "browser_window.h"

#include <sstream>
#include <string>
#include <windows.h>

#include "include/cef_app.h"
#include "include/wrapper/cef_helpers.h"

ClientHandler::ClientHandler() = default;

void ClientHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title) {
  CEF_REQUIRE_UI_THREAD();

  // Update the window title to show the page title
  BrowserWindow* window = BrowserWindow::GetInstance();
  if (window && window->GetHwnd()) {
    // Format: "Page Title — Softvim Browser"
    std::wstring window_title = title.ToWString();
    if (!window_title.empty()) {
      window_title += L" \u2014 Softvim Browser";
    } else {
      window_title = L"Softvim Browser";
    }
    SetWindowTextW(window->GetHwnd(), window_title.c_str());
  }
}

void ClientHandler::OnAddressChange(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    const CefString& url) {
  CEF_REQUIRE_UI_THREAD();

  // For Milestone 1, we only track the main frame URL.
  // In Milestone 2, this will update the address bar UI.
  if (frame->IsMain()) {
    // URL tracking will be used by the UI chrome in later milestones.
  }
}

void ClientHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  if (!browser_) {
    // Keep a reference to the main browser.
    browser_ = browser;
  }
  browser_count_++;
}

bool ClientHandler::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // If this is the last browser, allow close and destroy the window.
  if (browser_count_ == 1) {
    // Set a flag so we know the window is closing.
    // Allow the close to proceed.
  }

  // Allow the close. Return false to proceed with closing.
  return false;
}

void ClientHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  if (browser->IsSame(browser_)) {
    // Our main browser is closing. Release the reference.
    browser_ = nullptr;
  }

  browser_count_--;

  if (browser_count_ == 0) {
    // All browsers are closed. Quit the message loop.
    // This causes CefRunMessageLoop() in main.cc to return.
    CefQuitMessageLoop();

    // Destroy the native window
    BrowserWindow* window = BrowserWindow::GetInstance();
    if (window && window->GetHwnd()) {
      DestroyWindow(window->GetHwnd());
    }
  }
}

void ClientHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& errorText,
                                const CefString& failedUrl) {
  CEF_REQUIRE_UI_THREAD();

  // Don't display an error page for aborted navigations (e.g., download links)
  if (errorCode == ERR_ABORTED) {
    return;
  }

  // Show a minimal, clean error page styled to match our design.
  std::stringstream ss;
  ss << "<!DOCTYPE html>"
     << "<html><head>"
     << "<meta charset=\"utf-8\">"
     << "<style>"
     << "  * { margin: 0; padding: 0; box-sizing: border-box; }"
     << "  body {"
     << "    font-family: 'Segoe UI', system-ui, -apple-system, sans-serif;"
     << "    background: #1C1C1E; color: #F5F5F7;"
     << "    display: flex; align-items: center; justify-content: center;"
     << "    min-height: 100vh;"
     << "    padding: 2rem;"
     << "  }"
     << "  .error-container {"
     << "    text-align: center; max-width: 480px;"
     << "  }"
     << "  .error-icon {"
     << "    font-size: 48px; margin-bottom: 1.5rem; opacity: 0.5;"
     << "  }"
     << "  h1 {"
     << "    font-size: 20px; font-weight: 600;"
     << "    margin-bottom: 0.75rem; color: #F5F5F7;"
     << "  }"
     << "  p {"
     << "    font-size: 14px; color: #98989D;"
     << "    line-height: 1.6;"
     << "  }"
     << "  .url {"
     << "    font-size: 12px; color: #636366;"
     << "    margin-top: 1.5rem; word-break: break-all;"
     << "  }"
     << "</style></head><body>"
     << "<div class=\"error-container\">"
     << "  <div class=\"error-icon\">\xe2\x9a\xa0</div>"
     << "  <h1>This page can\xe2\x80\x99t be reached</h1>"
     << "  <p>" << errorText.ToString() << "</p>"
     << "  <p class=\"url\">" << failedUrl.ToString() << "</p>"
     << "</div></body></html>";

  frame->LoadURL(ResourceUtil::GetDataURI(ss.str(), "text/html"));
}


// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// client_handler.cc — Implementation of ClientHandler for tab browsers.

#include "client_handler.h"
#include "browser_window.h"

#include <sstream>
#include <string>
#include <windows.h>

#include "include/cef_app.h"
#include "include/wrapper/cef_helpers.h"
#include "resource_util.h"

ClientHandler::ClientHandler() = default;

void ClientHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title) {
  CEF_REQUIRE_UI_THREAD();
  BrowserWindow* window = BrowserWindow::GetInstance();
  if (window) {
    int browserId = browser->GetIdentifier();
    std::string currentUrl = browser->GetMainFrame() ? browser->GetMainFrame()->GetURL().ToString() : "";
    window->NotifyNavigationState(browserId, currentUrl, title.ToString(), browser->CanGoBack(), browser->CanGoForward());
  }
}

void ClientHandler::OnAddressChange(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    const CefString& url) {
  CEF_REQUIRE_UI_THREAD();
  if (frame->IsMain()) {
    BrowserWindow* window = BrowserWindow::GetInstance();
    if (window) {
      int browserId = browser->GetIdentifier();
      window->NotifyNavigationState(browserId, url.ToString(), "", browser->CanGoBack(), browser->CanGoForward());
    }
  }
}

void ClientHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  if (!browser_) {
    browser_ = browser;
  }
}

bool ClientHandler::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  return false;
}

void ClientHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  if (browser->IsSame(browser_)) {
    browser_ = nullptr;
  }
}

void ClientHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& errorText,
                                const CefString& failedUrl) {
  CEF_REQUIRE_UI_THREAD();
  if (errorCode == ERR_ABORTED) return;

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
     << "  .error-container { text-align: center; max-width: 480px; }"
     << "  .error-icon { font-size: 48px; margin-bottom: 1.5rem; opacity: 0.5; }"
     << "  h1 { font-size: 20px; font-weight: 600; margin-bottom: 0.75rem; color: #F5F5F7; }"
     << "  p { font-size: 14px; color: #98989D; line-height: 1.6; }"
     << "  .url { font-size: 12px; color: #636366; margin-top: 1.5rem; word-break: break-all; }"
     << "</style></head><body>"
     << "<div class=\"error-container\">"
     << "  <div class=\"error-icon\">\xe2\x9a\xa0</div>"
     << "  <h1>This page can\xe2\x80\x99t be reached</h1>"
     << "  <p>" << errorText.ToString() << "</p>"
     << "  <p class=\"url\">" << failedUrl.ToString() << "</p>"
     << "</div></body></html>";

  frame->LoadURL(ResourceUtil::GetDataURI(ss.str(), "text/html"));
}

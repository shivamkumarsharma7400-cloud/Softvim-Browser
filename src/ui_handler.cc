// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// ui_handler.cc — Implementation of UiHandler.

#include "ui_handler.h"
#include "browser_window.h"

#include <sstream>
#include "include/wrapper/cef_helpers.h"

namespace {

// Escapes special characters for dynamic JavaScript strings
std::string EscapeJsonString(const std::string& str) {
  std::string res;
  for (char c : str) {
    if (c == '"') res += "\\\"";
    else if (c == '\\') res += "\\\\";
    else if (c == '\n') res += "\\n";
    else if (c == '\r') res += "\\r";
    else res += c;
  }
  return res;
}

class UiQueryHandler : public CefMessageRouterBrowserSide::Handler {
 public:
  bool OnQuery(CefRefPtr<CefBrowser> browser,
               CefRefPtr<CefFrame> frame,
               int64_t query_id,
               const CefString& request,
               bool persistent,
               CefRefPtr<Callback> callback) override {
    std::string req = request.ToString();
    BrowserWindow* window = BrowserWindow::GetInstance();
    if (!window) return false;

    // Navigation Actions
    if (req.find("\"action\":\"navigate\"") != std::string::npos) {
      size_t urlPos = req.find("\"url\":\"");
      if (urlPos != std::string::npos) {
        urlPos += 7;
        size_t endPos = req.find("\"", urlPos);
        if (endPos != std::string::npos) {
          std::string url = req.substr(urlPos, endPos - urlPos);
          window->NavigateActiveTab(url);
          callback->Success("");
          return true;
        }
      }
    } else if (req.find("\"action\":\"back\"") != std::string::npos) {
      window->GoBackActiveTab();
      callback->Success("");
      return true;
    } else if (req.find("\"action\":\"forward\"") != std::string::npos) {
      window->GoForwardActiveTab();
      callback->Success("");
      return true;
    } else if (req.find("\"action\":\"reload\"") != std::string::npos) {
      window->ReloadActiveTab();
      callback->Success("");
      return true;
    }

    // Tab Actions
    else if (req.find("\"action\":\"new_tab\"") != std::string::npos) {
      int new_id = window->CreateTab();
      callback->Success(std::to_string(new_id));
      return true;
    } else if (req.find("\"action\":\"switch_tab\"") != std::string::npos) {
      size_t idPos = req.find("\"id\":");
      if (idPos != std::string::npos) {
        int tab_id = std::stoi(req.substr(idPos + 5));
        window->SwitchToTab(tab_id);
        callback->Success("");
        return true;
      }
    } else if (req.find("\"action\":\"close_tab\"") != std::string::npos) {
      size_t idPos = req.find("\"id\":");
      if (idPos != std::string::npos) {
        int tab_id = std::stoi(req.substr(idPos + 5));
        window->CloseTab(tab_id);
        callback->Success("");
        return true;
      }
    }

    callback->Failure(1, "Unknown action");
    return true;
  }
};

}  // namespace

UiHandler::UiHandler() {
  CefMessageRouterConfig config;
  message_router_ = CefMessageRouterBrowserSide::Create(config);
  message_router_->AddHandler(new UiQueryHandler(), true);
}

bool UiHandler::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) {
  CEF_REQUIRE_UI_THREAD();
  return message_router_->OnProcessMessageReceived(browser, frame, source_process, message);
}

void UiHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  if (!browser_) {
    browser_ = browser;
  }
}

void UiHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  message_router_->OnBeforeClose(browser);
  if (browser_->IsSame(browser)) {
    browser_ = nullptr;
  }
}

void UiHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                          CefRefPtr<CefFrame> frame,
                          int httpStatusCode) {
  CEF_REQUIRE_UI_THREAD();
}

void UiHandler::UpdateBrowserState(int tab_id,
                                   const std::string& url,
                                   const std::string& title,
                                   bool canGoBack,
                                   bool canGoForward) {
  CEF_REQUIRE_UI_THREAD();
  if (!browser_) return;

  CefRefPtr<CefFrame> frame = browser_->GetMainFrame();
  if (!frame) return;

  std::stringstream js;
  js << "if (window.updateBrowserState) { window.updateBrowserState({"
     << "tabId: " << tab_id << ","
     << "url: \"" << EscapeJsonString(url) << "\","
     << "title: \"" << EscapeJsonString(title) << "\","
     << "canGoBack: " << (canGoBack ? "true" : "false") << ","
     << "canGoForward: " << (canGoForward ? "true" : "false")
     << "}); }";

  frame->ExecuteJavaScript(js.str(), frame->GetURL(), 0);
}

void UiHandler::OnTabCreated(int tab_id, const std::string& title, const std::string& url) {
  CEF_REQUIRE_UI_THREAD();
  if (!browser_) return;

  CefRefPtr<CefFrame> frame = browser_->GetMainFrame();
  if (!frame) return;

  std::stringstream js;
  js << "if (window.onTabCreated) { window.onTabCreated({"
     << "id: " << tab_id << ","
     << "title: \"" << EscapeJsonString(title) << "\","
     << "url: \"" << EscapeJsonString(url) << "\""
     << "}); }";

  frame->ExecuteJavaScript(js.str(), frame->GetURL(), 0);
}

void UiHandler::OnTabClosed(int closed_id, int new_active_id) {
  CEF_REQUIRE_UI_THREAD();
  if (!browser_) return;

  CefRefPtr<CefFrame> frame = browser_->GetMainFrame();
  if (!frame) return;

  std::stringstream js;
  js << "if (window.onTabClosed) { window.onTabClosed(" << closed_id << ", " << new_active_id << "); }";
  frame->ExecuteJavaScript(js.str(), frame->GetURL(), 0);
}

void UiHandler::OnTabSwitched(int tab_id, const std::string& url, const std::string& title, bool canGoBack, bool canGoForward) {
  CEF_REQUIRE_UI_THREAD();
  if (!browser_) return;

  CefRefPtr<CefFrame> frame = browser_->GetMainFrame();
  if (!frame) return;

  std::stringstream js;
  js << "if (window.onTabSwitched) { window.onTabSwitched({"
     << "id: " << tab_id << ","
     << "url: \"" << EscapeJsonString(url) << "\","
     << "title: \"" << EscapeJsonString(title) << "\","
     << "canGoBack: " << (canGoBack ? "true" : "false") << ","
     << "canGoForward: " << (canGoForward ? "true" : "false")
     << "}); }";

  frame->ExecuteJavaScript(js.str(), frame->GetURL(), 0);
}

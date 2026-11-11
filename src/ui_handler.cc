// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// ui_handler.cc — Implementation of UiHandler.

#include "ui_handler.h"
#include "browser_window.h"

#include <sstream>
#include "include/wrapper/cef_helpers.h"

namespace {

// Handles cefQuery calls from the UI JavaScript layer
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

    // Simple JSON parsing for our known commands
    if (req.find("\"action\":\"navigate\"") != std::string::npos) {
      size_t urlPos = req.find("\"url\":\"");
      if (urlPos != std::string::npos) {
        urlPos += 7;
        size_t endPos = req.find("\"", urlPos);
        if (endPos != std::string::npos) {
          std::string url = req.substr(urlPos, endPos - urlPos);
          window->NavigateContent(url);
          callback->Success("");
          return true;
        }
      }
    } else if (req.find("\"action\":\"back\"") != std::string::npos) {
      window->GoBackContent();
      callback->Success("");
      return true;
    } else if (req.find("\"action\":\"forward\"") != std::string::npos) {
      window->GoForwardContent();
      callback->Success("");
      return true;
    } else if (req.find("\"action\":\"reload\"") != std::string::npos) {
      window->ReloadContent();
      callback->Success("");
      return true;
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

void UiHandler::UpdateBrowserState(const std::string& url,
                                   const std::string& title,
                                   bool canGoBack,
                                   bool canGoForward) {
  CEF_REQUIRE_UI_THREAD();
  if (!browser_) return;

  CefRefPtr<CefFrame> frame = browser_->GetMainFrame();
  if (!frame) return;

  // Escape quotes in string data
  auto escapeQuotes = [](const std::string& str) {
    std::string res;
    for (char c : str) {
      if (c == '"') res += "\\\"";
      else if (c == '\\') res += "\\\\";
      else res += c;
    }
    return res;
  };

  std::stringstream js;
  js << "if (window.updateBrowserState) { window.updateBrowserState({"
     << "url: \"" << escapeQuotes(url) << "\","
     << "title: \"" << escapeQuotes(title) << "\","
     << "canGoBack: " << (canGoBack ? "true" : "false") << ","
     << "canGoForward: " << (canGoForward ? "true" : "false")
     << "}); }";

  frame->ExecuteJavaScript(js.str(), frame->GetURL(), 0);
}

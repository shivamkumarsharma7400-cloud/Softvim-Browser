// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// ui_handler.h — CefClient implementation for the browser UI chrome view.

#ifndef SOFTVIM_UI_HANDLER_H_
#define SOFTVIM_UI_HANDLER_H_

#include "include/cef_client.h"
#include "include/cef_life_span_handler.h"
#include "include/cef_load_handler.h"
#include "include/wrapper/cef_message_router.h"

class UiHandler : public CefClient,
                  public CefLifeSpanHandler,
                  public CefLoadHandler {
 public:
  UiHandler();

  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
  CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }
  bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefProcessId source_process,
                                CefRefPtr<CefProcessMessage> message) override;

  // CefLifeSpanHandler methods
  void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

  // CefLoadHandler methods
  void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                 CefRefPtr<CefFrame> frame,
                 int httpStatusCode) override;

  // Helper to send state updates (url, title, nav buttons) to JavaScript UI
  void UpdateBrowserState(const std::string& url,
                          const std::string& title,
                          bool canGoBack,
                          bool canGoForward);

  CefRefPtr<CefBrowser> GetBrowser() const { return browser_; }

 private:
  CefRefPtr<CefBrowser> browser_;
  CefRefPtr<CefMessageRouterBrowserSide> message_router_;

  IMPLEMENT_REFCOUNTING(UiHandler);
  DISALLOW_COPY_AND_ASSIGN(UiHandler);
};

#endif  // SOFTVIM_UI_HANDLER_H_

// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// client_handler.h — CefClient implementation for content browser views.
//
// The ClientHandler receives callbacks from CEF about browser lifecycle,
// display changes, loading status, etc. For Milestone 1, this is a single
// handler for the one browser view.

#ifndef SOFTVIM_CLIENT_HANDLER_H_
#define SOFTVIM_CLIENT_HANDLER_H_

#include "include/cef_client.h"
#include "include/cef_display_handler.h"
#include "include/cef_life_span_handler.h"
#include "include/cef_load_handler.h"

class ClientHandler : public CefClient,
                      public CefDisplayHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler {
 public:
  ClientHandler();

  // CefClient methods — return handlers
  CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
  CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }

  // CefDisplayHandler methods
  void OnTitleChange(CefRefPtr<CefBrowser> browser,
                     const CefString& title) override;
  void OnAddressChange(CefRefPtr<CefBrowser> browser,
                       CefRefPtr<CefFrame> frame,
                       const CefString& url) override;

  // CefLifeSpanHandler methods
  void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  bool DoClose(CefRefPtr<CefBrowser> browser) override;
  void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

  // CefLoadHandler methods
  void OnLoadError(CefRefPtr<CefBrowser> browser,
                   CefRefPtr<CefFrame> frame,
                   ErrorCode errorCode,
                   const CefString& errorText,
                   const CefString& failedUrl) override;

  // Returns the main browser instance.
  CefRefPtr<CefBrowser> GetBrowser() const { return browser_; }

 private:
  CefRefPtr<CefBrowser> browser_;
  int browser_count_ = 0;

  IMPLEMENT_REFCOUNTING(ClientHandler);
  DISALLOW_COPY_AND_ASSIGN(ClientHandler);
};

#endif  // SOFTVIM_CLIENT_HANDLER_H_


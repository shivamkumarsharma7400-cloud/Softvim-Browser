// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// app.h — CefApp implementation for Softvim Browser.
//
// CefApp is the entry point that CEF uses to configure the browser and
// renderer processes. We implement it to set up our custom browser behavior.

#ifndef SOFTVIM_APP_H_
#define SOFTVIM_APP_H_

#include "include/cef_app.h"

// SoftvimApp handles the browser process lifecycle.
// It configures CEF settings and creates browser-process handlers.
class SoftvimApp : public CefApp,
                   public CefBrowserProcessHandler {
 public:
  SoftvimApp();

  // CefApp methods
  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
    return this;
  }

  void OnBeforeCommandLineProcessing(
      const CefString& process_type,
      CefRefPtr<CefCommandLine> command_line) override;

  // CefBrowserProcessHandler methods
  void OnContextInitialized() override;

 private:
  IMPLEMENT_REFCOUNTING(SoftvimApp);
  DISALLOW_COPY_AND_ASSIGN(SoftvimApp);
};

// SoftvimRendererApp handles the renderer subprocess.
// Kept minimal — the renderer just needs to exist.
class SoftvimRendererApp : public CefApp {
 public:
  SoftvimRendererApp() = default;

 private:
  IMPLEMENT_REFCOUNTING(SoftvimRendererApp);
  DISALLOW_COPY_AND_ASSIGN(SoftvimRendererApp);
};

#endif  // SOFTVIM_APP_H_


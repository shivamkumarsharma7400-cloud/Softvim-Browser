// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// app.h — CefApp declaration for browser and subprocesses.

#ifndef SOFTVIM_APP_H_
#define SOFTVIM_APP_H_

#include "include/cef_app.h"

// SoftvimApp handles the main browser process.
class SoftvimApp : public CefApp,
                   public CefBrowserProcessHandler {
 public:
  SoftvimApp();

  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
    return this;
  }

  void OnBeforeCommandLineProcessing(
      const CefString& process_type,
      CefRefPtr<CefCommandLine> command_line) override;

  void OnContextInitialized() override;

 private:
  IMPLEMENT_REFCOUNTING(SoftvimApp);
  DISALLOW_COPY_AND_ASSIGN(SoftvimApp);
};

// SoftvimRendererApp handles renderer / GPU / utility subprocesses.
class SoftvimRendererApp : public CefApp {
 public:
  SoftvimRendererApp() = default;

 private:
  IMPLEMENT_REFCOUNTING(SoftvimRendererApp);
  DISALLOW_COPY_AND_ASSIGN(SoftvimRendererApp);
};

#endif  // SOFTVIM_APP_H_

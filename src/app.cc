// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// app.cc — CefApp implementation.

#include "app.h"
#include "browser_window.h"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/wrapper/cef_helpers.h"

SoftvimApp::SoftvimApp() = default;

void SoftvimApp::OnBeforeCommandLineProcessing(
    const CefString& process_type,
    CefRefPtr<CefCommandLine> command_line) {
  // These flags optimize the browser for our use case.

  // Disable GPU shader disk cache to reduce disk writes.
  command_line->AppendSwitchWithValue("disable-gpu-shader-disk-cache", "1");

  // Enable smooth scrolling for a premium feel.
  command_line->AppendSwitchWithValue("enable-smooth-scrolling", "1");
}

void SoftvimApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  // CEF is fully initialized. Create the main browser window.
  BrowserWindow::Create();
}


// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// app.cc — CefApp and CefRenderProcessHandler implementation.

#include "app.h"
#include "browser_window.h"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/wrapper/cef_helpers.h"

// ──────────────────────────────────────────────
// SoftvimApp (Main Process + In-process Renderer)
// ──────────────────────────────────────────────

SoftvimApp::SoftvimApp() = default;

void SoftvimApp::OnBeforeCommandLineProcessing(
    const CefString& process_type,
    CefRefPtr<CefCommandLine> command_line) {
  command_line->AppendSwitchWithValue("disable-gpu-shader-disk-cache", "1");
  command_line->AppendSwitchWithValue("enable-smooth-scrolling", "1");
  command_line->AppendSwitch("allow-file-access-from-files");
}

void SoftvimApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();
  BrowserWindow::Create();
}

void SoftvimApp::OnWebKitInitialized() {
  CefMessageRouterConfig config;
  message_router_ = CefMessageRouterRendererSide::Create(config);
}

void SoftvimApp::OnContextCreated(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefV8Context> context) {
  if (message_router_) {
    message_router_->OnContextCreated(browser, frame, context);
  }
}

void SoftvimApp::OnContextReleased(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefFrame> frame,
                                   CefRefPtr<CefV8Context> context) {
  if (message_router_) {
    message_router_->OnContextReleased(browser, frame, context);
  }
}

bool SoftvimApp::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) {
  if (message_router_) {
    return message_router_->OnProcessMessageReceived(browser, frame, source_process, message);
  }
  return false;
}

// ──────────────────────────────────────────────
// SoftvimRendererApp (Subprocess Helper)
// ──────────────────────────────────────────────

SoftvimRendererApp::SoftvimRendererApp() = default;

void SoftvimRendererApp::OnWebKitInitialized() {
  CefMessageRouterConfig config;
  message_router_ = CefMessageRouterRendererSide::Create(config);
}

void SoftvimRendererApp::OnContextCreated(CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefFrame> frame,
                                          CefRefPtr<CefV8Context> context) {
  if (message_router_) {
    message_router_->OnContextCreated(browser, frame, context);
  }
}

void SoftvimRendererApp::OnContextReleased(CefRefPtr<CefBrowser> browser,
                                           CefRefPtr<CefFrame> frame,
                                           CefRefPtr<CefV8Context> context) {
  if (message_router_) {
    message_router_->OnContextReleased(browser, frame, context);
  }
}

bool SoftvimRendererApp::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) {
  if (message_router_) {
    return message_router_->OnProcessMessageReceived(browser, frame, source_process, message);
  }
  return false;
}

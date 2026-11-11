// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// subprocess_main.cc — Self-contained entry point for CEF subprocess helper.

#include <windows.h>
#include "include/cef_app.h"
#include "include/wrapper/cef_message_router.h"

// Dedicated renderer app with message router support
class HelperRendererApp : public CefApp,
                          public CefRenderProcessHandler {
 public:
  HelperRendererApp() = default;

  CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
    return this;
  }

  void OnWebKitInitialized() override {
    CefMessageRouterConfig config;
    message_router_ = CefMessageRouterRendererSide::Create(config);
  }

  void OnContextCreated(CefRefPtr<CefBrowser> browser,
                        CefRefPtr<CefFrame> frame,
                        CefRefPtr<CefV8Context> context) override {
    if (message_router_) {
      message_router_->OnContextCreated(browser, frame, context);
    }
  }

  void OnContextReleased(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         CefRefPtr<CefV8Context> context) override {
    if (message_router_) {
      message_router_->OnContextReleased(browser, frame, context);
    }
  }

  bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefProcessId source_process,
                                CefRefPtr<CefProcessMessage> message) override {
    if (message_router_) {
      return message_router_->OnProcessMessageReceived(browser, frame, source_process, message);
    }
    return false;
  }

 private:
  CefRefPtr<CefMessageRouterRendererSide> message_router_;

  IMPLEMENT_REFCOUNTING(HelperRendererApp);
  DISALLOW_COPY_AND_ASSIGN(HelperRendererApp);
};

int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPWSTR lpCmdLine,
                      int nCmdShow) {
  CefMainArgs main_args(hInstance);
  CefRefPtr<CefApp> app = new HelperRendererApp();
  return CefExecuteProcess(main_args, app, nullptr);
}

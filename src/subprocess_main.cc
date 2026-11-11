// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// subprocess_main.cc — Entry point for CEF subprocess helper.

#include <windows.h>
#include "app.h"
#include "include/cef_app.h"

int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPWSTR lpCmdLine,
                      int nCmdShow) {
  CefMainArgs main_args(hInstance);
  CefRefPtr<CefApp> app = new SoftvimRendererApp();
  return CefExecuteProcess(main_args, app, nullptr);
}

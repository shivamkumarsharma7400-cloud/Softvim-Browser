// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// subprocess_main.cc — Entry point for CEF subprocess helper.
//
// CEF runs multiple processes (renderer, GPU, network, etc.)
// This helper executable handles all non-browser subprocesses.
// It's a minimal entry point that delegates everything to CEF.

#include <windows.h>
#include "app.h"
#include "include/cef_app.h"

int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPWSTR lpCmdLine,
                      int nCmdShow) {
  CefMainArgs main_args(hInstance);

  // Create a minimal app for the subprocess.
  CefRefPtr<CefApp> app = new SoftvimRendererApp();

  // Execute the subprocess and return its exit code.
  return CefExecuteProcess(main_args, app, nullptr);
}


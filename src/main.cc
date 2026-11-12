// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// main.cc — Entry point for Softvim Browser.
//
// This is the WinMain function that starts everything:
// 1. Initialize CEF with our custom SoftvimApp.
// 2. CEF creates subprocesses (renderer, GPU, etc.) automatically.
// 3. The main browser process runs the message loop until quit.

#include <windows.h>
#include <shlwapi.h>
#include <shobjidl.h>

#include "app.h"
#include "include/cef_app.h"
#include "include/cef_command_line.h"

// Entry point for Windows GUI applications.
int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPWSTR lpCmdLine,
                      int nCmdShow) {
  // Explicitly associate process with Softvim Browser for Windows taskbar branding
  SetCurrentProcessExplicitAppUserModelID(L"Softvim.SoftvimBrowser.App");

  // Provide CEF with command-line arguments.
  CefMainArgs main_args(hInstance);

  // Parse the command line to determine if this is a subprocess.
  CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
  command_line->InitFromString(GetCommandLineW());

  // Determine the process type. CEF launches multiple processes
  // (browser, renderer, GPU, etc.) all from the same executable.
  // The subprocess helper handles non-browser processes.
  CefRefPtr<CefApp> app;
  std::string process_type = command_line->GetSwitchValue("type").ToString();

  if (process_type.empty()) {
    // This is the main browser process.
    app = new SoftvimApp();
  } else {
    // This is a subprocess (renderer, GPU, utility, etc.)
    app = new SoftvimRendererApp();
  }

  // Execute the subprocess if this is one.
  // CefExecuteProcess returns >= 0 for subprocesses and < 0 for the browser.
  int exit_code = CefExecuteProcess(main_args, app, nullptr);
  if (exit_code >= 0) {
    // This was a subprocess. Exit with its return code.
    return exit_code;
  }

  // ── From here on, we are the main browser process ──

  // Configure CEF settings.
  CefSettings settings;
  settings.no_sandbox = true;

  // Specify the dedicated subprocess helper path
  wchar_t helperPath[MAX_PATH];
  GetModuleFileNameW(nullptr, helperPath, MAX_PATH);
  PathRemoveFileSpecW(helperPath);
  PathAppendW(helperPath, L"SoftvimHelper.exe");
  if (GetFileAttributesW(helperPath) != INVALID_FILE_ATTRIBUTES) {
    CefString(&settings.browser_subprocess_path) = helperPath;
  }

  // Set the log severity (only warnings and errors in release).
#ifdef NDEBUG
  settings.log_severity = LOGSEVERITY_WARNING;
#else
  settings.log_severity = LOGSEVERITY_INFO;
#endif

  // Background color for the browser before any page loads.
  // Using our dark theme background to avoid white flash.
  settings.background_color = CefColorSetARGB(255, 0x1C, 0x1C, 0x1E);

  // Initialize CEF.
  // This starts the browser process and calls SoftvimApp::OnContextInitialized
  // when everything is ready.
  if (!CefInitialize(main_args, settings, app, nullptr)) {
    return 1;
  }

  // Run the CEF message loop.
  // This blocks until CefQuitMessageLoop() is called (from our ClientHandler
  // when the last browser window closes).
  CefRunMessageLoop();

  // Shut down CEF cleanly.
  CefShutdown();

  return 0;
}


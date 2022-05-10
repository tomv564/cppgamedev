#include <functional>
#include <iostream>

#include <spdlog/spdlog.h>
#include <docopt/docopt.h>

#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>

static constexpr auto USAGE =
  R"(Naval Fate.

    Usage:
          naval_fate ship new <name>...
          naval_fate ship <name> move <x> <y> [--speed=<kn>]
          naval_fate ship shoot <x> <y>
          naval_fate mine (set|remove) <x> <y> [--moored | --drifting]
          naval_fate (-h | --help)
          naval_fate --version
 Options:
          -h --help     Show this screen.
          --version     Show version.
          --speed=<kn>  Speed in knots [default: 10].
          --moored      Moored (anchored) mine.
          --drifting    Drifting mine.
)";

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


int main(int argc, const char **argv)
{
  //std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
  //  { std::next(argv), std::next(argv, argc) },
  //  true,// show help if requested
  //  "Naval Fate 2.0");// version string

  //for (auto const &arg : args) {
  //  std::cout << arg.first << "=" << arg.second << std::endl;
  //}


  //Use the default logger (stdout, multi-threaded, colored)
  spdlog::info("Hello, {}!", "World");

  fmt::print("Hello, from {}\n", "{fmt}");


      // create window (https://docs.microsoft.com/en-us/windows/win32/learnwin32/your-first-windows-program)
  
      // Register the window class.
      const wchar_t CLASS_NAME[] = L"Sample Window Class";
  
      HINSTANCE hInstance = GetModuleHandle(NULL);

      WNDCLASS wc = {};
  
      wc.lpfnWndProc = WindowProc;
      wc.hInstance = hInstance;
      wc.lpszClassName = CLASS_NAME;
  
      RegisterClass(&wc);
  
      // Create the window.
  
      HWND hwnd = nullptr;
      hwnd = CreateWindowEx(
        0,// Optional window styles.
        CLASS_NAME,// Window class
        L"Learn to Program Windows",// Window text
        WS_OVERLAPPEDWINDOW,// Window style
  
        // Size and position
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
  
        NULL,// Parent window
        NULL,// Menu
        hInstance,// Instance handle
        NULL// Additional application data
      );
  
      if (hwnd == nullptr) {
        return 0;
      }
  
      ShowWindow(hwnd, SW_SHOWDEFAULT);

      // main loop
      for (;;) {
        MSG msg = {};
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
          if (msg.message == WM_QUIT) {
            // Cleanup
            /*delete gWorkloadD3D11;
            delete gWorkloadD3D12;
            delete gWorkloadDE;
            SafeRelease(&gDXGIFactory);
            timeEndPeriod(1);
            EnableMouseInPointer(FALSE);*/
            return (int)msg.wParam;
          };

          TranslateMessage(&msg);
          DispatchMessage(&msg);
        }
      }



}
//
//
//
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
//{
//    // TODO: linker subsystem should be windows (https://stackoverflow.com/questions/33400777/error-lnk2019-unresolved-external-symbol-main-referenced-in-function-int-cde)
//
//    // create window (https://docs.microsoft.com/en-us/windows/win32/learnwin32/your-first-windows-program)
//    
//    // Register the window class.
//    const wchar_t CLASS_NAME[] = L"Sample Window Class";
//
//    WNDCLASS wc = {};
//
//    wc.lpfnWndProc = WindowProc;
//    wc.hInstance = hInstance;
//    wc.lpszClassName = CLASS_NAME;
//
//    RegisterClass(&wc);
//
//    // Create the window.
//
//    HWND hwnd = CreateWindowEx(
//      0,// Optional window styles.
//      CLASS_NAME,// Window class
//      L"Learn to Program Windows",// Window text
//      WS_OVERLAPPEDWINDOW,// Window style
//
//      // Size and position
//      CW_USEDEFAULT,
//      CW_USEDEFAULT,
//      CW_USEDEFAULT,
//      CW_USEDEFAULT,
//
//      NULL,// Parent window
//      NULL,// Menu
//      hInstance,// Instance handle
//      NULL// Additional application data
//    );
//
//    if (hwnd == NULL) {
//      return 0;
//    }
//
//    ShowWindow(hwnd, nCmdShow);
//
//    // Run the message loop.
//
//    MSG msg = {};
//    while (GetMessage(&msg, NULL, 0, 0) > 0) {
//      TranslateMessage(&msg);
//      DispatchMessage(&msg);
//    }
//
//    return 0;
//}
//
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;

  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    // All painting occurs here, between BeginPaint and EndPaint.

    FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

    EndPaint(hwnd, &ps);
  }
    return 0;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
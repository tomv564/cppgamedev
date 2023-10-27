#include <functional>
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>
#include <filesystem>

#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include "GameApp.hpp"
#include <gainput/gainput.h>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

std::unique_ptr<GameApp> g_pTheApp;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Define your user buttons
enum Button
{
  ButtonMenu,
  ButtonConfirm,
  MouseX,
  MouseY
};

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int cmdShow)
{
  g_pTheApp = std::make_unique<GameApp>();


  // create window (https://docs.microsoft.com/en-us/windows/win32/learnwin32/your-first-windows-program)

  // Register the window class.
  const wchar_t CLASS_NAME[] = L"Sample Window Class";

  // HINSTANCE hInstance = GetModuleHandle(NULL);

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
    L"Game Window",// Window text
    WS_OVERLAPPEDWINDOW,// Window style

    // Size and position
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    800, // CW_USEDEFAULT,
    600, // CW_USEDEFAULT,

    NULL,// Parent window
    NULL,// Menu
    hInstance,// Instance handle
    NULL// Additional application data
  );

  if (hwnd == nullptr) {
    return 0;
  }

  if (!g_pTheApp->InitializeDiligentEngine(hwnd))
  {
    return 0;
  }

  // Setup Gainput
  gainput::InputManager manager;
  manager.SetDisplaySize(800, 600);
  gainput::DeviceId mouseId = manager.CreateDevice<gainput::InputDeviceMouse>();
  gainput::DeviceId keyboardId = manager.CreateDevice<gainput::InputDeviceKeyboard>();
  gainput::DeviceId padId = manager.CreateDevice<gainput::InputDevicePad>();

  gainput::InputMap map(manager);
  map.MapBool(ButtonMenu, keyboardId, gainput::KeyEscape);
  map.MapBool(ButtonConfirm, mouseId, gainput::MouseButtonLeft);
  map.MapFloat(MouseX, mouseId, gainput::MouseAxisX);
  map.MapFloat(MouseY, mouseId, gainput::MouseAxisY);
  map.MapBool(ButtonConfirm, padId, gainput::PadButtonA);

  // Set up audio
  ma_result result;
  ma_engine engine;

  result = ma_engine_init(NULL, &engine);
  if (result != MA_SUCCESS) {
      printf("Failed to initialize audio engine.");
      return -1;
  }

  // Set up UI
  g_pTheApp->InitializeUIRenderer();  
  g_pTheApp->LoadTextures();
  g_pTheApp->BuildUI();
  
  ShowWindow(hwnd, SW_SHOWDEFAULT);

  // main loop
  for (;;) {
    MSG msg = {};

    manager.Update();

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        return (int)msg.wParam;
      };

      TranslateMessage(&msg);
      DispatchMessage(&msg);

      manager.HandleMessage(msg);
    }

    
    if (map.GetBoolWasDown(ButtonConfirm))
    {
      spdlog::info("Confirmed!!");
      ma_engine_play_sound(&engine, "mixkit-select-click-1109.wav", NULL);
    }

    if (map.GetBoolWasDown(ButtonMenu)) {
      spdlog::info("Open menu!!");
    }

    if (map.GetFloatDelta(MouseX) != 0.0f || map.GetFloatDelta(MouseY) != 0.0f)
    {
      spdlog::info("Mouse: %f, %f\n", map.GetFloat(MouseX), map.GetFloat(MouseY));
    }
    
    g_pTheApp->Update();
    g_pTheApp->Render();
    g_pTheApp->Present();
  }


  ma_engine_uninit(&engine);
}

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
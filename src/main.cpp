#include <functional>
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>
#include <docopt/docopt.h>

#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>


// DiligentEngine needs

#ifndef PLATFORM_WIN32
#    define PLATFORM_WIN32 1
#endif

#include <EngineFactoryD3D12.h>

#include <RenderDevice.h>
#include <DeviceContext.h>
#include <SwapChain.h>

#include "DiligentEngine/DiligentCore/Common/interface/RefCntAutoPtr.hpp"

using namespace Diligent;

// For this tutorial, we will use simple vertex shader
// that creates a procedural triangle

// Diligent Engine can use HLSL source on all supported platforms.
// It will convert HLSL to GLSL in OpenGL mode, while Vulkan backend will compile it directly to SPIRV.

static const char* VSSource = R"(
struct PSInput 
{ 
    float4 Pos   : SV_POSITION; 
    float3 Color : COLOR; 
};

void main(in  uint    VertId : SV_VertexID,
          out PSInput PSIn) 
{
    float4 Pos[3];
    Pos[0] = float4(-0.5, -0.5, 0.0, 1.0);
    Pos[1] = float4( 0.0, +0.5, 0.0, 1.0);
    Pos[2] = float4(+0.5, -0.5, 0.0, 1.0);

    float3 Col[3];
    Col[0] = float3(1.0, 0.0, 0.0); // red
    Col[1] = float3(0.0, 1.0, 0.0); // green
    Col[2] = float3(0.0, 0.0, 1.0); // blue

    PSIn.Pos   = Pos[VertId];
    PSIn.Color = Col[VertId];
}
)";

// Pixel shader simply outputs interpolated vertex color
static const char* PSSource = R"(
struct PSInput 
{ 
    float4 Pos   : SV_POSITION; 
    float3 Color : COLOR; 
};

struct PSOutput
{ 
    float4 Color : SV_TARGET; 
};

void main(in  PSInput  PSIn,
          out PSOutput PSOut)
{
    PSOut.Color = float4(PSIn.Color.rgb, 1.0);
}
)";


class GameApp
{
public:
  
  bool InitializeDiligentEngine(HWND hWnd)
  {
    SwapChainDesc SCDesc;

    // DX12 only for now.
    EngineD3D12CreateInfo EngineCI;

#    if ENGINE_DLL
                // Load the dll and import GetEngineFactoryD3D12() function
    auto GetEngineFactoryD3D12 = LoadGraphicsEngineD3D12();
#    endif

    auto* pFactoryD3D12 = GetEngineFactoryD3D12();
    pFactoryD3D12->CreateDeviceAndContextsD3D12(EngineCI, &m_pDevice, &m_pImmediateContext);
    Win32NativeWindow Window{hWnd};
    pFactoryD3D12->CreateSwapChainD3D12(m_pDevice, m_pImmediateContext, SCDesc, FullScreenModeDesc{}, Window, &m_pSwapChain);
    
    return true;
  }

  void CreateResources()
  {
          // Pipeline state object encompasses configuration of all GPU stages

      GraphicsPipelineStateCreateInfo PSOCreateInfo;

      // Pipeline state name is used by the engine to report issues.
      // It is always a good idea to give objects descriptive names.
      PSOCreateInfo.PSODesc.Name = "Simple triangle PSO";

      // This is a graphics pipeline
      PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

      // clang-format off
      // This tutorial will render to a single render target
      PSOCreateInfo.GraphicsPipeline.NumRenderTargets             = 1;
      // Set render target format which is the format of the swap chain's color buffer
      PSOCreateInfo.GraphicsPipeline.RTVFormats[0]                = m_pSwapChain->GetDesc().ColorBufferFormat;
      // Use the depth buffer format from the swap chain
      PSOCreateInfo.GraphicsPipeline.DSVFormat                    = m_pSwapChain->GetDesc().DepthBufferFormat;
      // Primitive topology defines what kind of primitives will be rendered by this pipeline state
      PSOCreateInfo.GraphicsPipeline.PrimitiveTopology            = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      // No back face culling for this tutorial
      PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode      = CULL_MODE_NONE;
      // Disable depth testing
      PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = False;
      // clang-format on

      ShaderCreateInfo ShaderCI;
      // Tell the system that the shader source code is in HLSL.
      // For OpenGL, the engine will convert this into GLSL under the hood
      ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
      // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
      ShaderCI.UseCombinedTextureSamplers = true;
      // Create a vertex shader
      RefCntAutoPtr<IShader> pVS;
      {
          ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
          ShaderCI.EntryPoint      = "main";
          ShaderCI.Desc.Name       = "Triangle vertex shader";
          ShaderCI.Source          = VSSource;
          m_pDevice->CreateShader(ShaderCI, &pVS);
      }

      // Create a pixel shader
      RefCntAutoPtr<IShader> pPS;
      {
          ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
          ShaderCI.EntryPoint      = "main";
          ShaderCI.Desc.Name       = "Triangle pixel shader";
          ShaderCI.Source          = PSSource;
          m_pDevice->CreateShader(ShaderCI, &pPS);
      }

      // Finally, create the pipeline state
      PSOCreateInfo.pVS = pVS;
      PSOCreateInfo.pPS = pPS;
      m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSO);

  }

  void Render()
  {
       // Set render targets before issuing any draw command.
        // Note that Present() unbinds the back buffer if it is set as render target.
        auto* pRTV = m_pSwapChain->GetCurrentBackBufferRTV();
        auto* pDSV = m_pSwapChain->GetDepthBufferDSV();
        m_pImmediateContext->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        // Clear the back buffer
        const float ClearColor[] = {0.350f, 0.350f, 0.350f, 1.0f};
        // Let the engine perform required state transitions
        m_pImmediateContext->ClearRenderTarget(pRTV, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        // Set the pipeline state in the immediate context
        m_pImmediateContext->SetPipelineState(m_pPSO);

        // Typically we should now call CommitShaderResources(), however shaders in this example don't
        // use any resources.

        DrawAttribs drawAttrs;
        drawAttrs.NumVertices = 3; // Render 3 vertices
        m_pImmediateContext->Draw(drawAttrs);
  }

  void Present()
  {
    if (m_pSwapChain)
      m_pSwapChain->Present();
  }

private:
  RefCntAutoPtr<IRenderDevice> m_pDevice;
  RefCntAutoPtr<IDeviceContext> m_pImmediateContext;
  RefCntAutoPtr<ISwapChain> m_pSwapChain;
  RefCntAutoPtr<IPipelineState> m_pPSO;
};

std::unique_ptr<GameApp> g_pTheApp;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


int main(int argc, const char **argv)
{

  //Use the default logger (stdout, multi-threaded, colored)
  spdlog::info("Hello, {}!", "World");

  fmt::print("Hello, from {}\n", "{fmt}");

  g_pTheApp.reset(new GameApp);


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

  if (!g_pTheApp->InitializeDiligentEngine(hwnd))
  {
    return 0;
  }

  g_pTheApp->CreateResources();
  
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

      g_pTheApp->Render();
      g_pTheApp->Present();

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
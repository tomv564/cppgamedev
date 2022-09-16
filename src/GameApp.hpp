
// DiligentEngine needs

#define NOMINMAX 1

#ifndef PLATFORM_WIN32
#    define PLATFORM_WIN32 1
#endif

#include <windows.h>

#include "DiligentEngine/DiligentCore/Common/interface/RefCntAutoPtr.hpp"
//#include "DiligentEngine/DiligentCore/Common/interface/BasicMath.hpp"

#include <RenderDevice.h>
#include <DeviceContext.h>
#include <SwapChain.h>

using namespace Diligent;


// class IRenderDevice;
// class IDeviceContext;
// class ISwapChain;
// class IPipelineState;

class GameApp
{
public:
  bool InitializeDiligentEngine(HWND hWnd);

  void CreatePipelineState();
  void CreateVertexBuffer();
  void CreateIndexBuffer();
  void Render();
  void Present();
  
  private:
  RefCntAutoPtr<IRenderDevice> m_pDevice;
  RefCntAutoPtr<IDeviceContext> m_pImmediateContext;
  RefCntAutoPtr<ISwapChain> m_pSwapChain;
  RefCntAutoPtr<IPipelineState> m_pPSO;
  RefCntAutoPtr<IBuffer> m_triangleVertexBuffer;
  RefCntAutoPtr<IBuffer> m_triangleIndexBuffer;
  
  // not used yet?
  RefCntAutoPtr<IBuffer> m_vertexShaderConstants;
  //float4x4 m_worldViewProjectionMatrix;
};

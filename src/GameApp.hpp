
// DiligentEngine needs

#define NOMINMAX 1

#ifndef PLATFORM_WIN32
#    define PLATFORM_WIN32 1
#endif

#include <windows.h>


#include "DiligentEngine/DiligentCore/Common/interface/RefCntAutoPtr.hpp"

#include "DiligentEngine/DiligentCore/Common/interface/BasicMath.hpp"


#include <RenderDevice.h>
#include <DeviceContext.h>
#include <SwapChain.h>

using namespace Diligent;


// class IRenderDevice;
// class IDeviceContext;
// class ISwapChain;
// class IPipelineState;

struct Point2D
{
  float x;
  float y;
};

struct Rect2D
{
  Point2D topLeft;
  Point2D bottomRight;
};

class GameApp
{
public:
  bool InitializeDiligentEngine(HWND hWnd);
  void BuildUI();
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
  std::vector<Rect2D> m_rects;
  // not used yet?
  RefCntAutoPtr<IBuffer> m_vertexShaderConstants;
  //float4x4 m_worldViewProjectionMatrix;
};

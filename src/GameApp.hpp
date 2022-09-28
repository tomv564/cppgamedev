
// DiligentEngine needs

#include "DiligentEngine/DiligentCore/Primitives/interface/BasicTypes.h"
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

struct Color
{
  int r;
  int g;
  int b;
  int a;
};

struct Rect2D
{
  Point2D topLeft;
  Point2D bottomRight;
  Color color;
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
  RefCntAutoPtr<IShaderResourceBinding> m_pSRB;
  RefCntAutoPtr<IBuffer> m_triangleVertexBuffer;
  RefCntAutoPtr<IBuffer> m_triangleIndexBuffer;
  std::vector<Rect2D> m_rects;
  RefCntAutoPtr<IBuffer> m_vertexShaderConstants;
  float4x4 m_worldViewProjectionMatrix;
};

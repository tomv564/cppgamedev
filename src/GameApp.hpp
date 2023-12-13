
// DiligentEngine needs

#include <memory>
#define NOMINMAX 1

#ifndef PLATFORM_WIN32
#    define PLATFORM_WIN32 1
#endif

#include <windows.h>


#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngine/interface/EngineFactory.h"
#include "DiligentEngine/DiligentCore/Primitives/interface/BasicTypes.h"
#include "DiligentEngine/DiligentCore/Common/interface/RefCntAutoPtr.hpp"
#include "DiligentEngine/DiligentCore/Common/interface/BasicMath.hpp"
#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h"

// #include <RenderDevice.h>
#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h"
// #include <DeviceContext.h>
// #include <SwapChain.h>
#include <vector>

#include <freetype/freetype.h>

#include "UIRenderer.h"

using namespace Diligent;


// class IRenderDevice;
// class IDeviceContext;
// class ISwapChain;
// class IPipelineState;


class GameApp
{
public:
    bool InitializeDiligentEngine(HWND hWnd);
    void InitializeUIRenderer();
    void BuildUI();
    void LoadTextures();
    void Update();
    void Render();
    void Present();
  
private:
    RefCntAutoPtr<IRenderDevice> m_pDevice;
    RefCntAutoPtr<IEngineFactory> m_engineFactory;


    std::unique_ptr<UIRenderer> m_uiRenderer;

    RefCntAutoPtr<IDeviceContext> m_pImmediateContext;
    RefCntAutoPtr<ISwapChain> m_pSwapChain;

    RefCntAutoPtr<IPipelineState> m_pPSO;
    RefCntAutoPtr<IPipelineState> m_pPSO2;
    RefCntAutoPtr<IShaderResourceBinding> m_pSRB;


    RefCntAutoPtr<IBuffer> m_triangleVertexBuffer;
    RefCntAutoPtr<IBuffer> m_triangleIndexBuffer;
    RefCntAutoPtr<ITextureView> m_characterTextureSRV;
    RefCntAutoPtr<ITextureView> m_textureSRV;

    bool m_uiUpdateNeeded = false;
    std::vector<Surface> m_surfaces;
    std::vector<Vertex> m_vertices;
    std::vector<Uint32> m_indices;

    RefCntAutoPtr<IBuffer> m_vertexShaderConstants;
    float4x4 m_worldViewProjectionMatrix;

};

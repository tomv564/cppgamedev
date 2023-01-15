
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
};


struct Vertex
{
    float3 pos;
    float4 color;
    float2 uv;
};

struct Quad
{
    Rect2D rect;
    Color color;
    char ch;
    std::string texture;
};

class Surface
{

public:
    Rect2D rect;
    Color backgroundColor;
    std::string text;
    std::string texture;

    void getIndices(std::vector<Uint32>& indices, Uint32& offset) const;
    void getVertices(std::vector<Vertex>& vertices) const;

    bool getMesh(std::vector<Vertex>& vertices, std::vector<Uint32>& indices) const;

    void createQuads();

private:
    std::vector<Quad> quads;

};

struct UIRenderBatch
{
    // RefCntAutoPtr<IPipelineState> m_pPSO;
    // RefCntAutoPtr<IPipelineState> m_pPSO2;
    // RefCntAutoPtr<IShaderResourceBinding> m_pSRB;
    // RefCntAutoPtr<IShaderResourceBinding> m_pSRB2;


    RefCntAutoPtr<IBuffer> vertexBuffer;
    RefCntAutoPtr<IBuffer> indexBuffer;


};

class UIRenderer
{

public:
    UIRenderer(RefCntAutoPtr<IRenderDevice>& renderDevice, RefCntAutoPtr<IDeviceContext>& deviceContext, RefCntAutoPtr<ISwapChain>& swapChain, RefCntAutoPtr<IEngineFactory>& engineFactory);
    void Render(const Surface& surface);

    void setTextureAtlas(RefCntAutoPtr<ITexture>& texture);
    void setCharacterAtlas(RefCntAutoPtr<ITexture>& texture);

private:
    // per 
    RefCntAutoPtr<IRenderDevice> m_pDevice;
    RefCntAutoPtr<ISwapChain> m_pSwapChain;

    RefCntAutoPtr<IEngineFactory> m_engineFactory;
    RefCntAutoPtr<IDeviceContext> m_deviceContext;



    // per batch/job  
    RefCntAutoPtr<IPipelineState> m_pPSO;
    RefCntAutoPtr<IPipelineState> m_pPSO2;
    
    RefCntAutoPtr<IShaderResourceBinding> m_pSRB;

    RefCntAutoPtr<ITextureView> m_characterTextureSRV;
    RefCntAutoPtr<ITextureView> m_textureSRV;

    RefCntAutoPtr<IBuffer> m_vertexShaderConstants;
    float4x4 m_worldViewProjectionMatrix;

};

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

    std::vector<Surface> m_surfaces;
    std::vector<Vertex> m_vertices;
    std::vector<Uint32> m_indices;

    RefCntAutoPtr<IBuffer> m_vertexShaderConstants;
    float4x4 m_worldViewProjectionMatrix;

};

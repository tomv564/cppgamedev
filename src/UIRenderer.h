
// DiligentEngine needs

#include <algorithm>
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
    Rect2D uv;
    // char ch;
    // std::string texture;
};

class Surface
{

public:
    Rect2D rect;
    Color backgroundColor;
    Color color;
    std::string text;
    std::string texture;

    void getIndices(std::vector<Uint32>& indices, Uint32& offset) const;
    void getVertices(std::vector<Vertex>& vertices) const;

    bool getMesh(std::vector<Vertex>& vertices, std::vector<Uint32>& indices) const;

    void createQuads(FT_Face& face);

// private:
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


struct Glyph
{
  uint top;
  uint left; 
  uint width;
  uint height;
  uint advance_x;
};

class GlyphCache
{

public:
  GlyphCache();
    const Glyph* getOrAddGlyph(char ch, FT_Face& face);
  const char* getData();

private:
    // TODO: fixed size?
    std::vector<char> pixels;
    uint leftOffset = 0;
  std::unordered_map<char, Glyph> glyphs;
};


class UIRenderer
{

public:
    UIRenderer(RefCntAutoPtr<IRenderDevice>& renderDevice, RefCntAutoPtr<IDeviceContext>& deviceContext, RefCntAutoPtr<ISwapChain>& swapChain, RefCntAutoPtr<IEngineFactory>& engineFactory);
    void Prepare(Surface& surface);
    void Render(const Surface& surface);

    void initializeFonts();
    void setTextureAtlas(RefCntAutoPtr<ITexture>& texture);
    void setCharacterAtlas(RefCntAutoPtr<ITexture>& texture);
    void createCharacterAtlas();
    void updateCharacterAtlas();

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

    RefCntAutoPtr<ITexture> m_characterTexture;
    RefCntAutoPtr<ITextureView> m_characterTextureSRV;
    RefCntAutoPtr<ITextureView> m_textureSRV;

    RefCntAutoPtr<IBuffer> m_vertexShaderConstants;
    float4x4 m_worldViewProjectionMatrix;

    FT_Library m_freeType;
    FT_Face m_face;

    std::unique_ptr<GlyphCache> m_glyphCache;

};
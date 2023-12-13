#define NOMINMAX 1

// DiligentEngine needs
#ifndef PLATFORM_WIN32
#    define PLATFORM_WIN32 1
#endif

#include <cstdint>
#include <cstdlib>
#include "UIRenderer.h"

// TODO ensure include path is known to clangd

#include "DiligentEngine/DiligentCore/Common/interface/RefCntAutoPtr.hpp"
#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngine/interface/InputLayout.h"
#include "DiligentEngine/DiligentCore/Graphics/GraphicsTools/interface/MapHelper.hpp"
#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngine/interface/Buffer.h"
// #include "DiligentEngine/DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h"

#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypes.h"
#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngine/interface/ShaderResourceVariable.h"
#include "DiligentEngine/DiligentTools/TextureLoader/interface/TextureUtilities.h"
#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h"
#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h"
// #include <EngineFactoryD3D12.h>

// #include <RenderDevice.h>
// #include <DeviceContext.h>
// #include <SwapChain.h>

#include <vector>
#include <algorithm>
#include <iterator>
#include <memory>
#include <numeric>
#include <tuple>


using namespace Diligent;
static const std::string characters = R"( !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~")";


constexpr float4 toFloat4(const Color& color)
{
    return float4(color.r / 255.F, color.g / 255.F, color.b / 255.F, color.a / 255.F);
} 

constexpr Rect2D getCharacterUVs(char c)
{
    int index = static_cast<int>(characters.find(c));
    int row = index / 16;
    int col = index % 16;

    const float rowHeight = 1.0F / 6;
    const float top = row * rowHeight;
    const float bottom = top + rowHeight;

    const float charWidth = 1.0F / 16; // should be 256 in 16x16px chunks
    const float left = col * charWidth;
    const float right = left + charWidth;
    return {{left, top}, {right, bottom}};

}


void Surface::getVertices(std::vector<Vertex>& vertices) const
{
    // add vertices for quad with texture coordinates
    // u = horizontal left->right 
    // v = vertical bottom->top

    for (const Quad& s : quads)
    {
        // winding order should be clockwise to get a front-facing triangles
        // const Rect2D uv = s.ch != 0 ? getCharacterUVs(s.ch) : ;
        // auto charUV = getCharacterUVs(); // should be A?

        // bottom left
        vertices.push_back({float3(s.rect.topLeft.x, s.rect.bottomRight.y, 0.0), toFloat4(s.color), float2(s.uv.topLeft.x, s.uv.bottomRight.y)});

        // top left
        vertices.push_back({float3(s.rect.topLeft.x, s.rect.topLeft.y, 0.0), toFloat4(s.color), float2(s.uv.topLeft.x, s.uv.topLeft.y)});

        // top right
        vertices.push_back({float3(s.rect.bottomRight.x, s.rect.topLeft.y, 0.0), toFloat4(s.color), float2(s.uv.bottomRight.x, s.uv.topLeft.y)});

        // bottom right
        vertices.push_back({float3(s.rect.bottomRight.x, s.rect.bottomRight.y, 0.0), toFloat4(s.color), float2(s.uv.bottomRight.x, s.uv.bottomRight.y)});

    }


}

void Surface::getIndices(std::vector<Uint32>& indices, Uint32& offset) const
{
    for ([[maybe_unused]] const Quad& s : quads)
    {
        // add 6 indices
        // bottomleft, topright, topleft
        indices.push_back(offset+0);
        indices.push_back(offset+1);
        indices.push_back(offset+2);

        // bottomleft, bottomright, topright
        indices.push_back(offset+0);
        indices.push_back(offset+2);
        indices.push_back(offset+3);
        offset += 4;
    }
}

bool Surface::getMesh(std::vector<Vertex>& vertices, std::vector<Uint32>& indices) const
{
    getVertices(vertices);
    Uint32 offset = 0;
    getIndices(indices, offset);

    return true;
}

constexpr int GLYPHCACHE_SIZE = 512;

GlyphCache::GlyphCache():
    pixels(GLYPHCACHE_SIZE * GLYPHCACHE_SIZE, 0)
{
}

const Glyph* GlyphCache::getOrAddGlyph(char ch, FT_Face& face)
{
  const auto glyphIt = glyphs.find(ch);

  if (glyphIt != glyphs.end()) {
    return &glyphIt->second;
  }


    // one liner for below:
    int error = FT_Load_Char( face, ch, FT_LOAD_RENDER );

    //FT_GlyphSlot slot = face->glyph
    if (error) {
      std::cout << "FT_Load_Char error" << std::endl;
      return nullptr;
    }
        
    // copy bitmap, 8 bit per pixel
    // one row at a time
    
    int b = 0;
    for (int i = 0; i < face->glyph->bitmap.rows; i++)
    {
        for (int j = 0; j < face->glyph->bitmap.width; j++)
        {
            pixels[i * GLYPHCACHE_SIZE + (j+leftOffset)] = face->glyph->bitmap.buffer[b++];
        }
    }

    
    // capture drawing information.
    Glyph &glyph = glyphs[ch];
    glyph.top = 0;
    glyph.left = leftOffset;
    glyph.width = face->glyph->bitmap.width;
    glyph.height = face->glyph->bitmap.rows;
    glyph.advance_x = face->glyph->advance.x;

    leftOffset += face->glyph->bitmap.width;
    
     return &glyph;
    
}

const char* GlyphCache::getData()
{
  return pixels.data();
}

UIRenderer::UIRenderer(RefCntAutoPtr<IRenderDevice>& renderDevice, RefCntAutoPtr<IDeviceContext>& deviceContext,RefCntAutoPtr<ISwapChain>& swapChain, RefCntAutoPtr<IEngineFactory>& engineFactory)
: m_pDevice(renderDevice)
, m_deviceContext(deviceContext)
, m_pSwapChain(swapChain)
, m_engineFactory(engineFactory)
, m_glyphCache(std::make_unique<GlyphCache>())
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
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;
    // clang-format on

    ShaderCreateInfo ShaderCI;
    // Tell the system that the shader source code is in HLSL.
    // For OpenGL, the engine will convert this into GLSL under the hood
    ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
    // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
    ShaderCI.UseCombinedTextureSamplers = true;

    // Create a shader source stream factory to load shaders from files.
    RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
    m_engineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;

    // Create a vertex shader
    RefCntAutoPtr<IShader> pVS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
        ShaderCI.EntryPoint      = "main";
        ShaderCI.Desc.Name       = "Triangle vertex shader";
        ShaderCI.FilePath = "ui_vertex.hlsl";
        m_pDevice->CreateShader(ShaderCI, &pVS);

        // Create dynamic uniform buffer that will store our transformation matrix
        // Dynamic buffers can be frequently updated by the CPU
        BufferDesc constantsBufferDesc;
        constantsBufferDesc.Name           = "VS constants CB";
        constantsBufferDesc.Size           = sizeof(float4x4);
        constantsBufferDesc.Usage          = USAGE_DYNAMIC;
        constantsBufferDesc.BindFlags      = BIND_UNIFORM_BUFFER;
        constantsBufferDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        m_pDevice->CreateBuffer(constantsBufferDesc, nullptr, &m_vertexShaderConstants);
    }

    // Create a pixel shader
    RefCntAutoPtr<IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint      = "main";
        ShaderCI.Desc.Name       = "Triangle pixel shader";
        ShaderCI.FilePath = "ui_pixel.hlsl";
        m_pDevice->CreateShader(ShaderCI, &pPS);
    }

    // memory layout of input data
    LayoutElement layoutElements[] = 
    {
        LayoutElement{0, 0, 3, VT_FLOAT32, False}, // position
        LayoutElement{1, 0, 4, VT_FLOAT32, False}, // color
        LayoutElement{2, 0, 2, VT_FLOAT32, False} // texture coordinates
    };

    PSOCreateInfo.pVS = pVS;
    PSOCreateInfo.pPS = pPS;

    PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = layoutElements;
    PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements = _countof(layoutElements);


    PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

    // Shader variables should typically be mutable, which means they are expected
    // to change on a per-instance basis
    ShaderResourceVariableDesc Vars[] = 
    {
        {SHADER_TYPE_PIXEL, "g_Texture", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}
    };

    PSOCreateInfo.PSODesc.ResourceLayout.Variables    = Vars;
    PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);


    // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
    SamplerDesc SamLinearClampDesc
    {
        FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, 
        TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP
    };
    ImmutableSamplerDesc ImtblSamplers[] = 
    {
        {SHADER_TYPE_PIXEL, "g_Texture", SamLinearClampDesc}
    };

    PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = ImtblSamplers;
    PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);

    // Finally, create the pipeline state
    m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSO);

    m_worldViewProjectionMatrix = float4x4::OrthoOffCenter(0.0, 1920.0, 1080.0, 0.0, 1.0, -1.0, false);

    // Since we did not explcitly specify the type for 'Constants' variable, default
    // type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never
    // change and are bound directly through the pipeline state object.
    m_pPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants")->Set(m_vertexShaderConstants);

    // Create a shader resource binding object and bind all static resources in it
    m_pPSO->CreateShaderResourceBinding(&m_pSRB, true);
}

void UIRenderer::initializeFonts()
{

    // we support one font and sizes for now.

    // FT_Library 
    int error;
    error = FT_Init_FreeType(&m_freeType);
    if (error)
        std::cout << "FT_Init_FreeType error" << std::endl;

    error = FT_New_Face(m_freeType, "C:\\windows\\fonts\\arial.ttf", 0, &m_face);
    if (error)
        std::cout << "FT_New_Face error" << std::endl;

    FT_Set_Char_Size(m_face, 0, 32*64, 96, 96);

    createCharacterAtlas();
}

void UIRenderer::setTextureAtlas(RefCntAutoPtr<ITexture>& texture)
{
    m_textureSRV = texture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
}

void UIRenderer::createCharacterAtlas()
{

    // todo try as rgba instead. sampler looks to expect many channels?


  TextureDesc TexDesc;
  TexDesc.Name = "Glyph Texture";
  TexDesc.Type = RESOURCE_DIM_TEX_2D;
  TexDesc.Width = GLYPHCACHE_SIZE;
  TexDesc.Height = GLYPHCACHE_SIZE;
  TexDesc.Format = TEX_FORMAT_R8_UNORM;
  TexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;

  TextureSubResData textureSubResData;
  textureSubResData.Stride = GLYPHCACHE_SIZE;
  textureSubResData.pData = m_glyphCache->getData();

  TextureData textureData;
  textureData.NumSubresources = 1;
  textureData.pSubResources = &textureSubResData;

  m_pDevice->CreateTexture(TexDesc, &textureData, &m_characterTexture);

    m_characterTextureSRV = m_characterTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
}

void UIRenderer::setCharacterAtlas(RefCntAutoPtr<ITexture> & texture)
{

    m_characterTextureSRV = texture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
}



void UIRenderer::Prepare(Surface& surface)
{
    
    // background rect
    surface.quads.push_back({surface.rect, surface.backgroundColor, Rect2D {{0, 0}, {1, 1}}});

    // text
    if (!surface.text.empty())
    {
        float left = surface.rect.topLeft.x;
        float top = surface.rect.topLeft.y;

        for (const auto ch : surface.text) 
        {
            const Glyph* glyph = m_glyphCache->getOrAddGlyph(ch, m_face);
            if (glyph) 
            {
                Rect2D uv;
                // todo handle topLeft correctly
                uv.topLeft.x = (float)glyph->left / GLYPHCACHE_SIZE;
                uv.topLeft.y = (float)glyph->top; 
                uv.bottomRight.x = (float)(glyph->width+glyph->left) / GLYPHCACHE_SIZE;
                uv.bottomRight.y = (float)glyph->height / GLYPHCACHE_SIZE;
                 surface.quads.push_back({ { { left, top }, { left + glyph->width, top + glyph->height } }, surface.color, uv });
                 
                 left += glyph->advance_x >> 6;
            }
            else
                std::cout << "Failed to get glyph: '" << ch << "'" << std::endl;

        }        
    }
}

void UIRenderer::updateCharacterAtlas()
{
  Box updateBox;
  updateBox.MaxX = GLYPHCACHE_SIZE;
  updateBox.MaxY = GLYPHCACHE_SIZE;

  //std::vector<Uint8> data(GLYPHCACHE_SIZE * GLYPHCACHE_SIZE, 255);

  TextureSubResData textureData;
  textureData.Stride = GLYPHCACHE_SIZE;
  textureData.pData = m_glyphCache->getData();

  m_deviceContext->UpdateTexture(m_characterTexture, 0 /* miplevel */, 0 /* slice*/, updateBox, textureData, RESOURCE_STATE_TRANSITION_MODE_NONE, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}


void UIRenderer::Render(const Surface& surface)
{

    // TODO: separate mesh & shader arguments from actual drawing.

    // TODO: use members instead.
    std::vector<Vertex> vertices;
    std::vector<Uint32> indices;
    // Uint32 offset = 0;

    // TODO: check result?
    surface.getMesh(vertices, indices);

    // vertices.clear();
    // indices.clear();

    UIRenderBatch batch;

    // CreateVertexBuffer
    {    
        Uint64 dataSize  = sizeof(vertices[0]) * vertices.size();
        
        BufferDesc vertBufferDesc;
        vertBufferDesc.Name = "Triangle vertex buffer";
        vertBufferDesc.Usage = USAGE_IMMUTABLE;
        vertBufferDesc.BindFlags = BIND_VERTEX_BUFFER;
        vertBufferDesc.Size = dataSize; //sizeof(triangleVertices);

        BufferData vertBufferData;  
        vertBufferData.pData = vertices.data();
        vertBufferData.DataSize = dataSize; //sizeof(triangleVertices);

        m_pDevice->CreateBuffer(vertBufferDesc, &vertBufferData, &batch.vertexBuffer);
    }


    // CreateIndexBuffer
    {
        Uint64 dataSize  = sizeof(indices[0]) * indices.size();

        BufferDesc indexBufferDesc;
        indexBufferDesc.Name = "Triangle index buffer";
        indexBufferDesc.Usage = USAGE_IMMUTABLE;
        indexBufferDesc.BindFlags = BIND_INDEX_BUFFER;
        indexBufferDesc.Size = dataSize;

        BufferData indexBufferData;
        indexBufferData.pData = indices.data();
        indexBufferData.DataSize = dataSize;

        m_pDevice->CreateBuffer(indexBufferDesc, &indexBufferData, &batch.indexBuffer);
    }


    // actually render the batch.  

    {
        // Map the buffer and write current world-view-projection matrix
        MapHelper<float4x4> CBConstants(m_deviceContext, m_vertexShaderConstants, MAP_WRITE, MAP_FLAG_DISCARD);
        *CBConstants = m_worldViewProjectionMatrix.Transpose();
    }


    const Uint64 offset = 0;
    IBuffer* pBuffs[] = {batch.vertexBuffer};
    m_deviceContext->SetVertexBuffers(0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
    m_deviceContext->SetIndexBuffer(batch.indexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    // Set the pipeline state in the immediate context
    m_deviceContext->SetPipelineState(m_pPSO);




    DrawIndexedAttribs drawAttrs;
    drawAttrs.IndexType = VT_UINT32;
    drawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;


    //// Pass two: remaining indices for text
    if (!surface.text.empty())
    {
        m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_Texture")->Set(m_characterTextureSRV);

        m_deviceContext->CommitShaderResources(m_pSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        drawAttrs.FirstIndexLocation = 6;
        drawAttrs.NumIndices = static_cast<uint32_t>(indices.size()) - 6;
        m_deviceContext->DrawIndexed(drawAttrs);
    }


    m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_Texture")->Set(m_textureSRV);

    // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
    // makes sure that resources are transitioned to required states.
    m_deviceContext->CommitShaderResources(m_pSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);


    //Pass one: 6 indices for the first quad.
    drawAttrs.FirstIndexLocation = 0;
    drawAttrs.NumIndices = 6;
    m_deviceContext->DrawIndexed(drawAttrs);


}
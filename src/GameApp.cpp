#define NOMINMAX 1

// DiligentEngine needs
#ifndef PLATFORM_WIN32
#    define PLATFORM_WIN32 1
#endif

#include <cstdint>
#include <cstdlib>
#include "GameApp.hpp"


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

// For this tutorial, we will use simple vertex shader
// that creates a procedural triangle

// Diligent Engine can use HLSL source on all supported platforms.
// It will convert HLSL to GLSL in OpenGL mode, while Vulkan backend will compile it directly to SPIRV.


static const std::string characters = R"( !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~")";


constexpr float4 toFloat4(const Color& color)
{
    return float4(color.r / 255.F, color.b / 255.F, color.g / 255.F, color.a / 255.F);
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

void Surface::createQuads()
{

    quads.push_back({rect, backgroundColor/*, char, texture */});


    if (!text.empty())
    {
        float left = 200;
        float top = 200;
        float width = 24;
        float height = 40; // TODO 6x32?
        float spacing = 0;
        for (const auto ch : text) 
        {
            quads.push_back({ { { left, top }, { left + width, top + height } }, { 255, 255, 255, 255 }, ch, "" });
            left += width + spacing;
        }        
    }
}


void Surface::getVertices(std::vector<Vertex>& vertices) const
{
    // add vertices for quad with texture coordinates
    // u = horizontal left->right 
    // v = vertical bottom->top

    for (const Quad& s : quads)
    {
        // winding order should be clockwise to get a front-facing triangles
        const Rect2D uv = s.ch != 0 ? getCharacterUVs(s.ch) : Rect2D {{0, 0}, {1, 1}};
        // auto charUV = getCharacterUVs(); // should be A?

        // bottom left
        vertices.push_back({float3(s.rect.topLeft.x, s.rect.bottomRight.y, 0.0), toFloat4(s.color), float2(uv.topLeft.x, uv.bottomRight.y)});

        // top left
        vertices.push_back({float3(s.rect.topLeft.x, s.rect.topLeft.y, 0.0), toFloat4(s.color), float2(uv.topLeft.x, uv.topLeft.y)});

        // top right
        vertices.push_back({float3(s.rect.bottomRight.x, s.rect.topLeft.y, 0.0), toFloat4(s.color), float2(uv.bottomRight.x, uv.topLeft.y)});

        // bottom right
        vertices.push_back({float3(s.rect.bottomRight.x, s.rect.bottomRight.y, 0.0), toFloat4(s.color), float2(uv.bottomRight.x, uv.bottomRight.y)});

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


UIRenderer::UIRenderer(RefCntAutoPtr<IRenderDevice>& renderDevice, RefCntAutoPtr<IDeviceContext>& deviceContext,RefCntAutoPtr<ISwapChain>& swapChain, RefCntAutoPtr<IEngineFactory>& engineFactory)
: m_pDevice(renderDevice)
, m_deviceContext(deviceContext)
, m_pSwapChain(swapChain)
, m_engineFactory(engineFactory)
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

bool GameApp::InitializeDiligentEngine(HWND hWnd)
{
    SwapChainDesc SCDesc;

    RENDER_DEVICE_TYPE deviceType = Diligent::RENDER_DEVICE_TYPE_D3D11;
    Win32NativeWindow Window{hWnd};

    switch (deviceType) 
    {
        case Diligent::RENDER_DEVICE_TYPE_D3D11:
        {

        #    if ENGINE_DLL
                        // Load the dll and import GetEngineFactoryD3D12() function
            auto GetEngineFactoryD3D11 = LoadGraphicsEngineD3D11();
        #    endif

            EngineD3D11CreateInfo EngineCI;
            EngineCI.GraphicsAPIVersion = {11, 0};

            auto* engineFactory = GetEngineFactoryD3D11();
            engineFactory->CreateDeviceAndContextsD3D11(EngineCI, &m_pDevice, &m_pImmediateContext);
            engineFactory->CreateSwapChainD3D11(m_pDevice, m_pImmediateContext, SCDesc, FullScreenModeDesc{}, Window, &m_pSwapChain);
            
            m_engineFactory = engineFactory;            
            break;
        }
        case Diligent::RENDER_DEVICE_TYPE_D3D12:
        {
        #    if ENGINE_DLL
                        // Load the dll and import GetEngineFactoryD3D12() function
            auto GetEngineFactoryD3D12 = LoadGraphicsEngineD3D12();
        #    endif

            EngineD3D12CreateInfo EngineCI;
            EngineCI.GraphicsAPIVersion = {11, 0};
            
            auto* engineFactory = GetEngineFactoryD3D12();
            engineFactory->CreateDeviceAndContextsD3D12(EngineCI, &m_pDevice, &m_pImmediateContext);
            engineFactory->CreateSwapChainD3D12(m_pDevice, m_pImmediateContext, SCDesc, FullScreenModeDesc{}, Window, &m_pSwapChain);
            
            m_engineFactory = engineFactory;
            break;
        }
    }
    // DX12 only for now.
    EngineD3D12CreateInfo EngineCI;

    return true;
}

void GameApp::InitializeUIRenderer()
{
    m_uiRenderer = std::make_unique<UIRenderer>(m_pDevice, m_pImmediateContext, m_pSwapChain, m_engineFactory);
}

void GameApp::BuildUI()
{

    Surface surface;
    surface.text = "Hello World!";
    surface.createQuads();
    m_surfaces.push_back(surface);

    Surface surface2;
    surface2.rect = {{0, 0}, {100, 100}};
    surface2.backgroundColor = {255, 0, 0, 255};
    surface2.createQuads();
    m_surfaces.push_back(surface2);

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


    if (surface.text.empty())
        m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_Texture")->Set(m_textureSRV);
    else
        m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_Texture")->Set(m_characterTextureSRV);

    // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
    // makes sure that resources are transitioned to required states.
    m_deviceContext->CommitShaderResources(m_pSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);


    DrawIndexedAttribs drawAttrs;
    drawAttrs.IndexType = VT_UINT32;
    drawAttrs.NumIndices = static_cast<uint32_t>(indices.size());
    drawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
    m_deviceContext->DrawIndexed(drawAttrs);
}

void GameApp::LoadTextures()
{
    TextureLoadInfo loadInfo;
    loadInfo.IsSRGB = true;
    RefCntAutoPtr<ITexture> texture;
   
    CreateTextureFromFile("white.png", loadInfo, m_pDevice, &texture);
    m_uiRenderer->setTextureAtlas(texture);

    CreateTextureFromFile("characters_on_white.png", loadInfo, m_pDevice, &texture);
    m_uiRenderer->setCharacterAtlas(texture);
}

void UIRenderer::setTextureAtlas(RefCntAutoPtr<ITexture>& texture)
{
    m_textureSRV = texture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
}

void UIRenderer::setCharacterAtlas(RefCntAutoPtr<ITexture>& texture)
{
    m_characterTextureSRV = texture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
}

void GameApp::Render()
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

    for (const Surface& surf : m_surfaces)
        m_uiRenderer->Render(surf);

}

void GameApp::Present()
{
    if (m_pSwapChain != nullptr)
        m_pSwapChain->Present();
}



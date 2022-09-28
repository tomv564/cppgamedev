
#define NOMINMAX 1

// DiligentEngine needs
#include <stdint.h>
#ifndef PLATFORM_WIN32
#    define PLATFORM_WIN32 1
#endif

#include <stdlib.h>
#include "GameApp.hpp"


#include "DiligentEngine/DiligentCore/Graphics/GraphicsTools/interface/MapHelper.hpp"

#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngine/interface/Buffer.h"
#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h"

#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypes.h"
#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngine/interface/ShaderResourceVariable.h"


#include <EngineFactoryD3D12.h>

#include <RenderDevice.h>
#include <DeviceContext.h>
#include <SwapChain.h>

#include <vector>


using namespace Diligent;

// For this tutorial, we will use simple vertex shader
// that creates a procedural triangle

// Diligent Engine can use HLSL source on all supported platforms.
// It will convert HLSL to GLSL in OpenGL mode, while Vulkan backend will compile it directly to SPIRV.

static const char* VSSource = R"(

cbuffer Constants
{
    float4x4 g_WorldViewProj;
};

struct VSInput
{
  float3 Pos: ATTRIB0;
};

struct PSInput 
{ 
    float4 Pos   : SV_POSITION;  
};

void main(in  VSInput VSIn,
          out PSInput PSIn) 
{
    PSIn.Pos   = mul(float4(VSIn.Pos, 1.0), g_WorldViewProj);
}
)";

// Pixel shader simply outputs interpolated vertex color
static const char* PSSource = R"(
struct PSInput 
{ 
    float4 Pos   : SV_POSITION; 
};

struct PSOutput
{ 
    float4 Color : SV_TARGET; 
};

void main(in  PSInput  PSIn,
          out PSOutput PSOut)
{
    PSOut.Color = float4(1.0, 0.0, 0.0, 1.0);
}
)";




  
  bool GameApp::InitializeDiligentEngine(HWND hWnd)
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
  void GameApp::BuildUI()
  {
    // m_rects.push_back({ { -0.5, 0.5 }, { 0, 0 } });
    // m_rects.push_back({ { 0, 0 }, { 0.5, -0.5 } });

    m_rects.push_back({ { 0, 0 }, { 300, 300 } });
    m_rects.push_back({ { 300, 300 }, { 600, 600 } });

  }
  void GameApp::CreatePipelineState()
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
      // Create a vertex shader
      RefCntAutoPtr<IShader> pVS;
      {
          ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
          ShaderCI.EntryPoint      = "main";
          ShaderCI.Desc.Name       = "Triangle vertex shader";
          ShaderCI.Source          = VSSource;
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
          ShaderCI.Source          = PSSource;
          m_pDevice->CreateShader(ShaderCI, &pPS);
      }

      // memory layout of input data
      LayoutElement layoutElements[] = 
      {
        LayoutElement{0, 0, 3, VT_FLOAT32, False}
      };

      PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = layoutElements;
      PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements = _countof(layoutElements);


      // Finally, create the pipeline state
      PSOCreateInfo.pVS = pVS;
      PSOCreateInfo.pPS = pPS;

      PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

      m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSO);

      // for now this is identity but we'd like to use screen coordinates to build UI instead - eg based on 1080px height
      m_worldViewProjectionMatrix = float4x4::OrthoOffCenter(0.0, 1920.0, 1080.0, 0.0, 1.0, -1.0, false); //float4x4::Identity();

      // Since we did not explcitly specify the type for 'Constants' variable, default
      // type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never
      // change and are bound directly through the pipeline state object.
      m_pPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants")->Set(m_vertexShaderConstants);

      // Create a shader resource binding object and bind all static resources in it
      m_pPSO->CreateShaderResourceBinding(&m_pSRB, true);

  }

  void GameApp::CreateVertexBuffer()
  {
      struct Vertex
      {
        float3 pos;
      };
      
      std::vector<Vertex> vertices;
      for (const auto& rect : m_rects)
      {
        // add 4 vertices 
        vertices.push_back({float3(rect.bottomRight.x, rect.topLeft.y, 0.0)});
        vertices.push_back({float3(rect.topLeft.x, rect.topLeft.y, 0.0)});
        vertices.push_back({float3(rect.bottomRight.x, rect.bottomRight.y, 0.0)});
        vertices.push_back({float3(rect.topLeft.x, rect.bottomRight.y, 0.0)});
      }

      // TODO: project from screen space here or probably in vertex shader?


      Uint64 dataSize  = sizeof(vertices[0]) * vertices.size();
      BufferDesc vertBufferDesc;
      vertBufferDesc.Name = "Triangle vertex buffer";
      vertBufferDesc.Usage = USAGE_IMMUTABLE;
      vertBufferDesc.BindFlags = BIND_VERTEX_BUFFER;
      vertBufferDesc.Size = dataSize; //sizeof(triangleVertices);
      
      BufferData vertBufferData;  
      vertBufferData.pData = vertices.data();
      vertBufferData.DataSize = dataSize; //sizeof(triangleVertices);

      m_pDevice->CreateBuffer(vertBufferDesc, &vertBufferData, &m_triangleVertexBuffer);
  }

  void GameApp::CreateIndexBuffer()
  {
      // Uint32 indices[] = 
      // {
      //   0, 1, 3, 0, 2, 3
      // };

      uint32_t offset = 0;
      std::vector<Uint32> indices;
      for (const auto& rect : m_rects)
      {
        // add 6 indices
        indices.push_back(offset+0);
        indices.push_back(offset+1);
        indices.push_back(offset+3);
        indices.push_back(offset+0);
        indices.push_back(offset+2);
        indices.push_back(offset+3);
        offset += 4;
      }

      Uint64 dataSize  = sizeof(indices[0]) * indices.size();

      BufferDesc indexBufferDesc;
      indexBufferDesc.Name = "Triangle index buffer";
      indexBufferDesc.Usage = USAGE_IMMUTABLE;
      indexBufferDesc.BindFlags = BIND_INDEX_BUFFER;
      indexBufferDesc.Size = dataSize;

      BufferData indexBufferData;
      indexBufferData.pData = indices.data();
      indexBufferData.DataSize = dataSize;

      m_pDevice->CreateBuffer(indexBufferDesc, &indexBufferData, &m_triangleIndexBuffer);

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

        {
            // Map the buffer and write current world-view-projection matrix
            MapHelper<float4x4> CBConstants(m_pImmediateContext, m_vertexShaderConstants, MAP_WRITE, MAP_FLAG_DISCARD);
            *CBConstants = m_worldViewProjectionMatrix.Transpose();
        }

        const Uint64 offset = 0;
        IBuffer* pBuffs[] = {m_triangleVertexBuffer};
        m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
        m_pImmediateContext->SetIndexBuffer(m_triangleIndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        // Set the pipeline state in the immediate context
        m_pImmediateContext->SetPipelineState(m_pPSO);

        // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
        // makes sure that resources are transitioned to required states.
        m_pImmediateContext->CommitShaderResources(m_pSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        DrawIndexedAttribs drawAttrs;
        drawAttrs.IndexType = VT_UINT32;
        drawAttrs.NumIndices = m_rects.size() * 6; // Render 6 vertices
        drawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
        m_pImmediateContext->DrawIndexed(drawAttrs);
  }

  void GameApp::Present()
  {
    if (m_pSwapChain)
      m_pSwapChain->Present();
  }



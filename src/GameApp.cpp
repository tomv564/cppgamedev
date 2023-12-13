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
    m_uiRenderer->initializeFonts();
}

void GameApp::BuildUI()
{
    // title
    Surface surface;
    surface.rect = {{0, 0}, { 300, 200}};
    surface.backgroundColor = { 255, 255, 204, 255 };
    surface.color = { 0, 255, 0, 255 };
    surface.text = "HELLO WORLD";
    m_surfaces.push_back(surface);

    //// button
    Surface surface2;
    surface2.rect = {{400, 300}, {700, 400}};
    surface2.backgroundColor = {255, 0, 0, 255};
    surface2.text = "Click to start";
    m_surfaces.push_back(surface2);

    m_uiUpdateNeeded = true;

}

void GameApp::Update()
{
    if (m_uiUpdateNeeded)
    {
        for (Surface& surf : m_surfaces)
            m_uiRenderer->Prepare(surf);

        // implementation details here?
        m_uiRenderer->updateCharacterAtlas();

        m_uiUpdateNeeded = false;
    }
}


void GameApp::LoadTextures()
{
    TextureLoadInfo loadInfo;
    loadInfo.IsSRGB = true;
    RefCntAutoPtr<ITexture> texture;
   
    CreateTextureFromFile("white.png", loadInfo, m_pDevice, &texture);
    m_uiRenderer->setTextureAtlas(texture);

   // CreateTextureFromFile("characters_on_white.png", loadInfo, m_pDevice, &texture);
    //m_uiRenderer->setCharacterAtlas(texture);

    //TextureDesc TexDesc;
    //TexDesc.Type = RESOURCE_DIM_TEX_2D;
    //TexDesc.Format = TEX_FORMAT_RGBA8_UNORM_SRGB;
    //TexDesc.Width = 1024;
    //TexDesc.Height = 1024;
    //TexDesc.MipLevels = 1;
    //TexDesc.BindFlags = BIND_SHADER_RESOURCE;
    //
    //TextureSubResData subResourceData;


    //TextureData InitData;
    //InitData.NumSubresources = 1;
    //InitData.pSubResources = &subResourceData;

    //m_pDevice->CreateTexture(TexDesc, &InitData, &texture);
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



#define NOMINMAX 1

// DiligentEngine needs
#ifndef PLATFORM_WIN32
#    define PLATFORM_WIN32 1
#endif

#include <cstdint>
#include <cstdlib>

#include <spdlog/spdlog.h>

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

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>


#pragma optimize("", off)

// Define your user buttons
enum Button
{
  ButtonMenu,
  ButtonConfirm,
  MouseX,
  MouseY
};

struct InputState
{
    bool ButtonMenuDown = false;
    bool ButtonConfirmDown = false;
    float MouseX;
    float MouseY;
};

using namespace Diligent;

// For this tutorial, we will use simple vertex shader
// that creates a procedural triangle

// Diligent Engine can use HLSL source on all supported platforms.
// It will convert HLSL to GLSL in OpenGL mode, while Vulkan backend will compile it directly to SPIRV.


// static int MyCppFunction(lua_State* L) // Lua callable functions must be this format
// {
//     const int num = (int)lua_tonumber(L, 1); // get first param from stack
//     const char* str = lua_tostring(L, 2); // get second param from stack
//     std::cout << "Hello from C++!" << std::endl;
//     std::cout << "num = " << num << ", str = " << str << std::endl;
//     return 0; // how many params we're passing to Lua
// }

namespace ScriptApi
{

    static int StartSound(lua_State* L)
    {
        //const int num = (int)lua_tonumber(L, 1); // get first param from stack

        const char* str = lua_tostring(L, 1); // get first param from stack
        GameApp::GetInstance()->StartSound(str);
        return 0;
    }

    static int CreateSurface(sol::table args)
    {
        Surface surface;
        sol::table rect = args["rect"];
        if (!rect.empty())
            surface.rect = { {rect[1][1], rect[1][2]}, {rect[2][1], rect[2][2]} };
        surface.text = args["text"];
        sol::table color = args["color"];
        if (!color.empty())
            surface.color = { color[1], color[2], color[3], color[4] };
        sol::table bgColor = args["backgroundColor"];
        if (!bgColor.empty())
            surface.backgroundColor = { bgColor[1], bgColor[2], bgColor[3], bgColor[4] };

        GameApp::GetInstance()->AddSurface(surface);

        return 0;
    }

    static int UpdateSurfaces(sol::table args)
    {
        GameApp::GetInstance()->ClearSurfaces();
        for (const auto& pair : args)
        {
            CreateSurface(pair.second.as<sol::table>());
        }

        return 0;
     /*   args.for_each([&](sol::object& val) {
            CreateSurface(val.as<sol::table>());
        })*/;
    }

}


GameApp::GameApp() 
{
    s_instance = this;
}

GameApp* GameApp::s_instance = nullptr;

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

void GameApp::SetupInput(gainput::InputManager& inputManager)
{
    // TODO: move to lua??

    gainput::DeviceId mouseId = inputManager.CreateDevice<gainput::InputDeviceMouse>();
    gainput::DeviceId keyboardId = inputManager.CreateDevice<gainput::InputDeviceKeyboard>();
    gainput::DeviceId padId = inputManager.CreateDevice<gainput::InputDevicePad>();

    m_inputMap = std::make_unique<gainput::InputMap>(inputManager);
    m_inputMap->MapBool(ButtonMenu, keyboardId, gainput::KeyEscape);
    m_inputMap->MapBool(ButtonConfirm, mouseId, gainput::MouseButtonLeft);
    m_inputMap->MapFloat(MouseX, mouseId, gainput::MouseAxisX);
    m_inputMap->MapFloat(MouseY, mouseId, gainput::MouseAxisY);
    m_inputMap->MapBool(ButtonConfirm, padId, gainput::PadButtonA);
}


void GameApp::SetupAudio()
{
    ma_result result = ma_engine_init(NULL, &m_audioEngine);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize audio engine.");
    }
}

void GameApp::Shutdown()
{
    ma_engine_uninit(&m_audioEngine);
}


void GameApp::SetupScripting()
{

    // set up Lua
    m_luaState = std::make_unique<sol::state>();
    m_luaState->open_libraries(sol::lib::base);
    m_luaState->open_libraries(sol::lib::table);
    
    // register our C++ function with Lua
    m_luaState->set_function("CreateSurface", &ScriptApi::CreateSurface); 
    m_luaState->set_function("UpdateSurfaces", &ScriptApi::UpdateSurfaces);
    m_luaState->set_function("PlaySound", ScriptApi::StartSound);
    
    m_luaState->script_file("main.lua");
    (*m_luaState)["init"]();

}
void GameApp::ClearSurfaces()
{
    m_surfaces.clear();
    m_uiUpdateNeeded = true;
}

void GameApp::AddSurface(Surface& surface)
{
    m_surfaces.push_back(surface);
    m_uiUpdateNeeded = true;
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
    // input
    if (m_inputMap->GetBoolWasDown(ButtonConfirm))
    {
    //  spdlog::info("Confirmed!!");
      //ma_engine_play_sound(&m_audioEngine, "mixkit-select-click-1109.wav", NULL);
        
        InputState state{ false, false, m_inputMap->GetFloat(MouseX)*800.f, m_inputMap->GetFloat(MouseY)*600.f };
        if (state.MouseX > 0.0f)
            spdlog::info("yay!");
            
    }

    //InputState inputState{ m_inputMap->GetBoolWasDown(ButtonMenu), m_inputMap->GetBoolWasDown(ButtonConfirm) };
    (*m_luaState)["inputState"] = m_luaState->create_table_with(
        "ButtonConfirmDown", m_inputMap->GetBoolWasDown(ButtonConfirm),
        "ButtonMenuDown", m_inputMap->GetBoolWasDown(ButtonMenu),
        "MouseX", m_inputMap->GetFloat(MouseX)*800.f,
        "MouseY", m_inputMap->GetFloat(MouseY)*600.f
    );

    // *** call Lua function from C++ ***
    //lua_getglobal(m_luaState, "update"); // find the Lua function
    //lua_pushnumber(L, 73); // push number as first param
    //lua_pushstring(L, "From C++ to Lua"); // push string as second param
    //lua_pcall(m_luaState, 0, 0, 0); // call the function passing 2 params

    (*m_luaState)["update"]();


    // if (map.GetBoolWasDown(ButtonConfirm))
    // {
    //   spdlog::info("Confirmed!!");
    //   ma_engine_play_sound(&engine, "mixkit-select-click-1109.wav", NULL);
    // }

    //if (map.GetBoolWasDown(ButtonMenu)) {
    //  spdlog::info("Open menu!!");
    //}

    //if (map.GetFloatDelta(MouseX) != 0.0f || map.GetFloatDelta(MouseY) != 0.0f)
    //{
    //  spdlog::info("Mouse: %f, %f\n", map.GetFloat(MouseX), map.GetFloat(MouseY));
    //}

    if (m_uiUpdateNeeded)
    {
        for (Surface& surf : m_surfaces)
            m_uiRenderer->Prepare(surf);

        // implementation details here?
        m_uiRenderer->updateCharacterAtlas();

        m_uiUpdateNeeded = false;
    }
}


void GameApp::StartSound(const char* assetPath)
{
    ma_engine_play_sound(&m_audioEngine, assetPath, NULL);
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



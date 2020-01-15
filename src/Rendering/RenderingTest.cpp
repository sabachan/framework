#include "stdafx.h"

#include <Core/Config.h>

#if SG_ENABLE_UNIT_TESTS

#include <Core/Assert.h>
#include <Core/FileSystem.h>
#include <Core/PerfLog.h>
#include <Core/StringFormat.h>
#include <Core/TestFramework.h>
#include <Image/Draw.h>
#include <System/Window.h>
#include <System/WindowedApplication.h>
#include "ExampleObjects.h"
#include "InitShutdown.h"
#include "RenderDevice.h"
#include "RenderWindow.h"
#include "ShaderCache.h"
#include "TextureFromMemory.h"

namespace sg {
namespace rendering {
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class TestApplication : public system::WindowedApplication
{
public:
    TestApplication()
    {
        m_renderDevice.reset(new RenderDevice());
        m_shaderconstantDatabase.reset(new ShaderConstantDatabase());

        for(size_t i = 0; i < 2; ++i)
        {
            m_windowHandles.push_back(new system::Window());
            m_renderWindows.push_back(new RenderWindow(m_renderDevice.get(), m_windowHandles.back().get()));
        }

        for(auto const& it : m_renderWindows)
            m_renderTargets.push_back(it.get());

        for(auto const& it : m_renderTargets)
            m_objects.push_back(new exampleobjects::ClearRenderTarget(it.get(), 0x00FF0000));

        m_camera.reset(new exampleobjects::Camera(m_renderDevice.get(), m_renderTargets[0]->RenderTargetResolution(), m_shaderconstantDatabase.get()));

        m_objects.push_back(new exampleobjects::SetRenderTargets(m_renderTargets[0].get(), nullptr));
        m_objects.push_back(new exampleobjects::TexturedQuad(m_renderDevice.get(), m_shaderconstantDatabase.get()));
        //m_objects.push_back(new exampleobjects::Triangle(m_renderDevice.get()));

        m_objects.push_back(new exampleobjects::SetRenderTargets(m_renderTargets[1].get(), nullptr));
        m_objects.push_back(new exampleobjects::Triangle(m_renderDevice.get()));

        SurfaceProperties surfaceProperties;
        surfaceProperties.baseFormat = rendering::SurfaceFormat::R8G8B8A8_TYPELESS;
        surfaceProperties.readFormat = rendering::SurfaceFormat::R8G8B8A8_UNORM_SRGB;
        surfaceProperties.writeFormat = rendering::SurfaceFormat::R8G8B8A8_UNORM_SRGB;
        surfaceProperties.mipLevels = 0;
        Surface* surface = new Surface(m_renderDevice.get(), m_renderTargets[0]->RenderTargetResolution(), &surfaceProperties);
        m_refs.push_back((IShaderResource*)surface);
        DepthStencilSurface* depthStencilSurface = new DepthStencilSurface(m_renderDevice.get(), m_renderTargets[0]->RenderTargetResolution());
        m_refs.push_back((IShaderResource*)depthStencilSurface);
        m_objects.push_back(new exampleobjects::ClearDepthStencil(depthStencilSurface));
        m_objects.push_back(new exampleobjects::ClearRenderTarget(surface, 0x00000000));
        m_objects.push_back(new exampleobjects::SetRenderTargets(surface, depthStencilSurface));
        //m_objects.push_back(new exampleobjects::SetRenderTargets(m_renderDevice->BackBuffer(), depthStencilSurface));

        //m_objects.push_back(new exampleobjects::SetRenderTargets(m_renderDevice->BackBuffer(), m_renderDevice->GetDepthStencilSurface()));

        if(SG_CONSTANT_CONDITION(1))
        {
            m_objects.push_back(new exampleobjects::Cube(m_renderDevice.get(), m_camera.get(), m_shaderconstantDatabase.get(), float3(0, 0, 0)));
            m_objects.push_back(new exampleobjects::Cube(m_renderDevice.get(), m_camera.get(), m_shaderconstantDatabase.get(), float3(0, 0, 2.5f)));
            //m_objects.push_back(new exampleobjects::Cube(m_renderDevice.get(), m_camera.get(), m_shaderconstantDatabase.get(), float3(0,0,5.f)));
            //m_objects.push_back(new exampleobjects::Cube(m_renderDevice.get(), m_camera.get(), m_shaderconstantDatabase.get(), float3(0,0,7.5f)));
            m_objects.push_back(new exampleobjects::Cube(m_renderDevice.get(), m_camera.get(), m_shaderconstantDatabase.get(), float3(0, 0, -2.5f)));
            //m_objects.push_back(new exampleobjects::Cube(m_renderDevice.get(), m_camera.get(), m_shaderconstantDatabase.get(), float3(0,0,-5.f)));
            //m_objects.push_back(new exampleobjects::Cube(m_renderDevice.get(), m_camera.get(), m_shaderconstantDatabase.get(), float3(0,0,-7.5f)));
            m_objects.push_back(new exampleobjects::Cube(m_renderDevice.get(), m_camera.get(), m_shaderconstantDatabase.get(), float3(0, 2.5f, 0)));
            //m_objects.push_back(new exampleobjects::Cube(m_renderDevice.get(), m_camera.get(), m_shaderconstantDatabase.get(), float3(0,5.f,0)));
            //m_objects.push_back(new exampleobjects::Cube(m_renderDevice.get(), m_camera.get(), m_shaderconstantDatabase.get(), float3(0,7.5f,0)));
            m_objects.push_back(new exampleobjects::Cube(m_renderDevice.get(), m_camera.get(), m_shaderconstantDatabase.get(), float3(0, -2.5f, 0)));
            //m_objects.push_back(new exampleobjects::Cube(m_renderDevice.get(), m_camera.get(), m_shaderconstantDatabase.get(), float3(0,-5.f,0)));
            //m_objects.push_back(new exampleobjects::Cube(m_renderDevice.get(), m_camera.get(), m_shaderconstantDatabase.get(), float3(0,-7.5f,0)));
        }

        if(SG_CONSTANT_CONDITION(1))
        {
            m_objects.push_back(new exampleobjects::SetRenderTargets(m_renderTargets[1].get(), nullptr));
            auto quad = new exampleobjects::TexturedQuad(m_renderDevice.get(), m_shaderconstantDatabase.get(), FilePath("src:/DevApplication/DevRendererShaders/ShowDepth.hlsl"));
            quad->SetShaderResource(depthStencilSurface);
            m_objects.push_back(quad);
        }
        m_objects.push_back(new exampleobjects::SetRenderTargets(m_renderTargets[0].get(), depthStencilSurface));

        if(SG_CONSTANT_CONDITION(1))
        {
            m_objects.push_back(new exampleobjects::Cube(m_renderDevice.get(), m_camera.get(), m_shaderconstantDatabase.get(), float3(0, 0, 5.f)));
            m_objects.push_back(new exampleobjects::Cube(m_renderDevice.get(), m_camera.get(), m_shaderconstantDatabase.get(), float3(0, 0, 7.5f)));
        }

        m_objects.push_back(new exampleobjects::GenerateMipmap(m_renderDevice.get(), surface));

        if(SG_CONSTANT_CONDITION(1))
        {
            m_objects.push_back(new exampleobjects::SetRenderTargets(m_renderTargets[0].get(), nullptr));
            auto quad = new exampleobjects::TexturedQuad(m_renderDevice.get(), m_shaderconstantDatabase.get(), FilePath("src:/DevApplication/DevRendererShaders/ShowTexture.hlsl"));
            quad->SetShaderResource(surface);
            m_objects.push_back(quad);
        }

        m_textureFromMemory = new TextureFromOwnMemory(m_renderDevice.get(), TextureFromMemory::ColorFormat::R8G8B8A8_SRGB, uint2(200, 200));
        m_textureFromMemory->UpdateIFN();

        if(SG_CONSTANT_CONDITION(1))
        {
            m_objects.push_back(new exampleobjects::SetRenderTargets(m_renderTargets[1].get(), nullptr));
            auto quad = new exampleobjects::TexturedQuad(m_renderDevice.get(), m_shaderconstantDatabase.get(), FilePath("src:/DevApplication/DevRendererShaders/ShowTexture.hlsl"));
            quad->SetShaderResource(m_textureFromMemory.get());
            m_objects.push_back(quad);
        }

        ShaderConstantName name_date_in_frames = "date_in_frames";
        m_shaderconstantDatabase->AddVariable(name_date_in_frames, new ShaderVariable<u32>());
    }
    virtual ~TestApplication() override
    {
    }
    virtual void TestApplication::VirtualOneTurn() override
    {
        m_camera->DummyEvolve(0.5f * float2(-1, 0.447f));
#if SG_ENABLE_TOOLS
        // TODO: Add a tool option.
        sg::rendering::shadercache::InvalidateOutdatedShaders();
#endif
        if(++m_dateInFrames > 500)
            PostQuitMessage(0);

        ArrayView<image::BitMapFont const> fonts = image::GetAlwaysAvailableBitmapFonts();

        {
            image::ImageView<ubyte4> image = m_textureFromMemory->GetAsImageForModification<ubyte4>();
            image::DrawRect(image, box2f::FromMinMax(float2(10, 10), float2(90, 90)), image::brush::Fill(ubyte4(0,0,0,0)));
            image::DrawText(image, uint2(15, 20), Format("Hello world!\ndate: %0", m_dateInFrames), image::brush::Fill(ubyte4(255,255,255,255)), fonts[2]);
            m_textureFromMemory->UpdateIFN();
        }

        m_renderDevice->BeginRender();

        {
            // TODO: TimeServer
            m_dateInFrames++;
            ShaderConstantName name_date_in_frames = "date_in_frames"; // TODO: keep as member
            IShaderVariable* ivar = m_shaderconstantDatabase->GetConstantForWriting(name_date_in_frames);
            ShaderVariable<u32>* var = checked_cast<ShaderVariable<u32>*>(ivar);
            var->Set(m_dateInFrames);
        }

        for(auto it : m_objects)
        {
            it->Draw(m_renderDevice.get());
        }

        m_renderDevice->EndRender();

        system::WindowedApplication::VirtualOneTurn();
    }
    virtual LRESULT VirtualWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override
    {
        switch (message)
        {
        case WM_KEYUP:
            {
                int vk = (int)wParam;
                exampleobjects::Camera* camera = m_camera.get();
                switch(vk)
                {
                case VK_NUMPAD1: camera->DummyEvolve(float2(-1, 1)); break;
                case VK_NUMPAD2: camera->DummyEvolve(float2( 0, 1)); break;
                case VK_NUMPAD3: camera->DummyEvolve(float2( 1, 1)); break;
                case VK_NUMPAD4: camera->DummyEvolve(float2(-1, 0)); break;
                case VK_NUMPAD5: break;
                case VK_NUMPAD6: camera->DummyEvolve(float2( 1, 0)); break;
                case VK_NUMPAD7: camera->DummyEvolve(float2(-1,-1)); break;
                case VK_NUMPAD8: camera->DummyEvolve(float2( 0,-1)); break;
                case VK_NUMPAD9: camera->DummyEvolve(float2(1, -1)); break;
#if SG_ENABLE_TOOLS
                case VK_F5: sg::rendering::shadercache::InvalidateAllShaders(); break;
#endif
                case VK_ESCAPE: PostQuitMessage(0); break;
                }
            }
            break;
        default:
            return system::WindowedApplication::VirtualWndProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }
private:
    std::vector<refptr<system::Window> > m_windowHandles;
    scopedptr<RenderDevice> m_renderDevice;
    std::vector<refptr<RenderWindow> > m_renderWindows;
    std::vector<safeptr<IRenderTarget> > m_renderTargets;
    scopedptr<ShaderConstantDatabase> m_shaderconstantDatabase;
    scopedptr<exampleobjects::Camera> m_camera;
    std::vector<refptr<VirtualRefCountable> > m_refs;
    std::vector<refptr<exampleobjects::IExampleObject> > m_objects;
    refptr<TextureFromOwnMemory> m_textureFromMemory;
    u32 m_dateInFrames;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
SG_TEST((sg, rendering), TestRendering, (Rendering, super slow))
{
    perflog::Init();
    //FastSymbol::Init();
    filesystem::Init();
    filesystem::MountDeclaredMountingPoints();
    //reflection::Init();
    rendering::Init();

    {
        TestApplication app;
        app.Run();
    }

    //reflection::Shutdown();
    rendering::Shutdown();
    filesystem::Shutdown();
    //FastSymbol::Shutdown();
    perflog::Shutdown();
}
//=============================================================================
}
}
#endif

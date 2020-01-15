#include "stdafx.h"

#include <Core/Config.h>

#if SG_ENABLE_UNIT_TESTS

#include <Core/Assert.h>
#include <Core/FileSystem.h>
#include <Core/Log.h>
#include <Core/PerfLog.h>
#include <Core/TestFramework.h>
#include <ObjectScript/Reader.h>
#include <Reflection/BaseClass.h>
#include <Reflection/InitShutdown.h>
#include <Reflection/Metaclass.h>
#include <Reflection/ObjectDatabase.h>
#include <Rendering/ExampleObjects.h>
#include <Rendering/InitShutdown.h>
#include <Rendering/RenderDevice.h>
#include <Rendering/RenderWindow.h>
#include <Rendering/VertexTypes.h>
#if SG_ENABLE_TOOLS
#include <Rendering/ShaderCache.h>
#endif
#include <System/Window.h>
#include <System/WindowedApplication.h>
#include <memory>
#include "Compositing.h"
#include "RenderBatch.h"

namespace sg {
namespace renderengine {
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class ITestDrawable : public SafeCountable
{
public:
    virtual ~ITestDrawable() {}
    virtual void Draw() = 0;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class TestApplication : public system::WindowedApplication
{
public:
    TestApplication(rendering::ShaderConstantDatabase* iShaderConstantDatabase)
        : m_dateInFrames(0)
        , m_initBlock("Init")
        , m_drawBlock("Draw")
        , m_shaderConstantDatabase(iShaderConstantDatabase)
    {
        m_renderDevice.reset(new rendering::RenderDevice());

        RenderWindow(0);
        std::vector<rendering::IRenderTarget*> windowsAsRenderTargets;
        for(auto const& it : m_renderWindows)
            windowsAsRenderTargets.push_back(it.get());

        m_camera.reset(new rendering::exampleobjects::Camera(m_renderDevice.get(), windowsAsRenderTargets[0]->RenderTargetResolution(), m_shaderConstantDatabase.get()));
    }
    virtual ~TestApplication() override
    {
    }
    rendering::RenderDevice const* RenderDevice() const { return m_renderDevice.get(); }
    void AddCompositing(ICompositing* iCompositing)
    {
        SG_ASSERT(iCompositing->HasInstructionBlock(m_drawBlock));
        m_compositings.emplace_back(iCompositing, 0);
    }
    void AddDrawable(ITestDrawable* iDrawable)
    {
        auto r = m_drawables.insert(iDrawable);
        SG_ASSERT_MSG(r.second, "drawable already in container !");
    }
    void RemoveDrawable(ITestDrawable* iDrawable)
    {
        size_t const r = m_drawables.erase(iDrawable);
        SG_ASSERT_MSG_AND_UNUSED(r, "drawable was not in container !");
    }
    virtual void TestApplication::VirtualOneTurn() override
    {
#if SG_ENABLE_TOOLS
        // TODO: Add a tool option.
        rendering::shadercache::InvalidateOutdatedShaders();
#endif
        if(++m_dateInFrames > 500)
            PostQuitMessage(0);

        for(auto const& it : m_drawables)
        {
            it->Draw();
        }

        {
            // TODO: TimeServer
            m_dateInFrames++;
            rendering::ShaderConstantName name_date_in_frames = "date_in_frames"; // TODO: keep as member
            rendering::IShaderVariable* ivar = m_shaderConstantDatabase->GetConstantForWriting(name_date_in_frames);
            rendering::ShaderVariable<u32>* var = checked_cast<rendering::ShaderVariable<u32>*>(ivar);
            var->Set(m_dateInFrames);
        }

        m_renderDevice->BeginRender();
        for(auto& it : m_compositings)
        {
            if(0 == (it.second & 0x1))
            {
                it.first->Execute(m_initBlock);
                it.second |= 0x1;
            }
            it.first->Execute(m_drawBlock);
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
                rendering::exampleobjects::Camera* camera = m_camera.get();
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
                case VK_NUMPAD9: camera->DummyEvolve(float2( 1,-1)); break;
    //#if SG_ENABLE_TOOLS
    //            case VK_F5: sg::rendering::shadercache::InvalidateAllShaders(); break;
    //#endif
                case VK_ESCAPE: PostQuitMessage(0); break;
                }
            }
            break;
        default:
            return system::WindowedApplication::VirtualWndProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }
    rendering::RenderWindow* RenderWindow(size_t i)
    {
        SG_ASSERT(m_windowHandles.size() == m_renderWindows.size());
        for(size_t j = m_renderWindows.size(); j < i+1; ++j)
        {
            m_windowHandles.push_back(new system::Window());
            m_renderWindows.push_back(new rendering::RenderWindow(m_renderDevice.get(), m_windowHandles.back().get()));
        }
        SG_ASSERT(i < m_renderWindows.size());
        return m_renderWindows[i].get();
    }
private:
    std::vector<refptr<system::Window> > m_windowHandles;
    scopedptr<rendering::RenderDevice> m_renderDevice;
    std::vector<refptr<rendering::RenderWindow> > m_renderWindows;
    std::vector<std::pair<refptr<ICompositing>, u32> > m_compositings;
    u32 m_dateInFrames;
    FastSymbol m_initBlock;
    FastSymbol m_drawBlock;
    safeptr<rendering::ShaderConstantDatabase> m_shaderConstantDatabase;
    scopedptr<rendering::exampleobjects::Camera> m_camera;
    std::unordered_set<safeptr<ITestDrawable> > m_drawables;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
typedef rendering::Vertex_Pos3f_Normal3f_Tex2f_Col4f CubeVertex;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class DrawCubes : public ITestDrawable
{
public:
    DrawCubes(ICompositing* iCompositing)
    {
        RenderBatchDescriptor renderBatchDescriptor;
        renderBatchDescriptor.indexSize = RenderBatchDescriptor::IndexSize::u32;
        renderBatchDescriptor.indexUpdateMode = RenderBatchDescriptor::UpdateMode::Map;
        renderBatchDescriptor.inputSlotCount = 1;
        renderBatchDescriptor.vertexSize[0] = sizeof(CubeVertex);
        renderBatchDescriptor.vertexUpdateMode[0] = RenderBatchDescriptor::UpdateMode::Map;
        refptr<rendering::VertexShaderDescriptor> vsDesc = new rendering::VertexShaderDescriptor(FilePath("src:/Rendering/Shaders/ExampleObjects/Cube.hlsl"), "vmain");
        renderBatchDescriptor.vertexShader = vsDesc->GetProxy();
        refptr<rendering::PixelShaderDescriptor> psDesc = new rendering::PixelShaderDescriptor(FilePath("src:/Rendering/Shaders/ExampleObjects/Cube.hlsl"), "pmain");
        renderBatchDescriptor.pixelShader = psDesc->GetProxy();
        renderBatchDescriptor.inputLayout = rendering::ShaderInputLayoutProxy(CubeVertex::InputEltDesc(), renderBatchDescriptor.vertexShader);
        renderBatchDescriptor.sorting = RenderBatchDescriptor::Sorting::NoSort;

        m_renderBatch.reset(new RenderBatch(iCompositing->RenderDevice(), renderBatchDescriptor));

        CompositingLayer* layer = iCompositing->GetLayer(FastSymbol("Draw"), FastSymbol("opaque"), FastSymbol("main render"));
        SG_ASSERT(nullptr != layer);
        m_layer = layer;
        m_layer->Register(m_renderBatch.get());
    }
    ~DrawCubes()
    {
        m_layer->Unregister(m_renderBatch.get());
    }
    virtual void Draw() override
    {
        static size_t anim = 0;
        anim++;
#if SG_ENABLE_ASSERT
        size_t const cubeCount[3] ={5, 3, 2};
        float const length = 0.5f;
        float const delta = length * (1.f + 1.5f);
#else
        size_t const cubeCount[3] ={50, 20, 5};
        float const length = 0.1f;
        float const delta = length * (1.f + 2.5f);
#endif

        size_t const cubeVertexCount = 24;
        SG_CODE_FOR_ASSERT(size_t const vertexCount = cubeVertexCount * cubeCount[0] * cubeCount[1] * cubeCount[2];)
        float3 const& center = float3(0, 0, 0);
        float2 const texs[4] ={float2(0, 0), float2(0, 1), float2(1, 0), float2(1, 1)};
        float3 const normals[6] ={float3(-1, 0, 0),
            float3(0, -1, 0),
            float3(0, 0, -1),
            float3(1, 0, 0),
            float3(0, 1, 0),
            float3(0, 0, 1)};
        float3 const exs[6] ={float3(0, 0, 1),
            float3(0, 0, 1),
            float3(1, 0, 0),
            float3(0, 0, 1),
            float3(0, 0, 1),
            float3(1, 0, 0)};
        float4 const cols[6] ={float4(1, 0, 0, 1),
            float4(0, 0, 1, 1),
            float4(0, 1, 0, 1),
            float4(1, 1, 0, 1),
            float4(0, 1, 1, 1),
            float4(1, 1, 1, 1)};

        float const t = (anim % 50) * 1.f/50.f;
        float const scale = 0.8f + 0.4f * (-4 * t*t + 4 * t);

        for(size_t c0 = 0; c0 < cubeCount[0]; ++c0)
        for(size_t c1 = 0; c1 < cubeCount[1]; ++c1)
        for(size_t c2 = 0; c2 < cubeCount[2]; ++c2)
        {
            int3 c(checked_numcastable(c0), checked_numcastable(c1), checked_numcastable(c2));
            float3 const cubeCenter = center + float3((-1+2*((c+1)%2)) * ((c+1)/2)) * delta;
            CubeVertex* vertices = m_renderBatch->GetVertexPointerForWriting<CubeVertex>(cubeVertexCount);
            SG_CODE_FOR_ASSERT(size_t writtenVertexCount = 0;)
            SG_ASSERT(nullptr != vertices);
            for(size_t f = 0; f < 6; ++f)
            {
                float3 normal = normals[f];
                float3 ex = exs[f];
                float3 ey = cross(normal, ex);
                for(size_t i = 0; i < 4; ++i)
                {
                    vertices[4*f+i].pos    = cubeCenter + length * scale * (0.5f*normal + (-0.5f+(i%2))*ex + (-0.5f+(i/2))*ey);
                    vertices[4*f+i].normal = normal;
                    vertices[4*f+i].col    = cols[f];
                    vertices[4*f+i].tex    = texs[i];
                    SG_CODE_FOR_ASSERT(++writtenVertexCount;)
                    SG_ASSERT(writtenVertexCount <= vertexCount);
                }
            }
            {
                u32 const refIndices[] =
                {
                    0, 1, 2, 2, 1, 3,
                    4, 5, 6, 6, 5, 7,
                    8, 9, 10, 10, 9, 11,
                    12, 13, 14, 14, 13, 15,
                    16, 17, 18, 18, 17, 19,
                    20, 21, 22, 22, 21, 23,
                };
                size_t const indexCount = SG_ARRAYSIZE(refIndices);
                u32* indices = m_renderBatch->GetIndex32PointerForWriting(indexCount);
                SG_ASSERT(nullptr != indices);
                SG_ASSERT(sizeof(indices[0]) == sizeof(refIndices[0]));
                memcpy(indices, refIndices, indexCount*sizeof(indices[0]));
                m_renderBatch->FinishWritingIndex(indexCount);
            }
            SG_ASSERT(writtenVertexCount == cubeVertexCount);
            m_renderBatch->FinishWritingVertex(cubeVertexCount);
        }
    }
private:
    std::unique_ptr<RenderBatch> m_renderBatch;
    safeptr<CompositingLayer> m_layer;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
SG_TEST((sg,renderengine), RenderEngine, (RenderEngine, super slow))
{
    perflog::Init();
    FastSymbol::Init();
    filesystem::Init();
    filesystem::MountDeclaredMountingPoints();
    reflection::Init();
    rendering::Init();

    {
        rendering::ShaderConstantDatabase constantDatabase;
        {
            rendering::ShaderConstantName name_date_in_frames = "date_in_frames";
            constantDatabase.AddVariable(name_date_in_frames, new rendering::ShaderVariable<u32>());
        }

        reflection::ObjectDatabase db;
        objectscript::ErrorHandler errorHandler;
        bool ok = objectscript::ReadObjectScriptWithRetryROK(FilePath("src:/RenderEngine/UnitTests/Simple.os"), db, errorHandler);
        SG_LOG_DEFAULT_DEBUG(errorHandler.GetErrorMessage().c_str());
        SG_ASSERT_AND_UNUSED(ok);

        reflection::ObjectDatabase::named_object_list namedObjects;
        db.GetExportedObjects(namedObjects);

        reflection::BaseClass const* bc = db.GetIFP(reflection::Identifier("::compositing"));
        SG_ASSERT(nullptr != bc);
        SG_ASSERT(bc->GetMetaclass() == CompositingDescriptor::StaticGetMetaclass());
        CompositingDescriptor const* compositingDesc = checked_cast<CompositingDescriptor const*>(bc);

        TestApplication app(&constantDatabase);
        rendering::RenderWindow* window = app.RenderWindow(0);
        rendering::IRenderTarget* renderTargets[] = { window->BackBuffer() };
        rendering::IShaderConstantDatabase const* constantDbArrray[] = { &constantDatabase };

        ArrayView<rendering::IShaderResource*> const inputSurfaces;
        ArrayView<rendering::IRenderTarget*> const outputSurfaces = AsArrayView(renderTargets);
        ArrayView<rendering::IShaderConstantDatabase const*> const constantDatabases = AsArrayView(constantDbArrray);
        ArrayView<rendering::IShaderResourceDatabase const*> const resourceDatabases;
        refptr<ICompositing> compositing = compositingDesc->CreateInstance(app.RenderDevice(), inputSurfaces, outputSurfaces, constantDatabases, resourceDatabases);

        DrawCubes drawCubes(compositing.get());
        app.AddDrawable(&drawCubes);
        app.AddCompositing(compositing.get());
        app.Run();
        app.RemoveDrawable(&drawCubes);
    }

    rendering::Shutdown();
    reflection::Shutdown();
    filesystem::Shutdown();
    FastSymbol::Shutdown();
    perflog::Shutdown();
}
//=============================================================================
}
}
#endif

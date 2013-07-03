#include "stdafx.h"

#include <Applications/AppUtils/ApplicationLauncher.h>
#include <Applications/AppUtils/UnitTestsUtils.h>
#include <Core/ArrayView.h>
#include <Core/Config.h>
#include <Core/FileSystem.h>
#include <Core/PerfLog.h>
#include <Rendering/InitShutdown.h>
#include <Reflection/InitShutdown.h>

#if SG_ENABLE_UNIT_TESTS
#define Include_DeclareTests
#include <Core/DeclareTests.h>
#include <Geometry/DeclareTests.h>
#include <Image/DeclareTests.h>
#include <Math/DeclareTests.h>
#include <ObjectScript/DeclareTests.h>
#include <Reflection/DeclareTests.h>
#include <RenderEngine/DeclareTests.h>
#include <Rendering/DeclareTests.h>
#include <UserInterface/DeclareTests.h>
#undef Include_DeclareTests
#endif

#if SG_ENABLE_TOOLS
#include <Applications/AppUtils/FontCodeGenerator.h>
#include <ToolsUI/DemoApplication.h>
#include <Core/Tool.h>
#endif

#define Include_DeclareMetaclasses
#include <Applications/AppUtils/DeclareMetaclasses.h>
#include <Reflection/DeclareMetaclasses.h>
#include <RenderEngine/DeclareMetaclasses.h>
#include <ToolsUI/DeclareMetaclasses.h>
#include <UserInterface/DeclareMetaclasses.h>
#undef Include_DeclareMetaclasses

namespace {

void Init()
{
#if SG_ENABLE_TOOLS
    sg::tools::Init();
#endif
#if SG_ENABLE_PERF_LOG
    sg::perflog::Init();
#endif
    sg::FastSymbol::Init();
    sg::filesystem::Init();
    sg::filesystem::MountDeclaredMountingPoints();
    sg::reflection::Init();
    sg::rendering::Init();
}
void Shutdown()
{
    sg::rendering::Shutdown();
    sg::reflection::Shutdown();
    sg::filesystem::Shutdown();
    sg::FastSymbol::Shutdown();
#if SG_ENABLE_PERF_LOG
    sg::perflog::Shutdown();
#endif
#if SG_ENABLE_TOOLS
    sg::tools::Shutdown();
#endif
}

sg::ArrayView<sg::ApplicationDescriptor> GetApplicationDescriptors();

template <typename T>
void LaunchApplication()
{
    Init();
    {
        T app;
        app.Run();
    }
    Shutdown();
}

template <void (*Fct)()>
void LaunchApplication()
{
    Init();
    {
        Fct();
    }
    Shutdown();
}

size_t g_nextAppIndex = 0;
template <>
void LaunchApplication<sg::ApplicationLauncher>()
{
    Init();
    {
        sg::ApplicationLauncher app;
        g_nextAppIndex = app.RunReturnNextAppIndex(GetApplicationDescriptors());
    }
    Shutdown();
}

void RunAllApplications()
{
    sg::ArrayView<sg::ApplicationDescriptor> descs = GetApplicationDescriptors();
    for_range(size_t, i, 2, descs.size())
    {
        descs[i].launch();
    }
}

void Quit() { g_nextAppIndex = -1; }

sg::ApplicationDescriptor applicationDescriptors[] = {
    { "Application Launcher",   LaunchApplication<sg::ApplicationLauncher> },
    { "Run all applications",   RunAllApplications },
#if SG_ENABLE_UNIT_TESTS
    { "Run fast unit tests",    sg::RunFastUnitTests },
    { "Run all unit tests",     sg::RunAllUnitTests },
    { "Run perf tests",         sg::RunPerfUnitTests },
#endif
#if SG_ENABLE_TOOLS
    { "Generate fonts code",    LaunchApplication<sg::image::tool::GenerateFontCodes> },
    { "Demo ToolsUI",           LaunchApplication<sg::toolsui::DemoApplication> },
#endif
    { "Quit",                   Quit },
};

sg::ArrayView<sg::ApplicationDescriptor> GetApplicationDescriptors()
{
    return AsArrayView(applicationDescriptors);
}

}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                       _In_opt_ HINSTANCE hPrevInstance,
                       _In_ LPTSTR    lpCmdLine,
                       _In_ int       nCmdShow)
{
#if 0
    LaunchApplication<sg::app::chess::ChessApplication>();
    return 0;
#endif

#if SG_ENABLE_UNIT_TESTS
    sg::RunUnitTestsAtStartup();
#endif

    do
    {
        SG_ASSERT(g_nextAppIndex < SG_ARRAYSIZE(applicationDescriptors));
        size_t const  appIndex = g_nextAppIndex;
        g_nextAppIndex = 0;
        applicationDescriptors[appIndex].launch();
        SG_ASSERT(g_nextAppIndex == -1 || g_nextAppIndex < SG_ARRAYSIZE(applicationDescriptors));
    } while(-1 != g_nextAppIndex);

    return 0;
}

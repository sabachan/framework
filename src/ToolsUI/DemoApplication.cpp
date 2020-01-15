#include "stdafx.h"

#include <Core/Config.h>
#if SG_ENABLE_TOOLS

#include "DemoApplication.h"

#include "Button.h"
#include "CheckBox.h"
#include "Common.h"
#include "Label.h"
#include "ScrollingContainer.h"
#include "Slider.h"
#include "TextField.h"
#include "TreeView.h"
#include "Window.h"
#include <Core/Log.h>
#include <Core/PerfLog.h>
#include <Core/Singleton.h>
#include <Core/StringFormat.h>
#include <Core/StringUtils.h>
#include <Core/Tool.h>
#include <Reflection/ObjectDatabase.h>
#include <RenderEngine/Compositing.h>
#include <RenderEngine/CompositingLayer.h>
#include <Rendering/RenderDevice.h>
#include <Rendering/RenderWindow.h>
#include <Rendering/ShaderCache.h>
#include <ObjectScript/Reader.h>
#include <System/KeyboardUtils.h>
#include <System/Window.h>
#include <UserInterface/Component.h>
#include <UserInterface/Container.h>
#include <UserInterface/Context.h>
#include <UserInterface/FramingContainer.h>
#include <UserInterface/GenericStyleGuide.h>
#include <UserInterface/ListLayout.h>
#include <UserInterface/PointerEvent.h>
#include <UserInterface/Root.h>
#include <UserInterface/SensitiveArea.h>


#if SG_ENABLE_TOOLS
//#include <Core/Tool.h>
#endif

namespace sg {
namespace toolsui {
//=============================================================================
DemoApplication::DemoApplication()
    : name_viewport_resolution("viewport_resolution")
{
    m_renderDevice.reset(new rendering::RenderDevice());

    for(size_t i = 0; i < 1; ++i)
    {
        m_windowHandles.push_back(new system::Window());
        m_renderWindows.push_back(new rendering::RenderWindow(m_renderDevice.get(), m_windowHandles.back().get()));
    }

    m_windowHandles[0]->SetClientPlacement(box2i::FromMinDelta(int2(100, 100), int2(800, 600)));

    std::vector<rendering::IRenderTarget*> windowsAsRenderTargets;
    for(auto const& it : m_renderWindows)
        windowsAsRenderTargets.push_back(it.get());

    this->RegisterUserInputListener(this, 0);

    m_shaderConstantDatabase = new rendering::ShaderConstantDatabase;
    m_shaderConstantDatabase->AddVariable(name_viewport_resolution, new rendering::ShaderVariable<float2>());

    rendering::ShaderConstantName name_date_in_frames = "date_in_frames";
    m_shaderConstantDatabase->AddVariable(name_date_in_frames, new rendering::ShaderVariable<u32>());

    m_windowHandles[0]->ClientSize().RegisterObserver(this);
    VirtualOnNotified(&(m_windowHandles[0]->ClientSize()));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
DemoApplication::~DemoApplication()
{
    m_windowHandles[0]->ClientSize().UnregisterObserver(this);
    this->UnregisterUserInputListener(this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void DemoApplication::Run()
{
    reflection::ObjectDatabase db;
    objectscript::ErrorHandler errorHandler;
    bool ok = objectscript::ReadObjectScriptROK(FilePath("src:/ToolsUI/Data/DemoApplication.os"), db, errorHandler);
    SG_LOG_DEFAULT_DEBUG(errorHandler.GetErrorMessage().c_str());
    SG_ASSERT(ok);

    Common& common = Common::Get();

    reflection::ObjectDatabase::named_object_list namedObjects;
    db.GetExportedObjects(namedObjects);

    reflection::BaseClass const* bcCompositingDesc = db.GetIFP(reflection::Identifier("::compositing"));
    SG_ASSERT(nullptr != bcCompositingDesc);
    SG_ASSERT(bcCompositingDesc->GetMetaclass() == renderengine::CompositingDescriptor::StaticGetMetaclass());
    renderengine::CompositingDescriptor const* compositingDesc = checked_cast<renderengine::CompositingDescriptor const*>(bcCompositingDesc);

    rendering::RenderWindow* window = m_renderWindows[0].get();
    //rendering::IRenderTarget* renderTargets[] = { window->BackBufferAsLinearRGB() };
    rendering::IRenderTarget* renderTargets[] = { window->BackBuffer() };

    ArrayView<rendering::IShaderResource*> const inputSurfaces;
    ArrayView<rendering::IRenderTarget*> const outputSurfaces = AsArrayView(renderTargets);
    rendering::IShaderConstantDatabase const* constantDatabases_data[] = { m_shaderConstantDatabase.get() };
    ArrayView<rendering::IShaderConstantDatabase const*> const constantDatabases = AsArrayView(constantDatabases_data);
    ArrayView<rendering::IShaderResourceDatabase const*> const resourceDatabases;
    refptr<renderengine::ICompositing> compositing = compositingDesc->CreateInstance(m_renderDevice.get(), inputSurfaces, outputSurfaces, constantDatabases, resourceDatabases);
    m_compositing = compositing;

    renderengine::CompositingLayer* layer = compositing->GetLayer(FastSymbol("Draw"), FastSymbol("gui2D"));
    SG_ASSERT(nullptr != layer);
    m_layer = layer;

    m_uiRoot.reset(new ui::Root(m_windowHandles[0].get(), m_layer.get()));

    m_uiRoot->AddComponent(common.ModalContainer(), 100);
    m_uiRoot->AddComponent(common.WindowContainer());

    Window* w = new Window;
    w->SetTitle(L"A Window");
    w->SetInfo(L"This is really a beautiful window, it even comes with an info box!");
    w->SetClientSize(float2(60, 300));
    w->SetClientOffset(float2(50, 50));
    //w->SetMinClientSizeRelativeToContent(ui::Relative(float2(1,1)));
    w->SetMinClientSize_RelativeToContent(ui::Relative(float2(1,0)) + ui::Magnifiable(float2(0,40)));
    w->SetMaxClientSize_RelativeToContent(ui::Relative(float2(1,1)));
    WindowHandler whnd(w);
    whnd.Show(true);

    //m_uiRoot->AddComponent(w);
    ui::VerticalListLayout* list = new ui::VerticalListLayout(common.GetMagnifier(), common.VerticalListProperties());
    //m_uiRoot->AddComponent(list);
    list->AppendItem(new Label(L"Hello world!"));

    Foldable* foldable = new Foldable();
    SG_CODE_FOR_TOOLS(foldable->SetNameForTools("foldable");)
    foldable->SetNonFoldableContent(new Label(L"Tree 0"));
    foldable->SetFoldableContent(new Label(L"This content is not visible when folded but, now, it's unfolded."));
    list->AppendItem(foldable);
    Foldable* foldable1 = new Foldable();
    SG_CODE_FOR_TOOLS(foldable1->SetNameForTools("foldable1");)
    foldable1->SetNonFoldableContent(new Label(L"Long description for a simple foldable component"));
    FoldableList* foldable2 = new FoldableList();
    SG_CODE_FOR_TOOLS(foldable2->SetNameForTools("foldable2");)
    foldable2->SetNonFoldableContent(new Label(L"A foldable list"));
    FoldableList* foldable3 = new FoldableList();
    SG_CODE_FOR_TOOLS(foldable3->SetNameForTools("foldable3");)
    foldable3->SetFraming(Foldable::NoFrame);
    //foldable3->SetNonFoldableContent(new Label(L"Sub list 1"));
    foldable3->SetNonFoldableContent(new TextButton(L"Sub list 1"));
    foldable3->AppendFoldableContent(new Label(L"This content was folded 3 times, but you managed to unfold it!"));
    foldable3->AppendFoldableContent(new Label(L"Bravo!"));
    foldable2->AppendFoldableContent(foldable3);
    FoldableList* foldable4 = new FoldableList();
    SG_CODE_FOR_TOOLS(foldable4->SetNameForTools("foldable3");)
    foldable4->SetNonFoldableContent(new Label(L"Sub list 2"));
    foldable4->AppendFoldableContent(new Label(L"Hey!"));
    foldable2->AppendFoldableContent(foldable4);
    foldable2->AppendFoldableContent(new Label(L"End of the foldable List"));
    foldable1->SetFoldableContent(foldable2);
    list->AppendItem(foldable1);

    TreeView* treeView = new TreeView();
    list->AppendItem(treeView);

    treeView->Insert("Root/Toto/Field1", new Label(L"Field 1"));
    treeView->Insert("Root/Toto/Field2", new Label(L"Field 2"));
    treeView->Insert("Root/Tata", new Label(L"Tata overriden"));
    treeView->Insert("Root/Tata/Field1", new Label(L"Field 1"));
    treeView->Insert("Root/Tata/Field2", new TextField(L"Field 2"));
    treeView->Insert("Root/Titi/Field1", new Label(L"Field 1"));
    treeView->Insert("Root/Titi/Field2", new Label(L"Field 2"));

    treeView->Remove("Root/Tata");
    treeView->Remove("Root/Toto/Field1");
    treeView->Remove("Root/Titi/Field1");
    treeView->Remove("Root/Titi/Field2");

    list->AppendItem(new Label(ConvertUTF8ToUCS2("#kanji{鯖ちゃん}")));
    list->AppendItem(new Label(ConvertUTF8ToUCS2("#small{a small label}")));
    list->AppendItem(new Label(ConvertUTF8ToUCS2("#xsmall{an extra small label}")));
    list->AppendItem(new Label(ConvertUTF8ToUCS2("#xsmall{portez un wisky au vieux juge blond qui fume la pipe}")));
    list->AppendItem(new Label(ConvertUTF8ToUCS2("#xsmall{ctrl + C}")));
    list->AppendItem(new Label(ConvertUTF8ToUCS2("#xsmall{ctrl + M}")));
    list->AppendItem(new TextButton(L"Button"));
    list->AppendItem(new TextButton(L"Button with a lot of text that may need to be split into multiple lines if it exceeds the width of the container."));
    list->AppendItem(new TextButton(L"Another button, with a long-big-word-with-a-lot-of-letters that may go out of the frame."));
    list->AppendItem(new TextCheckBox(L"CheckBox"));
    list->AppendItem(new TextIntSlider(L"My slider", -10, 10));
    list->AppendItem(new TextFloatSlider(L"My float slider", 0, 100, 0.1f));

    ScrollingContainer* sc = new ScrollingContainer(ui::FitMode2(ui::FitMode::FitToMaxContentAndFrame, ui::FitMode::FitToMinContentAndFrame));
    sc->SetContent(list);

    w->SetContent(sc);

    ui::VerticalListLayout* list2 = new ui::VerticalListLayout(common.GetMagnifier(), common.VerticalListProperties());
    for_range(size_t, i, 0, 100)
        list2->AppendItem(new Label(ConvertUTF8ToUCS2(Format("Label %0", i))));

    ScrollingContainer* sc2 = new ScrollingContainer(ui::FitMode2(ui::FitMode::FitToMaxContentAndFrame, ui::FitMode::FitToMinContentAndFrame));
    sc2->SetContent(list2);
    ui::FrameProperty frameProp;
    frameProp.size = ui::Magnifiable(float2(0, 100)) + ui::Relative(float2(1, 0));
    //ui::FramingContainer* fc = new ui::FramingContainer(common.GetMagnifier(), frameProp, ui::FitMode2(ui::FitMode::FitToMaxContentAndFrame, ui::FitMode::FitToMaxContentAndFrame));
    //fc->SetContent(sc2);

    //list->AppendItem(fc);
    list->AppendExpansibleItem(sc2, ui::Magnifiable(100), 1);

    list->AppendItem(new TextCheckBox(L"A second check box"));
    list->AppendItem(new TextField(L"Please write me"));
    list->AppendItem(new TextField(L"Write me too"));

    system::WindowedApplication::Run();

    whnd.Show(false);
    whnd.ReleaseWindow_AssumeNotShown();

    m_uiRoot.reset();
    m_layer = nullptr;
    m_compositing = nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void DemoApplication::VirtualOnNotified(ObservableValue<uint2> const* iObservable)
{
    float2 wh = float2(iObservable->Get());
    rendering::IShaderVariable* ivar = m_shaderConstantDatabase->GetConstantForWriting(name_viewport_resolution);
    rendering::ShaderVariable<float2>* var = checked_cast<rendering::ShaderVariable<float2>*>(ivar);
    var->Set(wh);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void DemoApplication::VirtualOneTurn()
{
    SG_CPU_PERF_LOG_SCOPE(0);

#if SG_ENABLE_TOOLS
    // TODO: Add a tool option.
    sg::rendering::shadercache::InvalidateOutdatedShaders();
#endif

    {
        // TODO: TimeServer
        static u32 dateInFrames = 0;
        dateInFrames++;
        rendering::ShaderConstantName name_date_in_frames = "date_in_frames"; // TODO: keep as member
        rendering::IShaderVariable* ivar = m_shaderConstantDatabase->GetConstantForWriting(name_date_in_frames);
        rendering::ShaderVariable<u32>* var = checked_cast<rendering::ShaderVariable<u32>*>(ivar);
        var->Set(dateInFrames);
    }

    //m_uiTimeServer.Update();

    m_renderDevice->BeginRender();

    {
        //SIMPLE_CPU_PERF_LOG_SCOPE("ApplicationLauncher::OneTurn - Draw UI");
        m_uiRoot->Draw();
    }
    {
        //SIMPLE_CPU_PERF_LOG_SCOPE("ApplicationLauncher::OneTurn - Render");
        m_compositing->Execute(FastSymbol("Draw"));
    }

    m_renderDevice->EndRender();

    system::WindowedApplication::VirtualOneTurn();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void DemoApplication::OnUserInputEvent(system::UserInputEvent const& iEvent)
{
    system::KeyboardShortcutAdapter const quitEntry(system::KeyboardKey::Escape, indeterminate, indeterminate, indeterminate, system::KeyboardLayout::UserLayout);
    if(quitEntry.IsTriggered(iEvent))
    {
        iEvent.SetMasked();
        PostQuitMessage(0);
    }

    int a = 0;
}
//=============================================================================
}
}
#endif

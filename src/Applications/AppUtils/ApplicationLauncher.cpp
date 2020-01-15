#include "stdafx.h"
#include "ApplicationLauncher.h"

//#include "DevRenderer.h"
#include <Core/Log.h>
#include <Core/PerfLog.h>
#include <Core/StringFormat.h>
#include <Core/StringUtils.h>
#include <Core/VectorOfScopedPtr.h>
#include <Math/NumericalUtils.h>
#include <ObjectScript/Reader.h>
#include <Reflection/BaseClass.h>
#include <Reflection/CommonTypes.h>
#include <Reflection/ObjectDatabase.h>
#include <RenderEngine/Compositing.h>
#include <RenderEngine/CompositingLayer.h>
#include <Rendering/ExampleObjects.h>
#include <Rendering/RenderDevice.h>
#include <Rendering/RenderWindow.h>
#include <Rendering/ShaderCache.h>
#include <System/KeyboardUtils.h>
#include <System/UserInputEvent.h>
#include <System/Window.h>
#if SG_ENABLE_TOOLS
#include <ToolsUI/Button.h>
#include <ToolsUI/Label.h>
#include <ToolsUI/ScrollingContainer.h>
#include <ToolsUI/TreeView.h>
#endif
#include <UserInterface/AnimFactor.h>
#include <UserInterface/Component.h>
#include <UserInterface/Container.h>
#include <UserInterface/FrameProperty.h>
#include <UserInterface/GenericStyleGuide.h>
#include <UserInterface/ListLayout.h>
#include <UserInterface/Magnifier.h>
#include <UserInterface/Movable.h>
#include <UserInterface/Root.h>
#include <UserInterface/SensitiveArea.h>
#include <UserInterface/Text.h>
#include <UserInterface/TextFormatScript.h>
#include <UserInterface/TextStyle.h>
#include <UserInterface/TextureDrawer.h>
#include <UserInterface/TypefaceFromBitMapFont.h>
#include <UserInterface/UniformDrawer.h>
#include <sstream>

namespace sg {
//=============================================================================
REFLECTION_ABSTRACT_CLASS_BEGIN((sg),ILaunchable)
REFLECTION_m_PROPERTY(name)
REFLECTION_CLASS_END
//=============================================================================
void Append(ArrayList<ApplicationDescriptor>& oDescriptors, ArrayView<ApplicationDescriptor const> iDescriptors)
{
    for(auto& it : iDescriptors)
        oDescriptors.emplace_back(it);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
namespace {
    void CallLaunch(void* arg) { ILaunchable* l = static_cast<ILaunchable*>(arg); l->Launch(); }
}
void Append(ArrayList<ApplicationDescriptor>& oDescriptors, ILaunchable* iLaunchable)
{
    auto& d = oDescriptors.EmplaceBack();
    d.name = iLaunchable->Name();
    d.launch = CallLaunch;
    d.arg = iLaunchable;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void AppendLaunchablesInObjectScript(ArrayList<ApplicationDescriptor>& oDescriptors, ArrayList<refptr<ILaunchable>>& oLifeHandler, FilePath const& iFilepath)
{
    reflection::ObjectDatabase db;
    objectscript::ErrorHandler errorHandler;
    bool ok = objectscript::ReadObjectScriptWithRetryROK(iFilepath, db, errorHandler);
    SG_LOG_DEFAULT_DEBUG(errorHandler.GetErrorMessage().c_str());
    SG_ASSERT_AND_UNUSED(ok);

    reflection::ObjectDatabase::named_object_list namedObjects;
    db.GetExportedObjects(namedObjects);

    for(auto const& it : namedObjects)
    {
        reflection::Metaclass const* mc = it.second->GetMetaclass();
        if(ILaunchable::StaticGetMetaclass()->IsBaseOf(mc))
        {
            ILaunchable* launchable = checked_cast<ILaunchable*>(it.second.get());
            oLifeHandler.EmplaceBack(launchable);
            Append(oDescriptors, launchable);
        }
    }
}
//=============================================================================
class ApplicationLauncherStyleGuide : public ui::GenericStyleGuide
{
    REFLECTION_CLASS_HEADER(ApplicationLauncherStyleGuide, ui::GenericStyleGuide)
public:
    ApplicationLauncherStyleGuide()
        : m_magnifier(1)
    {
        rendering::RenderDevice* renderDevice = rendering::RenderDevice::GetIFP();
        SG_ASSERT(nullptr != renderDevice);
        m_typeface.reset(new ui::TypefaceFromBitMapFont(renderDevice));
    }
    using ui::GenericStyleGuide::GetUniformDrawer;
    using ui::GenericStyleGuide::GetTextureDrawer;
    ui::UniformDrawer const* GetUniformDrawer() const { return ui::GenericStyleGuide::GetUniformDrawer(Default); }
    ui::TextureDrawer const* GetTetureDrawer() const { return ui::GenericStyleGuide::GetTextureDrawer(Default); }
    ui::ITypeface const* GetTypeface() const { return ui::GenericStyleGuide::GetTypeface(Default);}
    ui::Magnifier const& GetMagnifier() const { return m_magnifier; }
    float Magnification() const { return m_magnifier.Magnification(); }
public:
    FastSymbol FillColor { "FillColor" };
    FastSymbol LineColor { "LineColor" };
    FastSymbol HighlightColor { "HighlightColor" };
    FastSymbol ClickColor { "ClickColor" };
    FastSymbol LineThickness { "LineThickness" };
    FastSymbol LineThickness2 { "LineThickness2" };
    FastSymbol TextHMargin { "TextHMargin" };
    FastSymbol TextTopMargin { "TextTopMargin" };
    FastSymbol TextBottomMargin { "TextBottomMargin" };
    FastSymbol Default { "Default" };
private:
    scopedptr<ui::ITypeface const> m_typeface;
    ui::Magnifier m_magnifier;
};
REFLECTION_CLASS_BEGIN((sg), ApplicationLauncherStyleGuide)
    REFLECTION_CLASS_DOC("")
REFLECTION_CLASS_END
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class AppButton final : public ui::Component
                      , public ui::IMovable
                      , public UnsharableObservable<AppButton>
                      , private ui::ISensitiveAreaListener
                      , private ui::IAnimFactorEventListener
{
    typedef ui::Component parent_type;
public:
    AppButton(ApplicationLauncherStyleGuide const& iStyleGuide, std::wstring const& iName, size_t index)
        : m_styleGuide(&iStyleGuide)
        , m_frameProperty()
        , m_sensitiveArea()
        , m_text()
        , m_textPos()
        , m_index(index)
        , m_visibleAnimFactor(0.5f)
        , m_clickAnimFactor(0.3f)
    {
        m_frameProperty.size.x() = {0, 0, 1.f};
        ApplicationLauncherStyleGuide const& styleGuide = iStyleGuide;
        m_text.SetStyles(iStyleGuide.GetTypeface(), &styleGuide.GetTextStyle(styleGuide.Default), styleGuide.GetTFS(styleGuide.Default), &styleGuide.GetParagraphStyle(styleGuide.Default));
        m_text.SetText(iName);
        m_clickAnimFactor.AddEvent(1.f, this, ui::AnimFactorEventFlag::Forward_StopAtKey);
    }
    ~AppButton()
    {
    }
    size_t Index() const { return m_index; }
    virtual void VirtualResetOffset() override
    {
        m_frameProperty.offset.x().unit = 0;
        m_frameProperty.offset.y().unit = 0;
    }
    virtual void VirtualAddOffset(float2 const& iOffset) override
    {
        m_frameProperty.offset.x().unit += iOffset.x();
        m_frameProperty.offset.y().unit += iOffset.y();
    }
private:
    bool IsHover() const { return m_sensitiveArea.IsPointerInsideFree(); }
    bool IsClicked() const { return m_sensitiveArea.IsClickedValidable(0); }
    virtual void VirtualOnAnimFactorKey(void* iAnimFactor, float iKey, ui::AnimFactorEventFlag iFlags) override
    {
        SG_UNUSED((iAnimFactor, iKey, iFlags));
        NotifyObservers();
    }
    virtual void VirtualOnPointerEvent(ui::PointerEventContext const& iContext, ui::PointerEvent const& iPointerEvent) override
    {
        parent_type::VirtualOnPointerEvent(iContext, iPointerEvent);
        m_sensitiveArea.OnPointerEvent(iContext, iPointerEvent, m_boxArea, *this);
    }
    virtual void OnButtonDownToUp(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override
    {
        SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_ONE_BUTTON;
        SG_ASSERT(&m_sensitiveArea == iSensitiveArea);
        if(0 == iButton)
        {
            MoveToFrontOfAllUI();
            m_clickAnimFactor.PlayForward();
        }
    }
    virtual void VirtualOnDraw(ui::DrawContext const& iContext) override
    {
        ApplicationLauncherStyleGuide const& styleGuide = *m_styleGuide;
        float const magnification = m_styleGuide->Magnification();
        //box2f const& parentBox = Parent()->PlacementBox();
        box2f const& placementBox = PlacementBox();
        float const visibleFactor = m_visibleAnimFactor.GetValue();
        float const clickFactor = m_clickAnimFactor.GetValue();
        float2 const a = lerp(float2(0.1f, 0.3f), float2(1.f), tweening::decelerate(visibleFactor));
        float2 const b = lerp(float2(0), float2(8), tweening::parabolicArc(tweening::linearStep(0.f, 0.8f, clickFactor)));
        box2f const box = box2f::FromCenterDelta(placementBox.Center(), placementBox.Delta() * a + b);
        ui::UniformDrawer const* drawer = m_styleGuide->GetUniformDrawer();
        float const lineThickness = styleGuide.GetLength(IsClicked() ? styleGuide.LineThickness2 : styleGuide.LineThickness).Resolve(magnification, std::numeric_limits<float>::lowest());
        if(AllGreaterStrict(box.Delta(), float2(2.f * lineThickness)))
        {
            box2f const inBox = box2f::FromMinMax(box.Min() + lineThickness, box.Max() - lineThickness);
            Color4f const color = styleGuide.GetLinearColor(IsHover() ? styleGuide.HighlightColor : IsClicked() ? styleGuide.ClickColor : styleGuide.FillColor);
            drawer->DrawQuad(iContext, inBox, color);
            Color4f const lineColor = styleGuide.GetLinearColor(styleGuide.LineColor);
            drawer->DrawFrame(iContext, inBox, box, lineColor);
        }
        m_text.Draw(iContext, m_textPos);
        // TODO: enable dynamic color modification of text
        //m_text.Draw(iContext, m_textPos+float2(1,1));
        parent_type::VirtualOnDraw(iContext);
    }
    virtual void VirtualUpdatePlacement() override
    {
        ApplicationLauncherStyleGuide const& styleGuide = *m_styleGuide;
        float const magnification = m_styleGuide->Magnification();
        box2f const& parentBox = Parent()->PlacementBox();
        box2f const textBox = m_text.Box();
        float const hMargin = styleGuide.GetLength(styleGuide.TextHMargin).Resolve(magnification, parentBox.Delta().x());
        float const topMargin = styleGuide.GetLength(styleGuide.TextTopMargin).Resolve(magnification, parentBox.Delta().y());
        float const bottomMargin = styleGuide.GetLength(styleGuide.TextBottomMargin).Resolve(magnification, parentBox.Delta().y());
        float2 const contentSize = textBox.Delta() + float2(2.f * hMargin, topMargin + bottomMargin);
        box2f const frame = m_frameProperty.Resolve(magnification, parentBox, contentSize);
        SetPlacementBox(frame);
        m_boxArea.SetBox(frame);
        ui::Text::Alignment textAlignment;
        textAlignment.alignementToAnchor = float2(0.5f,0.5f);
        textAlignment.useBaselinesInY = 0;
        textAlignment.useTextAlignmentInX = 0;
        m_textPos = frame.Center() + m_text.ComputeOffset(textAlignment);
    }
    virtual void VirtualOnInsertInUI() override
    {
        parent_type::VirtualOnInsertInUI();
        m_visibleAnimFactor.SetValue(0);
        m_visibleAnimFactor.PlayForward();
        m_clickAnimFactor.SetValue(0);
        m_clickAnimFactor.Pause();
    }
    virtual ui::Component* VirtualAsComponent() override { return this; }
private:
    safeptr<ApplicationLauncherStyleGuide const> m_styleGuide;
    ui::FrameProperty m_frameProperty;
    ui::SensitiveArea m_sensitiveArea;
    ui::BoxArea m_boxArea;
    ui::Text m_text;
    float2 m_textPos;
    size_t m_index;
    ui::AnimFactorOnTimeServer<ApplicationLauncher::UITimeServer> m_visibleAnimFactor;
    ui::AnimFactorWithEventsOnTimeServer<ApplicationLauncher::UITimeServer> m_clickAnimFactor;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class MainWidget final : public ui::Container
                       , public Observer<AppButton>
{
public:
    MainWidget(ApplicationLauncherStyleGuide const& iStyleGuide
               , sg::ArrayView<sg::ApplicationDescriptor const> const& iAppDescriptors
               , size_t* oNextAppIndex)
        : Observer<AppButton>(SG_CODE_FOR_ASSERT(allow_destruction_of_observable))
        , m_styleGuide(&iStyleGuide)
        , m_pNextAppIndex(oNextAppIndex)
    {
        ui::VerticalListLayout::Properties prop;
        prop.margins.interItem = ui::Magnifiable(10);
        prop.margins.left = ui::Magnifiable(15);
        prop.margins.top = ui::Magnifiable(20);
        prop.margins.right = ui::Magnifiable(15);
        prop.margins.bottom = ui::Magnifiable(30);
        prop.widthFitMode = ui::FitMode::FitToContentOnly;
        ui::VerticalListLayout* list = new ui::VerticalListLayout(iStyleGuide.GetMagnifier(), prop);
        RequestAddToFront(list);
        for(sg::ApplicationDescriptor const& it : iAppDescriptors)
        {
            std::wstring const name = ConvertUTF8ToUCS2(it.name);
            size_t const index = &it - &*iAppDescriptors.begin();
            AppButton* button = new AppButton(iStyleGuide, name, index);
            button->RegisterObserver(this);
            list->AppendItem(button);
        }
    }
    ~MainWidget()
    {
        RequestRemoveAll();
    }
    virtual void VirtualOnNotified(AppButton const* iButton) override
    {
        *m_pNextAppIndex = iButton->Index();
        PostQuitMessage(0);
    }
private:
    safeptr<ApplicationLauncherStyleGuide const> m_styleGuide;
    size_t* m_pNextAppIndex;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
ApplicationLauncher::ApplicationLauncher()
    : name_viewport_resolution("viewport_resolution")
{
    m_renderDevice.reset(new rendering::RenderDevice());

    for(size_t i = 0; i < 1; ++i)
    {
        m_windowHandles.push_back(new system::Window());
        m_renderWindows.push_back(new rendering::RenderWindow(m_renderDevice.get(), m_windowHandles.back().get()));
    }

    m_windowHandles[0]->SetClientSize(uint2(400, 600));

    std::vector<rendering::IRenderTarget*> windowsAsRenderTargets;
    for(auto const& it : m_renderWindows)
        windowsAsRenderTargets.push_back(it.get());

    this->RegisterUserInputListener(this, 0);

    m_shaderConstantDatabase = new rendering::ShaderConstantDatabase;
    m_shaderConstantDatabase->AddVariable(name_viewport_resolution, new rendering::ShaderVariable<float2>());
    m_windowHandles[0]->ClientSize().RegisterObserver(this);
    VirtualOnNotified(&(m_windowHandles[0]->ClientSize()));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ApplicationLauncher::~ApplicationLauncher()
{
    m_windowHandles[0]->ClientSize().UnregisterObserver(this);
    this->UnregisterUserInputListener(this);
    for(auto& it : m_windowHandles)
        it->CloseIFP();
    m_renderWindows.clear();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
size_t ApplicationLauncher::RunReturnNextAppIndex(sg::ArrayView<sg::ApplicationDescriptor const> const& iAppDescriptors)
{
    reflection::ObjectDatabase db;
    objectscript::ErrorHandler errorHandler;
    bool ok = objectscript::ReadObjectScriptWithRetryROK(FilePath("src:/Applications/AppUtils/Data/ApplicationLauncher.os"), db, errorHandler);
    SG_LOG_DEFAULT_DEBUG(errorHandler.GetErrorMessage().c_str());
    SG_ASSERT_AND_UNUSED(ok);

    reflection::ObjectDatabase::named_object_list namedObjects;
    db.GetExportedObjects(namedObjects);

    reflection::BaseClass const* bcCompositingDesc = db.GetIFP(reflection::Identifier("::compositing"));
    SG_ASSERT(nullptr != bcCompositingDesc);
    SG_ASSERT(bcCompositingDesc->GetMetaclass() == renderengine::CompositingDescriptor::StaticGetMetaclass());
    renderengine::CompositingDescriptor const* compositingDesc = checked_cast<renderengine::CompositingDescriptor const*>(bcCompositingDesc);

    rendering::RenderWindow* window = m_renderWindows[0].get();
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

    reflection::BaseClass const* bcStyleGuide = db.GetIFP(reflection::Identifier("::styleGuide"));
    SG_ASSERT(nullptr != bcStyleGuide);
    SG_ASSERT(bcStyleGuide->GetMetaclass() == ApplicationLauncherStyleGuide::StaticGetMetaclass());
    ApplicationLauncherStyleGuide const* styleGuide = checked_cast<ApplicationLauncherStyleGuide const*>(bcStyleGuide);
    size_t nextAppIndex = all_ones;
    m_uiRoot->AddComponent(new MainWidget(*styleGuide, iAppDescriptors, &nextAppIndex));

    system::WindowedApplication::Run();

    m_uiRoot.reset();
    m_layer = nullptr;
    m_compositing = nullptr;

    return nextAppIndex;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ApplicationLauncher::VirtualOnNotified(ObservableValue<uint2> const* iObservable)
{
    float2 wh = float2(iObservable->Get());
    rendering::IShaderVariable* ivar = m_shaderConstantDatabase->GetConstantForWriting(name_viewport_resolution);
    rendering::ShaderVariable<float2>* var = checked_cast<rendering::ShaderVariable<float2>*>(ivar);
    var->Set(wh);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ApplicationLauncher::VirtualOneTurn()
{
    SG_CPU_PERF_LOG_SCOPE(0);
#if SG_ENABLE_TOOLS
    // TODO: Add a tool option.
    sg::rendering::shadercache::InvalidateOutdatedShaders();
#endif
#if 0
    static float2 p = float2(1,0);
    p.x() += 0.01f * p.y();
    p.y() -= 0.01f * p.x();
    uint2 const pos = uint2(roundi(p * 100 + float2(200)));
    m_windowHandles[0]->SetClientPlacement(box2u::FromMinDelta(pos, uint2(400, 600)));
    uint2 const wh = m_windowHandles[0]->ClientSize().Get();
#endif

    m_uiTimeServer.Update();

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
void ApplicationLauncher::OnUserInputEvent(system::UserInputEvent const& iEvent)
{
    // proto
    system::KeyboardEntryAdapter const quitEntry(system::KeyboardKey::Escape, system::KeyboardLayout::UserLayout);
    if(system::UserInputEventType::OffToOn == iEvent.EventType() && quitEntry.DoesMatch(iEvent))
        PostQuitMessage(0);

    SG_BREAKABLE_POS;
}
//=============================================================================
#if SG_ENABLE_TOOLS
//=============================================================================
ApplicationLauncherV2::ApplicationLauncherV2()
    : Observer<toolsui::Button>(allow_destruction_of_observable)
    , name_viewport_resolution("viewport_resolution")
    , m_nextAppIndex(0)
{
    m_renderDevice.reset(new rendering::RenderDevice());

    for(size_t i = 0; i < 1; ++i)
    {
        m_windowHandles.push_back(new system::Window());
        m_renderWindows.push_back(new rendering::RenderWindow(m_renderDevice.get(), m_windowHandles.back().get()));
    }

    m_windowHandles[0]->SetClientSize(uint2(400, 600));

    std::vector<rendering::IRenderTarget*> windowsAsRenderTargets;
    for(auto const& it : m_renderWindows)
        windowsAsRenderTargets.push_back(it.get());

    this->RegisterUserInputListener(this, 0);

    m_shaderConstantDatabase = new rendering::ShaderConstantDatabase;
    m_shaderConstantDatabase->AddVariable(name_viewport_resolution, new rendering::ShaderVariable<float2>());
    m_windowHandles[0]->ClientSize().RegisterObserver(this);
    VirtualOnNotified(&(m_windowHandles[0]->ClientSize()));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ApplicationLauncherV2::~ApplicationLauncherV2()
{
    m_windowHandles[0]->ClientSize().UnregisterObserver(this);
    this->UnregisterUserInputListener(this);
    for(auto& it : m_windowHandles)
        it->CloseIFP();
    m_renderWindows.clear();
    if(nullptr != m_launchable)
        m_launchable->Launch();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
size_t ApplicationLauncherV2::RunReturnNextAppIndex(sg::ArrayView<sg::ApplicationDescriptor const> const& iAppDescriptors)
{
    reflection::ObjectDatabase db;
    objectscript::ErrorHandler errorHandler;
    bool ok = objectscript::ReadObjectScriptWithRetryROK(FilePath("src:/Applications/AppUtils/Data/ApplicationLauncherV2.os"), db, errorHandler);
    SG_LOG_DEFAULT_DEBUG(errorHandler.GetErrorMessage().c_str());
    SG_ASSERT_AND_UNUSED(ok);

    reflection::ObjectDatabase::named_object_list namedObjects;
    db.GetExportedObjects(namedObjects);

    reflection::BaseClass const* bcCompositingDesc = db.GetIFP(reflection::Identifier("::compositing"));
    SG_ASSERT(nullptr != bcCompositingDesc);
    SG_ASSERT(bcCompositingDesc->GetMetaclass() == renderengine::CompositingDescriptor::StaticGetMetaclass());
    renderengine::CompositingDescriptor const* compositingDesc = checked_cast<renderengine::CompositingDescriptor const*>(bcCompositingDesc);

    rendering::RenderWindow* window = m_renderWindows[0].get();
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

    toolsui::Common& common = toolsui::Common::Get();
    m_uiRoot->AddComponent(common.ModalContainer(), 100);
    m_uiRoot->AddComponent(common.WindowContainer());

    m_nextAppIndex = all_ones;

    toolsui::ScrollingContainer* sc = new toolsui::ScrollingContainer();
    m_uiRoot->AddComponent(sc);

    toolsui::TreeView* treeView = new toolsui::TreeView();
    sc->SetContent(treeView);

    for(auto const& it : iAppDescriptors)
    {
        char const* name = strrchr(it.name, '/');
        if(nullptr == name)
            name = it.name;
        else
            name += 1;
        toolsui::TextButton* b = new toolsui::TextButton(ConvertUTF8ToUCS2(name));
        b->SetUserData(&it-iAppDescriptors.Data());
        b->RegisterObserver(this);
        treeView->Insert(it.name, b);
    }

    treeView->RequestMoveFocusReturnHasMoved(ui::FocusDirection::Down);

    system::WindowedApplication::Run();

    m_uiRoot.reset();
    m_layer = nullptr;
    m_compositing = nullptr;

    return m_nextAppIndex;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ApplicationLauncherV2::VirtualOnNotified(toolsui::Button const* iObservable)
{
    m_nextAppIndex = iObservable->UserData();
    PostQuitMessage(0);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ApplicationLauncherV2::VirtualOnNotified(ObservableValue<uint2> const* iObservable)
{
    SG_UNUSED(iObservable);
    float2 wh = float2(iObservable->Get());
    rendering::IShaderVariable* ivar = m_shaderConstantDatabase->GetConstantForWriting(name_viewport_resolution);
    rendering::ShaderVariable<float2>* var = checked_cast<rendering::ShaderVariable<float2>*>(ivar);
    var->Set(wh);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ApplicationLauncherV2::VirtualOnDropFile(char const* iFilePath, system::Window* iWindow, uint2 const& iPosition)
{
    SG_UNUSED((iWindow, iPosition));
    FilePath file = FilePath::CreateFromFullSystemPath(iFilePath);
    OpenFile(file);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ApplicationLauncherV2::OpenFile(FilePath const& iPath)
{
    if(iPath.Extension() == "os")
    {
        reflection::ObjectDatabase db;
        objectscript::ErrorHandler errorHandler;
        bool ok = objectscript::ReadObjectScriptWithRetryROK(iPath, db, errorHandler);
        SG_LOG_DEFAULT_DEBUG(errorHandler.GetErrorMessage().c_str());
        SG_ASSERT_AND_UNUSED(ok);

        reflection::ObjectDatabase::named_object_list namedObjects;
        db.GetExportedObjects(namedObjects);

        for(auto const& it : namedObjects)
        {
            reflection::Metaclass const* mc = it.second->GetMetaclass();
            if(ILaunchable::StaticGetMetaclass()->IsBaseOf(mc))
            {
                m_launchable = checked_cast<ILaunchable*>(it.second.get());
                PostQuitMessage(0);
            }
            else
            {
                SG_ASSERT_MSG(false, "unsupported object"); // todo: message box
            }
        }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ApplicationLauncherV2::VirtualOneTurn()
{
    SG_CPU_PERF_LOG_SCOPE(0);
#if SG_ENABLE_TOOLS
    // TODO: Add a tool option.
    sg::rendering::shadercache::InvalidateOutdatedShaders();
#endif
#if 0
    static float2 p = float2(1,0);
    p.x() += 0.01f * p.y();
    p.y() -= 0.01f * p.x();
    uint2 const pos = uint2(roundi(p * 100 + float2(200)));
    m_windowHandles[0]->SetClientPlacement(box2u::FromMinDelta(pos, uint2(400, 600)));
    uint2 const wh = m_windowHandles[0]->ClientSize().Get();
#endif

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
void ApplicationLauncherV2::OnUserInputEvent(system::UserInputEvent const& iEvent)
{
    // proto
    system::KeyboardEntryAdapter const quitEntry(system::KeyboardKey::Escape, system::KeyboardLayout::UserLayout);
    if(system::UserInputEventType::OffToOn == iEvent.EventType() && quitEntry.DoesMatch(iEvent))
        PostQuitMessage(0);

    SG_BREAKABLE_POS;
}
//=============================================================================
#endif
//=============================================================================
}

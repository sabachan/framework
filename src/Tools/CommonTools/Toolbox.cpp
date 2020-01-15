#include "stdafx.h"

#include <Core/Config.h>

#define SG_TOOLBOX_CPP

#include "Toolbox.h"

#if SG_ENABLE_TOOLS
#include <ToolsUI/Button.h>
#include <ToolsUI/CheckBox.h>
#include <ToolsUI/Common.h>
#include <ToolsUI/Label.h>
#include <ToolsUI/ScrollingContainer.h>
#include <ToolsUI/Slider.h>
#include <ToolsUI/TreeView.h>
#include <Core/StringFormat.h>
#include <Core/StringUtils.h>
#include <Core/Tool.h>
#include <UserInterface/AnimFactor.h>
#include <UserInterface/FrameProperty.h>
#include <UserInterface/Movable.h>
#include <UserInterface/Text.h>
#endif

namespace sg {
namespace commontools {
//=============================================================================
#if SG_ENABLE_TOOLS
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float const showDuration = 0.3f;
float const insertDuration = 2.f;
float const hidenOffset = 12.f;
class ToolboxMainButton : public toolsui::TextButton
                        , private Observer<toolsui::Button>
{
    typedef TextButton parent_type;
public:
    ToolboxMainButton(Toolbox* iToolbox)
        : TextButton(L"Toolbox")
        , m_toolbox(iToolbox)
        , m_show(showDuration)
        , m_insertAnim(insertDuration)
    {
        SG_ASSERT(nullptr != m_toolbox);
        SetFitMode(ui::FitMode2(ui::FitMode::FitToMaxContentAndFrame, ui::FitMode::FitToMaxContentAndFrame));
        RegisterObserver(this);
    }
    ~ToolboxMainButton()
    {
        UnregisterObserver(this);
    }
private:
    virtual void VirtualOnNotified(Button const* iObservable) override
    {
        SG_ASSERT_AND_UNUSED(iObservable == this);
        m_toolbox->ShowToolboxWindow(true);
    }
    virtual void VirtualOnDraw(ui::DrawContext const& iContext) override
    {
        toolsui::Common const& common = toolsui::Common::Get();
        float const magnification = common.GetMagnifier().Magnification();

        bool paused = m_show.IsPaused() & m_insertAnim.IsPaused();
        float const t0 = m_insertAnim.GetValue();
        float const t1 = m_show.GetValue();
        if(SensitiveArea().IsPointerInside() || SensitiveArea().IsClicked(0))
        {
            if(1 != t1 || m_show.IsBackward()) m_show.PlayForward();
        }
        else
        {
            if(0 != t1 || !m_show.IsBackward()) m_show.PlayBackward();
        }

        float drawOffset = 0.f;

        if(!paused)
        {
            float const endInsert = tweening::linearStep(1.f - showDuration / insertDuration, 1.f, t0);
            float const t2 = t0 >= 1.f ? 0.f : tweening::decelerate(tweening::linearStep(0.05f, 0.05f + showDuration / insertDuration, t0)) - endInsert;
            float const t3 = t0 >= 1.f ? 0.f : 1 - endInsert;
            float const t4 = std::max(t1, t2);
            float const t5 = 1-std::max(t1, t3);

            ui::FrameProperty fp;
            fp.anchorAlignment = float2(0.5f, 1.f);
            fp.alignmentToAnchor = float2(0.5f, lerp(0.f, 1.f, t4));
            fp.offset = ui::Magnifiable(float2(0.f, lerp(0.f, -hidenOffset, t5)));
            fp.size = ui::Relative(float2(0.3f, 0.f));
            SetFrameProperty(fp);

            drawOffset = -fp.offset.y().Resolve(magnification, 0.f);

            if(1 == t0 && !m_insertAnim.IsBackward()) m_insertAnim.Pause();
            else if(0 == t0 && m_insertAnim.IsBackward()) m_insertAnim.Pause();
            if(1 == t1 && !m_show.IsBackward()) m_show.Pause();
            else if(0 == t1 && m_show.IsBackward()) m_show.Pause();
        }
        else
        {
            if(t1 == 0)
                drawOffset = ui::Magnifiable(hidenOffset).Resolve(magnification, 0.f);;
        }

        UpdatePlacementIFN();

        float4x4 const transform = matrix::HomogeneousTranslation(float3(0, drawOffset, 0));
        float4x4 const invtransform = matrix::HomogeneousTranslation(float3(0,-drawOffset, 0));
        ui::DrawContext context(iContext, transform, invtransform, ui::Context::TransformType::Translate2D);

        parent_type::VirtualOnDraw(context);
    }
    virtual void VirtualOnInsertInUI() override
    {
        parent_type::VirtualOnInsertInUI();
        m_insertAnim.SetValue(0.f);
        m_insertAnim.PlayForward();
        m_show.SetValue(0.f);
        m_show.PlayBackward();
        SG_ASSERT(m_insertAnim.GetValue() == 0.f);
        SG_ASSERT(m_show.GetValue() == 0.f);
        ui::FrameProperty fp;
        fp.anchorAlignment = float2(0.5f, 1.f);
        fp.alignmentToAnchor = float2(0.5f, 0.f);
        fp.size = ui::Relative(float2(0.3f, 0.f));
        SetFrameProperty(fp);
    }
private:
    safeptr<Toolbox> m_toolbox;
    ui::AnimFactorOnTimeServer<toolsui::CommonTimeServer> m_show;
    ui::AnimFactorOnTimeServer<toolsui::CommonTimeServer> m_insertAnim;

};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class TriggerWidget : public toolsui::TextButton
                            , private Observer<toolsui::Button>
{
    PARENT_SAFE_COUNTABLE(TextButton)
    typedef toolsui::Button parent_type;
public:
    TriggerWidget(tools::Trigger* iTool)
        : TextButton(ConvertUTF8ToUCS2(iTool->ShortName()))
        , m_tool(iTool)
    {
        RegisterObserver(this);
    }
    ~TriggerWidget() { UnregisterObserver(this); }
private:
    virtual void VirtualOnNotified(Button const* iObservable) override
    {
        SG_ASSERT_AND_UNUSED(this == iObservable);
        m_tool->NotifyObservers();
    }
private:
    safeptr<tools::Trigger> m_tool;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class EditableBoolWidget : public toolsui::TextCheckBox
                         , private Observer<toolsui::CheckBox>
                         , private Observer<tools::EditableValueBool>
{
    PARENT_SAFE_COUNTABLE(TextCheckBox)
    typedef toolsui::TextCheckBox parent_type;
public:
    EditableBoolWidget(tools::EditableValueBool* iTool)
        : TextCheckBox(ConvertUTF8ToUCS2(iTool->ShortName()))
        , m_tool(iTool)
    {
        RegisterObserver(this);
        iTool->RegisterObserver(this);
        SetState(m_tool->Value());
    }
    ~EditableBoolWidget() { UnregisterObserver(this); m_tool->UnregisterObserver(this); }
private:
    virtual void VirtualOnNotified(CheckBox const* iObservable) override
    {
        SG_ASSERT_AND_UNUSED(this == iObservable);
        if(m_tool->Value() != GetState())
            m_tool->SetValue(GetState());
    }
    virtual void VirtualOnNotified(tools::EditableValueBool const* iObservable) override
    {
        SG_ASSERT_AND_UNUSED(m_tool == iObservable);
        if(m_tool->Value() != GetState())
            SetState(m_tool->Value());
    }
    virtual void VirtualOnDraw(ui::DrawContext const& iContext) override
    {
        SG_ASSERT(m_tool->Value() == GetState());
        parent_type::VirtualOnDraw(iContext);
    }
private:
    safeptr<tools::EditableValueBool> m_tool;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T = int>
class EditableIntWidget : public toolsui::TextIntegralSlider<T>
                        , private Observer<toolsui::IntegralSlider<T>>
                        , private Observer<tools::EditableValue<T>>
{
    PARENT_SAFE_COUNTABLE(toolsui::TextIntegralSlider<T>)
    typedef toolsui::TextIntegralSlider<T> parent_type;
public:
    EditableIntWidget(tools::EditableValue<T>* iTool)
        : TextIntegralSlider(ConvertUTF8ToUCS2(iTool->ShortName()), iTool->Min(), iTool->Max(), iTool->Step())
        , m_tool(iTool)
    {
        RegisterObserver(this);
        iTool->RegisterObserver(this);
        SetValue(m_tool->Value());
    }
    ~EditableIntWidget() { UnregisterObserver(this); m_tool->UnregisterObserver(this); }
private:
    virtual void VirtualOnNotified(IntegralSlider<T> const* iObservable) override
    {
        SG_ASSERT_AND_UNUSED(this == iObservable);
        if(m_tool->Value() != GetValue())
            m_tool->SetValue(GetValue());
    }
    virtual void VirtualOnNotified(tools::EditableValue<T> const* iObservable) override
    {
        SG_ASSERT_AND_UNUSED(m_tool == iObservable);
        if(m_tool->Value() != GetValue())
            SetValue(m_tool->Value());
    }
    virtual void VirtualOnDraw(ui::DrawContext const& iContext) override
    {
        SG_ASSERT(m_tool->Value() == GetValue());
        parent_type::VirtualOnDraw(iContext);
    }
private:
    safeptr<tools::EditableValue<T>> m_tool;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T = float>
class EditableFloatWidget : public toolsui::TextFloatSlider
                          , private Observer<toolsui::FloatSlider>
                          , private Observer<tools::EditableValue<T>>
{
    PARENT_SAFE_COUNTABLE(toolsui::TextFloatSlider)
    typedef toolsui::TextFloatSlider parent_type;
public:
    EditableFloatWidget(tools::EditableValue<T>* iTool)
        : TextFloatSlider(ConvertUTF8ToUCS2(iTool->ShortName()), iTool->Min(), iTool->Max(), iTool->Step())
        , m_tool(iTool)
    {
        RegisterObserver(this);
        iTool->RegisterObserver(this);
        SetValue(m_tool->Value());
    }
    ~EditableFloatWidget() { UnregisterObserver(this); m_tool->UnregisterObserver(this); }
private:
    virtual void VirtualOnNotified(FloatSlider const* iObservable) override
    {
        SG_ASSERT_AND_UNUSED(this == iObservable);
        if(m_tool->Value() != GetValue())
            m_tool->SetValue(GetValue());
    }
    virtual void VirtualOnNotified(tools::EditableValue<T> const* iObservable) override
    {
        SG_ASSERT_AND_UNUSED(m_tool == iObservable);
        if(m_tool->Value() != GetValue())
            SetValue(m_tool->Value());
    }
    virtual void VirtualOnDraw(ui::DrawContext const& iContext) override
    {
        SG_ASSERT(m_tool->Value() == GetValue());
        parent_type::VirtualOnDraw(iContext);
    }
private:
    safeptr<tools::EditableValue<T>> m_tool;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
Toolbox::Toolbox()
    : m_mainButton()
    , m_toolboxModificationStamp(all_ones)
{
    m_mainButton = new ToolboxMainButton(this);
    RequestAddToBack(m_mainButton.get());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Toolbox::~Toolbox()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Toolbox::ShowToolboxWindow(bool iShow)
{
    if(iShow != m_windowHandler.IsShown())
    {
        if(iShow)
        {
            SG_ASSERT(m_mainButton->Parent() == this);
            RequestRemove(m_mainButton.get());
        }
        m_windowHandler.Show(iShow);
        UpdateToolboxWindowIFN();
    }
    SG_ASSERT(iShow == m_windowHandler.IsShown());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Toolbox::CreateToolboxWindow()
{
    if(nullptr == toolsui::Common::GetIFP())
        return;

    toolsui::Window* w = new toolsui::Window;
    w->SetTitle(L"Toolbox");
    //w->SetInfo(L"This is really a beautiful window, it even comes with an info box!");
    w->SetClientSize(float2(250, 300));
    w->SetClientOffset(float2(150, 50));
    w->SetMinClientSize_RelativeToContent(ui::Relative(float2(1,0)) + ui::Magnifiable(float2(0,40)));
    w->SetMaxClientSize_RelativeToContent(ui::Relative(float2(1,0)) + ui::Unit(float2(0, std::numeric_limits<float>::infinity())));
    m_windowHandler.SetWindow_AssumeNone(w);

    toolsui::ScrollingContainer* sc = new toolsui::ScrollingContainer(ui::FitMode2(ui::FitMode::FitToMaxContentAndFrame, ui::FitMode::FitToMinContentAndFrame));
    w->SetContent(sc);

    toolsui::TreeView* tree = new toolsui::TreeView();
    sc->SetContent(tree);
    m_tree = tree;

    m_toolboxModificationStamp =  tools::Toolbox::Get().GetModificationStamp() - 1;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Toolbox::UpdateToolboxWindowIFN()
{
    if(m_windowHandler.IsShown() && m_toolboxModificationStamp != tools::Toolbox::Get().GetModificationStamp())
        UpdateToolboxWindow();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
namespace {
class EmptyComponent : public ui::Component {};
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Toolbox::UpdateToolboxWindow()
{
    tools::Toolbox& toolbox = tools::Toolbox::Get();
    ArrayView<refptr<tools::ITool> const> tools = toolbox.Tools();

    for(refptr<tools::ITool> const& it : tools)
    {
        tools::ToolType type = it->GetType();
        if(nullptr != m_tree->GetIFP(it->Name()))
            continue;
        ui::IMovable* toAdd = nullptr;
        switch(type)
        {
        case tools::ToolType::EditableValue_bool:
            toAdd = new EditableBoolWidget(checked_cast<tools::EditableValueBool*>(it.get()));
            break;
        case tools::ToolType::EditableValue_int:
            toAdd = new EditableIntWidget<int>(checked_cast<tools::EditableValue<int>*>(it.get()));
            break;
        case tools::ToolType::EditableValue_size_t:
            toAdd = new EditableIntWidget<size_t>(checked_cast<tools::EditableValue<size_t>*>(it.get()));
            break;
        case tools::ToolType::EditableValue_float:
            toAdd = new EditableFloatWidget<float>(checked_cast<tools::EditableValue<float>*>(it.get()));
            break;
        case tools::ToolType::Command:
            toAdd = new toolsui::Label(ConvertUTF8ToUCS2(Format("%0 (command)", it->ShortName())));
            break;
        case tools::ToolType::Trigger:
            toAdd = new TriggerWidget(checked_cast<tools::Trigger*>(it.get()));
            break;
        default:
            SG_ASSUME_NOT_REACHED();
        }
        SG_ASSERT(nullptr != toAdd);
        m_tree->Insert(it->Name(), toAdd);
    }
    m_toolboxModificationStamp = toolbox.GetModificationStamp();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Toolbox::VirtualOnPointerEvent(ui::PointerEventContext const& iContext, ui::PointerEvent const& iPointerEvent)
{
    parent_type::VirtualOnPointerEvent(iContext, iPointerEvent);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Toolbox::VirtualOnDraw(ui::DrawContext const& iContext)
{
    parent_type::VirtualOnDraw(iContext);
    UpdateToolboxWindowIFN();
    if(!m_windowHandler.IsShown() && !m_mainButton->IsInGUI())
        RequestAddToBack(m_mainButton.get());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Toolbox::VirtualOnInsertInUI()
{
    parent_type::VirtualOnInsertInUI();
    CreateToolboxWindow();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Toolbox::VirtualOnRemoveFromUI()
{
    m_windowHandler.Show(false);
    m_windowHandler.ReleaseWindow_AssumeNotShown();
    parent_type::VirtualOnRemoveFromUI();
}
//=============================================================================
#if SG_ENABLE_TOOLS
void ToolboxLoader::VirtualOnCreated(reflection::ObjectCreationContext& iContext)
{
    reflection_parent_type::VirtualOnCreated(iContext);
    m_common = toolsui::Common::GetIFP();
    m_common->EndCreationIFN(iContext);
    m_toolbox = new Toolbox;
    m_common->WindowContainer()->RequestAddToFront(m_toolbox.get(), 1);
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#endif
//=============================================================================
REFLECTION_CLASS_BEGIN((sg,commontools), ToolboxLoader)
    REFLECTION_CLASS_DOC("")
REFLECTION_CLASS_END
//=============================================================================
}
}


#undef SG_TOOLBOX_CPP

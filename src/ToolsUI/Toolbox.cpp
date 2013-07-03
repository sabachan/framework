#include "stdafx.h"

#include <Core/Config.h>
#if SG_ENABLE_TOOLS

#include "Toolbox.h"

#include "Button.h"
#include "CheckBox.h"
#include "Common.h"
#include "Label.h"
#include "ScrollingContainer.h"
#include "Slider.h"
#include <Core/StringFormat.h>
#include <Core/StringUtils.h>
#include <Core/Tool.h>
#include <UserInterface/AnimFactor.h>
#include <UserInterface/FrameProperty.h>

namespace sg {
namespace toolsui {
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float const showDuration = 0.3f;
float const insertDuration = 2.f;
float const hidenOffset = 12.f;
class ToolboxMainButton : public TextButton
                        , private Observer<Button>
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
        SG_ASSERT(iObservable == this);
        m_toolbox->ShowToolboxWindow(true);
    }
    virtual void VirtualOnDraw(ui::DrawContext const& iContext) override
    {
        Common const& common = Common::Get();
        float const magnification = common.GetMagnifier().Magnification();
        box2f const& parentBox = Parent()->PlacementBox();

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
        ui::DrawContext context(iContext, transform, ui::Context::TransformType::Translate2D);

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
class EditableBoolWidget : public TextCheckBox
                         , private Observer<CheckBox>
                         , private Observer<tools::EditableValueBool>
{
    PARENT_SAFE_COUNTABLE(TextCheckBox)
    typedef TextCheckBox parent_type;
public:
    EditableBoolWidget(tools::EditableValueBool* iTool)
        : TextCheckBox(ConvertUTF8ToUCS2(iTool->Name()))
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
        SG_ASSERT(this == iObservable);
        if(m_tool->Value() != GetState())
            m_tool->SetValue(GetState());
    }
    virtual void VirtualOnNotified(tools::EditableValueBool const* iObservable) override
    {
        SG_ASSERT(m_tool == iObservable);
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
class EditableIntWidget : public TextIntegralSlider<T>
                        , private Observer<IntegralSlider<T>>
                        , private Observer<tools::EditableValue<T>>
{
    PARENT_SAFE_COUNTABLE(TextCheckBox)
    typedef TextIntegralSlider parent_type;
public:
    EditableIntWidget(tools::EditableValue<T>* iTool)
        : TextIntegralSlider(ConvertUTF8ToUCS2(iTool->Name()), iTool->Min(), iTool->Max(), iTool->Step())
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
        SG_ASSERT(this == iObservable);
        if(m_tool->Value() != GetValue())
            m_tool->SetValue(GetValue());
    }
    virtual void VirtualOnNotified(tools::EditableValue<T> const* iObservable) override
    {
        SG_ASSERT(m_tool == iObservable);
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
    , m_toolboxModificationStamp(-1)
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
    if(nullptr == Common::GetIFP()) return;
    Common const& common = Common::Get();

    Window* w = new Window;
    w->SetTitle(L"Toolbox");
    //w->SetInfo(L"This is really a beautiful window, it even comes with an info box!");
    w->SetClientSize(float2(60, 30));
    w->SetClientOffset(float2(150, 50));
    w->SetMinClientSize_RelativeToContent(ui::Relative(float2(1,0)) + ui::Magnifiable(float2(0,40)));
    w->SetMaxClientSize_RelativeToContent(ui::Relative(float2(1,1)));
    m_windowHandler.SetWindow_AssumeNone(w);

    ScrollingContainer* sc = new ScrollingContainer(ui::FitMode2(ui::FitMode::FitToMaxContentAndFrame, ui::FitMode::FitToMinContentAndFrame));
    w->SetContent(sc);

    ui::VerticalListLayout::Properties listprop;
    listprop.margins.left = ui::Magnifiable(4);
    listprop.margins.top = ui::Magnifiable(4);
    listprop.margins.right = ui::Magnifiable(4);
    listprop.margins.bottom = ui::Magnifiable(4);
    listprop.margins.interItem = ui::Magnifiable(5);
    listprop.widthFitMode = ui::FitMode::FitToMaxContentAndFrame;
    ui::VerticalListLayout* list = new ui::VerticalListLayout(common.GetMagnifier(), listprop);
    sc->SetContent(list);

    m_list = list;

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
    SG_ASSERT(nullptr != m_list);
    m_list->RemoveAllItems();

    tools::Toolbox& toolbox = tools::Toolbox::Get();
    ArrayView<refptr<tools::ITool> const> tools = toolbox.Tools();
    std::vector<tools::ITool*> sortedTools;
    sortedTools.reserve(tools.size());
    for(auto const& t : tools)
        sortedTools.push_back(t.get());
    std::sort(sortedTools.begin(), sortedTools.end(), [](tools::ITool const* a, tools::ITool const* b)
    {
        return a->Name() < b->Name();
    });
    for(tools::ITool* it : sortedTools)
    {
        tools::ToolType type = it->GetType();
        switch(type)
        {
        case tools::ToolType::EditableValue_bool:
            m_list->AppendItem(new EditableBoolWidget(checked_cast<tools::EditableValueBool*>(it)));
            break;
        case tools::ToolType::EditableValue_int:
            m_list->AppendItem(new EditableIntWidget<int>(checked_cast<tools::EditableValue<int>*>(it)));
            break;
        case tools::ToolType::EditableValue_size_t:
            m_list->AppendItem(new EditableIntWidget<size_t>(checked_cast<tools::EditableValue<size_t>*>(it)));
            break;
        case tools::ToolType::EditableValue_float:
            m_list->AppendItem(new Label(ConvertUTF8ToUCS2(Format("%0 (float)", it->Name()))));
            break;
        case tools::ToolType::Command:
            m_list->AppendItem(new Label(ConvertUTF8ToUCS2(Format("%0 (command)", it->Name()))));
            break;
        case tools::ToolType::Trigger:
            m_list->AppendItem(new Label(ConvertUTF8ToUCS2(Format("%0 (trigger)", it->Name()))));
            break;
        default:
            SG_ASSUME_NOT_REACHED();
        }
    }
    m_list->AppendExpansibleItem(new EmptyComponent, ui::Length(), 1.f);
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
}
}

#endif

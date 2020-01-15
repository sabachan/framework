#include "stdafx.h"

#include <Core/Config.h>
#if SG_ENABLE_TOOLS

#include "Slider.h"

#include "Label.h"
#include <Core/Tool.h>
#include <Image/FontSymbols.h>
#include <UserInterface/GenericStyleGuide.h>
#include <UserInterface/UniformDrawer.h>
#include <sstream>


namespace sg {
namespace toolsui {
//=============================================================================
Slider::Slider()
    : Slider(ui::FitMode2(ui::FitMode::FitToMaxContentAndFrame, ui::FitMode::FitToContentOnly))
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Slider::Slider(ui::FitMode2 iFitMode)
    : m_frameProperty()
    , m_sensitiveArea()
    , m_boxArea()
    , m_content()
    , m_fitMode(iFitMode)
    , m_prevPointerPosition()
    , m_maxValue(1)
    , m_value(0)
{
    m_frameProperty.size = ui::Relative(float2(1, 1));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Slider::~Slider()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Slider::SetInternalMaxValueAndValue(size_t iMaxValue, size_t iValue)
{
    if(iMaxValue != m_maxValue)
    {
        m_maxValue = iMaxValue;
        m_value = iValue + 1; // in order to trigger NotifyObservers()
    }
    SetInternalValue(iValue);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Slider::SetInternalValue(size_t iValue)
{
    SG_ASSERT(0 <= iValue);
    SG_ASSERT(iValue <= m_maxValue);
    if(iValue != m_value)
    {
        m_value = iValue;
        VirtualOnValueModified();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Slider::SetContent(ui::Component* iContent)
{
    if(iContent == m_content)
        return;
    if(nullptr != m_content)
    {
        RequestRemove(m_content.get());
    }
    if(nullptr != iContent)
    {
        this->RequestAddToFront(iContent);
    }
    m_content = iContent;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if SG_ENABLE_ASSERT
void Slider::CheckConstraintsOnContent(ui::IMovable* iContent)
{
    ui::Component* c = iContent->AsComponent();
    // In the case the iTem is transfered from one parent to another, it
    // may be impossible to check it, as it cannot be added to the test
    // container.
    if(nullptr != c->Parent())
        return;

    // TODO
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Slider::VirtualResetOffset()
{
    m_frameProperty.offset = ui::Unit(float2(0));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Slider::VirtualAddOffset(float2 const& iOffset)
{
    m_frameProperty.offset += ui::Unit(iOffset);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Slider::VirtualOnPointerEvent(ui::PointerEventContext const& iContext, ui::PointerEvent const& iPointerEvent)
{
    parent_type::VirtualOnPointerEvent(iContext, iPointerEvent);
    m_sensitiveArea.OnPointerEvent(iContext, iPointerEvent, m_boxArea, *this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Slider::OnButtonUpToDown(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON)
{
    SG_ASSERT(&m_sensitiveArea == iSensitiveArea);
    if(0 == iButton)
    {
        RequestFocusIFN();
        MoveToFrontOfAllUI();
    }
    m_prevPointerPosition = iPointerLocalPosition.xy();
    OnDrag(iContext, iEvent, iPointerLocalPosition, iButton);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Slider::OnPointerMoveButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON)
{
    SG_ASSERT(&m_sensitiveArea == iSensitiveArea);
    OnDrag(iContext, iEvent, iPointerLocalPosition, iButton);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Slider::OnPointerLeaveButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON)
{
    SG_ASSERT(&m_sensitiveArea == iSensitiveArea);
    OnDrag(iContext, iEvent, iPointerLocalPosition, iButton);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Slider::OnPointerMoveOutsideButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON)
{
    SG_ASSERT(&m_sensitiveArea == iSensitiveArea);
    OnDrag(iContext, iEvent, iPointerLocalPosition, iButton);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Slider::OnButtonDownToUp(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON)
{
    SG_ASSERT(&m_sensitiveArea == iSensitiveArea);
    //OnDrag(iContext, iEvent, iPointerLocalPosition, iButton);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Slider::OnDrag(ui::PointerEventContext const& iContext, ui::PointerEvent const& iEvent, float3 const& iPointerLocalPosition, size_t iButton)
{
    if(0 == iButton)
    {
        box2f const& placementBox = PlacementBox();
        float const lo = placementBox.Min().x();
        float const hi = placementBox.Max().x();
        float const delta = hi - lo;
#if 1 // tentative ergonomy for precise setting
        // Move the pointer inside the box to move reference point broadly.
        // Move the pointer outside the box for precision.
        float const Ox = m_prevPointerPosition.x();
        float const predy = iPointerLocalPosition.y() - placementBox.Center().y();
        float const dy = std::max(0.f, std::abs(predy) / placementBox.Delta().y() - 0.5f);
        if(0 == dy)
            m_prevPointerPosition = iPointerLocalPosition.xy();
        float const expectedMinWidthInPixels = 20.f;
        float const minScale = std::min(1.f, expectedMinWidthInPixels / m_maxValue);
        float const minScaleDistanceInBoxHeight = 4.f;
        float const scale = lerp(1.f, minScale, saturate(dy / minScaleDistanceInBoxHeight));
        //float const scale = std::max(minScale, 1.f / std::exp(dy + 1.f));
        float const x = (iPointerLocalPosition.x() - Ox) * scale + Ox;
#else
        float const x = iPointerLocalPosition.x();
#endif
        float const v = (x - lo) * m_maxValue / delta;
        size_t const value = v <= 0.f ? 0 : std::min(size_t(roundi(v)), m_maxValue);
        SetInternalValue(value);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Slider::VirtualOnFocusableEvent(ui::FocusableEvent const& iFocusableEvent)
{
    focusable_parent_type::VirtualOnFocusableEvent(iFocusableEvent);
    // TODO:
    // - shift + left/right (big steps)
    // - ctrl + left/right (smallest steps)
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Slider::VirtualMoveFocusReturnHasMoved(ui::FocusDirection iDirection)
{
    if(!HasFocus())
    {
        RequestFocusIFN();
        return true;
    }
    size_t const x = m_value;
    switch(iDirection)
    {
    case ui::FocusDirection::Left:
    {
        size_t const value = x > 0 ? x-1 : 0;
        SetInternalValue(value);
        return true;
    }
    case ui::FocusDirection::Right:
    {
        size_t const value = x < m_maxValue ? x+1 : m_maxValue;
        SetInternalValue(value);
        return true;
    }
    default:
        return false;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Slider::VirtualOnDraw(ui::DrawContext const& iContext)
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
    ui::UniformDrawer const* drawer = styleGuide.GetUniformDrawer(common.Default);
    float const magnification = common.GetMagnifier().Magnification();
    box2f const& placementBox = PlacementBox();

    ButtonLikeRenderParam renderParam;
    GetButtonLikeRenderParam(renderParam, placementBox, true, IsHover() || IsClicked(), false, HasTerminalFocus());

    float2 const outDelta = renderParam.outBox.Delta();
    float2 const inDelta = renderParam.inBox.Delta();
    if(AllGreaterStrict(inDelta, float2(0.f)))
    {
        float const x = lerp(placementBox.Min().x(), placementBox.Max().x(), float(m_value) / float(m_maxValue));
        Color4f const colorRight = renderParam.fillColor;
        Color4f const colorLeft = renderParam.fillColor2;
        box2f const leftBox = box2f::FromMinMax(renderParam.inBox.Min(), float2(x, renderParam.inBox.Max().y()));
        box2f const rightBox = box2f::FromMinMax(float2(x, renderParam.inBox.Min().y()), renderParam.inBox.Max());
        if(0 < leftBox.NVolume_NegativeIfNonConvex()) drawer->DrawQuad(iContext, leftBox, colorLeft);
        if(0 < rightBox.NVolume_NegativeIfNonConvex()) drawer->DrawQuad(iContext, rightBox, colorRight);
        if(IsHover() || IsClicked())
        {
            box2f const hlbox = box2f::FromCenterDelta(leftBox.Center(), leftBox.Delta() - 4);
            if(0 < hlbox.NVolume_NegativeIfNonConvex()) drawer->DrawQuad(iContext, hlbox, lerp(colorLeft, colorRight, 0.5f));
        }
    }
    if(outDelta != inDelta && AllGreaterStrict(outDelta, float2(0.f)))
        drawer->DrawFrame(iContext, renderParam.inBox, renderParam.outBox, renderParam.lineColor);

    parent_type::VirtualOnDraw(iContext);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Slider::VirtualUpdatePlacement()
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
    float const magnification = common.GetMagnifier().Magnification();
    box2f const& parentBox = Parent()->PlacementBox();

    float2 const contentSize = [&]() {
        if(m_fitMode.DoesNeedContentSize())
        {
            box2f const preFrame = m_frameProperty.Resolve(magnification, parentBox, float2(0), m_fitMode);
            SetPlacementBox(preFrame);
            box2f const contentBox = m_content->PlacementBox();
            return contentBox.Delta();
        }
        return float2(0);
    } ();

    box2f const frame = m_frameProperty.Resolve(magnification, parentBox, contentSize, m_fitMode);
    SetPlacementBox(frame);
    m_boxArea.SetBox(frame);
}
//=============================================================================
FloatSlider::FloatSlider(float iMinValue, float iMaxValue, float iStep)
    : Slider()
    , m_minValue(iMinValue)
    , m_maxValue(iMaxValue)
    , m_valueStep(iStep)
{
    int const maxInternalValue = roundi((m_maxValue - m_minValue) / m_valueStep);
    SetInternalMaxValueAndValue(maxInternalValue, 0);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
FloatSlider::~FloatSlider()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void FloatSlider::SetValue(float iValue)
{
    SG_ASSERT(m_minValue <= iValue && iValue <= m_maxValue);
    size_t const internalValue = roundi((iValue - m_minValue) / m_valueStep);
    SG_ASSERT(0 <= internalValue && internalValue <= GetInternalMaxValue());
    SetInternalValue(internalValue);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float FloatSlider::GetValue() const
{
    return lerp(m_minValue, m_maxValue, float(GetInternalValue()) / float(GetInternalMaxValue()));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void FloatSlider::VirtualOnValueModified()
{
    NotifyObservers();
}
//=============================================================================
template<typename T>
TextIntegralSlider<T>::TextIntegralSlider(std::wstring const& iText, T iMinValue, T iMaxValue, T iStep)
    : parent_type(iMinValue, iMaxValue, iStep)
    , m_list()
    , m_labelName()
    , m_labelValue()
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();

    m_labelName = new Label(iText); //, ui::Unit(-1.f));
    m_labelValue = new Label(L"", ui::Unit(-1.f));

    ui::Length2 margins = styleGuide.GetVector(common.LineMargin1);
    ui::HorizontalListLayout::Properties listprop;
    listprop.margins.left = margins.x();
    listprop.margins.right = margins.x();
    listprop.margins.top = margins.y();
    listprop.margins.bottom = margins.y();
    listprop.margins.interItem; // = 0;
    listprop.widthFitMode = ui::FitMode::FitToContentOnly;
    m_list = new ui::HorizontalListLayout(common.GetMagnifier(), listprop);

    m_list->AppendExpansibleItem(m_labelName.get(), ui::Unit(10), 1);
    m_list->AppendItem(m_labelValue.get());

    this->SetContent(m_list.get());

    VirtualOnValueModified();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T>
TextIntegralSlider<T>::~TextIntegralSlider()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T>
void TextIntegralSlider<T>::SetText(std::wstring const& iText)
{
    m_labelName->SetText(iText);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T>
void TextIntegralSlider<T>::VirtualOnValueModified()
{
    parent_type::VirtualOnValueModified();

    std::wostringstream oss;
    oss << GetValue();
    m_labelValue->SetText(oss.str());
}
//=============================================================================
template class TextIntegralSlider<int>;
template class TextIntegralSlider<size_t>;
//=============================================================================
TextFloatSlider::TextFloatSlider(std::wstring const& iText, float iMinValue, float iMaxValue, float iStep)
    : parent_type(iMinValue, iMaxValue, iStep)
    , m_list()
    , m_labelName()
    , m_labelValue()
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();

    m_labelName = new Label(iText); //, ui::Unit(-1.f));
    m_labelValue = new Label(L"", ui::Unit(-1.f));

    ui::Length2 margins = styleGuide.GetVector(common.LineMargin1);
    ui::HorizontalListLayout::Properties listprop;
    listprop.margins.left = margins.x();
    listprop.margins.right = margins.x();
    listprop.margins.top = margins.y();
    listprop.margins.bottom = margins.y();
    listprop.margins.interItem; // = 0;
    listprop.widthFitMode = ui::FitMode::FitToContentOnly;
    m_list = new ui::HorizontalListLayout(common.GetMagnifier(), listprop);

    m_list->AppendExpansibleItem(m_labelName.get(), ui::Unit(10), 1);
    m_list->AppendItem(m_labelValue.get());

    this->SetContent(m_list.get());

    VirtualOnValueModified();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TextFloatSlider::~TextFloatSlider()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TextFloatSlider::SetText(std::wstring const& iText)
{
    m_labelName->SetText(iText);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TextFloatSlider::VirtualOnValueModified()
{
    parent_type::VirtualOnValueModified();

    std::wostringstream oss;
    oss << GetValue();
    m_labelValue->SetText(oss.str());
}
//=============================================================================
}
}

#endif

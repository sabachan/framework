#ifndef ToolsUI_Slider_H
#define ToolsUI_Slider_H

#include <Core/Config.h>
#if !SG_ENABLE_TOOLS
#error "This file should be included only with SG_ENABLE_TOOLS"
#endif

#include "Common.h"
#include <UserInterface/Container.h>
#include <UserInterface/Component.h>
#include <UserInterface/FitMode.h>
#include <UserInterface/Focusable.h>
#include <UserInterface/FrameProperty.h>
#include <UserInterface/ListLayout.h>
#include <UserInterface/Movable.h>
#include <UserInterface/SensitiveArea.h>
#include <UserInterface/Text.h>
#include <Core/Cast.h>
#include <Core/Observer.h>

namespace sg {
namespace toolsui {
//=============================================================================
class Label;
//=============================================================================
class Slider : public ui::Container
             , public ui::IMovable
             , private ui::IFocusable
             , private ui::ISensitiveAreaListener
{
    PARENT_SAFE_COUNTABLE(ui::Container)
    typedef ui::Container parent_type;
    typedef ui::IFocusable focusable_parent_type;
//public:
//    struct Properties
//    {
//    };
public:
    ~Slider();
    void SetFrameProperty(ui::FrameProperty const& iFrameProperty)  { InvalidatePlacement(); m_frameProperty = iFrameProperty; }
    void SetFitMode(ui::FitMode2 iFitMode) { InvalidatePlacement(); m_fitMode = iFitMode; }
protected:
    Slider();
    Slider(ui::FitMode2 iFitMode);
    void SetContent(ui::Component* iContent);
    void SetInternalMaxValueAndValue(size_t iMaxValue, size_t iValue);
    void SetInternalValue(size_t iValue);
    size_t GetInternalValue() const { return m_value; }
    size_t GetInternalMaxValue() const { return m_maxValue; }
    virtual void VirtualResetOffset() override;
    virtual void VirtualAddOffset(float2 const& iOffset) override;
    virtual void VirtualOnPointerEvent(ui::PointerEventContext const& iContext, ui::PointerEvent const& iPointerEvent) override;
    virtual void OnButtonUpToDown(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override;
    virtual void OnPointerMoveButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override;
    virtual void OnPointerLeaveButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override;
    virtual void OnPointerMoveOutsideButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override;
    virtual void OnButtonDownToUp(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override;
    virtual void VirtualOnDraw(ui::DrawContext const& iContext) override;
    virtual void VirtualUpdatePlacement() override;
    virtual void VirtualOnFocusableEvent(ui::FocusableEvent const& iFocusableEvent) override;
    virtual bool VirtualMoveFocusReturnHasMoved(ui::FocusDirection iDirection) override;
    virtual ui::Component* VirtualAsComponent() override { return this; }
    virtual IFocusable* VirtualAsFocusableIFP() override { return this; }
    virtual void VirtualOnInsertInUI() override { parent_type::VirtualOnInsertInUI(); OnInsertFocusableInUI(); }
    virtual void VirtualOnRemoveFromUI() override { OnRemoveFocusableFromUI(); parent_type::VirtualOnRemoveFromUI(); }

    ui::SensitiveArea const& SensitiveArea() const { return m_sensitiveArea; }
    virtual void VirtualOnValueModified() = 0;
private:
    bool IsHover() const { return m_sensitiveArea.IsPointerInsideFree(); }
    bool IsClicked() const { return m_sensitiveArea.IsClicked(0); }
    void OnDrag(ui::PointerEventContext const& iContext, ui::PointerEvent const& iEvent, float3 const& iPointerLocalPosition, size_t iButton);
#if SG_ENABLE_ASSERT
    void CheckConstraintsOnContent(ui::IMovable* iContent);
#endif
private:
    ui::FrameProperty m_frameProperty;
    ui::SensitiveArea m_sensitiveArea;
    ui::BoxArea m_boxArea;
    safeptr<ui::Component> m_content;
    ui::FitMode2 m_fitMode;
    float2 m_prevPointerPosition;
    size_t m_maxValue;
    size_t m_value;
};
//=============================================================================
template<typename T>
class IntegralSlider : public Slider
                , public UnsharableObservable<IntegralSlider<T>>
{
    typedef Slider parent_type;
    static_assert(std::is_same<T, int>::value || std::is_same<T, size_t>::value, "");
public:
    IntegralSlider(T iMinValue, T iMaxValue, T iStep = 1)
        : Slider()
        , m_minValue(iMinValue)
        , m_step(iStep)
    {
        SG_ASSERT(0 == (iMaxValue - iMinValue) % iStep);
        SetInternalMaxValueAndValue((iMaxValue - iMinValue) / iStep, 0);
        SG_ASSERT(m_minValue + GetInternalMaxValue() == iMaxValue);
    }
    ~IntegralSlider() {}
    void SetValue(T iValue)
    {
        SG_ASSERT(m_minValue <= iValue && iValue <= T(m_minValue + GetInternalMaxValue()));
        SG_ASSERT(0 == (iValue - m_minValue) % m_step);
        SetInternalValue(size_t((iValue - m_minValue) / m_step));
    }
    int GetValue() const { return checked_numcastable(GetInternalValue() * m_step + m_minValue); }
protected:
    virtual void VirtualOnValueModified() override
    {
        NotifyObservers();
    }
private:
    T m_minValue;
    T m_step;
};
typedef IntegralSlider<int>    IntSlider;
typedef IntegralSlider<size_t> SizeTSlider;
//=============================================================================
class FloatSlider : public Slider
                  , public UnsharableObservable<FloatSlider>
{
    typedef Slider parent_type;
public:
    FloatSlider(float iMinValue, float iMaxValue, float iStep);
    ~FloatSlider();
    void SetValue(float iValue);
    float GetValue() const;
protected:
    virtual void VirtualOnValueModified() override;
private:
    float m_minValue;
    float m_maxValue;
    float m_valueStep;
};
//=============================================================================
template<typename T>
class TextIntegralSlider : public IntegralSlider<T>
{
    typedef IntegralSlider parent_type;
public:
    TextIntegralSlider(std::wstring const& iText, T iMinValue, T iMaxValue, T iStep = 1);
    ~TextIntegralSlider();
    void SetText(std::wstring const& iText);
protected:
    virtual void VirtualOnValueModified() override;
private:
    safeptr<ui::HorizontalListLayout> m_list;
    refptr<Label> m_labelName;
    refptr<Label> m_labelValue;
};
typedef TextIntegralSlider<int>    TextIntSlider;
typedef TextIntegralSlider<size_t> TextSizeTSlider;
//=============================================================================
class TextFloatSlider : public FloatSlider
{
    typedef FloatSlider parent_type;
public:
    TextFloatSlider(std::wstring const& iText, float iMinValue, float iMaxValue, float iStep);
    ~TextFloatSlider();
    void SetText(std::wstring const& iText);
protected:
    virtual void VirtualOnValueModified() override;
private:
    safeptr<ui::HorizontalListLayout> m_list;
    refptr<Label> m_labelName;
    refptr<Label> m_labelValue;
};
//=============================================================================
}
}

#endif

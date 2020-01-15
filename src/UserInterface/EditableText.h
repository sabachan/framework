#ifndef UserInterface_EditableText_H
#define UserInterface_EditableText_H

#include "Component.h"
#include "Focusable.h"
#include "FrameProperty.h"
#include "Length.h"
#include "Magnifier.h"
#include "Movable.h"
#include "SensitiveArea.h"
#include "Text.h"
#include "TextFormatScript.h"
#include "TextRenderer.h"
#include "Typeface.h"

namespace sg {
namespace ui {
//=============================================================================
class EditableText : public Component
                   , public IMovable
                   , public IFocusable
                   , private ISensitiveAreaListener
{
    typedef Component parent_type;
    typedef IFocusable focusable_parent_type;
public:
    struct Properties
    {
        LengthBox2 margins {};
        Length lineWidth {};
        bool overrideParagraphLineWidth { false };
    };
    EditableText(Magnifier const& iMagnifier, Properties const* iProperties = nullptr);
    ~EditableText();
    void SetStyles(ITypeface const* iTypeface, TextStyle const* iTextStyle, ParagraphStyle const* iParagraphStyle);
    void SetStyles(ITypeface const* iTypeface, TextFormatScript const* iTFS, ParagraphStyle const* iParagraphStyle);
    void SetStyles(ITypeface const* iTypeface, TextStyle const* iTextStyle, TextFormatScript const* iTFS, ParagraphStyle const* iParagraphStyle);
    //void SetLineWidth(Length const& iLineWidth);
    void SetProperties(Properties const& iProperties);
    void SetText(std::wstring const& iText);
    void SelectAll();
protected:
    virtual void VirtualResetOffset() override;
    virtual void VirtualAddOffset(float2 const& iOffset) override;
    virtual void VirtualOnPointerEvent(PointerEventContext const& iContext, PointerEvent const& iPointerEvent) override;
    virtual void OnButtonUpToDown(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override;
    virtual void OnButtonDownToUp(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override;
    virtual void OnButtonDownToUpOutside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override { OnButtonDownToUp(SG_UI_SENSITIVE_AREA_LISTENER_FORWARD_PARAMETERS_ONE_BUTTON); }

    virtual void OnPointerMoveButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override;
    virtual void OnPointerEnterButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override { OnPointerMoveButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_FORWARD_PARAMETERS_ONE_BUTTON); }
    virtual void OnPointerLeaveButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override { OnPointerMoveButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_FORWARD_PARAMETERS_ONE_BUTTON); }
    virtual void OnPointerMoveOutsideButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override { OnPointerMoveButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_FORWARD_PARAMETERS_ONE_BUTTON); }

    virtual void VirtualOnFocusableEvent(FocusableEvent const& iFocusableEvent) override;
    virtual bool VirtualMoveFocusReturnHasMoved(FocusDirection iDirection) override;
    virtual void VirtualOnGainFocus() override;
    virtual Component* VirtualAsComponent() override { return this; }
    virtual IFocusable* VirtualAsFocusableIFP() override { return this; }
    virtual void VirtualOnInsertInUI() override { m_acceptNextChar = false; parent_type::VirtualOnInsertInUI(); OnInsertFocusableInUI(); }
    virtual void VirtualOnRemoveFromUI() override { OnRemoveFocusableFromUI(); parent_type::VirtualOnRemoveFromUI(); }
    virtual void VirtualOnDraw(DrawContext const& iContext) override;
    void SkipVirtualOnDraw(DrawContext const& iContext);
    virtual void VirtualUpdatePlacement() override;
protected:
    virtual void VirtualOnRefreshCursor() { /* do nothing */ }
    bool IsInEditionMode() const { return HasFocus() && m_isActivated; }
    float2 TextPosition() const { return m_textPos; }
    ArrayView<box2f const> SelectionBoxes() const { SG_ASSERT_MSG(IsInEditionMode(), "You shouldn't draw selection if text is not in edition mode"); return AsArrayView(m_selectionBoxes); }
    box2f CursorBox() const { SG_ASSERT_MSG(IsInEditionMode(), "You shouldn't draw cursor if text is not in edition mode"); return m_cursorBox + m_cursorPosition; }
    Text /* TODO: const */& DrawableText() { return m_text; }
    SensitiveArea const& GetSensitiveArea() const { return m_sensitiveArea; }
private:
    void SetTextInternal(std::wstring const& iText);
    void UpdateFontMetrics();
    void UpdateSelectionBoxes();
    enum Command
    {
        None,
        Escape,
        EraseLeft,
        EraseRight,
        MoveRight,
        MoveLeft,
        SelectRight,
        SelectLeft,
        SelectSkipRight,
        SelectSkipLeft,
        SkipRight,
        SkipLeft,
        MoveBeginLine,
        MoveEndLine,
        SelectBeginLine,
        SelectEndLine,
        MoveBeginAll,
        MoveEndAll,
        SelectBeginAll,
        SelectEndAll,
        MoveUp,
        MoveDown,
        SelectUp,
        SelectDown,
        Cut,
        Copy,
        Paste,
    };
    void ApplyCommand(Command iCommand);
    void Insert(wchar_t iChar);
    void Insert(std::wstring const& iText);
private:
    safeptr<Magnifier const> m_magnifier;
    std::wstring m_str;
    FrameProperty m_frameProperty;
    SensitiveArea m_sensitiveArea;
    BoxArea m_boxArea;
    ParagraphStyle m_paragraphStyle;
    Text m_text;
    float2 m_textPos;
    Properties m_properties;
    size_t m_cursorBegin;
    size_t m_cursorEnd;
    box2f m_cursorBox;
    float2 m_cursorPosition;
    std::vector<box2f> m_selectionBoxes;
    float2 m_verticalMovePenPosition;
    float m_leading;
    bool m_isLastMoveVertical;
    bool m_isActivated;
    bool m_acceptNextChar;
    SG_CODE_FOR_ASSERT(bool m_areStylesSet;)
};
//=============================================================================
}
}

#endif

//namespace sg {
//namespace ui {
////=============================================================================
//class EditableText : public Component
//                   , public IMovable
//                   , public IFocusable
//{
//    typedef Component parent_type;
//public:
//    EditableText();
//    EditableText(ITypeface const* iTypeface, TextStyle const* iTextStyle, ParagraphStyle const* iParagraphStyle);
//    EditableText(ITypeface const* iTypeface, TextFormatScript const* iTFS, ParagraphStyle const* iParagraphStyle);
//    EditableText(ITypeface const* iTypeface, TextStyle const* iTextStyle, TextFormatScript const* iTFS, ParagraphStyle const* iParagraphStyle);
//    ~EditableText() {}
//    void SetParagraphStyle(ParagraphStyle const* iParagraphStyle);
//    void SetStyles(ITypeface const* iTypeface, TextStyle const* iTextStyle, ParagraphStyle const* iParagraphStyle);
//    void SetStyles(ITypeface const* iTypeface, TextFormatScript const* iTFS, ParagraphStyle const* iParagraphStyle);
//    void SetStyles(ITypeface const* iTypeface, TextStyle const* iTextStyle, TextFormatScript const* iTFS, ParagraphStyle const* iParagraphStyle);
//    std::wstring const& GetText(std::wstring const& iText) const;
//    void SetText(std::wstring const& iText);
//    void SetLineWidth(Length const& iLineWidth);
//    void ResetLineWitdh();
//    void SetTextBoxAlignment(TextBoxAlignment const& iAlignment, Length2 anchor);
//
//private:
//    //box2f Box();
//    //void Draw(DrawContext const& iContext, float2 const& iPosition, size_t iLayer = -1);
//    //void Draw(DrawContext const& iContext, float2 const& iPosition, Alignment const& iAlignment, size_t iLayer = -1);
//    //float2 ComputeOffset(Alignment const& iAlignment);
//
//    virtual void VirtualResetOffset() override;
//    virtual void VirtualAddOffset(float2 const& iOffset) override;
//private:
//    virtual void VirtualOnDraw(DrawContext const& iContext) override;
//    virtual void VirtualUpdatePlacement() override;
//    virtual Component* VirtualAsComponent() override { return this; }
//
//private:
//    //void SetStyles_Impl(ITypeface const* iTypeface, TextStyle const* iTextStyle, TextFormatScript const* iTFS, ParagraphStyle const* iParagraphStyle);
//    //void PrepareIFN() { if(!m_isUpToDate) Prepare(); SG_ASSERT(m_isUpToDate); }
//    //void Prepare();
//private:
//    std::wstring m_text;
//    FrameProperty m_frameProperty;
////    safeptr<TextFormatScript const> m_tfs;
////    safeptr<TextStyle const> m_textStyle;
//    safeptr<ParagraphStyle const> m_paragraphStyle;
////    safeptr<ITypeface const> m_typeface;
//    Text m_text;
//    float2 m_textPos;
//    Length m_lineWidth;
//    ParagraphStyle m_genParagraphStyle;
//    //TextRenderer::Cache m_textCache;
//    bool m_overrideLineWidth;
//    //bool m_isUpToDate;
//};
////=============================================================================
//}
//}
//#endif
//#endif
//
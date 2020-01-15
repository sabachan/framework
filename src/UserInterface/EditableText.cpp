#include "stdafx.h"

#include "EditableText.h"

#include "Container.h"
#include <System/Clipboard.h>
#include <System/KeyboardUtils.h>
#include <Core/StringFormat.h>
#include <cwchar>
#include <locale>
#include <sstream>

namespace sg {
namespace ui {
//=============================================================================
EditableText::EditableText(Magnifier const& iMagnifier, Properties const* iProperties)
    : m_magnifier(&iMagnifier)
    , m_str()
    , m_frameProperty()
    , m_sensitiveArea()
    , m_boxArea()
    , m_paragraphStyle()
    , m_text()
    , m_textPos()
    , m_properties()
    , m_cursorBegin()
    , m_cursorEnd()
    , m_cursorBox()
    , m_selectionBoxes()
    , m_verticalMovePenPosition()
    , m_leading()
    , m_isLastMoveVertical(false)
    , m_isActivated(false)
    , m_acceptNextChar(false)
    SG_CODE_FOR_ASSERT(SG_COMMA m_areStylesSet(false))
{
    m_paragraphStyle.lineWidth = -1.f;
    if(nullptr != iProperties)
        m_properties = *iProperties;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
EditableText::~EditableText()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void EditableText::SetStyles(ITypeface const* iTypeface, TextStyle const* iTextStyle, ParagraphStyle const* iParagraphStyle)
{
    SG_ASSERT(nullptr != iParagraphStyle);
    m_paragraphStyle = *iParagraphStyle;
    m_text.SetStyles(iTypeface, iTextStyle, &m_paragraphStyle);
    SG_CODE_FOR_ASSERT(m_areStylesSet = true);
    UpdateFontMetrics();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void EditableText::SetStyles(ITypeface const* iTypeface, TextFormatScript const* iTFS, ParagraphStyle const* iParagraphStyle)
{
    SG_ASSERT(nullptr != iParagraphStyle);
    m_paragraphStyle = *iParagraphStyle;
    m_text.SetStyles(iTypeface, iTFS, &m_paragraphStyle);
    SG_CODE_FOR_ASSERT(m_areStylesSet = true);
    UpdateFontMetrics();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void EditableText::SetStyles(ITypeface const* iTypeface, TextStyle const* iTextStyle, TextFormatScript const* iTFS, ParagraphStyle const* iParagraphStyle)
{
    SG_ASSERT(nullptr != iParagraphStyle);
    m_paragraphStyle = *iParagraphStyle;
    m_text.SetStyles(iTypeface, iTextStyle, iTFS, &m_paragraphStyle);
    SG_CODE_FOR_ASSERT(m_areStylesSet = true);
    UpdateFontMetrics();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void EditableText::SetText(std::wstring const& iText)
{
    SG_ASSERT(m_areStylesSet);
    SetTextInternal(iText);
    SelectAll();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void EditableText::SetTextInternal(std::wstring const& iText)
{
    m_str = iText;
    m_text.SetText(m_str);
    InvalidatePlacement();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void EditableText::SetProperties(Properties const& iProperties)
{
    m_properties = iProperties;
    InvalidatePlacement();
}
//void EditableText::SetLineWidth(Length const& iLineWidth)
//{
//    m_lineWidth = iLineWidth;
//    m_overrideParagraphLineWidth = true;
//}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void EditableText::UpdateFontMetrics()
{
    SG_ASSERT(m_areStylesSet);
    ITypeface const* typeface = m_text.GetTypeface();
    TextStyle const* textStyle = m_text.GetTextStyle();
    IFont const* font = typeface->GetFont(*textStyle);
    FontInfo fontInfo;
    font->GetFontInfo(fontInfo, *textStyle);
    m_cursorBox = box2f::FromMinMax(float2(0.f, -fontInfo.ascent), float2(1.f, fontInfo.descent));
    ParagraphStyle const* paragraphStyle = m_text.GetParagraphStyle();
    m_leading = fontInfo.ascent + fontInfo.descent + paragraphStyle->lineGap;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void EditableText::VirtualResetOffset()
{
    m_frameProperty.offset.x().unit = 0;
    m_frameProperty.offset.y().unit = 0;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void EditableText::VirtualAddOffset(float2 const& iOffset)
{
    m_frameProperty.offset.x().unit += iOffset.x();
    m_frameProperty.offset.y().unit += iOffset.y();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void EditableText::VirtualOnPointerEvent(PointerEventContext const& iContext, PointerEvent const& iPointerEvent)
{
    SG_ASSERT(m_areStylesSet);
    parent_type::VirtualOnPointerEvent(iContext, iPointerEvent);
    m_sensitiveArea.OnPointerEvent(iContext, iPointerEvent, m_boxArea, *this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void EditableText::OnButtonUpToDown(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON)
{
    SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_ONE_BUTTON;
    SG_ASSERT(&m_sensitiveArea == iSensitiveArea);
    if(0 == iButton)
    {
        RequestFocusIFN();
        MoveToFrontOfAllUI();
        m_isActivated = true;

        SG_ASSERT(iPointerLocalPosition.z() == 1.f);
        size_t const i = m_text.GetNearestInterGlyph(iPointerLocalPosition.xy() - m_textPos);
        m_cursorBegin = i;
        m_cursorEnd = i;
        SG_ASSERT(m_cursorBegin <= m_str.length());
        SG_ASSERT(m_cursorEnd <= m_str.length());
        m_isLastMoveVertical = false;
        UpdateSelectionBoxes();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void EditableText::OnButtonDownToUp(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON)
{
    SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_ONE_BUTTON;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void EditableText::OnPointerMoveButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON)
{
    SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_ONE_BUTTON;
    SG_ASSERT(&m_sensitiveArea == iSensitiveArea);
    if(0 == iButton)
    {
        if(iPointerLocalPosition.z() == 1.f)
        {
            size_t const i = m_text.GetNearestInterGlyph(iPointerLocalPosition.xy() - m_textPos);
            m_cursorEnd = i;
            SG_ASSERT(m_cursorBegin <= m_str.length());
            SG_ASSERT(m_cursorEnd <= m_str.length());
        }  
        else
        {
            SG_ASSERT_NOT_IMPLEMENTED();
        }
        UpdateSelectionBoxes();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void EditableText::SelectAll()
{
    m_cursorBegin = 0;
    m_cursorEnd = m_str.length();
    UpdateSelectionBoxes();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void EditableText::VirtualOnFocusableEvent(FocusableEvent const& iFocusableEvent)
{
    SG_ASSERT(m_areStylesSet);
    focusable_parent_type::VirtualOnFocusableEvent(iFocusableEvent);

    if(!m_isActivated)
        return;

    system::UserInputEvent const& event = iFocusableEvent.Event();
    if(system::KeyboardShortcutAdapter::IsTriggerableEvent(event))
    {
        std::pair<system::KeyboardShortcutAdapter, Command> const shortcuts[] = {
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Escape, indeterminate, indeterminate, indeterminate, system::KeyboardLayout::UserLayout), Command::Escape),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Backspace, false, false, false, system::KeyboardLayout::UserLayout), Command::EraseLeft),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Delete,    false, false, false, system::KeyboardLayout::UserLayout), Command::EraseRight),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Right,     false, false, false, system::KeyboardLayout::UserLayout), Command::MoveRight),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Left,      false, false, false, system::KeyboardLayout::UserLayout), Command::MoveLeft),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Right,      true, false, false, system::KeyboardLayout::UserLayout), Command::SelectRight),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Left,       true, false, false, system::KeyboardLayout::UserLayout), Command::SelectLeft),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Right,     false,  true, false, system::KeyboardLayout::UserLayout), Command::SkipRight),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Left,      false,  true, false, system::KeyboardLayout::UserLayout), Command::SkipLeft),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Right,      true,  true, false, system::KeyboardLayout::UserLayout), Command::SelectSkipRight),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Left,       true,  true, false, system::KeyboardLayout::UserLayout), Command::SelectSkipLeft),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Home,      false, false, false, system::KeyboardLayout::UserLayout), Command::MoveBeginLine),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::End,       false, false, false, system::KeyboardLayout::UserLayout), Command::MoveEndLine),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Home,       true, false, false, system::KeyboardLayout::UserLayout), Command::SelectBeginLine),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::End,        true, false, false, system::KeyboardLayout::UserLayout), Command::SelectEndLine),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Home,      false,  true, false, system::KeyboardLayout::UserLayout), Command::MoveBeginAll),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::End,       false,  true, false, system::KeyboardLayout::UserLayout), Command::MoveEndAll),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Home,       true,  true, false, system::KeyboardLayout::UserLayout), Command::SelectBeginAll),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::End,        true,  true, false, system::KeyboardLayout::UserLayout), Command::SelectEndAll),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Up,        false, false, false, system::KeyboardLayout::UserLayout), Command::MoveUp),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Down,      false, false, false, system::KeyboardLayout::UserLayout), Command::MoveDown),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Up,         true, false, false, system::KeyboardLayout::UserLayout), Command::SelectUp),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Down,       true, false, false, system::KeyboardLayout::UserLayout), Command::SelectDown),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Key_C,     false,  true, false, system::KeyboardLayout::UserLayout), Command::Copy),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Key_X,     false,  true, false, system::KeyboardLayout::UserLayout), Command::Cut),
            std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Key_V,     false,  true, false, system::KeyboardLayout::UserLayout), Command::Paste),
        };
        for(auto const& s : AsArrayView(shortcuts))
        {
            if(s.first.IsTriggered_AssumeTriggerableEvent(event))
            {
                ApplyCommand(s.second);
                event.SetMasked();
                VirtualOnRefreshCursor();
                break;
            }
        }
    }
    if(system::UserInputDeviceType::Keyboard == event.DeviceType())
    {
        if(system::UserInputEventType::OffToOn == event.EventType() || system::UserInputEventType::OnRepeat == event.EventType())
        {
            m_acceptNextChar = !event.IsMasked();
            if(!event.IsMasked())
                event.SetMasked();
        }
        if(system::UserInputEventType::Message == event.EventType())
        {
            if(m_acceptNextChar)
            {
                wchar_t const escapeCharCode = 27;
                wchar_t const charCode = checked_numcastable(event.AdditionalData());
                if(L'\r' == charCode) { Insert(L'\n'); }
                else if(L'\b' == charCode) { /* do nothing */ }
                else if(L'\t' == charCode) { Insert(L"    "); }
                else if(escapeCharCode == charCode) { /* do nothing */ }
                else if(L'#' == charCode || L'{' == charCode || L'}' == charCode) { SG_ASSERT_NOT_IMPLEMENTED(); } // TODO: Interaction with TFS needs work
                else { Insert(charCode); }
                event.SetMasked();VirtualOnRefreshCursor();
                m_isLastMoveVertical = false;
                UpdateSelectionBoxes();
            }
        }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void EditableText::Insert(wchar_t iChar)
{
    size_t const length = m_str.length();
    SG_ASSERT(m_cursorBegin <= length);
    SG_ASSERT(m_cursorEnd <= length);
    size_t b = std::min(m_cursorBegin, m_cursorEnd);
    size_t e = std::max(m_cursorBegin, m_cursorEnd);
    SetTextInternal(m_str.substr(0, b) + iChar + m_str.substr(e, length-e));
    m_cursorBegin = b+1;
    m_cursorEnd = b+1;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void EditableText::Insert(std::wstring const& iText)
{
    size_t const length = m_str.length();
    SG_ASSERT(m_cursorBegin <= length);
    SG_ASSERT(m_cursorEnd <= length);
    size_t const b = std::min(m_cursorBegin, m_cursorEnd);
    size_t const e = std::max(m_cursorBegin, m_cursorEnd);
    SetTextInternal(m_str.substr(0, b) + iText + m_str.substr(e, length-e));
    m_cursorBegin = b + iText.length();
    m_cursorEnd = m_cursorBegin;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void EditableText::UpdateSelectionBoxes()
{
    m_cursorPosition = m_text.GetPenPosition(m_cursorEnd);
    m_selectionBoxes.clear();
    if(m_cursorBegin != m_cursorEnd)
    {
        size_t const length = m_str.length();
        SG_ASSERT(m_cursorBegin <= length);
        SG_ASSERT(m_cursorEnd <= length);
        size_t const b = std::min(m_cursorBegin, m_cursorEnd);
        size_t const e = std::max(m_cursorBegin, m_cursorEnd);
        m_text.GetSelectionBoxes(m_selectionBoxes, b, e);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void EditableText::ApplyCommand(Command iCommand)
{
    size_t const length = m_str.length();
    SG_ASSERT(m_cursorBegin <= length);
    SG_ASSERT(m_cursorEnd <= length);

    auto Skip = [](std::wstring const& str, size_t cursor, int direction)
    {
        size_t const length = str.length();
        SG_ASSERT(cursor <= length);

        std::locale locale("en");
        if(size_t(cursor + direction) < length)
        {
            do
            {
                cursor += direction;
            } while(cursor + direction <= length && (std::isspace(str[cursor], locale) || std::ispunct(str[cursor], locale)));
            while(cursor + direction <= length && !(std::isspace(str[cursor], locale) || std::ispunct(str[cursor], locale)))
            {
                cursor += direction;
            }
        }
        else
        {
            if(direction == 1)
                cursor = length;
            else
                cursor = 0;
        }
        return cursor;
    };
    auto FindNewLine = [](std::wstring const& str, size_t cursor, int direction)
    {
        size_t const length = str.length();
        SG_ASSERT(cursor <= length);

        if(size_t(cursor + direction) < length)
        {
            do
            {
                cursor += direction;
            } while(cursor + direction <= length && str[cursor] != L'\n');
        }
        else
        {
            if(direction == 1)
                cursor = length;
            else
                cursor = 0;
        }
        return cursor;
    };
    // By default, a command loose the position for vertical move.
    // Commands that keep this position do that explicitely.
    bool const isLlastMoveVertical = m_isLastMoveVertical;
    m_isLastMoveVertical = false;
    auto VerticalMove = [this, isLlastMoveVertical](size_t cursor, int direction)
    {
        if(!isLlastMoveVertical)
            m_verticalMovePenPosition = m_text.GetPenPosition(cursor);
        float2 const p = m_verticalMovePenPosition + float2(0, direction * m_leading);
        cursor = m_text.GetNearestInterGlyph(p);
        m_verticalMovePenPosition.y() = m_text.GetPenPosition(cursor).y();
        m_isLastMoveVertical = true;
        return cursor;
    };

    switch(iCommand)
    {
    case Escape:
    {
        if(m_cursorBegin != m_cursorEnd)
            m_cursorBegin = m_cursorEnd;
        else
            m_isActivated = false;
        break;
    }
    case EraseLeft:
    {
        if(m_cursorBegin == m_cursorEnd)
        {
            if(m_cursorBegin != 0)
            {
                SetTextInternal(m_str.substr(0, m_cursorBegin-1) + m_str.substr(m_cursorBegin, length-m_cursorBegin));
                --m_cursorBegin;
                m_cursorEnd = m_cursorBegin;
            }
        }
        else
        {
            size_t b = std::min(m_cursorBegin, m_cursorEnd);
            size_t e = std::max(m_cursorBegin, m_cursorEnd);
            SetTextInternal(m_str.substr(0, b) + m_str.substr(e, length-e));
            m_cursorBegin = b;
            m_cursorEnd = b;
        }
        break;
    }
    case EraseRight:
    {
        if(m_cursorBegin == m_cursorEnd)
        {
            if(m_cursorBegin < length)
            {
                SetTextInternal(m_str.substr(0, m_cursorBegin) + m_str.substr(m_cursorBegin+1, length-m_cursorBegin-1));
            }
        }
        else
        {
            size_t b = std::min(m_cursorBegin, m_cursorEnd);
            size_t e = std::max(m_cursorBegin, m_cursorEnd);
            SetTextInternal(m_str.substr(0, b) + m_str.substr(e, length-e));
            m_cursorBegin = b;
            m_cursorEnd = b;
        }
        break;
    }
    case MoveRight:
    {
        if(m_cursorEnd < length)
            ++m_cursorEnd;
        m_cursorBegin = m_cursorEnd;
        break;
    }
    case MoveLeft:
    {
        if(m_cursorEnd != 0)
            --m_cursorEnd;
        m_cursorBegin = m_cursorEnd;
        break;
    }
    case SelectRight:
    {
        if(m_cursorEnd < length)
            ++m_cursorEnd;
        break;
    }
    case SelectLeft:
    {
        if(m_cursorEnd != 0)
            --m_cursorEnd;
        break;
    }
    case SelectSkipRight:
    {
        m_cursorEnd = Skip(m_str, m_cursorEnd, 1);
        break;
    }
    case SelectSkipLeft:
    {
        m_cursorEnd = Skip(m_str, m_cursorEnd, -1);
        break;
    }
    case SkipRight:
    {
        m_cursorEnd = Skip(m_str, m_cursorEnd, 1);
        m_cursorBegin = m_cursorEnd;
        break;
    }
    case SkipLeft:
    {
        m_cursorEnd = Skip(m_str, m_cursorEnd, -1);
        m_cursorBegin = m_cursorEnd;
        break;
    }
    case MoveBeginLine:
    {
        m_cursorEnd = FindNewLine(m_str, m_cursorEnd, -1);
        m_cursorBegin = m_cursorEnd;
        break;
    }
    case MoveEndLine:
    {
        m_cursorEnd = FindNewLine(m_str, m_cursorEnd, 1);
        m_cursorBegin = m_cursorEnd;
        break;
    }
    case SelectBeginLine:
    {
        m_cursorEnd = FindNewLine(m_str, m_cursorEnd, -1);
        break;
    }
    case SelectEndLine:
    {
        m_cursorEnd = FindNewLine(m_str, m_cursorEnd, 1);
        break;
    }
    case MoveBeginAll:
    {
        m_cursorEnd = 0;
        m_cursorBegin = m_cursorEnd;
        break;
    }
    case MoveEndAll:
    {
        m_cursorEnd = length;
        m_cursorBegin = m_cursorEnd;
        break;
    }
    case SelectBeginAll:
    {
        m_cursorEnd = 0;
        break;
    }
    case SelectEndAll:
    {
        m_cursorEnd = length;
        break;
    }
    case MoveUp:
    {
        m_cursorEnd = VerticalMove(m_cursorEnd, -1);
        m_cursorBegin = m_cursorEnd;
        break;
    }
    case MoveDown:
    {
        m_cursorEnd = VerticalMove(m_cursorEnd, 1);
        m_cursorBegin = m_cursorEnd;
        break;
    }
    case SelectUp:
    {
        m_cursorEnd = VerticalMove(m_cursorEnd, -1);
        break;
    }
    case SelectDown:
    {
        m_cursorEnd = VerticalMove(m_cursorEnd, 1);
        break;
    }
    case Cut:
    {
        size_t const b = std::min(m_cursorBegin, m_cursorEnd);
        size_t const e = std::max(m_cursorBegin, m_cursorEnd);
        system::CopyTextToClipboard(m_str.substr(b, e-b));
        SetTextInternal(m_str.substr(0, b) + m_str.substr(e, length-e));
        m_cursorBegin = b;
        m_cursorEnd = b;
        break;
    }
    case Copy:
    {
        size_t const b = std::min(m_cursorBegin, m_cursorEnd);
        size_t const e = std::max(m_cursorBegin, m_cursorEnd);
        system::CopyTextToClipboard(m_str.substr(b, e-b));
        m_isLastMoveVertical = isLlastMoveVertical;
        break;
    }
    case Paste:
    {
        std::wstring pasted;
        system::GetTextInClipboard(pasted);
        Insert(pasted);
        break;
    }
    default:
        SG_ASSERT_NOT_REACHED();
    }
    SG_ASSERT(m_cursorBegin <= m_str.length());
    SG_ASSERT(m_cursorEnd <= m_str.length());
    UpdateSelectionBoxes();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void EditableText::VirtualOnGainFocus()
{
    focusable_parent_type::VirtualOnGainFocus();
    m_isLastMoveVertical = false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool EditableText::VirtualMoveFocusReturnHasMoved(FocusDirection iDirection)
{
    if(!HasFocus())
    {
        RequestFocusIFN();
        switch(iDirection)
        {
        case FocusDirection::None:
            SG_ASSERT_NOT_REACHED();
            break;
        case FocusDirection::Left:
        case FocusDirection::Right:
        case FocusDirection::Up:
        case FocusDirection::Down:
            m_isActivated = false;
            return true;
        case FocusDirection::MoveIn:
        case FocusDirection::MoveOut:
            m_isActivated = false;
            return true;
        case FocusDirection::Activate:
        case FocusDirection::Validate:
        case FocusDirection::Cancel:
            SG_ASSERT_NOT_REACHED(); // How should that happen?
            m_isActivated = false;
            return true;
        case FocusDirection::Next:
        case FocusDirection::Previous:
            m_isActivated = true;
            return true;
        default:
            SG_ASSERT_NOT_REACHED();
            return false;
        }
        SG_ASSERT_NOT_REACHED();
        return false;
    }
    if(m_isActivated)
    {
        switch(iDirection)
        {
        case FocusDirection::None:
            SG_ASSERT_NOT_REACHED();
            return false;
        case FocusDirection::Left:
        case FocusDirection::Right:
        case FocusDirection::Up:
        case FocusDirection::Down:
        case FocusDirection::MoveIn:
        case FocusDirection::Activate:
        case FocusDirection::Validate:
            return true;
        case FocusDirection::Next:
        case FocusDirection::Previous:
            m_isActivated = false;
            return false;
        case FocusDirection::MoveOut:
        case FocusDirection::Cancel:
            m_isActivated = false;
            return true;
        default:
            return false;
        }
    }
    else
    {
        switch(iDirection)
        {
        case FocusDirection::None:
            SG_ASSERT_NOT_REACHED();
            return false;
        case FocusDirection::Left:
        case FocusDirection::Right:
        case FocusDirection::Up:
        case FocusDirection::Down:
        case FocusDirection::Next:
        case FocusDirection::Previous:
        case FocusDirection::MoveOut:
        case FocusDirection::Cancel:
            return false;
        case FocusDirection::MoveIn:
        case FocusDirection::Activate:
        case FocusDirection::Validate:
            m_isActivated = true;
            m_isLastMoveVertical = false;
            return true;
        default:
            return false;
        }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void EditableText::VirtualOnDraw(DrawContext const& iContext)
{
    float const magnification = m_magnifier->Magnification();
    box2f const& placementBox = PlacementBox();
    bool const mustDraw = IntersectStrict(iContext.BoundingClippingBox(), placementBox);
    if(mustDraw)
    {
        SG_ASSERT(m_cursorBegin <= m_str.length());
        SG_ASSERT(m_cursorEnd <= m_str.length());

        if(HasFocus() && m_isActivated)
        {
            // TODO: use TFS to change selection text style
        }
        m_text.Draw(iContext, m_textPos);
    }
    parent_type::VirtualOnDraw(iContext);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void EditableText::SkipVirtualOnDraw(DrawContext const& iContext)
{
    parent_type::VirtualOnDraw(iContext);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void EditableText::VirtualUpdatePlacement()
{
    float const magnification = m_magnifier->Magnification();
    box2f const& parentBox = Parent()->PlacementBox();
    box2f const textMargin = m_properties.margins.Resolve(magnification, parentBox.Delta());

    SG_ASSERT_MSG(0.f == m_frameProperty.offset.x().unit || 0.f == m_properties.lineWidth.relative, "A Label in an horizontal list must have a non-relative line width");

    float2 const marginDelta = textMargin.Delta();
    if(m_properties.overrideParagraphLineWidth)
    {
        float const minParentInducedLineWidth = 0.f;
        float const lineWidth = m_properties.lineWidth.Resolve(magnification, std::max(minParentInducedLineWidth, parentBox.Delta().x() - marginDelta.x()));
        SG_ASSERT(0 <= lineWidth);
        if(!EqualsWithTolerance(lineWidth, m_paragraphStyle.lineWidth, 0.1f))
        {
            m_paragraphStyle.lineWidth = std::max(0.f, lineWidth);
            m_text.SetParagraphStyle(&m_paragraphStyle);
            box2f const textBox = m_text.Box();
            float const textWidth = textBox.Delta().x();
            if(textWidth > lineWidth)
            {
                // this is for all text to use the longest line width.
                m_paragraphStyle.lineWidth = textWidth;
                m_text.SetParagraphStyle(&m_paragraphStyle);
            }
        }
    }
    box2f const textBox = m_text.Box();
    float const contentWidth = std::max(m_paragraphStyle.lineWidth, textBox.Delta().x());
    float2 const contentInducedSize = float2(contentWidth, textBox.Delta().y()) + marginDelta;
    box2f const frame = m_frameProperty.Resolve(magnification, parentBox, contentInducedSize);
    SetPlacementBox(frame);
    TextBoxAlignment textAlignment;
    textAlignment.alignementToAnchor = float2(0.5f,0.5f);
    textAlignment.useBaselinesInY = 0;
    textAlignment.useTextAlignmentInX = 0;
    m_textPos = frame.Center() + 0.5f * textMargin.Center() + m_text.ComputeOffset(textAlignment);

    m_boxArea.SetBox(frame);
}
//=============================================================================
}
}

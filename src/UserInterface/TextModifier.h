#ifndef UserInterface_TextModifier_H
#define UserInterface_TextModifier_H

#include <Core/Cast.h>
#include <Core/IntTypes.h>
#include <Core/SmartPtr.h>
#include "TextStyle.h"

namespace sg {
namespace ui {
//=============================================================================
class ITextModifier : public SafeCountable
{
    friend class TextRenderer;
    friend class TextRenderer_internal;
public:
    virtual ~ITextModifier() {}
    enum class Type : u8 { DynamicColor, Insert, Jump, MovePen, StyleChange, RelativeStyleChange, StylePush, StylePop, HidePush, HidePop };
    Type GetType() const { return m_type; }
    size_t Position() const { return m_position; }
protected:
    ITextModifier(Type iType, size_t iPos)
        : m_type(iType)
        , m_position(checked_numcastable(iPos))
    {
        SG_ASSERT(-1 != iPos);
    }
private:
    Type const m_type;
    size_t const m_position;
};
//=============================================================================
class TextModifier_DynamicColor : public ITextModifier
{
public:

};
//=============================================================================
class TextModifier_AbstractInsert : public ITextModifier
{
public:
    TextModifier_AbstractInsert(size_t iPos)
        : ITextModifier(ITextModifier::Type::Insert, iPos)
    {}
};
//=============================================================================
class TextModifier_Jump : public ITextModifier
{
public:
    TextModifier_Jump(size_t iPos, size_t iTo)
        : ITextModifier(ITextModifier::Type::Jump, iPos)
        , m_to(iTo)
    {}
    size_t To() const { return m_to; }
private:
    size_t const m_to;
};
//=============================================================================
class TextModifier_MovePen : public ITextModifier
{
public:
};
//=============================================================================
class TextModifier_StyleChange : public ITextModifier
{
public:
    TextModifier_StyleChange(TextStyle const* iTextStyle, size_t iPos)
        : ITextModifier(ITextModifier::Type::StyleChange, iPos)
        , m_textStyle(iTextStyle)
    {
        SG_ASSERT(nullptr != iTextStyle);
    }
    TextStyle const& Style() const { return *m_textStyle; }
private:
    safeptr<TextStyle const> m_textStyle;
};
//=============================================================================
class TextModifier_RelativeStyleChange : public ITextModifier
{
public:
};
//=============================================================================
class TextModifier_StylePush : public ITextModifier
{
public:
    TextModifier_StylePush(size_t iPos)
        : ITextModifier(ITextModifier::Type::StylePush, iPos)
    {}
};
//=============================================================================
class TextModifier_StylePop : public ITextModifier
{
public:
    TextModifier_StylePop(size_t iPos)
        : ITextModifier(ITextModifier::Type::StylePop, iPos)
    {}
};
//=============================================================================
class TextModifier_HidePush : public ITextModifier
{
public:
    TextModifier_HidePush(size_t iPos)
        : ITextModifier(ITextModifier::Type::HidePush, iPos)
    {}
};
//=============================================================================
class TextModifier_HidePop : public ITextModifier
{
public:
    TextModifier_HidePop(size_t iPos)
        : ITextModifier(ITextModifier::Type::HidePop, iPos)
    {}
};
//=============================================================================
}
}

#endif

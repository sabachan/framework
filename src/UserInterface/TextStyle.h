#ifndef UserInterface_TexStyle_H
#define UserInterface_TexStyle_H

#include <Core/FastSymbol.h>
#include <Core/ModificationStamped.h>
#include <Rendering/Color.h>
#include <Reflection/BaseClass.h>

namespace sg {
namespace ui {
//=============================================================================
struct TextStyle : public reflection::BaseType
                 , public SafeCountable
{
    REFLECTION_TYPE_HEADER(TextStyle, reflection::BaseType)
public:
    R8G8B8A8 fillColor;
    R8G8B8A8 strokeColor;
    float strokeSize;
    float size;
    FastSymbol fontFamilyName;
    bool bold;
    bool italic;
    bool superscript;
    bool subscript;
    bool underlined;
    bool strikeThrough;
public:
    TextStyle()
        : fillColor()
        , strokeColor()
        , strokeSize()
        , size()
        , fontFamilyName()
        , bold(false)
        , italic(false)
        , superscript(false)
        , subscript(false)
        , underlined(false)
        , strikeThrough(false)
    {
    }
    TextStyle(uninitialized_t) {}
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class TextStyleStamped : public ModificationStamped
                       , private TextStyle
{
public:
    TextStyle const& Get() const { return *this; }
    void Set(TextStyle const& iStyle) { BeginModification(); *(TextStyle*)this = iStyle; EndModification(); }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if 0
class TextStyleStamped : public ModificationStamped
{
public:
    TextStyle const& Get() const { return _; }
    void Set(TextStyle const& iStyle) { BeginModification(); _ = iStyle; EndModification(); }
private:
    TextStyle _;
};
#endif
//=============================================================================
struct ParagraphStyle : public reflection::BaseType
                      , public SafeCountable
{
    REFLECTION_TYPE_HEADER(ParagraphStyle, reflection::BaseType)
public:
    float alignment;
    float justified;
    float lineWidth;
    float lineGap;
    bool balanced;
public:
    ParagraphStyle()
        : alignment(0)
        , justified(0)
        , lineWidth(0)
        , lineGap(0)
        , balanced(false)
    {
    }
    ParagraphStyle(uninitialized_t) {}
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class ParagraphStyleStamped : public ModificationStamped
{
public:
    ParagraphStyle const& Get() { return _; }
    void Set(ParagraphStyle const& iStyle) { BeginModification(); _ = iStyle; EndModification(); }
private:
    ParagraphStyle _;
};
//=============================================================================
}
}

#endif

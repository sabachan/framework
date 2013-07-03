#ifndef UserInterface_TextFormatScript_H
#define UserInterface_TextFormatScript_H

// Text Format Script is used to generate text modifiers from a string
// containing instructions.
// The syntax of the script instructions in the string is a '#' caracter
// followed by a keyword ( [a-z,A-Z,_] [0-9,a-z,A-Z,_]* ). Some instructions
// are scoped, in which case they are directly followed by an opening '{'
// caracter which must be matched by a closing '}' caracter at the end of the
// desired scope. If the instruction is not scoped, it can be followed by a
// space-like caracter (new line, ponctuations, etc.), or by the sequence "{}".
// In order to print a reserved caracter ('#', '{' or '}'), one can prefix it
// with '#'.
// eg: 
// let's assume that 'bold' and 'italic' are style type instructions that apply
// a style to a scope, and that 'smiley' and 'star' are insert type instructions
// that are replaced by an image (and do not apply to a scope). Following are
// valid scripts:
//      "#bold{This text is bold} and #italic{this one is italic} #smiley."
//      "You are ##1 of the classement with #star{}#star{}#star."

#include <Core/SmartPtr.h>
#include "TextStyle.h"

namespace sg {
template<typename> class VectorOfScopedPtr;
}

namespace sg {
namespace ui {
//=============================================================================
class ITextModifier;
class TextFormatScript;
struct TextStyle;
class TFSInstruction;
//=============================================================================
class TextFormatScript : public reflection::BaseClass
{
    REFLECTION_CLASS_HEADER(TextFormatScript, reflection::BaseClass)
public:
    TextFormatScript(auto_initialized_t);
    void Run(VectorOfScopedPtr<ui::ITextModifier const>& oModifiers, std::wstring const& iStr) const;
    void AddInstruction(FastSymbol iKeyword, TFSInstruction const* iInstruction);
    TFSInstruction const* GetInstructionIFP(FastSymbol iKeyword) const;
private:
    TextFormatScript() {}
private:
    std::unordered_map<FastSymbol, refptr<TFSInstruction const> > m_instructions;
};
//=============================================================================
class TFSInstruction : public reflection::BaseClass
{
    REFLECTION_CLASS_HEADER(TFSInstruction, reflection::BaseClass)
public:
    enum class Type { Insert, Style, };
    TFSInstruction(Type iType) : m_type(iType) {}
    virtual ~TFSInstruction() {}
    Type GetType() const { return m_type; }
    bool HasScope() const
    {
        switch(m_type)
        {
        case Type::Insert: return false;
        case Type::Style:  return true;
        default:
            SG_ASSUME_NOT_REACHED();
        }
        return false;
    }
private:
    Type const m_type;
};
//=============================================================================
class TFSInsert : public TFSInstruction
{
    REFLECTION_CLASS_HEADER(TFSInsert, TFSInstruction)
public:
    TFSInsert() : TFSInstruction(TFSInstruction::Type::Style) { SG_ASSERT_NOT_IMPLEMENTED(); }
};
//=============================================================================
class TFSStyle final : public TFSInstruction
{
    REFLECTION_CLASS_HEADER(TFSStyle, TFSInstruction)
private:
    TFSStyle();
public:
    TFSStyle(TextStyle const& iStyle);
    ~TFSStyle();
    TextStyle const* GetStyle() const { return &m_style; }
private:
    TextStyle m_style;
};
//=============================================================================
}
}

#endif

#include "stdafx.h"

#include "TextFormatScript.h"

#include <Core/MaxSizeVector.h>
#include <Core/VectorOfScopedPtr.h>
#include "TextModifier.h"
#include "TextStyle.h"

namespace sg {
namespace ui {
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
u32 const INSTRUCTION_PREFFIX = '#';
u32 const OPEN_SCOPE = '{';
u32 const CLOSE_SCOPE = '}';
size_t const MAX_STACK_DEPTH = 64;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
FastSymbol ReadInstruction(std::wstring const& iStr, size_t& iPos)
{
    size_t const beginPos = iPos;
    size_t i = iPos;
    u32 c = iStr[i];
    size_t const KEYWORD_MAX_SIZE = 1024;
    char keywordBuffer[KEYWORD_MAX_SIZE];
    char* dst = keywordBuffer;
    char* bufferEnd = keywordBuffer + KEYWORD_MAX_SIZE;
    SG_UNUSED(bufferEnd);
    if(('a' <= c &&  c <= 'z')
       || ('A' <= c && c <= 'Z')
       || '_' == c)
    {
        do
        {
            *dst++ = checked_numcastable(c);
            SG_ASSERT(dst != bufferEnd);
            ++i;
            c = iStr[i];
        } while(('a' <= c &&  c <= 'z')
                || ('A' <= c && c <= 'Z')
                || '_' == c);
        *dst = 0;
    }

    iPos = i;
    return FastSymbol(keywordBuffer);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct StackItem
{
    safeptr<TFSInstruction const> instruction;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ApplyUnscopedInstruction(VectorOfScopedPtr<ITextModifier const>& oModifiers, TFSInstruction const* instruction, size_t pos)
{
    if(nullptr != instruction)
    {
        switch(instruction->GetType())
        {
        case TFSInstruction::Type::Insert:
            SG_ASSERT_NOT_IMPLEMENTED();
            break;
        case TFSInstruction::Type::Style:
            {
                TFSStyle const* inst = checked_cast<TFSStyle const*>(instruction);
#if 1
                ITextModifier const* modifierBack = oModifiers.empty() ? nullptr : oModifiers.back();
                if(nullptr != modifierBack && modifierBack->GetType() == ITextModifier::Type::StylePop && modifierBack->Position() == pos)
                    oModifiers.pop_back();
                else
#endif
                    oModifiers.push_back(new TextModifier_StylePush(pos));
                oModifiers.push_back(new TextModifier_StyleChange(inst->GetStyle(), pos));
                oModifiers.push_back(new TextModifier_StylePop(pos));
            }
            break;
        default:
            SG_ASSUME_NOT_REACHED();
        }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void OpenScopedInstruction(VectorOfScopedPtr<ITextModifier const>& oModifiers, TFSInstruction const* instruction, size_t pos, StackItem& stackItem)
{
    if(nullptr != instruction)
    {
        switch(instruction->GetType())
        {
        case TFSInstruction::Type::Insert:
            SG_ASSERT_NOT_REACHED();
            break;
        case TFSInstruction::Type::Style:
        {
            TFSStyle const* inst = checked_cast<TFSStyle const*>(instruction);
#if 1
            ITextModifier const* modifierBack = oModifiers.empty() ? nullptr : oModifiers.back();
            if(nullptr != modifierBack && modifierBack->GetType() == ITextModifier::Type::StylePop && modifierBack->Position() == pos)
                oModifiers.pop_back();
            else
#endif
                oModifiers.push_back(new TextModifier_StylePush(pos));
            oModifiers.push_back(new TextModifier_StyleChange(inst->GetStyle(), pos));
            break;
        }
        case TFSInstruction::Type::Skip:
        {
            oModifiers.push_back(new TextModifier_HidePush(pos));
            break;
        }
        default:
            SG_ASSUME_NOT_REACHED();
        }
    }
    stackItem.instruction = instruction;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CloseInstructionScope(VectorOfScopedPtr<ITextModifier const>& oModifiers, size_t pos, StackItem const& stackItem)
{
    TFSInstruction const* instruction = stackItem.instruction.get();
    if(nullptr != instruction)
    {
        switch(instruction->GetType())
        {
        case TFSInstruction::Type::Insert:
            SG_ASSERT_NOT_IMPLEMENTED();
            break;
        case TFSInstruction::Type::Style:
            {
                oModifiers.push_back(new TextModifier_StylePop(pos));
            }
            break;
        case TFSInstruction::Type::Skip:
        {
            oModifiers.push_back(new TextModifier_HidePop(pos));
            break;
        }
        default:
            SG_ASSUME_NOT_REACHED();
        }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RunTextFormatScript(VectorOfScopedPtr<ITextModifier const>& oModifiers, std::wstring const& iStr, TextFormatScript const& iTFS)
{
    MaxSizeVector<StackItem, MAX_STACK_DEPTH> stack;

    size_t const len = iStr.length();
    for(size_t i = 0; i < len; ++i)
    {
        u32 const c = iStr[i];
        if(OPEN_SCOPE == c)
        {
            SG_ASSERT_MSG(false, "You should protect a { caracter by a # prefix");
        }
        else if(CLOSE_SCOPE == c)
        {
            if(stack.empty())
                SG_ASSERT_MSG(false, "You should protect a } caracter by a # prefix");
            else
            {
                CloseInstructionScope(oModifiers, i, stack.back());
                stack.pop_back();
                oModifiers.push_back(new TextModifier_Jump(i, i+1));
            }
        }
        else if(INSTRUCTION_PREFFIX == c)
        {
            size_t const instructionBegin = i;
            ++i;
            u32 const c2 = iStr[i];
            if(OPEN_SCOPE == c2 || CLOSE_SCOPE == c2 || INSTRUCTION_PREFFIX == c2)
            {
                oModifiers.push_back(new TextModifier_Jump(instructionBegin, i));
            }
            else
            {
                FastSymbol const keyword = ReadInstruction(iStr, i);
                TFSInstruction const* instruction = iTFS.GetInstructionIFP(keyword);
                SG_ASSERT_MSG(nullptr != instruction, "instruction not reckognized!");
                u32 const c3 = iStr[i];
                if(OPEN_SCOPE == c3)
                {
                    ++i;
                    u32 const c4 = iStr[i];
                    if(CLOSE_SCOPE == c4)
                    {
                        oModifiers.push_back(new TextModifier_Jump(instructionBegin, i));
                        size_t const modifierIndex = oModifiers.size();
                        oModifiers.emplace_back();
                        ApplyUnscopedInstruction(oModifiers, instruction, i);
                    }
                    else
                    {
                        oModifiers.push_back(new TextModifier_Jump(instructionBegin, i));
                        SG_ASSERT(nullptr == instruction || instruction->HasScope());
                        stack.emplace_back();
                        OpenScopedInstruction(oModifiers, instruction, i, stack.back());
                        --i;
                    }
                }
                else
                {
                    SG_ASSERT(nullptr == instruction || !instruction->HasScope());
                    oModifiers.push_back(new TextModifier_Jump(instructionBegin, i));
                    ApplyUnscopedInstruction(oModifiers, instruction, i);
                    --i;
                }
            }
        }
    }
    SG_ASSERT(stack.empty());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
TextFormatScript::TextFormatScript(auto_initialized_t)
    : TextFormatScript()
{
    EndAutoCreation();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TextFormatScript::Run(VectorOfScopedPtr<ITextModifier const>& oModifiers, std::wstring const& iStr) const
{
    RunTextFormatScript(oModifiers, iStr, *this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TextFormatScript::AddInstruction(FastSymbol iKeyword, TFSInstruction const* iInstruction)
{
    auto r = m_instructions.emplace(iKeyword, iInstruction);
    SG_ASSERT_MSG(r.second, "keyword already declared!");
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TFSInstruction const* TextFormatScript::GetInstructionIFP(FastSymbol iKeyword) const
{
    auto f = m_instructions.find(iKeyword);
    if(f != m_instructions.end())
        return f->second.get();
    else
        return nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg,ui), TextFormatScript)
REFLECTION_CLASS_DOC("Text Format Script")
REFLECTION_m_PROPERTY_DOC(instructions, "")
REFLECTION_CLASS_END
//=============================================================================
REFLECTION_ABSTRACT_CLASS_BEGIN((sg,ui), TFSInstruction)
REFLECTION_CLASS_DOC("")
REFLECTION_CLASS_END
//=============================================================================
REFLECTION_ABSTRACT_CLASS_BEGIN((sg,ui), TFSInsert)
REFLECTION_CLASS_DOC("")
REFLECTION_CLASS_END
//=============================================================================
REFLECTION_CLASS_BEGIN((sg,ui), TFSSkip)
REFLECTION_CLASS_DOC("Skip contained text")
REFLECTION_CLASS_END
//=============================================================================
TFSStyle::TFSStyle()
    : TFSInstruction(TFSInstruction::Type::Style)
    , m_style()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TFSStyle::TFSStyle(TextStyle const& iStyle)
    : TFSInstruction(TFSInstruction::Type::Style)
    , m_style(iStyle)
{
    EndAutoCreation();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TFSStyle::~TFSStyle()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg,ui), TFSStyle)
REFLECTION_CLASS_DOC("Text style")
REFLECTION_m_PROPERTY_DOC(style, "")
REFLECTION_CLASS_END
//=============================================================================
}
}

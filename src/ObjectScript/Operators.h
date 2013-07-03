#ifndef ObjectScript_Operators_H
#define ObjectScript_Operators_H

#include <Reflection/BaseClass.h>
#include <Reflection/ObjectDatabase.h>
#include "SemanticTree.h"
#include "Tokenizer.h"

namespace sg {
namespace objectscript {
//=============================================================================
typedef semanticTree::ITreeNode* (*CreateTreeNodeFct)(OperatorTraits const& iOperatorTraits);
enum class Arrity { Unspecified, Nullary, PrefixUnary, SuffixUnary, Binary, BinaryEnableTrailing, Ternary };
enum class Associativity { NotApplicable, LeftToRight, RightToLeft };
enum class AllowSeparators { No, Yes };
struct OperatorTraits
{
    TokenType op;
    Arrity arrity;
    size_t precedence;
    Associativity associativity;
    AllowSeparators allowSeparators;
    CreateTreeNodeFct createTreeNode;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T> semanticTree::ITreeNode* CreateTreeNode(OperatorTraits const& iOperatorTraits) { SG_UNUSED(iOperatorTraits); return new T(); }
template<typename T> semanticTree::ITreeNode* CreateTreeNodeWithToken(OperatorTraits const& iOperatorTraits) { return new T(iOperatorTraits.op); }
template<typename T> semanticTree::ITreeNode* CreateTreeNodeWithTraits(OperatorTraits const& iOperatorTraits) { return new T(iOperatorTraits); }
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
OperatorTraits const operatorTraits[] = {
    { TokenType::operator_dollar,           Arrity::PrefixUnary,    7, Associativity::NotApplicable, AllowSeparators::Yes, CreateTreeNode<semanticTree::Identifierize> },
    { TokenType::keyword_export,            Arrity::PrefixUnary,    2, Associativity::RightToLeft,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::Qualifier> },
    { TokenType::keyword_public,            Arrity::PrefixUnary,    2, Associativity::RightToLeft,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::Qualifier> },
    { TokenType::keyword_protected,         Arrity::PrefixUnary,    2, Associativity::RightToLeft,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::Qualifier> },
    { TokenType::keyword_private,           Arrity::PrefixUnary,    2, Associativity::RightToLeft,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::Qualifier> },
    { TokenType::keyword_const,             Arrity::PrefixUnary,    2, Associativity::RightToLeft,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::Qualifier> },
    { TokenType::keyword_var,               Arrity::PrefixUnary,    2, Associativity::RightToLeft,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::Qualifier> },
    { TokenType::open_bloc,                 Arrity::Nullary,        4, Associativity::NotApplicable, AllowSeparators::Yes, CreateTreeNode<semanticTree::Struct> },
    { TokenType::open_parenthesis,          Arrity::Nullary,        4, Associativity::NotApplicable, AllowSeparators::Yes, CreateTreeNode<semanticTree::Parenthesis> },
    { TokenType::open_bracket,              Arrity::Nullary,        4, Associativity::NotApplicable, AllowSeparators::Yes, CreateTreeNode<semanticTree::List> },
    { TokenType::operator_double_colon,     Arrity::PrefixUnary,   10, Associativity::LeftToRight,   AllowSeparators::No,  CreateTreeNode<semanticTree::ScopeResolution> },
    { TokenType::operator_double_colon,     Arrity::Binary,        10, Associativity::LeftToRight,   AllowSeparators::No,  CreateTreeNode<semanticTree::ScopeResolution> },
    { TokenType::operator_plus_plus,        Arrity::SuffixUnary,   20, Associativity::LeftToRight,   AllowSeparators::No,  CreateTreeNodeWithTraits<semanticTree::IncDecRement> },
    { TokenType::operator_minus_minus,      Arrity::SuffixUnary,   20, Associativity::LeftToRight,   AllowSeparators::No,  CreateTreeNodeWithTraits<semanticTree::IncDecRement> },
    { TokenType::open_bloc,                 Arrity::SuffixUnary,   20, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNode<semanticTree::Object> },
    { TokenType::open_parenthesis,          Arrity::SuffixUnary,   20, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNode<semanticTree::FunctionCall> },
    { TokenType::open_bracket,              Arrity::SuffixUnary,   20, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNode<semanticTree::Indexing> },
    { TokenType::operator_dot,              Arrity::Binary,        20, Associativity::LeftToRight,   AllowSeparators::Yes, nullptr },
    { TokenType::keyword_is,                Arrity::Binary,        25, Associativity::NotApplicable, AllowSeparators::Yes, CreateTreeNode<semanticTree::ObjectDefinition> },
    { TokenType::operator_plus_plus,        Arrity::PrefixUnary,   30, Associativity::RightToLeft,   AllowSeparators::No,  CreateTreeNodeWithTraits<semanticTree::IncDecRement> },
    { TokenType::operator_minus_minus,      Arrity::PrefixUnary,   30, Associativity::RightToLeft,   AllowSeparators::No,  CreateTreeNodeWithTraits<semanticTree::IncDecRement> },
    { TokenType::operator_plus,             Arrity::PrefixUnary,   30, Associativity::RightToLeft,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::UnaryExpression> },
    { TokenType::operator_minus,            Arrity::PrefixUnary,   30, Associativity::RightToLeft,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::UnaryExpression> },
    { TokenType::operator_not,              Arrity::PrefixUnary,   30, Associativity::RightToLeft,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::UnaryExpression> },
    { TokenType::operator_bitnot,           Arrity::PrefixUnary,   30, Associativity::RightToLeft,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::UnaryExpression> },
    { TokenType::operator_multiply,         Arrity::Binary,        50, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::BinaryExpression> },
    { TokenType::operator_divide,           Arrity::Binary,        50, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::BinaryExpression> },
    { TokenType::operator_modulo,           Arrity::Binary,        50, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::BinaryExpression> },
    { TokenType::operator_plus,             Arrity::Binary,        60, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::BinaryExpression> },
    { TokenType::operator_minus,            Arrity::Binary,        60, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::BinaryExpression> },
    { TokenType::operator_shift_left,       Arrity::Binary,        70, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::BinaryExpression> },
    { TokenType::operator_shift_right,      Arrity::Binary,        70, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::BinaryExpression> },
    { TokenType::operator_less,             Arrity::Binary,        80, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::BinaryExpression> },
    { TokenType::operator_less_equal,       Arrity::Binary,        80, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::BinaryExpression> },
    { TokenType::operator_greater,          Arrity::Binary,        80, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::BinaryExpression> },
    { TokenType::operator_greater_equal,    Arrity::Binary,        80, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::BinaryExpression> },
    { TokenType::operator_equal_equal,      Arrity::Binary,        90, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::BinaryExpression> },
    { TokenType::operator_not_equal,        Arrity::Binary,        90, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::BinaryExpression> },
    { TokenType::operator_bitand,           Arrity::Binary,       100, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::BinaryExpression> },
    { TokenType::operator_bitxor,           Arrity::Binary,       110, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::BinaryExpression> },
    { TokenType::operator_bitor,            Arrity::Binary,       120, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::BinaryExpression> },
    { TokenType::operator_and,              Arrity::Binary,       130, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::BinaryExpression> },
    { TokenType::operator_or,               Arrity::Binary,       140, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::BinaryExpression> },
    { TokenType::operator_interrogation,    Arrity::Ternary,      150, Associativity::RightToLeft,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::TernaryExpression> },
    { TokenType::operator_equal,            Arrity::Binary,       160, Associativity::RightToLeft,   AllowSeparators::Yes, CreateTreeNode<semanticTree::Assignment> },
    { TokenType::operator_plus_equal,       Arrity::Binary,       160, Associativity::RightToLeft,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::CompoundAssignment> },
    { TokenType::operator_minus_equal,      Arrity::Binary,       160, Associativity::RightToLeft,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::CompoundAssignment> },
    { TokenType::operator_multiply_equal,   Arrity::Binary,       160, Associativity::RightToLeft,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::CompoundAssignment> },
    { TokenType::operator_divide_equal,     Arrity::Binary,       160, Associativity::RightToLeft,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::CompoundAssignment> },
    { TokenType::operator_modulo_equal,     Arrity::Binary,       160, Associativity::RightToLeft,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::CompoundAssignment> },
    { TokenType::operator_shift_left_equal, Arrity::Binary,       160, Associativity::RightToLeft,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::CompoundAssignment> },
    { TokenType::operator_shift_right_equal,Arrity::Binary,       160, Associativity::RightToLeft,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::CompoundAssignment> },
    { TokenType::operator_bitand_equal,     Arrity::Binary,       160, Associativity::RightToLeft,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::CompoundAssignment> },
    { TokenType::operator_bitor_equal,      Arrity::Binary,       160, Associativity::RightToLeft,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::CompoundAssignment> },
    { TokenType::operator_bitxor_equal,     Arrity::Binary,       160, Associativity::RightToLeft,   AllowSeparators::Yes, CreateTreeNodeWithToken<semanticTree::CompoundAssignment> },

    { TokenType::operator_comma,    Arrity::BinaryEnableTrailing, 180, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNode<semanticTree::Comma> },
    { TokenType::operator_colon,            Arrity::Binary,       190, Associativity::NotApplicable, AllowSeparators::Yes, CreateTreeNode<semanticTree::PropertyAffectation> },
    { TokenType::keyword_return,            Arrity::PrefixUnary,  200, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNode<semanticTree::Return> },
    { TokenType::keyword_assert,            Arrity::PrefixUnary,  200, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNode<semanticTree::Assert> },
    { TokenType::keyword_import,            Arrity::PrefixUnary,  200, Associativity::LeftToRight,   AllowSeparators::Yes, CreateTreeNode<semanticTree::Import> },

    { TokenType::unused,                    Arrity::Nullary,      210, Associativity::NotApplicable, AllowSeparators::Yes, nullptr },
};
size_t const operatorCount = SG_ARRAYSIZE(operatorTraits);
//=============================================================================
}
}

#endif



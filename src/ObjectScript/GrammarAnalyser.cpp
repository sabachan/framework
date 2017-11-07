#include "stdafx.h"

#include "GrammarAnalyser.h"

#include <Reflection\PrimitiveData.h>
#include "Operators.h"
#include "SemanticTree.h"
#include "Tokenizer.h"
#include <sstream>

namespace sg {
namespace objectscript {
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if 0
size_t GetOperatorTraitsIndex(TokenType iTokenType, bool hasSeparatorBefore = true, bool hasSeparatorAfter = true)
{
    size_t index = -1;
    for(size_t i = 0; i < operatorCount; ++i)
    {
        if(operatorTraits[i].op == iTokenType)
        {
            if(operatorTraits[i].allowSeparators == AllowSeparators::Yes)
            {
                SG_ASSERT_MSG(-1 == index, "operator associativity must be given to remove ambiguity");
                index = i;
            }
            else
            {
                switch(operatorTraits[i].arrity)
                {
                case Arrity::Binary:
                    if(!hasSeparatorBefore && !hasSeparatorAfter)
                    {
                        SG_ASSERT(-1 == index);
                        index = i;
                    }
                    break;
                case Arrity::PrefixUnary:
                    if(hasSeparatorBefore && !hasSeparatorAfter)
                    {
                        SG_ASSERT(-1 == index);
                        index = i;
                    }
                    break;
                case Arrity::SuffixUnary:
                    if(!hasSeparatorBefore && hasSeparatorAfter)
                    {
                        SG_ASSERT(-1 == index);
                        index = i;
                    }
                    break;
                default:
                    SG_ASSERT_NOT_REACHED();
                }
            }
#if !SG_ENABLE_ASSERT
            if(-1 != index)
                break;
#endif
        }
    }
    SG_ASSERT(-1 != index);
    return index;
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if SG_ENABLE_ASSERT
size_t GetOperatorTraitsIndex(TokenType iTokenType, Arrity iArrity)
{
    for(size_t i = 0; i < operatorCount; ++i)
    {
        if(operatorTraits[i].op == iTokenType && operatorTraits[i].arrity == iArrity)
            return i;
    }
    SG_ASSERT_NOT_REACHED();
    return all_ones;
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <TokenType tokenType> struct OperatorTraitsIndexGetter { };
#define DEFINE_GET_OPERATOR_INDEX_FAST(TOKEN_TYPE, INDEX) \
    template<> struct OperatorTraitsIndexGetter<TokenType::TOKEN_TYPE> \
    { \
        SG_FORCE_INLINE static size_t const index() \
        { \
            SG_ASSERT_MSG(operatorCount > INDEX, "Index for operator "#TOKEN_TYPE" has not been correctly set!"); \
            SG_ASSERT_MSG(operatorTraits[INDEX].op == TokenType::TOKEN_TYPE, "Incorrect index ("#INDEX") for operator "#TOKEN_TYPE); \
            SG_ASSERT_MSG(operatorTraits[INDEX].allowSeparators == AllowSeparators::Yes, "Operator " #TOKEN_TYPE " does not allow separators!"); \
            return INDEX; \
        } \
    };
#define DEFINE_GET_OPERATOR_INDEX_FAST_ARRITY(TOKEN_TYPE, INDEX_NULLARY, INDEX_PREFIX_UNARY, INDEX_SUFFIX_UNARY, INDEX_BINARY) \
    template<> struct OperatorTraitsIndexGetter<TokenType::TOKEN_TYPE> \
    { \
        SG_FORCE_INLINE static size_t const index(Arrity iArrity) \
        { \
            size_t const idx = (Arrity::Binary == iArrity) ? INDEX_BINARY : \
                               (Arrity::Nullary == iArrity) ? INDEX_NULLARY : \
                               (Arrity::PrefixUnary == iArrity) ? INDEX_PREFIX_UNARY : \
                               (Arrity::SuffixUnary == iArrity) ? INDEX_SUFFIX_UNARY : -1; \
            SG_ASSERT_MSG(operatorCount > idx, "Index for operator "#TOKEN_TYPE" has not been correctly set!"); \
            SG_ASSERT_MSG(operatorTraits[idx].op == TokenType::TOKEN_TYPE, "Incorrect indices ("#INDEX_NULLARY", "#INDEX_PREFIX_UNARY", "#INDEX_SUFFIX_UNARY", "#INDEX_BINARY") for operator "#TOKEN_TYPE); \
            SG_ASSERT_MSG(operatorTraits[idx].allowSeparators == AllowSeparators::Yes, "Operator " #TOKEN_TYPE " does not allow separators!"); \
            SG_ASSERT_MSG(operatorTraits[idx].arrity == iArrity, "Incorrect indices ("#INDEX_NULLARY", "#INDEX_PREFIX_UNARY", "#INDEX_SUFFIX_UNARY", "#INDEX_BINARY") for operator "#TOKEN_TYPE); \
            return idx; \
        } \
    };
#define DEFINE_GET_OPERATOR_INDEX_FAST_SEPARATORS(TOKEN_TYPE, INDEX11, INDEX10, INDEX01, INDEX00) \
    template<> struct OperatorTraitsIndexGetter<TokenType::TOKEN_TYPE> \
    { \
        SG_FORCE_INLINE static size_t const index(bool hasSeparatorBefore, bool hasSeparatorAfter) \
        { \
            size_t const idx = hasSeparatorBefore ? (hasSeparatorAfter ? INDEX11 : INDEX10) : (hasSeparatorAfter ? INDEX01 : INDEX00); \
            SG_ASSERT_MSG(-1 != idx, "Unexpected context for operator "#TOKEN_TYPE); \
            SG_ASSERT_MSG(-1 == idx || operatorTraits[idx].op == TokenType::TOKEN_TYPE, "Incorrect index for operator "#TOKEN_TYPE); \
            SG_ASSERT_MSG(-1 == idx || operatorTraits[idx].allowSeparators == AllowSeparators::No, "Operator " #TOKEN_TYPE " does allow separators!"); \
            SG_ASSERT_MSG(-1 == idx || operatorTraits[idx].arrity == (hasSeparatorBefore ? (hasSeparatorAfter ? Arrity::Unspecified : Arrity::PrefixUnary) : (hasSeparatorAfter ? Arrity::SuffixUnary : Arrity::Binary)), "Incorrect arrity for operator "#TOKEN_TYPE); \
            return idx; \
        } \
    };

DEFINE_GET_OPERATOR_INDEX_FAST(operator_dollar, 0)
DEFINE_GET_OPERATOR_INDEX_FAST(keyword_export, 1)
DEFINE_GET_OPERATOR_INDEX_FAST(keyword_public, 2)
DEFINE_GET_OPERATOR_INDEX_FAST(keyword_protected, 3)
DEFINE_GET_OPERATOR_INDEX_FAST(keyword_private, 4)
DEFINE_GET_OPERATOR_INDEX_FAST(keyword_const, 5)
DEFINE_GET_OPERATOR_INDEX_FAST(keyword_var, 6)
DEFINE_GET_OPERATOR_INDEX_FAST_ARRITY(open_bloc, 7, -1, 14, -1)
DEFINE_GET_OPERATOR_INDEX_FAST_ARRITY(open_parenthesis, 8, -1, 15, -1)
DEFINE_GET_OPERATOR_INDEX_FAST_ARRITY(open_bracket, 9, -1, 16, -1)
DEFINE_GET_OPERATOR_INDEX_FAST_SEPARATORS(operator_double_colon, -1, 10, -1, 11)
DEFINE_GET_OPERATOR_INDEX_FAST_SEPARATORS(operator_plus_plus, -1, 19, 12, -1)
DEFINE_GET_OPERATOR_INDEX_FAST_SEPARATORS(operator_minus_minus, -1, 20, 13, -1)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_dot, 17)
DEFINE_GET_OPERATOR_INDEX_FAST(keyword_is, 18)
DEFINE_GET_OPERATOR_INDEX_FAST_ARRITY(operator_plus, -1, 21, -1, 28)
DEFINE_GET_OPERATOR_INDEX_FAST_ARRITY(operator_minus, -1, 22, -1, 29)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_not, 23)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_bitnot, 24)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_multiply, 25)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_divide, 26)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_modulo, 27)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_shift_left, 30)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_shift_right, 31)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_less, 32)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_less_equal, 33)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_greater, 34)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_greater_equal, 35)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_equal_equal, 36)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_not_equal, 37)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_bitand, 38)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_bitxor, 39)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_bitor, 40)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_and, 41)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_or, 42)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_interrogation, 43)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_equal, 44)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_plus_equal, 45)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_minus_equal, 46)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_multiply_equal, 47)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_divide_equal, 48)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_modulo_equal, 49)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_shift_left_equal, 50)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_shift_right_equal, 51)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_bitand_equal, 52)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_bitor_equal, 53)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_bitxor_equal, 54)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_comma, 55)
DEFINE_GET_OPERATOR_INDEX_FAST(operator_colon, 56)
DEFINE_GET_OPERATOR_INDEX_FAST(keyword_return, 57)
DEFINE_GET_OPERATOR_INDEX_FAST(keyword_assert, 58)
DEFINE_GET_OPERATOR_INDEX_FAST(keyword_import, 59)
#undef DEFINE_GET_OPERATOR_INDEX_FAST
#undef DEFINE_GET_OPERATOR_INDEX_FAST_ARRITY
#undef DEFINE_GET_OPERATOR_INDEX_FAST_SEPARATORS
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
GrammarAnalyser::SemanticNode::SemanticNode(semanticTree::ITreeNode* iTreeNode)
    : treeNode(iTreeNode)
    , operatorTraitsIndexIfIncomplete(all_ones)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
GrammarAnalyser::SemanticNode::SemanticNode(semanticTree::ITreeNode* iTreeNode, size_t iOperatorIndex)
    : treeNode(iTreeNode)
    , operatorTraitsIndexIfIncomplete(iOperatorIndex)
{
    SG_ASSERT(-1 != iOperatorIndex);
}
//=============================================================================
GrammarAnalyser::StackNode::StackNode()
    : semanticNodes()
    , expectedClosingToken(TokenType::unknown)
    , grammarConstructs()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
GrammarAnalyser::StackNode::~StackNode()
{
}
//=============================================================================
GrammarAnalyser::GrammarAnalyser(char const* str, IErrorHandler* iErrorHandler)
    : m_tokenizer(str, iErrorHandler)
    , m_stack()
    , m_errorHandler(iErrorHandler)
{
    m_stack.emplace_back();
    m_stack.back().expectedClosingToken = TokenType::eof;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
GrammarAnalyser::~GrammarAnalyser()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void GrammarAnalyser::PushError(ErrorType iErrorType, Token const& iToken, char const* msg)
{
    Error err;
    err.type = iErrorType;
    err.filebegin = iToken.filebegin;
    err.begin = iToken.begin;
    err.end = iToken.end;
    err.fileid = iToken.fileid;
    err.col = iToken.col;
    err.line = iToken.line;
    err.msg = msg;
    m_errorHandler->OnObjectScriptError(err);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
namespace {
    bool IsNewInstruction(Token const& iToken, bool iIsValueExpected)
    {
        if(iIsValueExpected)
            return false;
        switch(iToken.type)
        {
        case TokenType::invalid: break;
        case TokenType::identifier:
        case TokenType::hexadecimal:
        case TokenType::integer:
        case TokenType::number:
        case TokenType::string: return true;
        case TokenType::open_bloc: return false; // depends on previous token ?
        case TokenType::close_bloc: return false;
        case TokenType::open_parenthesis: return false; // depends on previous token ?
        case TokenType::close_parenthesis: return false;
        case TokenType::open_bracket: return false; // depends on previous token ?
        case TokenType::close_bracket: return false;
        case TokenType::operator_colon:
        case TokenType::operator_semicolon:
        case TokenType::operator_double_colon:
        case TokenType::operator_comma:
        case TokenType::operator_equal:
        case TokenType::operator_plus:
        case TokenType::operator_minus:
        case TokenType::operator_divide:
        case TokenType::operator_multiply:
        case TokenType::operator_plus_equal:
        case TokenType::operator_minus_equal:
        case TokenType::operator_multiply_equal:
        case TokenType::operator_divide_equal:
        case TokenType::operator_plus_plus:
        case TokenType::operator_minus_minus:
        case TokenType::operator_not:
        case TokenType::operator_bitand:
        case TokenType::operator_bitor:
        case TokenType::operator_bitxor:
        case TokenType::operator_and:
        case TokenType::operator_or:
        case TokenType::operator_equal_equal:
        case TokenType::operator_not_equal:
        case TokenType::operator_less:
        case TokenType::operator_greater:
        case TokenType::operator_less_equal:
        case TokenType::operator_greater_equal:
        case TokenType::operator_interrogation:
        case TokenType::operator_dot:
        case TokenType::operator_dollar: return false;
        case TokenType::keyword_do:
        case TokenType::keyword_if: return true;
        case TokenType::keyword_is: return false;
        case TokenType::keyword_in: return false;
        case TokenType::keyword_for:
        case TokenType::keyword_case:
        case TokenType::keyword_else: return true;
        case TokenType::keyword_null: return false;
        case TokenType::keyword_true: return false;
        case TokenType::keyword_alias:
        case TokenType::keyword_break:
        case TokenType::keyword_const: return true;
        case TokenType::keyword_false: return false;
        case TokenType::keyword_while:
        case TokenType::keyword_assert:
        case TokenType::keyword_export:
        case TokenType::keyword_public:
        case TokenType::keyword_return:
        case TokenType::keyword_switch:
        case TokenType::keyword_private:
        case TokenType::keyword_continue:
        case TokenType::keyword_function:
        case TokenType::keyword_template:
        case TokenType::keyword_namespace:
        case TokenType::keyword_protected: return true;
        case TokenType::eof: return true;
        default:
            SG_ASSERT_NOT_REACHED();
        }

        return false;
    }
    bool IsSeparator(Token const& iTokenLeft, Token const& iTokenRight)
    {
        switch(iTokenLeft.type)
        {
            case TokenType::operator_double_colon:
                switch(iTokenRight.type)
                {
                    case TokenType::identifier:
                    case TokenType::hexadecimal:
                    case TokenType::integer:
                    case TokenType::number:
                    case TokenType::string:
                    case TokenType::operator_double_colon:
                    case TokenType::operator_colon:
                    case TokenType::open_bloc:
                    case TokenType::open_parenthesis:
                    case TokenType::open_bracket:
                        return false;
                }
                break;
            case TokenType::operator_plus_plus:
            case TokenType::operator_minus_minus:
                switch(iTokenRight.type)
                {
                    case TokenType::identifier:
                    case TokenType::hexadecimal:
                    case TokenType::integer:
                    case TokenType::number:
                    case TokenType::string:
                    case TokenType::operator_plus_plus:
                    case TokenType::operator_minus_minus:
                    case TokenType::operator_plus:
                    case TokenType::operator_minus:
                    case TokenType::open_bloc:
                    case TokenType::open_parenthesis:
                    case TokenType::open_bracket:
                        return false;
                }
                break;
            default:
                break;
        }
        switch(iTokenRight.type)
        {
            case TokenType::operator_double_colon:
                switch(iTokenLeft.type)
                {
                    case TokenType::identifier:
                    case TokenType::hexadecimal:
                    case TokenType::integer:
                    case TokenType::number:
                    case TokenType::string:
                    case TokenType::operator_double_colon:
                    case TokenType::operator_colon:
                    case TokenType::close_bloc:
                    case TokenType::close_parenthesis:
                    case TokenType::close_bracket:
                        return false;
                }
                break;
            case TokenType::operator_plus_plus:
            case TokenType::operator_minus_minus:
                switch(iTokenLeft.type)
                {
                    case TokenType::identifier:
                    case TokenType::hexadecimal:
                    case TokenType::integer:
                    case TokenType::number:
                    case TokenType::string:
                    case TokenType::operator_plus_plus:
                    case TokenType::operator_minus_minus:
                    case TokenType::operator_plus:
                    case TokenType::operator_minus:
                    case TokenType::close_bloc:
                    case TokenType::close_parenthesis:
                    case TokenType::close_bracket:
                        return false;
                }
                break;
            default:
                break;
        }
        return true;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
semanticTree::Root* GrammarAnalyser::Run()
{
    bool hasSeparatorBefore = true;
    bool hasSeparatorAfter = true;
    Token token;
    Token nextToken;
    m_tokenizer.Tokenize(nextToken);
    do
    {
        token = nextToken;
        hasSeparatorBefore = hasSeparatorAfter;
        m_tokenizer.Tokenize(nextToken);
        if(m_errorHandler->DidErrorHappen())
            return nullptr;
        hasSeparatorAfter = nextToken.begin != token.end || TokenType::eof == nextToken.type || IsSeparator(token, nextToken);
        ProcessToken(token, hasSeparatorBefore, hasSeparatorAfter);
        if(m_errorHandler->DidErrorHappen())
            return nullptr;
    } while(TokenType::eof != nextToken.type);
    ProcessToken(nextToken, true, true);

    if(m_errorHandler->DidErrorHappen())
        return nullptr;

    std::vector<refptr<semanticTree::ITreeNode> > instructions;
    {
        StackNode& stackNode = m_stack.back();
        size_t const instructionsCount = stackNode.semanticNodes.size();
        instructions.reserve(instructionsCount);
        auto const begin = stackNode.semanticNodes.begin();
        auto const end = stackNode.semanticNodes.end();
        for(auto it = begin; it != end; ++it)
        {
            SG_ASSERT(it->subTreeNodes.empty());
            SG_ASSERT(-1 == it->operatorTraitsIndexIfIncomplete);
            instructions.push_back(it->treeNode.get());
        }
    }
    semanticTree::Root* root = new semanticTree::Root;
    root->SetInstructions(instructions);
    return root;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
// TODO: how to share beetween grammar constructs ?
bool GrammarAnalyser::ProcessGrammarConstructIFNReturnDone(Token const& iToken, bool iIsValueExpected)
{
    bool poped;
    do
    {
        poped = false;
        SG_ASSERT(!m_stack.back().grammarConstructs.empty());
        switch(m_stack.back().grammarConstructs.back())
        {
        case GrammarConstruct::None:
            SG_ASSERT_NOT_REACHED();
            break;
        case GrammarConstruct::Namespace_Name:
            SG_ASSERT(!IsNewInstruction(iToken, iIsValueExpected));
            if(TokenType::open_bloc == iToken.type)
            {
                PopStackNode(iToken, TokenType::unknown);
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::Namespace* treeNode = checked_cast<semanticTree::Namespace*>(node.treeNode.get());
                SG_ASSERT(node.subTreeNodes.size() == 1);
                treeNode->SetName(node.subTreeNodes[0].get());
                node.subTreeNodes.clear();
                PushStackNode(TokenType::open_bloc, GrammarConstruct::Namespace_Bloc);
                return true;
            }
            break;
        case GrammarConstruct::Namespace_Bloc:
            if(TokenType::close_bloc == iToken.type)
            {
                PopStackNode(iToken, TokenType::close_bloc);
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::Namespace* treeNode = checked_cast<semanticTree::Namespace*>(node.treeNode.get());
                treeNode->SetInstructions(node.subTreeNodes);
                node.subTreeNodes.clear();
                return true;
            }
            break;
        case GrammarConstruct::If:
            {
                if(TokenType::open_parenthesis != iToken.type)
                {
                    PushError(ErrorType::if_missing_parentheses, iToken, "mising parentheses after if");
                    return true;
                }
                PushStackNode(TokenType::open_parenthesis, GrammarConstruct::If_Expr);
                return true;
            }
            break;
        case GrammarConstruct::If_Expr:
            if(TokenType::close_parenthesis == iToken.type)
            {
                PopStackNode(iToken, TokenType::close_parenthesis);
                SG_ASSERT(GrammarConstruct::If == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::If* treeNode = checked_cast<semanticTree::If*>(node.treeNode.get());
                if(node.subTreeNodes.empty())
                {
                    PushError(ErrorType::if_empty_condition, iToken, "empty condition in if");
                    return false;
                }
                else if(node.subTreeNodes.size() > 1)
                {
                    PushError(ErrorType::if_invalid_condition, iToken, "too many expression in if");
                    return false;
                }
                treeNode->SetExpr(node.subTreeNodes[0].get());
                node.subTreeNodes.clear();
                m_stack.back().grammarConstructs.back() = GrammarConstruct::If_PreInstruction;
                return true;
            }
            break;
        case GrammarConstruct::If_PreInstruction:
            if(TokenType::open_bloc == iToken.type)
            {
                PushStackNode(TokenType::open_bloc, GrammarConstruct::If_InstructionBloc);
                return true;
            }
            else
            {
                PushStackNode(TokenType::unknown, GrammarConstruct::If_Instruction);
                return false; // token must still be processed
            }
            break;
        case GrammarConstruct::If_Instruction:
            if(IsNewInstruction(iToken, iIsValueExpected))
            {
                PopStackNode(iToken, TokenType::unknown);
                SG_ASSERT(GrammarConstruct::If_PreInstruction == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::If* treeNode = checked_cast<semanticTree::If*>(node.treeNode.get());
                SG_ASSERT(node.subTreeNodes.size() == 1);
                treeNode->SetInstructions(node.subTreeNodes);
                node.subTreeNodes.clear();
                if(TokenType::keyword_else == iToken.type)
                {
                    m_stack.back().grammarConstructs.back() = GrammarConstruct::If_ElsePreInstruction;
                    return true;
                }
                else
                {
                    poped = true;
                    m_stack.back().grammarConstructs.pop_back();
                }
            }
            break;
        case GrammarConstruct::If_InstructionBloc:
            if(TokenType::close_bloc == iToken.type)
            {
                PopStackNode(iToken, TokenType::close_bloc);
                SG_ASSERT(GrammarConstruct::If_PreInstruction == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::If* treeNode = checked_cast<semanticTree::If*>(node.treeNode.get());
                treeNode->SetInstructions(node.subTreeNodes);
                node.subTreeNodes.clear();
                m_stack.back().grammarConstructs.back() = GrammarConstruct::If_PreElse;
                return true;
            }
            break;
        case GrammarConstruct::If_PreElse:
            if(TokenType::keyword_else == iToken.type)
            {
                m_stack.back().grammarConstructs.back() = GrammarConstruct::If_ElsePreInstruction;
                return true;
            }
            else
            {
                poped = true;
                m_stack.back().grammarConstructs.pop_back();
                // return false; // TO REMOVE ?
            }
            break;
        case GrammarConstruct::If_ElsePreInstruction:
            if(TokenType::open_bloc == iToken.type)
            {
                PushStackNode(TokenType::open_bloc, GrammarConstruct::If_ElseInstructionBloc);
                return true;
            }
            else
            {
                PushStackNode(TokenType::unknown, GrammarConstruct::If_ElseInstruction);
                return false; // token must still be processed
            }
            break;
        case GrammarConstruct::If_ElseInstruction:
            if(IsNewInstruction(iToken, iIsValueExpected))
            {
                PopStackNode(iToken, TokenType::unknown);
                SG_ASSERT(GrammarConstruct::If_ElsePreInstruction == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::If* treeNode = checked_cast<semanticTree::If*>(node.treeNode.get());
                SG_ASSERT(node.subTreeNodes.size() == 1);
                treeNode->SetElseInstructions(node.subTreeNodes);
                node.subTreeNodes.clear();
                poped = true;
                m_stack.back().grammarConstructs.pop_back();
            }
            break;
        case GrammarConstruct::If_ElseInstructionBloc:
            if(TokenType::close_bloc == iToken.type)
            {
                PopStackNode(iToken, TokenType::close_bloc);
                SG_ASSERT(GrammarConstruct::If_ElsePreInstruction == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::If* treeNode = checked_cast<semanticTree::If*>(node.treeNode.get());
                treeNode->SetElseInstructions(node.subTreeNodes);
                node.subTreeNodes.clear();
                poped = true;
                m_stack.back().grammarConstructs.pop_back();
                return true;
            }
            break;
        case GrammarConstruct::While:
            {
                if(TokenType::open_parenthesis != iToken.type)
                {
                    PushError(ErrorType::while_missing_parentheses, iToken, "mising parentheses after while");
                    return true;
                }
                PushStackNode(TokenType::open_parenthesis, GrammarConstruct::While_Expr);
                return true;
            }
            break;
        case GrammarConstruct::While_Expr:
            if(TokenType::close_parenthesis == iToken.type)
            {
                PopStackNode(iToken, TokenType::close_parenthesis);
                SG_ASSERT(GrammarConstruct::While == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::While* treeNode = checked_cast<semanticTree::While*>(node.treeNode.get());
                if(node.subTreeNodes.size() > 1)
                {
                    PushError(ErrorType::while_invalid_condition, iToken, "too many expressions in condition");
                    return false;
                }
                if(node.subTreeNodes.empty())
                {
                    PushError(ErrorType::while_empty_condition, iToken, "missing condition");
                    return false;
                }
                treeNode->SetExpr(node.subTreeNodes[0].get());
                node.subTreeNodes.clear();
                m_stack.back().grammarConstructs.back() = GrammarConstruct::While_PreInstruction;
                return true;
            }
            break;
        case GrammarConstruct::While_PreInstruction:
            if(TokenType::open_bloc == iToken.type)
            {
                PushStackNode(TokenType::open_bloc, GrammarConstruct::While_InstructionBloc);
                return true;
            }
            else
            {
                PushStackNode(TokenType::unknown, GrammarConstruct::While_Instruction);
                return false; // token must still be processed
            }
            break;
        case GrammarConstruct::While_Instruction:
            if(IsNewInstruction(iToken, iIsValueExpected))
            {
                PopStackNode(iToken, TokenType::unknown);
                SG_ASSERT(GrammarConstruct::While_PreInstruction == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::While* treeNode = checked_cast<semanticTree::While*>(node.treeNode.get());
                SG_ASSERT(node.subTreeNodes.size() == 1);
                treeNode->SetInstructions(node.subTreeNodes);
                node.subTreeNodes.clear();
                poped = true;
                m_stack.back().grammarConstructs.pop_back();
            }
            break;
        case GrammarConstruct::While_InstructionBloc:
            if(TokenType::close_bloc == iToken.type)
            {
                PopStackNode(iToken, TokenType::close_bloc);
                SG_ASSERT(GrammarConstruct::While_PreInstruction == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::While* treeNode = checked_cast<semanticTree::While*>(node.treeNode.get());
                treeNode->SetInstructions(node.subTreeNodes);
                node.subTreeNodes.clear();
                m_stack.back().grammarConstructs.pop_back();
                return true;
            }
            break;
        case GrammarConstruct::For:
            {
                if(TokenType::open_parenthesis != iToken.type)
                {
                    PushError(ErrorType::for_missing_parentheses, iToken, "mising parentheses after for");
                    return true;
                }
                PushStackNode(TokenType::open_parenthesis, GrammarConstruct::For_Expr);
                return true;
            }
            break;
        case GrammarConstruct::For_Expr:
            if(TokenType::keyword_in == iToken.type)
            {
                SG_ASSERT(m_stack.size() >= 2);
                StackNode& forStackNode = m_stack[m_stack.size()-2];
                SG_ASSERT(GrammarConstruct::For == forStackNode.grammarConstructs.back());
                SemanticNode& node = forStackNode.semanticNodes.back();
                semanticTree::For* treeNode = checked_cast<semanticTree::For*>(node.treeNode.get());
                semanticTree::For::ConstructProgress progress = treeNode->GetConstructProgress();
                SG_ASSERT_AND_UNUSED(semanticTree::For::ConstructProgress::Begin == progress);
                ForceSolve(iToken);
                SG_ASSERT(m_stack.back().semanticNodes.size() == 1);
                treeNode->SetVar(m_stack.back().semanticNodes.back().treeNode.get());
                m_stack.back().semanticNodes.pop_back();
                return true;
            }
            else if(TokenType::operator_semicolon == iToken.type)
            {
                SG_ASSERT(m_stack.size() >= 2);
                StackNode& forStackNode = m_stack[m_stack.size()-2];
                SG_ASSERT(GrammarConstruct::For == forStackNode.grammarConstructs.back());
                SemanticNode& node = forStackNode.semanticNodes.back();
                semanticTree::For* treeNode = checked_cast<semanticTree::For*>(node.treeNode.get());
                semanticTree::For::ConstructProgress progress = treeNode->GetConstructProgress();
                ForceSolve(iToken);
                switch(progress)
                {
                case semanticTree::For::ConstructProgress::Begin:
                    if(m_stack.back().semanticNodes.empty())
                    {
                        semanticTree::ITreeNode* noop = new semanticTree::NoOp;
                        noop->SetToken(iToken);
                        treeNode->SetInit(noop);
                    }
                    else if(m_stack.back().semanticNodes.size() == 1)
                    {
                        treeNode->SetInit(m_stack.back().semanticNodes.back().treeNode.get());
                        m_stack.back().semanticNodes.pop_back();
                    }
                    else
                    {
                        std::vector<refptr<semanticTree::ITreeNode>> init;
                        for(auto const& it : m_stack.back().semanticNodes)
                            init.push_back(it.treeNode.get());
                        treeNode->SetInit(AsArrayView(init));
                        m_stack.back().semanticNodes.clear();
                    }
                    SG_ASSERT(m_stack.back().semanticNodes.empty());
                    break;
                case semanticTree::For::ConstructProgress::Init:
                    if(m_stack.back().semanticNodes.size() > 1)
                    {
                        PushError(ErrorType::for_invalid_construct, iToken, "too many expressions in condition");
                        return false;
                    }
                    if(m_stack.back().semanticNodes.empty())
                    {
                        // An empty condition means true
                        semanticTree::ITreeNode* cond = new semanticTree::Value(new reflection::PrimitiveData<bool>(true));
                        cond->SetToken(iToken);
                        treeNode->SetCond(cond);
                    }
                    else
                    {
                        treeNode->SetCond(m_stack.back().semanticNodes.back().treeNode.get());
                        m_stack.back().semanticNodes.pop_back();
                    }
                    SG_ASSERT(m_stack.back().semanticNodes.empty());
                    break;
                default:
                    PushError(ErrorType::for_too_many_semicolons, iToken, "too many semicolon in for construct");
                    return false;
                }
                return true;
            }
            else if(TokenType::close_parenthesis == iToken.type)
            {
                PopStackNode(iToken, TokenType::close_parenthesis);
                SG_ASSERT(GrammarConstruct::For == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::For* treeNode = checked_cast<semanticTree::For*>(node.treeNode.get());
                semanticTree::For::ConstructProgress progress = treeNode->GetConstructProgress();
                switch(progress)
                {
                case semanticTree::For::ConstructProgress::In:
                    if(node.subTreeNodes.empty())
                    {
                        PushError(ErrorType::for_invalid_construct, iToken, "invalid for construct");
                        break;
                    }
                    SG_ASSERT(node.subTreeNodes.size() == 1);
                    treeNode->SetList(node.subTreeNodes[0].get());
                    break;
                case semanticTree::For::ConstructProgress::Cond:
                    if(node.subTreeNodes.empty())
                    {
                        semanticTree::ITreeNode* noop = new semanticTree::NoOp;
                        noop->SetToken(iToken);
                        treeNode->SetIncr(noop);
                    }
                    else
                    {
                        treeNode->SetIncr(AsArrayView(node.subTreeNodes));
                    }
                    break;
                case semanticTree::For::ConstructProgress::Begin:
                    if(node.subTreeNodes.empty())
                        PushError(ErrorType::for_empty_construct, iToken, "missing expression in for parentheses");
                    else
                        PushError(ErrorType::for_invalid_construct, iToken, "invalid for construct");
                    break;
                case semanticTree::For::ConstructProgress::Init:
                    PushError(ErrorType::for_invalid_construct, iToken, "invalid for construct");
                    break;
                default:
                    SG_ASSERT_NOT_REACHED(); // TODO: Error message
                }
                node.subTreeNodes.clear();
                m_stack.back().grammarConstructs.back() = GrammarConstruct::For_PreInstruction;
                return true;
            }
            break;
        case GrammarConstruct::For_PreInstruction:
            if(TokenType::open_bloc == iToken.type)
            {
                PushStackNode(TokenType::open_bloc, GrammarConstruct::For_InstructionBloc);
                return true;
            }
            else
            {
                PushStackNode(TokenType::unknown, GrammarConstruct::For_Instruction);
                return false; // token must still be processed
            }
            break;
        case GrammarConstruct::For_Instruction:
            if(IsNewInstruction(iToken, iIsValueExpected))
            {
                PopStackNode(iToken, TokenType::unknown);
                SG_ASSERT(GrammarConstruct::For_PreInstruction == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::For* treeNode = checked_cast<semanticTree::For*>(node.treeNode.get());
                SG_ASSERT(node.subTreeNodes.size() == 1);
                treeNode->SetInstructions(node.subTreeNodes);
                node.subTreeNodes.clear();
                poped = true;
                m_stack.back().grammarConstructs.pop_back();
            }
            break;
        case GrammarConstruct::For_InstructionBloc:
            if(TokenType::close_bloc == iToken.type)
            {
                PopStackNode(iToken, TokenType::close_bloc);
                SG_ASSERT(GrammarConstruct::For_PreInstruction == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::For* treeNode = checked_cast<semanticTree::For*>(node.treeNode.get());
                treeNode->SetInstructions(node.subTreeNodes);
                node.subTreeNodes.clear();
                m_stack.back().grammarConstructs.pop_back();
                return true;
            }
            break;
        case GrammarConstruct::Function:
            SG_ASSERT_NOT_REACHED();
            break;
        case GrammarConstruct::Function_Name:
            if(TokenType::open_parenthesis == iToken.type)
            {
                PopStackNode(iToken, TokenType::unknown);
                SG_ASSERT(GrammarConstruct::Function == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::FunctionDeclaration* treeNode = checked_cast<semanticTree::FunctionDeclaration*>(node.treeNode.get());
                SG_ASSERT(node.subTreeNodes.size() == 1);
                treeNode->SetName(node.subTreeNodes[0].get());
                node.subTreeNodes.clear();
                PushStackNode(TokenType::unknown, GrammarConstruct::Function_Args);
                return true;
            }
            else
            {
                SG_ASSERT(!IsNewInstruction(iToken, iIsValueExpected));
            }
        case GrammarConstruct::Function_Args:
            if(TokenType::close_parenthesis == iToken.type)
            {
                PopStackNode(iToken, TokenType::unknown);
                SG_ASSERT(GrammarConstruct::Function == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::FunctionDeclaration* treeNode = checked_cast<semanticTree::FunctionDeclaration*>(node.treeNode.get());
                if(!node.subTreeNodes.empty())
                {
                    SG_ASSERT(node.subTreeNodes.size() == 1);
                    treeNode->PushArgumentName(node.subTreeNodes[0].get());
                    node.subTreeNodes.clear();
                }
                m_stack.back().grammarConstructs.back() = GrammarConstruct::Function_Prototype;
                return true;
            }
            else if(TokenType::operator_equal == iToken.type)
            {
                PopStackNode(iToken, TokenType::unknown);
                SG_ASSERT(GrammarConstruct::Function == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::FunctionDeclaration* treeNode = checked_cast<semanticTree::FunctionDeclaration*>(node.treeNode.get());
                SG_ASSERT(node.subTreeNodes.size() == 1);
                treeNode->PushArgumentName(node.subTreeNodes[0].get());
                node.subTreeNodes.clear();
                PushStackNode(TokenType::unknown, GrammarConstruct::Function_ArgDefault);
                return true;
            }
            else if(TokenType::operator_comma == iToken.type)
            {
                PopStackNode(iToken, TokenType::unknown);
                SG_ASSERT(GrammarConstruct::Function == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::FunctionDeclaration* treeNode = checked_cast<semanticTree::FunctionDeclaration*>(node.treeNode.get());
                SG_ASSERT(node.subTreeNodes.size() == 1);
                treeNode->PushArgumentName(node.subTreeNodes[0].get());
                node.subTreeNodes.clear();
                PushStackNode(TokenType::unknown, GrammarConstruct::Function_Args);
                return true;
            }
            break;
        case GrammarConstruct::Function_ArgDefault:
            if(TokenType::close_parenthesis == iToken.type)
            {
                PopStackNode(iToken, TokenType::unknown);
                SG_ASSERT(GrammarConstruct::Function == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::FunctionDeclaration* treeNode = checked_cast<semanticTree::FunctionDeclaration*>(node.treeNode.get());
                SG_ASSERT(node.subTreeNodes.size() == 1);
                treeNode->PushArgumentDefaultValue(node.subTreeNodes[0].get());
                node.subTreeNodes.clear();
                m_stack.back().grammarConstructs.back() = GrammarConstruct::Function_Prototype;
                return true;
            }
            else if(TokenType::operator_comma == iToken.type)
            {
                PopStackNode(iToken, TokenType::unknown);
                SG_ASSERT(GrammarConstruct::Function == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::FunctionDeclaration* treeNode = checked_cast<semanticTree::FunctionDeclaration*>(node.treeNode.get());
                SG_ASSERT(node.subTreeNodes.size() == 1);
                treeNode->PushArgumentDefaultValue(node.subTreeNodes[0].get());
                node.subTreeNodes.clear();
                PushStackNode(TokenType::unknown, GrammarConstruct::Function_Args);
                return true;
            }
            break;
        case GrammarConstruct::Function_Prototype:
            if(TokenType::open_bloc == iToken.type)
            {
                PushStackNode(TokenType::open_bloc, GrammarConstruct::Function_Body);
                return true;
            }
            else
            {
                SG_ASSERT(GrammarConstruct::Function_Prototype == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::FunctionDeclaration* treeNode = checked_cast<semanticTree::FunctionDeclaration*>(node.treeNode.get());
                treeNode->SetIsPrototypeOnly();
                m_stack.back().grammarConstructs.pop_back();
                SG_ASSERT(m_stack.back().grammarConstructs.empty());
                return false; // token must still be processed
            }
            break;
        case GrammarConstruct::Function_Body:
            if(TokenType::close_bloc == iToken.type)
            {
                PopStackNode(iToken, TokenType::close_bloc);
                SG_ASSERT(GrammarConstruct::Function_Prototype == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::FunctionDeclaration* treeNode = checked_cast<semanticTree::FunctionDeclaration*>(node.treeNode.get());
                treeNode->SetInstructions(node.subTreeNodes);
                node.subTreeNodes.clear();
                m_stack.back().grammarConstructs.pop_back();
                return true;
            }
            break;
        case GrammarConstruct::Template:
            SG_ASSERT_NOT_REACHED();
            break;
        case GrammarConstruct::Template_Name:
            if(TokenType::open_parenthesis == iToken.type)
            {
                PopStackNode(iToken, TokenType::unknown);
                SG_ASSERT(GrammarConstruct::Template == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::TemplateDeclaration* treeNode = checked_cast<semanticTree::TemplateDeclaration*>(node.treeNode.get());
                SG_ASSERT(node.subTreeNodes.size() == 1);
                treeNode->SetName(node.subTreeNodes[0].get());
                node.subTreeNodes.clear();
                PushStackNode(TokenType::unknown, GrammarConstruct::Template_Args);
                return true;
            }
            else
            {
                SG_ASSERT(!IsNewInstruction(iToken, iIsValueExpected));
            }
        case GrammarConstruct::Template_Args:
            if(TokenType::close_parenthesis == iToken.type)
            {
                PopStackNode(iToken, TokenType::unknown);
                SG_ASSERT(GrammarConstruct::Template == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::TemplateDeclaration* treeNode = checked_cast<semanticTree::TemplateDeclaration*>(node.treeNode.get());
                if(!node.subTreeNodes.empty())
                {
                    SG_ASSERT(node.subTreeNodes.size() == 1);
                    treeNode->PushArgumentName(node.subTreeNodes[0].get());
                    node.subTreeNodes.clear();
                }
                m_stack.back().grammarConstructs.back() = GrammarConstruct::Template_Prototype;
                return true;
            }
            else if(TokenType::operator_equal == iToken.type)
            {
                PopStackNode(iToken, TokenType::unknown);
                SG_ASSERT(GrammarConstruct::Template == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::TemplateDeclaration* treeNode = checked_cast<semanticTree::TemplateDeclaration*>(node.treeNode.get());
                SG_ASSERT(node.subTreeNodes.size() == 1);
                treeNode->PushArgumentName(node.subTreeNodes[0].get());
                node.subTreeNodes.clear();
                PushStackNode(TokenType::unknown, GrammarConstruct::Template_ArgDefault);
                return true;
            }
            else if(TokenType::operator_comma == iToken.type)
            {
                PopStackNode(iToken, TokenType::unknown);
                SG_ASSERT(GrammarConstruct::Template == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::TemplateDeclaration* treeNode = checked_cast<semanticTree::TemplateDeclaration*>(node.treeNode.get());
                SG_ASSERT(node.subTreeNodes.size() == 1);
                treeNode->PushArgumentName(node.subTreeNodes[0].get());
                node.subTreeNodes.clear();
                PushStackNode(TokenType::unknown, GrammarConstruct::Template_Args);
                return true;
            }
            break;
        case GrammarConstruct::Template_ArgDefault:
            if(TokenType::close_parenthesis == iToken.type)
            {
                PopStackNode(iToken, TokenType::unknown);
                SG_ASSERT(GrammarConstruct::Template == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::TemplateDeclaration* treeNode = checked_cast<semanticTree::TemplateDeclaration*>(node.treeNode.get());
                SG_ASSERT(node.subTreeNodes.size() == 1);
                treeNode->PushArgumentDefaultValue(node.subTreeNodes[0].get());
                node.subTreeNodes.clear();
                m_stack.back().grammarConstructs.back() = GrammarConstruct::Template_Prototype;
                return true;
            }
            else if(TokenType::operator_comma == iToken.type)
            {
                PopStackNode(iToken, TokenType::unknown);
                SG_ASSERT(GrammarConstruct::Template == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::TemplateDeclaration* treeNode = checked_cast<semanticTree::TemplateDeclaration*>(node.treeNode.get());
                SG_ASSERT(node.subTreeNodes.size() == 1);
                treeNode->PushArgumentDefaultValue(node.subTreeNodes[0].get());
                node.subTreeNodes.clear();
                PushStackNode(TokenType::unknown, GrammarConstruct::Template_Args);
                return true;
            }
            break;
        case GrammarConstruct::Template_Prototype:
            {
                SG_ASSERT(GrammarConstruct::Template_Prototype == m_stack.back().grammarConstructs.back());
                SG_ASSERT(TokenType::keyword_is == iToken.type); // TODO: Error message
                PushStackNode(TokenType::unknown, GrammarConstruct::Template_Type);
                return true;
            }
            break;
        case GrammarConstruct::Template_Type:
            SG_ASSERT(!IsNewInstruction(iToken, iIsValueExpected)); // TODO: Error message
            if(TokenType::open_bloc == iToken.type)
            {
                PopStackNode(iToken, TokenType::unknown);
                SG_ASSERT(GrammarConstruct::Template_Prototype == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::TemplateDeclaration* treeNode = checked_cast<semanticTree::TemplateDeclaration*>(node.treeNode.get());
                SG_ASSERT(node.subTreeNodes.size() == 1);
                treeNode->SetType(node.subTreeNodes[0].get());
                node.subTreeNodes.clear();
                PushStackNode(TokenType::open_bloc, GrammarConstruct::Template_Body);
                return true;
            }
            break;
        case GrammarConstruct::Template_Body:
            if(TokenType::close_bloc == iToken.type)
            {
                PopStackNode(iToken, TokenType::close_bloc);
                SG_ASSERT(GrammarConstruct::Template_Prototype == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::TemplateDeclaration* treeNode = checked_cast<semanticTree::TemplateDeclaration*>(node.treeNode.get());
                treeNode->SetInstructions(node.subTreeNodes);
                node.subTreeNodes.clear();
                m_stack.back().grammarConstructs.pop_back();
                return true;
            }
            break;
        case GrammarConstruct::Typedef:
            SG_ASSERT_NOT_REACHED();
            break;
        case GrammarConstruct::Typedef_BeforeType:
            SG_ASSERT(TokenType::identifier == iToken.type || TokenType::operator_double_colon == iToken.type); // Else, is it a correct typedef?
            m_stack.back().grammarConstructs.back() = GrammarConstruct::Typedef_Type;
            break;
        case GrammarConstruct::Typedef_Type:
            SG_ASSERT(TokenType::identifier == iToken.type || TokenType::operator_double_colon == iToken.type); // Else, is it a correct typedef?
            if(TokenType::identifier == iToken.type && !iIsValueExpected)
            {
                PopStackNode(iToken, TokenType::unknown);
                SG_ASSERT(GrammarConstruct::Typedef == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::TypedefDeclaration* treeNode = checked_cast<semanticTree::TypedefDeclaration*>(node.treeNode.get());
                SG_ASSERT(node.subTreeNodes.size() == 1);
                treeNode->SetType(node.subTreeNodes[0].get());
                node.subTreeNodes.clear();
                PushStackNode(TokenType::unknown, GrammarConstruct::Typedef_Alias);
                break;
            }
            break;
        case GrammarConstruct::Typedef_Alias:
            {
                PopStackNode(iToken, TokenType::unknown);
                SG_ASSERT(GrammarConstruct::Typedef == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::TypedefDeclaration* treeNode = checked_cast<semanticTree::TypedefDeclaration*>(node.treeNode.get());
                SG_ASSERT(node.subTreeNodes.size() == 1); // TODO: Error message
                SG_ASSERT(node.subTreeNodes[0]->IsValue());
                treeNode->SetAlias(node.subTreeNodes[0].get());
                node.subTreeNodes.clear();
                m_stack.back().grammarConstructs.pop_back();
                break;
            }
            break;
        case GrammarConstruct::Alias:
            SG_ASSERT_NOT_REACHED();
            break;
        case GrammarConstruct::Alias_Name:
        {
            if(TokenType::identifier != iToken.type)
            {
                PushError(ErrorType::expected_identifier_for_alias_name, iToken, "alias expects an identifier");
                return true;
            }
            SG_ASSERT(TokenType::identifier == iToken.type);
            PopStackNode(iToken, TokenType::unknown);
            PushStackNode(TokenType::unknown, GrammarConstruct::Alias_Is);
            break;
        }
        case GrammarConstruct::Alias_Is:
        {
            if(TokenType::keyword_is != iToken.type)
            {
                PushError(ErrorType::alias_missing_is, iToken, "mising keyword is in alias construct");
                return true;
            }
            PopStackNode(iToken, TokenType::unknown);
            SG_ASSERT(GrammarConstruct::Alias == m_stack.back().grammarConstructs.back());
            SemanticNode& node = m_stack.back().semanticNodes.back();
            semanticTree::TypedefDeclaration* treeNode = checked_cast<semanticTree::TypedefDeclaration*>(node.treeNode.get());
            SG_ASSERT(node.subTreeNodes.size() == 1); // TODO: Error message
            SG_ASSERT(node.subTreeNodes[0]->IsValue());
            treeNode->SetAlias(node.subTreeNodes[0].get());
            node.subTreeNodes.clear();
            PushStackNode(TokenType::unknown, GrammarConstruct::Alias_Type);
            return true;
        }
        case GrammarConstruct::Alias_Type:
        {
            if(TokenType::operator_double_colon != iToken.type && !iIsValueExpected)
            {
                PopStackNode(iToken, TokenType::unknown);
                SG_ASSERT(GrammarConstruct::Alias == m_stack.back().grammarConstructs.back());
                SemanticNode& node = m_stack.back().semanticNodes.back();
                semanticTree::TypedefDeclaration* treeNode = checked_cast<semanticTree::TypedefDeclaration*>(node.treeNode.get());
                SG_ASSERT(node.subTreeNodes.size() == 1);
                treeNode->SetType(node.subTreeNodes[0].get());
                node.subTreeNodes.clear();
                m_stack.back().grammarConstructs.pop_back();
            }
            else
            {
                if(TokenType::identifier != iToken.type && TokenType::operator_double_colon != iToken.type && iIsValueExpected)
                {
                    PushError(ErrorType::expected_type_for_alias_type, iToken, "alias expects a type");
                    return true;
                }
            }
            break;
        }
        default:
            SG_ASSERT_NOT_REACHED();
        }
    } while(poped && !m_stack.back().grammarConstructs.empty());

    return false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool GrammarAnalyser::IsValueExpected() const
{
    if(m_stack.back().semanticNodes.empty())
    {
        return true;
    }
    else
    {
        size_t const prevOpIndex = m_stack.back().semanticNodes.back().operatorTraitsIndexIfIncomplete;
        if(all_ones != prevOpIndex
            && (
            operatorTraits[prevOpIndex].arrity == Arrity::Binary
            || operatorTraits[prevOpIndex].arrity == Arrity::BinaryEnableTrailing
            || operatorTraits[prevOpIndex].arrity == Arrity::PrefixUnary
            || operatorTraits[prevOpIndex].arrity == Arrity::Ternary))
        {
            return true;
        }
    }
    return false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void GrammarAnalyser::ProcessToken(Token const& iToken, bool hasSeparatorBefore, bool hasSeparatorAfter)
{
    if(!m_stack.back().grammarConstructs.empty())
    {
        bool done = ProcessGrammarConstructIFNReturnDone(iToken, IsValueExpected());
        if(done || DidErrorHappen())
            return;
    }

    switch(iToken.type)
    {
    case TokenType::invalid:
        SG_ASSERT_MSG(m_errorHandler->DidErrorHappen(), "Error should have already been rised by Tokenizer");
        break;
    case TokenType::identifier:
        SolveOnValueIFN(iToken);
        ProcessIdentifier(iToken);
        break;
    case TokenType::hexadecimal:
        {
            SolveOnValueIFN(iToken);
            SG_ASSERT(iToken.begin+2 < iToken.end);
            SG_ASSERT('0' == *(iToken.begin) && 'x' == *(iToken.begin+1));
            SG_ASSERT(0 == errno);
            errno = 0;
            unsigned long const value = strtoul(std::string(iToken.begin+2, iToken.end - (iToken.begin+2)).c_str(), nullptr, 16);
            if(0 != errno)
            {
                if(ERANGE == errno)
                {
                    PushError(ErrorType::integer_value_too_big, iToken, "integer value is too big to be represented");
                    errno = 0;
                    return;
                }
                else
                    SG_ASSERT_NOT_REACHED();
                errno = 0;
            }
            u32 const value32 = checked_numcastable(value);
            m_stack.back().semanticNodes.emplace_back(new semanticTree::Value(new reflection::PrimitiveData<i32>(value32)));
            m_stack.back().semanticNodes.back().treeNode->SetToken(iToken);
        }
        break;
    case TokenType::integer:
        {
            SolveOnValueIFN(iToken);
            if('0' == *(iToken.begin) && iToken.begin+1 != iToken.end)
            {
                PushError(ErrorType::octal_format_is_forbidden, iToken, "You don't want me to read octal numbers, do you ?!\n(Numbers begining by 0 are forbidden to remove ambiguity with unsupported octal notation)");
                m_stack.back().semanticNodes.emplace_back(new semanticTree::Value(new reflection::PrimitiveData<i32>(0)));
            }
            else
            {
                SG_ASSERT(0 == errno);
                errno = 0;
                unsigned long const value = strtoul(std::string(iToken.begin, iToken.end - iToken.begin).c_str(), nullptr, 10);
                if(0 != errno)
                {
                    if(ERANGE == errno)
                    {
                        PushError(ErrorType::integer_value_too_big, iToken, "integer value is too big to be represented");
                        errno = 0;
                        return;
                    }
                    else
                        SG_ASSERT_NOT_REACHED();
                    errno = 0;
                }
                u32 const value32 = checked_numcastable(value);
                m_stack.back().semanticNodes.emplace_back(new semanticTree::Value(new reflection::PrimitiveData<i32>(value32)));
            }
            m_stack.back().semanticNodes.back().treeNode->SetToken(iToken);
        }
        break;
    case TokenType::number:
        {
            SolveOnValueIFN(iToken);
            float value = (float)atof(std::string(iToken.begin, iToken.end - iToken.begin).c_str());
            m_stack.back().semanticNodes.emplace_back(new semanticTree::Value(new reflection::PrimitiveData<float>(value)));
            m_stack.back().semanticNodes.back().treeNode->SetToken(iToken);
        }
        break;
    case TokenType::string:
        {
            SolveOnValueIFN(iToken);
            SG_ASSERT('\"' == *iToken.begin);
            SG_ASSERT('\"' == *(iToken.end-1));
            std::string value(iToken.begin+1, iToken.end - iToken.begin -2);
            m_stack.back().semanticNodes.emplace_back(new semanticTree::Value(new reflection::PrimitiveData<std::string>(value)));
            m_stack.back().semanticNodes.back().treeNode->SetToken(iToken);
        }
        break;
    case TokenType::open_bloc:
        {
            bool const isSuffix = !IsValueExpected();
            ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::open_bloc>::index(isSuffix ? Arrity::SuffixUnary : Arrity::Nullary));
            PushStackNode(iToken.type);
        }
        break;
    case TokenType::close_bloc:
        PopStackNode(iToken, iToken.type);
        break;
    case TokenType::open_parenthesis:
        {
            bool const isSuffix = !IsValueExpected();
            ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::open_parenthesis>::index(isSuffix ? Arrity::SuffixUnary : Arrity::Nullary));
            PushStackNode(iToken.type);
        }
        break;
    case TokenType::close_parenthesis:
        PopStackNode(iToken, iToken.type);
        break;
    case TokenType::open_bracket:
        {
            bool const isSuffix = !IsValueExpected();
            ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::open_bracket>::index(isSuffix ? Arrity::SuffixUnary : Arrity::Nullary));
            PushStackNode(iToken.type);
        }
        break;
    case TokenType::close_bracket:
        PopStackNode(iToken, iToken.type);
        break;
    case TokenType::operator_colon:
        {
            if(TokenType::operator_colon == m_stack.back().expectedClosingToken)
                PopStackNode(iToken, iToken.type);
            else
                ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_colon>::index());
            break;
        }
    case TokenType::operator_semicolon:
        PushError(ErrorType::incorrect_use_of_semicolon, iToken, "incorrect use of semicolon");
        break;
    case TokenType::operator_double_colon:      ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_double_colon>::index(hasSeparatorBefore, hasSeparatorAfter)); break;
    case TokenType::operator_comma:             ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_comma>::index()); break;
    case TokenType::operator_equal:             ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_equal>::index()); break;
    case TokenType::operator_plus:
        {
            bool const isPrefix = IsValueExpected();
            ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_plus>::index(isPrefix ? Arrity::PrefixUnary : Arrity::Binary));
        }
        break;
    case TokenType::operator_minus:
        {
            bool const isPrefix = IsValueExpected();
            ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_minus>::index(isPrefix ? Arrity::PrefixUnary : Arrity::Binary));
        }
        break;
    case TokenType::operator_multiply:          ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_multiply>::index()); break;
    case TokenType::operator_divide:            ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_divide>::index()); break;
    case TokenType::operator_modulo:            ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_modulo>::index()); break;
    case TokenType::operator_plus_equal:        ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_plus_equal>::index()); break;
    case TokenType::operator_minus_equal:       ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_minus_equal>::index()); break;
    case TokenType::operator_multiply_equal:    ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_multiply_equal>::index()); break;
    case TokenType::operator_divide_equal:      ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_divide_equal>::index()); break;
    case TokenType::operator_modulo_equal:      ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_modulo_equal>::index()); break;
    case TokenType::operator_plus_plus:         ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_plus_plus>::index(hasSeparatorBefore, hasSeparatorAfter)); break;
    case TokenType::operator_minus_minus:       ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_minus_minus>::index(hasSeparatorBefore, hasSeparatorAfter)); break;
    case TokenType::operator_not:               ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_not>::index()); break;
    case TokenType::operator_bitand:            ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_bitand>::index()); break;
    case TokenType::operator_bitor:             ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_bitor>::index()); break;
    case TokenType::operator_bitxor:            ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_bitxor>::index()); break;
    case TokenType::operator_bitand_equal:      ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_bitand_equal>::index()); break;
    case TokenType::operator_bitor_equal:       ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_bitor_equal>::index()); break;
    case TokenType::operator_bitxor_equal:      ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_bitxor_equal>::index()); break;
    case TokenType::operator_bitnot:            ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_bitnot>::index()); break;
    case TokenType::operator_and:               ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_and>::index()); break;
    case TokenType::operator_or:                ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_or>::index()); break;
    case TokenType::operator_equal_equal:       ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_equal_equal>::index()); break;
    case TokenType::operator_not_equal:         ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_not_equal>::index()); break;
    case TokenType::operator_less:              ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_less>::index()); break;
    case TokenType::operator_greater:           ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_greater>::index()); break;
    case TokenType::operator_less_equal:        ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_less_equal>::index()); break;
    case TokenType::operator_greater_equal:     ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_greater_equal>::index()); break;
    case TokenType::operator_shift_left:        ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_shift_left>::index()); break;
    case TokenType::operator_shift_right:       ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_shift_right>::index()); break;
    case TokenType::operator_shift_left_equal:  ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_shift_left_equal>::index()); break;
    case TokenType::operator_shift_right_equal: ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_shift_right_equal>::index()); break;
    case TokenType::operator_interrogation:
        ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_interrogation>::index());
        PushStackNode(iToken.type);
        break;
    case TokenType::operator_dot:               ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_dot>::index()); break;
    case TokenType::operator_dollar:            ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::operator_dollar>::index()); break;
    case TokenType::keyword_do:
        {
            ForceSolve(iToken);
            semanticTree::While* w = new semanticTree::While();
            w->SetAlwaysPerformFirstIteration(true);
            m_stack.back().semanticNodes.emplace_back(w);
            m_stack.back().semanticNodes.back().treeNode->SetToken(iToken);
            m_stack.back().grammarConstructs.push_back(GrammarConstruct::DoWhile);
        }
        break;
    case TokenType::keyword_if:
        ForceSolve(iToken);
        m_stack.back().semanticNodes.emplace_back(new semanticTree::If());
        m_stack.back().semanticNodes.back().treeNode->SetToken(iToken);
        m_stack.back().grammarConstructs.push_back(GrammarConstruct::If);
        break;
    case TokenType::keyword_is: ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::keyword_is>::index()); break;
    case TokenType::keyword_in:
        PushError(ErrorType::incorrect_use_of_keyword_in, iToken, "incorrect use of keyword in");
        break;
    case TokenType::keyword_for:
        ForceSolve(iToken);
        m_stack.back().semanticNodes.emplace_back(new semanticTree::For());
        m_stack.back().semanticNodes.back().treeNode->SetToken(iToken);
        m_stack.back().grammarConstructs.push_back(GrammarConstruct::For);
        break;
    case TokenType::keyword_var:
        ForceSolve(iToken);
        ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::keyword_var>::index());
        break;
    case TokenType::keyword_else:
        PushError(ErrorType::incorrect_use_of_keyword_else, iToken, "incorrect use of keyword else. Should directly follow an if insruction block");
        break;
    case TokenType::keyword_null:
        SolveOnValueIFN(iToken);
        m_stack.back().semanticNodes.emplace_back(new semanticTree::Value(new reflection::PrimitiveData<nullptr_t>(nullptr)));
        m_stack.back().semanticNodes.back().treeNode->SetToken(iToken);
        break;
    case TokenType::keyword_true:
        SolveOnValueIFN(iToken);
        m_stack.back().semanticNodes.emplace_back(new semanticTree::Value(new reflection::PrimitiveData<bool>(true)));
        m_stack.back().semanticNodes.back().treeNode->SetToken(iToken);
        break;
    case TokenType::keyword_alias:
        ForceSolve(iToken);
        m_stack.back().semanticNodes.emplace_back(new semanticTree::TypedefDeclaration());
        m_stack.back().semanticNodes.back().treeNode->SetToken(iToken);
        m_stack.back().grammarConstructs.push_back(GrammarConstruct::Alias);
        PushStackNode(TokenType::unknown, GrammarConstruct::Alias_Name);
        break;
    case TokenType::keyword_break:
        ForceSolve(iToken);
        m_stack.back().semanticNodes.emplace_back(new semanticTree::Break());
        m_stack.back().semanticNodes.back().treeNode->SetToken(iToken);
        break;
    case TokenType::keyword_const:
        ForceSolve(iToken);
        ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::keyword_const>::index());
        break;
    case TokenType::keyword_false:
        SolveOnValueIFN(iToken);
        m_stack.back().semanticNodes.emplace_back(new semanticTree::Value(new reflection::PrimitiveData<bool>(false)));
        m_stack.back().semanticNodes.back().treeNode->SetToken(iToken);
        break;
    case TokenType::keyword_typedef:
#if SG_OBJECTSCRIPT_TYPEDEF_IS_DEPRECATED
        PushError(ErrorType::typedef_is_deprecated, iToken, "typedef is deprecated. Please use alias instead.");
#endif
        ForceSolve(iToken);
        m_stack.back().semanticNodes.emplace_back(new semanticTree::TypedefDeclaration());
        m_stack.back().semanticNodes.back().treeNode->SetToken(iToken);
        m_stack.back().grammarConstructs.push_back(GrammarConstruct::Typedef);
        PushStackNode(TokenType::unknown, GrammarConstruct::Typedef_BeforeType);
        break;
    case TokenType::keyword_while:
        ForceSolve(iToken);
        m_stack.back().semanticNodes.emplace_back(new semanticTree::While());
        m_stack.back().semanticNodes.back().treeNode->SetToken(iToken);
        m_stack.back().grammarConstructs.push_back(GrammarConstruct::While);
        break;
    case TokenType::keyword_assert:
        SolveOnValueIFN(iToken);
        ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::keyword_assert>::index());
        break;
    case TokenType::keyword_export:
        SolveOnValueIFN(iToken);
        ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::keyword_export>::index());
        break;
    case TokenType::keyword_import:
        SolveOnValueIFN(iToken);
        ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::keyword_import>::index());
        break;
    case TokenType::keyword_public:
        SolveOnValueIFN(iToken);
        ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::keyword_public>::index());
        break;
    case TokenType::keyword_return:
        ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::keyword_return>::index());
        break;
    case TokenType::keyword_private:
        SolveOnValueIFN(iToken);
        ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::keyword_private>::index());
        break;
    case TokenType::keyword_continue:
        ForceSolve(iToken);
        m_stack.back().semanticNodes.emplace_back(new semanticTree::Continue());
        m_stack.back().semanticNodes.back().treeNode->SetToken(iToken);
        break;
    case TokenType::keyword_function:
        ForceSolve(iToken);
        m_stack.back().semanticNodes.emplace_back(new semanticTree::FunctionDeclaration());
        m_stack.back().semanticNodes.back().treeNode->SetToken(iToken);
        m_stack.back().grammarConstructs.push_back(GrammarConstruct::Function);
        PushStackNode(TokenType::unknown, GrammarConstruct::Function_Name);
        break;
    case TokenType::keyword_template:
        ForceSolve(iToken);
        m_stack.back().semanticNodes.emplace_back(new semanticTree::TemplateDeclaration());
        m_stack.back().semanticNodes.back().treeNode->SetToken(iToken);
        m_stack.back().grammarConstructs.push_back(GrammarConstruct::Template);
        PushStackNode(TokenType::unknown, GrammarConstruct::Template_Name);
        break;
    case TokenType::keyword_intrinsic:
        SolveOnValueIFN(iToken);
        m_stack.back().semanticNodes.emplace_back(new semanticTree::IntrinsicKeyword());
        m_stack.back().semanticNodes.back().treeNode->SetToken(iToken);
        break;
    case TokenType::keyword_namespace:
        ForceSolve(iToken);
        m_stack.back().semanticNodes.emplace_back(new semanticTree::Namespace());
        m_stack.back().semanticNodes.back().treeNode->SetToken(iToken);
        PushStackNode(TokenType::unknown);
        m_stack.back().grammarConstructs.push_back(GrammarConstruct::Namespace_Name);
        break;
    case TokenType::keyword_protected:
        SolveOnValueIFN(iToken);
        ProcessOperator(iToken, OperatorTraitsIndexGetter<TokenType::keyword_protected>::index());
        break;
    case TokenType::eof:
        ForceSolve(iToken);
        if(m_stack.size() > 1)
        {
            switch(m_stack.back().expectedClosingToken)
            {
            case TokenType::close_bloc:
                PushError(ErrorType::unexpected_end_of_file_in_bloc, iToken, "unexpected end of file in bloc, a closing brace was expected");
                return;
            case TokenType::close_bracket:
                PushError(ErrorType::unexpected_end_of_file_in_bracket, iToken, "unexpected end of file, a closing bracket was expected.");
                return;
            case TokenType::close_parenthesis:
                PushError(ErrorType::unexpected_end_of_file_in_parenthesis, iToken, "unexpected end of file, a closing parenthesis was expected.");
                return;
            case TokenType::unknown:
                PushError(ErrorType::unexpected_end_of_file_in_construct, iToken, "unexpected end of file in grammar construct");
                return;
            case TokenType::operator_colon:
                PushError(ErrorType::unexpected_end_of_file_in_ternary_op, iToken, "unexpected end of file in ternary operator, a colon was expected");
                return;
            default:
                SG_ASSERT_NOT_REACHED();
            }
        }
        break;
    default:
        SG_ASSERT_NOT_REACHED();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void GrammarAnalyser::ProcessIdentifier(Token const& iToken)
{
    SG_ASSERT(TokenType::identifier == iToken.type);
    std::string identifier_str(iToken.begin, iToken.end - iToken.begin);
    semanticTree::Identifier* identifier = new semanticTree::Identifier(identifier_str);
    m_stack.back().semanticNodes.emplace_back(identifier);
    m_stack.back().semanticNodes.back().treeNode->SetToken(iToken);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void GrammarAnalyser::ProcessOperator(Token const& iToken, size_t iOperatorIndex)
{
    SG_ASSERT(iToken.type == operatorTraits[iOperatorIndex].op);
    SG_ASSERT(GetOperatorTraitsIndex(iToken.type, operatorTraits[iOperatorIndex].arrity) == iOperatorIndex);
    Solve(iToken, iOperatorIndex);

    //SemanticNode& node = m_stack.back().semanticNodes.back()
    OperatorTraits const& optraits = operatorTraits[iOperatorIndex];
        SG_ASSERT(nullptr != optraits.createTreeNode);
    semanticTree::ITreeNode* newnode = optraits.createTreeNode(optraits);
    newnode->SetToken(iToken);

    m_stack.back().semanticNodes.emplace_back(newnode, iOperatorIndex);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void GrammarAnalyser::PushStackNode(TokenType iTokenType, GrammarConstruct iGrammarConstruct)
{
    m_stack.emplace_back();
    switch(iTokenType)
    {
    case TokenType::unknown:
        m_stack.back().expectedClosingToken = TokenType::unknown;
        break;
    case TokenType::open_bloc:
        m_stack.back().expectedClosingToken = TokenType::close_bloc;
        break;
    case TokenType::open_parenthesis:
        m_stack.back().expectedClosingToken = TokenType::close_parenthesis;
        break;
    case TokenType::open_bracket:
        m_stack.back().expectedClosingToken = TokenType::close_bracket;
        break;
    case TokenType::operator_interrogation:
        m_stack.back().expectedClosingToken = TokenType::operator_colon;
        break;
    default:
        SG_ASSERT_NOT_REACHED();
    }
    if(GrammarConstruct::None != iGrammarConstruct)
        m_stack.back().grammarConstructs.push_back(iGrammarConstruct);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void GrammarAnalyser::PopStackNode(Token const& iToken, TokenType iTokenType)
{
    SG_ASSERT_AND_UNUSED(iTokenType == m_stack.back().expectedClosingToken); // TODO: error message
    ForceSolve(iToken);
    std::vector<refptr<semanticTree::ITreeNode> > subNodes;
    {
        StackNode& stackNode = m_stack.back();
        size_t const subNodeCount = stackNode.semanticNodes.size();
        subNodes.reserve(subNodeCount);
        auto const begin = stackNode.semanticNodes.begin();
        auto const end = stackNode.semanticNodes.end();
        for(auto it = begin; it != end; ++it)
        {
            SG_ASSERT(it->subTreeNodes.empty());
            SG_ASSERT(-1 == it->operatorTraitsIndexIfIncomplete);
            subNodes.push_back(it->treeNode.get());
        }
    }
    SG_ASSERT(m_stack.back().grammarConstructs.size() <= 1);
    SG_CODE_FOR_ASSERT(bool const inGrammarConstruct = !m_stack.back().grammarConstructs.empty());
    m_stack.pop_back();
    SG_ASSERT(!m_stack.empty());
    SG_ASSERT(m_stack.back().semanticNodes.back().subTreeNodes.empty());
    SG_ASSERT(-1 != m_stack.back().semanticNodes.back().operatorTraitsIndexIfIncomplete || inGrammarConstruct);
    using std::swap;
    swap(m_stack.back().semanticNodes.back().subTreeNodes, subNodes);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void GrammarAnalyser::SolveOnValueIFN(Token const& iToken)
{
    if(!IsValueExpected())
        ForceSolve(iToken);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void GrammarAnalyser::ForceSolve(Token const& iToken)
{
    Solve(iToken, operatorCount-1);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void GrammarAnalyser::Solve(Token const& iToken, size_t iLastOperatorIndex)
{
    StackNode& stackNode = m_stack.back();
    if(stackNode.semanticNodes.empty())
        return;
    SG_ASSERT(iLastOperatorIndex < operatorCount);
    if(Arrity::PrefixUnary == operatorTraits[iLastOperatorIndex].arrity)
    {
        SemanticNode const& leftNode = stackNode.semanticNodes.back();
        bool const leftIsAValue = -1 == leftNode.operatorTraitsIndexIfIncomplete;
        bool const leftIsASuffixOperator = Arrity::SuffixUnary == operatorTraits[leftNode.operatorTraitsIndexIfIncomplete].arrity;
        bool const forceSolve = leftIsAValue || leftIsASuffixOperator;
        if(forceSolve)
            iLastOperatorIndex = operatorCount-1;
    }
    OperatorTraits const& lastoptraits = operatorTraits[iLastOperatorIndex];
    size_t const lastprecedence = lastoptraits.precedence;
    size_t i = stackNode.semanticNodes.size();
    while(i != 0)
    {
        --i;
        SemanticNode& node = stackNode.semanticNodes[i];
        if(all_ones != node.operatorTraitsIndexIfIncomplete)
        {
            OperatorTraits const& optraits = operatorTraits[node.operatorTraitsIndexIfIncomplete];
            if(optraits.precedence > lastprecedence || (optraits.precedence == lastprecedence && optraits.associativity == Associativity::RightToLeft))
                return;
            SG_ASSERT(optraits.precedence < lastprecedence || lastoptraits.associativity == Associativity::LeftToRight);
            semanticTree::ITreeNode* treenode = node.treeNode.get();
            if(!node.subTreeNodes.empty())
            {
                treenode->SetSubNodes(node.subTreeNodes);
                node.subTreeNodes.clear();
            }
            node.operatorTraitsIndexIfIncomplete = all_ones;
            switch(optraits.arrity)
            {
            case Arrity::Nullary:
                break;
            case Arrity::PrefixUnary:
                SG_ASSERT(node.subTreeNodes.empty());
                {
                    if(stackNode.semanticNodes.size() < i+2)
                    {
                        PushError(ErrorType::missing_term_after_prefix_operator, node.treeNode->GetToken(), "missing term after prefix operator");
                        return;
                    }
                    SG_ASSERT(i+2 == stackNode.semanticNodes.size());
                    SemanticNode& right = stackNode.semanticNodes[i+1];
                    SG_ASSERT(-1 == right.operatorTraitsIndexIfIncomplete);
                    SG_ASSERT(right.treeNode->IsValue());
                    SG_ASSERT(right.subTreeNodes.empty());
                    treenode->SetArgument(0, right.treeNode.get());
                    stackNode.semanticNodes.pop_back();
                }
                break;
            case Arrity::SuffixUnary:
                SG_ASSERT(node.subTreeNodes.empty());
                {
                    SG_ASSERT(i+1 == stackNode.semanticNodes.size());
                    SG_ASSERT(i > 0);
                    SemanticNode& left = stackNode.semanticNodes[i-1];
                    SG_ASSERT(-1 == left.operatorTraitsIndexIfIncomplete);
                    SG_ASSERT(left.treeNode->IsValue());
                    SG_ASSERT(left.subTreeNodes.empty());
                    treenode->SetArgument(0, left.treeNode.get());
                    using std::swap;
                    swap(stackNode.semanticNodes[i-1], stackNode.semanticNodes[i]);
                    --i;
                    stackNode.semanticNodes.pop_back();
                }
                break;
            case Arrity::Binary:
            case Arrity::BinaryEnableTrailing:
            case Arrity::Ternary:
                // Note: For ternary operator, center arguments comes from sub stack node.
                SG_ASSERT(node.subTreeNodes.empty());
                {
                    SG_ASSERT(i > 0);
                    SemanticNode& left = stackNode.semanticNodes[i-1];
                    SG_ASSERT(-1 == left.operatorTraitsIndexIfIncomplete);
                    SG_ASSERT(left.treeNode->IsValue());
                    SG_ASSERT(left.subTreeNodes.empty());
                    treenode->SetArgument(0, left.treeNode.get());
                }
                if(Arrity::BinaryEnableTrailing == optraits.arrity)
                {
                    if(i+1 == stackNode.semanticNodes.size())
                    {
                        // trailing comma
                        treenode->SetArgument(1, nullptr);
                        using std::swap;
                        swap(stackNode.semanticNodes[i-1], stackNode.semanticNodes[i]);
                        --i;
                        stackNode.semanticNodes.pop_back();
                        break;
                    }
                }
                if(i+1 == stackNode.semanticNodes.size())
                {
                    if(TokenType::eof == iToken.type)
                        PushError(ErrorType::unexpected_end_of_file_value_expected, iToken, "unexpected end of file, value was expected!");
                    else
                        PushError(ErrorType::value_expected, iToken, "unexpected token, value was expected!");
                    treenode->SetArgument(1, new semanticTree::Value());
                    return;
                }
                {
                    SG_ASSERT(i+2 == stackNode.semanticNodes.size());
                    SemanticNode& right = stackNode.semanticNodes[i+1];
                    SG_ASSERT(-1 == right.operatorTraitsIndexIfIncomplete);
                    SG_ASSERT(right.treeNode->IsValue());
                    SG_ASSERT(right.subTreeNodes.empty());
                    treenode->SetArgument(1, right.treeNode.get());

                    using std::swap;
                    swap(stackNode.semanticNodes[i-1], stackNode.semanticNodes[i]);
                    --i;
                    stackNode.semanticNodes.pop_back();
                    stackNode.semanticNodes.pop_back();
                }
                break;
            default:
                SG_ASSERT_NOT_REACHED();
            }
            SG_ASSERT(i+1 == stackNode.semanticNodes.size());
        }
    }
}
//=============================================================================
}
}

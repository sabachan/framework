#ifndef ObjectScript_GrammarAnalyser_H
#define ObjectScript_GrammarAnalyser_H

#include "Tokenizer.h"
#include <list>
#include <vector>

namespace sg {
namespace reflection {
//=============================================================================
class IPrimitiveData;
//=============================================================================
}
}

namespace sg {
namespace objectscript {
//=============================================================================
namespace semanticTree {
class ITreeNode;
class Root;
}
//=============================================================================
struct Token;
//=============================================================================
enum class GrammarConstruct
{
    None,
    Namespace, Namespace_Name, Namespace_Bloc,
    If, If_Expr, If_PreInstruction, If_Instruction, If_InstructionBloc, If_PreElse, If_ElsePreInstruction, If_ElseInstruction, If_ElseInstructionBloc,
    For, For_Expr, For_PreInstruction, For_Instruction, For_InstructionBloc,
    While, While_Expr, While_PreInstruction, While_Instruction, While_InstructionBloc,
    DoWhile, DoWhile_Instruction, DoWhile_InstructionBloc, DoWhile_While, DoWhile_PreInstruction, DoWhile_Expr,
    Function, Function_Name, Function_Args, Function_ArgDefault, Function_Prototype, Function_Body,
    Template, Template_Name, Template_Args, Template_ArgDefault, Template_Prototype, Template_Type, Template_Body,
    Typedef, Typedef_BeforeType, Typedef_Type, Typedef_Alias,
    Alias, Alias_Name, Alias_Is, Alias_Type,
    Import,
    Include,
};
//=============================================================================
class GrammarAnalyser
{
    SG_NON_COPYABLE(GrammarAnalyser)
public:
    GrammarAnalyser(char const* str, IErrorHandler* iErrorHandler);
    ~GrammarAnalyser();
    semanticTree::Root* Run();
private:
    bool ProcessGrammarConstructIFNReturnDone(Token const& iToken, bool iIsValueExpected);
    void ProcessToken(Token const& iToken, bool hasSeparatorBefore, bool hasSeparatorAfter);
    bool IsValueExpected() const;
    void PushStackNode(TokenType iTokenType, GrammarConstruct iGrammarConstruct = GrammarConstruct::None);
    void PopStackNode(Token const& iToken, TokenType iTokenType);
    void ProcessOperator(Token const& iToken, size_t iOperatorIndex);
    void ProcessKeyword(Token const& iToken);
    void ProcessIdentifier(Token const& iToken);
    void ProcessLiteral(Token const& iToken);
    void ProcessEof(Token const& iToken);
    void SolveOnValueIFN(Token const& iToken);
    void Solve(Token const& iToken, size_t iLastOperatorIndex);
    void ForceSolve(Token const& iToken);
    void PushError(ErrorType iErrorType, Token const& iToken, char const* msg);
    bool DidErrorHappen() const { return m_errorHandler->DidErrorHappen(); }
private:
    struct TokenEx
    {
        Token token;
        size_t operatorTraitsIndex;
        size_t containerNode;
    public:
        TokenEx(Token const& iToken) : token(iToken), operatorTraitsIndex(all_ones), containerNode(all_ones) {}
        TokenEx(Token const& iToken, size_t iOperatorTraitsIndex) : token(iToken), operatorTraitsIndex(iOperatorTraitsIndex), containerNode(all_ones) {}
    };
    struct SemanticNode
    {
        refptr<semanticTree::ITreeNode> treeNode;
        size_t operatorTraitsIndexIfIncomplete;
        std::vector<refptr<semanticTree::ITreeNode> > subTreeNodes;
    public:
        explicit SemanticNode(semanticTree::ITreeNode* iTreeNode);
        SemanticNode(semanticTree::ITreeNode* iTreeNode, size_t iOperatorIndex);
    };
    struct StackNode
    {
        std::vector<SemanticNode> semanticNodes;
        TokenType expectedClosingToken;
        std::vector<GrammarConstruct> grammarConstructs;
    public:
        StackNode();
        ~StackNode();
    };
private:
    Tokenizer m_tokenizer;
    std::vector<StackNode> m_stack;
    safeptr<IErrorHandler> m_errorHandler;
};
//=============================================================================
}
}
#endif

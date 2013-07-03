#ifndef ObjectScript_SemanticTree_H
#define ObjectScript_SemanticTree_H

#include <Core/ArrayView.h>
#include <Core/Cast.h>
#include <Core/Config.h>
#include <Reflection/BaseClass.h>
#include <Reflection/Identifier.h>
#include <Reflection/ObjectDatabase.h>
#include "ImportDatabase.h"
#include "Tokenizer.h"


namespace sg {
namespace objectscript {
struct OperatorTraits;
}
}

namespace sg {
namespace objectscript {
namespace semanticTree {
//=============================================================================
class BreakingInstruction;
class ITreeNode;
//=============================================================================
enum class Construction { None, Object, Struct };
//=============================================================================
struct EvaluationInputOutput
{
public:
    // inputs, heritable by default
    safeptr<ImportDatabase> importDatabase;
    safeptr<Imported> imported;
    safeptr<IErrorHandler> errorHandler;
    safeptr<reflection::ObjectDatabase> objectDatabase;
    safeptr<reflection::ObjectDatabase> scriptDatabase;
    safeptr<reflection::Identifier const> pathForObjects;
    safeptr<reflection::Identifier const> pathForScript;
    safeptr<reflection::Identifier const> restrictedVariableScope;
    // input, non-heritable
    safeptr<reflection::Identifier const> objectIdentifier;
    Construction constructionInProgress;
    // input/output
    refptr<reflection::IPrimitiveData> returnValue;
    // output
    refptr<reflection::IPrimitiveData>* returnLValueReference;
    std::vector<TokenType> qualifiers;
    safeptr<BreakingInstruction> breakingInstruction;
    reflection::Identifier returnIdentifier;
    refptr<ITreeNode> presesolvedNodeIFN;

    // inputs, heritable by default
    bool isPreresolutionPass : 8;
    bool enableComma : 8;
    bool enableImports : 8;
    bool enableScriptDefinitions : 8;
    bool enableNonConstVariableDefinitions : 8;
    bool enableObjectDefinitions : 8;
    bool enableObjectReferences : 8;
    bool enableNamespaceDefinition : 8;
    bool enableVariableRead : 8;
    bool variablesAreRestricted : 8;
    // output
    bool returnValueIsLValue : 8;
    bool returnValueIsCommaSeparatedList : 8;
    bool returnValueContainsUnresolvedIdentifier : 8;
public:
    EvaluationInputOutput(reflection::ObjectDatabase* iObjectDatabase,
                          reflection::ObjectDatabase* iScriptDatabase,
                          ImportDatabase* iImportDatabase,
                          Imported* iImported,
                          IErrorHandler* iErrorHandler)
        : importDatabase(iImportDatabase)
        , imported(iImported)
        , errorHandler(iErrorHandler)
        , objectDatabase(iObjectDatabase)
        , scriptDatabase(iScriptDatabase)
        , pathForObjects()
        , pathForScript()
        , restrictedVariableScope()
        , objectIdentifier()
        , constructionInProgress(Construction::None)
        , returnValue(nullptr)
        , returnLValueReference(nullptr)
        , qualifiers()
        , breakingInstruction(nullptr)
        , returnIdentifier()
        , presesolvedNodeIFN()
        , isPreresolutionPass(false)
        , enableComma(false)
        , enableImports(true)
        , enableScriptDefinitions(true)
        , enableNonConstVariableDefinitions(true)
        , enableObjectDefinitions(true)
        , enableObjectReferences(true)
        , enableNamespaceDefinition(true)
        , enableVariableRead(true)
        , variablesAreRestricted(false)
        , returnValueIsLValue(false)
        , returnValueIsCommaSeparatedList(false)
        , returnValueContainsUnresolvedIdentifier(false)
    {
    }
    explicit EvaluationInputOutput(EvaluationInputOutput const& parentio)
        : importDatabase(parentio.importDatabase)
        , imported(parentio.imported)
        , errorHandler(parentio.errorHandler)
        , objectDatabase(parentio.objectDatabase)
        , scriptDatabase(parentio.scriptDatabase)
        , pathForObjects(parentio.pathForObjects)
        , pathForScript(parentio.pathForScript)
        , restrictedVariableScope(parentio.restrictedVariableScope)
        , objectIdentifier()
        , constructionInProgress(Construction::None)
        , returnValue()
        , returnLValueReference(nullptr)
        , qualifiers()
        , breakingInstruction()
        , returnIdentifier()
        , presesolvedNodeIFN()
        , isPreresolutionPass(parentio.isPreresolutionPass)
        , enableComma(parentio.enableComma)
        , enableImports(parentio.enableImports)
        , enableScriptDefinitions(parentio.enableScriptDefinitions)
        , enableNonConstVariableDefinitions(parentio.enableNonConstVariableDefinitions)
        , enableObjectDefinitions(parentio.enableObjectDefinitions)
        , enableObjectReferences(parentio.enableObjectReferences)
        , enableNamespaceDefinition(parentio.enableNamespaceDefinition)
        , enableVariableRead(parentio.enableVariableRead)
        , variablesAreRestricted(parentio.variablesAreRestricted)
        , returnValueIsLValue(false)
        , returnValueIsCommaSeparatedList(false)
        , returnValueContainsUnresolvedIdentifier(false)
    {
        SG_ASSERT(nullptr == parentio.breakingInstruction);
    }
    ~EvaluationInputOutput()
    {
        SG_ASSERT_MSG(nullptr == breakingInstruction, "A breaking instruction has not been treated!");
    }
    void ForwardBreakingInstruction(EvaluationInputOutput& io)
    {
        SG_ASSERT(nullptr == io.returnValue && io.returnIdentifier.Empty());
        io.breakingInstruction = breakingInstruction;
        io.returnIdentifier = returnIdentifier;
        io.returnValue = returnValue;
        io.returnLValueReference = returnLValueReference;
        io.returnValueContainsUnresolvedIdentifier = returnValueContainsUnresolvedIdentifier;
        io.returnValueIsCommaSeparatedList = returnValueIsCommaSeparatedList;
        io.returnValueIsLValue = returnValueIsLValue;
        SG_ASSERT(qualifiers.empty());
        SG_ASSERT(nullptr == objectIdentifier);
        breakingInstruction = nullptr;
    }
};
//=============================================================================
class ITreeNode : public RefAndSafeCountable
{
public:
    ITreeNode() : m_token() {}
    virtual ~ITreeNode() {}
    virtual void SetArgument(size_t i, ITreeNode* iArg) { SG_UNUSED((i, iArg)); SG_ASSERT_NOT_REACHED(); } // = 0;
    virtual void SetSubNodes(std::vector<refptr<ITreeNode> >& iSubNodesCanBeCleared) { SG_UNUSED(iSubNodesCanBeCleared); SG_ASSERT_NOT_REACHED(); } // = 0;
    virtual bool EvaluateROK(EvaluationInputOutput& io) = 0; // { SG_ASSERT_NOT_REACHED(); } // = 0; // TODO : return IPrimitiveData
    virtual bool IsValue() const = 0;
    virtual bool IsImport() const { return false; }
    virtual bool IsInstruction() const { return false; }
    void SetToken(Token const& iToken) { m_token = iToken; }
    Token const& GetToken() const { return m_token; }
private:
    Token m_token;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class Root : public ITreeNode
{
public:
    void SetInstructions(std::vector<refptr<ITreeNode> >& iInstructions) { SG_ASSERT(m_instructions.empty()); using std::swap; swap(iInstructions, m_instructions); }
    virtual bool IsValue() const override { return false; }
    bool EvaluateScriptROK(EvaluationInputOutput& io);
    bool EvaluateImportROK(EvaluationInputOutput& io);
private:
    bool EvaluateImplROK(EvaluationInputOutput& io);
    virtual bool EvaluateROK(EvaluationInputOutput& io) override { SG_UNUSED(io); SG_ASSERT_NOT_REACHED(); return false; }
private:
    std::vector<refptr<ITreeNode> > m_instructions;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class Import : public ITreeNode
{
public:
    virtual void SetArgument(size_t i, ITreeNode* iArg) override { SG_ASSERT_AND_UNUSED(0 == i); SG_ASSERT(nullptr == m_filename); m_filename = iArg; }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return false; }
    virtual bool IsImport() const { return true; }
    virtual bool IsInstruction() const { return true; }
private:
    refptr<ITreeNode> m_filename;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class Value : public ITreeNode
{
public:
    Value() : m_value(nullptr) {}
    explicit Value(reflection::IPrimitiveData* iValue) : m_value(iValue) {}
    virtual void SetArgument(size_t i, ITreeNode* iArg) override { SG_UNUSED((i, iArg)); SG_ASSERT_NOT_REACHED(); }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return true; }
private:
    refptr<reflection::IPrimitiveData> m_value;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
// IntrinsicKeyword is the equivalent to an identifier with value "intrinsic".
// It is tokenized separately in order to be valid only in FunctionCall
// construct and to have a specific behavior there: call a C++ function.
class IntrinsicKeyword : public ITreeNode
{
public:
    IntrinsicKeyword() {}
    virtual ~IntrinsicKeyword() override {}
    virtual void SetArgument(size_t i, ITreeNode* iArg) override { SG_UNUSED((i, iArg)); SG_ASSERT_NOT_REACHED(); }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return true; }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class Identifier : public ITreeNode
{
public:
    Identifier() : m_identifier() {}
    virtual ~Identifier() override {}
    explicit Identifier(std::string const& iIdentifier) : m_identifier(iIdentifier) {}
    virtual void SetArgument(size_t i, ITreeNode* iArg) override { SG_UNUSED((i, iArg)); SG_ASSERT_NOT_REACHED(); }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return true; }
private:
    std::string m_identifier;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class PreresolvedIdentifierAndQualifiers : public ITreeNode
{
public:
    virtual ~PreresolvedIdentifierAndQualifiers() override {}
    explicit PreresolvedIdentifierAndQualifiers(reflection::Identifier const& iIdentifier) : m_identifier(iIdentifier), m_qualifiers() {}
    explicit PreresolvedIdentifierAndQualifiers(reflection::Identifier const& iIdentifier, TokenType iQualifier) : m_identifier(iIdentifier), m_qualifiers(1, iQualifier) {}
    explicit PreresolvedIdentifierAndQualifiers(reflection::Identifier const& iIdentifier, std::vector<TokenType>& iQualifiersWillBeStolen) : m_identifier(iIdentifier), m_qualifiers() { swap(m_qualifiers, iQualifiersWillBeStolen); }
    virtual void SetArgument(size_t i, ITreeNode* iArg) override { SG_UNUSED((i, iArg)); SG_ASSERT_NOT_REACHED(); }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return true; }
private:
    reflection::Identifier m_identifier;
    std::vector<TokenType> m_qualifiers;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class Qualifier : public ITreeNode
{
public:
    explicit Qualifier(TokenType iToken) : m_token(iToken) {}
    virtual void SetArgument(size_t i, ITreeNode* iArg) override { SG_ASSERT_AND_UNUSED(0 == i); SG_ASSERT(nullptr == m_arg); m_arg = iArg; }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return true; }
private:
    TokenType m_token;
    refptr<ITreeNode> m_arg;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class ScopeResolution : public ITreeNode
{
public:
    ScopeResolution() {}
    virtual void SetArgument(size_t i, ITreeNode* iArg) override { SG_ASSERT(i < 2); SG_ASSERT(nullptr == m_args[i]); m_args[i] = iArg; }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return true; }
private:
    refptr<ITreeNode> m_args[2];
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class Identifierize : public ITreeNode
{
public:
    virtual ~Identifierize() override {}
    virtual void SetArgument(size_t i, ITreeNode* iArg) override { SG_ASSERT(i < 1); SG_ASSERT(nullptr == m_arg); m_arg = iArg; }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return true; }
private:
    refptr<ITreeNode> m_arg;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class ObjectDefinition : public ITreeNode // [export|public|protected|private] name is type { ...}
{
public:
    ObjectDefinition() : m_name(nullptr), m_object(nullptr) {}
    virtual void SetArgument(size_t i, ITreeNode* iArg) override
    {
        if(0 == i) { m_name = iArg; }
        else if(1 == i) { m_object = iArg; }
    }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return true; }
    virtual bool IsInstruction() const { return true; }
private:
    refptr<ITreeNode> m_name;
    refptr<ITreeNode> m_object;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class Object : public ITreeNode // type { ... }
{
public:
    virtual void SetArgument(size_t i, ITreeNode* iArg) override { SG_ASSERT_AND_UNUSED(0 == i); m_type = iArg; }
    virtual void SetSubNodes(std::vector<refptr<ITreeNode> >& iSubNodesCanBeCleared) override { SG_ASSERT(m_instructions.empty()); using std::swap; swap(iSubNodesCanBeCleared, m_instructions); }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return true; }
    virtual bool IsInstruction() const { return true; }
private:
    refptr<ITreeNode> m_type;
    std::vector<refptr<ITreeNode> > m_instructions;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class Struct : public ITreeNode // { ... }
{
public:
    virtual void SetArgument(size_t i, ITreeNode* iArg) override { SG_UNUSED((i, iArg)); SG_ASSERT_NOT_REACHED(); }
    virtual void SetSubNodes(std::vector<refptr<ITreeNode> >& iSubNodesCanBeCleared) override { SG_ASSERT(m_instructions.empty()); using std::swap; swap(iSubNodesCanBeCleared, m_instructions); }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return true; }
private:
    std::vector<refptr<ITreeNode> > m_instructions;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class List : public ITreeNode
{
public:
    virtual void SetArgument(size_t i, ITreeNode* iArg) override { SG_UNUSED((i, iArg)); SG_ASSERT_NOT_REACHED(); }
    virtual void SetSubNodes(std::vector<refptr<ITreeNode> >& iSubNodesCanBeCleared) override { SG_ASSERT(nullptr == m_values); SG_ASSERT(iSubNodesCanBeCleared.size() == 1); m_values = iSubNodesCanBeCleared[0]; }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return true; }
private:
    refptr<ITreeNode> m_values;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class Parenthesis : public ITreeNode
{
public:
    virtual void SetArgument(size_t i, ITreeNode* iArg) override { SG_UNUSED((i, iArg)); SG_ASSERT_NOT_REACHED(); }
    virtual void SetSubNodes(std::vector<refptr<ITreeNode> >& iSubNodesCanBeCleared) override { SG_ASSERT(nullptr == m_expression); SG_ASSERT(iSubNodesCanBeCleared.size() == 1); m_expression = iSubNodesCanBeCleared[0]; }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return true; }
private:
    refptr<ITreeNode> m_expression;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class Indexing : public ITreeNode
{
public:
    virtual ~Indexing() override {}
    virtual void SetArgument(size_t i, ITreeNode* iArg) override { SG_ASSERT_AND_UNUSED(0 == i); m_indexed = iArg;  }
    virtual void SetSubNodes(std::vector<refptr<ITreeNode> >& iSubNodesCanBeCleared) override { SG_ASSERT(nullptr == m_index); SG_ASSERT(iSubNodesCanBeCleared.size() == 1); m_index = iSubNodesCanBeCleared[0]; }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return true; }
private:
    virtual void SetIndex(ITreeNode* iIndex) { SG_ASSERT(nullptr == m_index); m_index = iIndex; }
private:
    refptr<ITreeNode> m_indexed;
    refptr<ITreeNode> m_index;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class PropertyAffectation : public ITreeNode
{
public:
    PropertyAffectation() : m_left(nullptr), m_right(nullptr) {}
    //explicit ObjectDefinition(OperatorTraits const& iOperatorTraits) {}
    virtual void SetArgument(size_t i, ITreeNode* iArg) override
    {
        if(0 == i) { SG_ASSERT(nullptr == m_left); m_left = iArg; }
        else if(1 == i) { SG_ASSERT(nullptr == m_right); m_right = iArg; }
    }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return false; }
    virtual bool IsInstruction() const { return true; }
private:
    refptr<ITreeNode> m_left;
    refptr<ITreeNode> m_right;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class Assignment : public ITreeNode
{
public:
    Assignment() : m_left(nullptr), m_right(nullptr) {}
    //explicit ObjectDefinition(OperatorTraits const& iOperatorTraits) {}
    virtual void SetArgument(size_t i, ITreeNode* iArg) override
    {
        if(0 == i) { SG_ASSERT(nullptr == m_left); m_left = iArg; }
        else if(1 == i) { SG_ASSERT(nullptr == m_right); m_right = iArg; }
    }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return true; }
    virtual bool IsInstruction() const { return true; }
private:
    refptr<ITreeNode> m_left;
    refptr<ITreeNode> m_right;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class IncDecRement : public ITreeNode
{
private:
    IncDecRement(TokenType iToken, bool iIsSuffix) : m_arg(), m_token(iToken), m_isSuffix(iIsSuffix) {}
public:
    IncDecRement(OperatorTraits const& iTraits);
    virtual ~IncDecRement() override {}
    //explicit ObjectDefinition(OperatorTraits const& iOperatorTraits) {}
    virtual void SetArgument(size_t i, ITreeNode* iArg) override
    {
        SG_ASSERT_AND_UNUSED(0 == i); SG_ASSERT_AND_UNUSED(nullptr == m_arg); m_arg = iArg;
    }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return true; }
    virtual bool IsInstruction() const { return true; }
private:
    refptr<ITreeNode> m_arg;
    TokenType m_token;
    bool m_isSuffix;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class Comma : public ITreeNode
{
public:
    virtual void SetArgument(size_t i, ITreeNode* iArg) override { SG_ASSERT(2 > i); SG_ASSERT(nullptr == m_args[i]); m_args[i] = iArg; }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return true; }
private:
    refptr<ITreeNode> m_args[2];
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class Namespace : public ITreeNode
{
public:
    void SetName(ITreeNode* iTreeNode) { m_name = iTreeNode; }
    void SetInstructions(std::vector<refptr<ITreeNode> >& iSubNodesCanBeCleared) { SG_ASSERT(m_instructions.empty()); using std::swap; swap(iSubNodesCanBeCleared, m_instructions); }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return false; }
    virtual bool IsInstruction() const { return true; }
private:
    refptr<ITreeNode> m_name;
    std::vector<refptr<ITreeNode> > m_instructions;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class If : public ITreeNode
{
public:
    virtual ~If() override {}
    void SetExpr(ITreeNode* iTreeNode) { m_expr = iTreeNode; }
    void SetInstructions(std::vector<refptr<ITreeNode> >& iSubNodesCanBeCleared) { SG_ASSERT(m_instructions.empty()); using std::swap; swap(iSubNodesCanBeCleared, m_instructions); }
    void SetElseInstructions(std::vector<refptr<ITreeNode> >& iSubNodesCanBeCleared) { SG_ASSERT(m_elseinstructions.empty()); using std::swap; swap(iSubNodesCanBeCleared, m_elseinstructions); }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return false; }
    virtual bool IsInstruction() const { return true; }
private:
    refptr<ITreeNode> m_expr;
    std::vector<refptr<ITreeNode> > m_instructions;
    std::vector<refptr<ITreeNode> > m_elseinstructions;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class While : public ITreeNode
{
public:
    While() : m_alwaysPerformFirstIteration(false) {}
    void SetAlwaysPerformFirstIteration(bool b) { m_alwaysPerformFirstIteration = b; }
    void SetExpr(ITreeNode* iTreeNode) { m_expr = iTreeNode; }
    void SetInstructions(std::vector<refptr<ITreeNode> >& iSubNodesCanBeCleared) { SG_ASSERT(m_instructions.empty()); using std::swap; swap(iSubNodesCanBeCleared, m_instructions); }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return false; }
    virtual bool IsInstruction() const { return true; }
private:
    bool m_alwaysPerformFirstIteration;
    refptr<ITreeNode> m_expr;
    std::vector<refptr<ITreeNode> > m_instructions;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class NoOp : public ITreeNode
{
public:
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return false; }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class For : public ITreeNode
{
public:
    enum class ConstructProgress { Begin, End, In, Init, Cond };
    ConstructProgress GetConstructProgress()
    {
        if(m_init.empty())
            if(nullptr == m_var)
                return ConstructProgress::Begin;
            else
                if(nullptr == m_list)
                    return ConstructProgress::In;
                else
                    return ConstructProgress::End;
        else
            if(nullptr == m_cond)
                return ConstructProgress::Init;
            else if(m_incr.empty())
                return ConstructProgress::Cond;
            else
                return ConstructProgress::End;
    }
    void SetInit(ITreeNode* iTreeNode) { m_init.clear(); m_init.push_back(iTreeNode); }
    void SetInit(ArrayView<refptr<ITreeNode>> iTreeNodes) { m_init.clear(); for(auto const& it : iTreeNodes) { m_init.push_back(it.get()); } }
    void SetCond(ITreeNode* iTreeNode) { m_cond = iTreeNode; }
    void SetIncr(ITreeNode* iTreeNode) { m_incr.clear(); m_incr.push_back(iTreeNode); }
    void SetIncr(ArrayView<refptr<ITreeNode>> iTreeNodes) { m_incr.clear(); for(auto const& it : iTreeNodes) { m_incr.push_back(it.get()); } }
    void SetVar(ITreeNode* iTreeNode) { m_var = iTreeNode; }
    void SetList(ITreeNode* iTreeNode) { m_list = iTreeNode; }
    void SetInstructions(std::vector<refptr<ITreeNode> >& iSubNodesCanBeCleared) { SG_ASSERT(m_instructions.empty()); using std::swap; swap(iSubNodesCanBeCleared, m_instructions); }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return false; }
    virtual bool IsInstruction() const { return true; }
private:
    // for(init; cond; incr) mode
    std::vector<refptr<ITreeNode> > m_init;
    refptr<ITreeNode> m_cond;
    std::vector<refptr<ITreeNode> > m_incr;
    // for(var in list) mode
    refptr<ITreeNode> m_var;
    refptr<ITreeNode> m_list;
    std::vector<refptr<ITreeNode> > m_instructions;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class Assert : public ITreeNode
{
public:
    Assert() {}
    virtual void SetArgument(size_t i, ITreeNode* iArg) override { SG_ASSERT_AND_UNUSED(0 == i); SG_ASSERT(nullptr == m_arg); m_arg = iArg; }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return false; }
    virtual bool IsInstruction() const { return true; }
private:
    refptr<ITreeNode> m_arg;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class BreakingInstruction : public ITreeNode
{
public:
    enum class Type { Continue, Break, Return };
    BreakingInstruction(Type iType) : m_type(iType) {}
    Type GetType() const { return m_type; }
    virtual bool IsValue() const override { return false; }
    virtual bool IsInstruction() const { return true; }
private:
    Type m_type;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class Continue : public BreakingInstruction
{
public:
    Continue() : BreakingInstruction(Type::Continue) {}
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
private:
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class Break : public BreakingInstruction
{
public:
    Break() : BreakingInstruction(Type::Break) {}
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
private:
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class Return : public BreakingInstruction
{
public:
    Return() : BreakingInstruction(Type::Return) {}
    virtual void SetArgument(size_t i, ITreeNode* iArg) override { SG_ASSERT_AND_UNUSED(0 == i); SG_ASSERT(nullptr == m_arg); m_arg = iArg; }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
private:
    refptr<ITreeNode> m_arg;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class FunctionCall : public ITreeNode
{
public:
    virtual void SetArgument(size_t i, ITreeNode* iArg) override { SG_ASSERT_AND_UNUSED(0 == i); SG_ASSERT(nullptr == m_callable); m_callable = iArg;  }
    virtual void SetSubNodes(std::vector<refptr<ITreeNode> >& iSubNodesCanBeCleared) override { SG_ASSERT(nullptr == m_args); SG_ASSERT(iSubNodesCanBeCleared.size() == 1); m_args = iSubNodesCanBeCleared[0]; }
    virtual void SetArguments(ITreeNode* iArgs) { SG_ASSERT(nullptr == m_args); m_args = iArgs; }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return true; }
    virtual bool IsInstruction() const { return true; }
private:
    refptr<ITreeNode> m_callable;
    refptr<ITreeNode> m_args;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class FunctionDeclaration : public ITreeNode
{
public:
    FunctionDeclaration() : m_isPrototype(false) {}
    void SetName(ITreeNode* iTreeNode) { SG_ASSERT(nullptr == m_name); m_name = iTreeNode; }
    void PushArgumentName(ITreeNode* iName) { m_argumentNamesAndDefaultValues.push_back(std::make_pair(iName, nullptr)); }
    void PushArgumentDefaultValue(ITreeNode* iDefault) { SG_ASSERT(!m_argumentNamesAndDefaultValues.empty()); SG_ASSERT(nullptr == m_argumentNamesAndDefaultValues.back().second); m_argumentNamesAndDefaultValues.back().second = iDefault; }
    void SetInstructions(std::vector<refptr<ITreeNode> >& iSubNodesCanBeCleared) { SG_ASSERT(m_instructions.empty()); using std::swap; swap(iSubNodesCanBeCleared, m_instructions); }
    void SetIsPrototypeOnly() { SG_ASSERT(!m_isPrototype); m_isPrototype = true; }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return false; }
    virtual bool IsInstruction() const { return true; }
private:
    bool m_isPrototype;
    refptr<ITreeNode> m_name;
    std::vector<std::pair<refptr<ITreeNode>, refptr<ITreeNode> > > m_argumentNamesAndDefaultValues;
    std::vector<refptr<ITreeNode> > m_instructions;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class TemplateDeclaration : public ITreeNode
{
public:
    TemplateDeclaration() {}
    void SetName(ITreeNode* iTreeNode) { SG_ASSERT(nullptr == m_name); m_name = iTreeNode; }
    void SetType(ITreeNode* iTreeNode) { SG_ASSERT(nullptr == m_type); m_type = iTreeNode; }
    void PushArgumentName(ITreeNode* iName) { m_argumentNamesAndDefaultValues.push_back(std::make_pair(iName, nullptr)); }
    void PushArgumentDefaultValue(ITreeNode* iDefault) { SG_ASSERT(!m_argumentNamesAndDefaultValues.empty()); SG_ASSERT(nullptr == m_argumentNamesAndDefaultValues.back().second); m_argumentNamesAndDefaultValues.back().second = iDefault; }
    void SetInstructions(std::vector<refptr<ITreeNode> >& iSubNodesCanBeCleared) { SG_ASSERT(m_instructions.empty()); using std::swap; swap(iSubNodesCanBeCleared, m_instructions); }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return false; }
    virtual bool IsInstruction() const { return true; }
private:
    refptr<ITreeNode> m_name;
    refptr<ITreeNode> m_type;
    std::vector<std::pair<refptr<ITreeNode>, refptr<ITreeNode> > > m_argumentNamesAndDefaultValues;
    std::vector<refptr<ITreeNode> > m_instructions;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class TypedefDeclaration : public ITreeNode
{
public:
    TypedefDeclaration() {}
    void SetType(ITreeNode* iTreeNode) { SG_ASSERT(nullptr == m_type); m_type = iTreeNode; }
    void SetAlias(ITreeNode* iTreeNode) { SG_ASSERT(nullptr == m_alias); m_alias = iTreeNode; }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return false; }
    virtual bool IsInstruction() const { return true; }
private:
    refptr<ITreeNode> m_type;
    refptr<ITreeNode> m_alias;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class UnaryExpression : public ITreeNode
{
public:
    explicit UnaryExpression(TokenType iOp) : m_op(iOp) {}
    virtual void SetArgument(size_t i, ITreeNode* iArg) override { SG_ASSERT_AND_UNUSED(0 == i); SG_ASSERT(nullptr == m_arg); m_arg = iArg; }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return true; }
private:
    TokenType m_op;
    refptr<ITreeNode> m_arg;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class BinaryExpression : public ITreeNode
{
public:
    explicit BinaryExpression(TokenType iOp) : m_op(iOp) { for(size_t i=0; i<2; ++i) { m_args[i] = nullptr; } }
    virtual void SetArgument(size_t i, ITreeNode* iArg) override { SG_ASSERT(2 > i); SG_ASSERT(nullptr == m_args[i]); m_args[i] = iArg; }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return true; }
private:
    TokenType m_op;
    refptr<ITreeNode> m_args[2];
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class CompoundAssignment : public ITreeNode
{
public:
    explicit CompoundAssignment(TokenType iOp) : m_op(iOp) { for(size_t i=0; i<2; ++i) { m_args[i] = nullptr; } }
    virtual void SetArgument(size_t i, ITreeNode* iArg) override { SG_ASSERT(2 > i); SG_ASSERT(nullptr == m_args[i]); m_args[i] = iArg; }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual bool IsValue() const override { return true; }
    virtual bool IsInstruction() const { return true; }
private:
    TokenType m_op;
    refptr<ITreeNode> m_args[2];
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class TernaryExpression : public ITreeNode
{
public:
    explicit TernaryExpression(TokenType iOp) { SG_ASSERT_AND_UNUSED(TokenType::operator_interrogation == iOp); }
    virtual void SetArgument(size_t i, ITreeNode* iArg) override { SG_ASSERT(2 > i); SG_ASSERT(nullptr == m_args[i*2]); m_args[i*2] = iArg; }
    virtual bool EvaluateROK(EvaluationInputOutput& io) override;
    virtual void SetSubNodes(std::vector<refptr<ITreeNode> >& iSubNodesCanBeCleared) override { SG_ASSERT(nullptr == m_args[1] ); SG_ASSERT(iSubNodesCanBeCleared.size() == 1); m_args[1] = iSubNodesCanBeCleared[0]; }
    virtual bool IsValue() const override { return true; }
private:
    void SetCenterNode(ITreeNode* iNode) { SG_ASSERT(nullptr == m_args[1] ); m_args[1] = iNode; }
private:
    refptr<ITreeNode> m_args[3];
};
//=============================================================================
}
}
}

namespace sg {
namespace objectscript {
//=============================================================================
class Variable : public reflection::BaseClass
{
    SG_NON_COPYABLE(Variable)
    REFLECTION_CLASS_HEADER(Variable, reflection::BaseClass);
public:
    Variable() : m_value(), m_isConst(false) { }
    explicit Variable(bool iIsConst) : m_value(nullptr), m_isConst(iIsConst) { }
    Variable(bool iIsConst, reflection::IPrimitiveData const* iValue) : m_value(nullptr), m_isConst(iIsConst) { SetValue(iValue); }
    reflection::IPrimitiveData* Value() { return m_value.get(); }
    refptr<reflection::IPrimitiveData>* ValueReference() { return &m_value; }
    void SetValue(reflection::IPrimitiveData const* iValue)
    {
        SG_ASSERT_MSG(iValue->IsRefCounted_ForAssert(), "Input PrimitiveData should be owned by client to prevent memory leak.");
        iValue->CopyTo(m_value);
    }
    bool IsConst() const { return m_isConst; }
private:
    refptr<reflection::IPrimitiveData> m_value;
    bool m_isConst;
};
//=============================================================================
// A FreeVariabe is a variable that does not have a value yet during a
// preresolution pass.
class FreeVariable : public reflection::BaseClass
{
    SG_NON_COPYABLE(FreeVariable)
    REFLECTION_CLASS_HEADER(FreeVariable, reflection::BaseClass);
public:
    FreeVariable() : m_isConst(false) { }
    explicit FreeVariable(bool iIsConst) : m_isConst(iIsConst) { }
    bool IsConst() const { return m_isConst; }
private:
    bool m_isConst;
};
//=============================================================================
class Alias : public reflection::BaseClass
{
    SG_NON_COPYABLE(Alias)
    REFLECTION_CLASS_HEADER(Alias, reflection::BaseClass);
    Alias() : m_value() { SG_ASSERT_NOT_REACHED(); }
public:
    Alias(reflection::Identifier const& iValue) : m_value(iValue) {}
    reflection::Identifier const& Value() const { return m_value; }
private:
    reflection::Identifier const m_value;
};
//=============================================================================
struct ArgumentNameAndValue
{
    bool isConst;
    reflection::IdentifierNode name;
    refptr<reflection::IPrimitiveData const> value;

    ArgumentNameAndValue(bool iIsConst, reflection::IdentifierNode const& iName, reflection::IPrimitiveData const* iValue)
        : isConst(iIsConst)
        , name(iName)
        , value(iValue)
    {
    }
};
//=============================================================================
class Callable : public reflection::BaseClass
{
    SG_NON_COPYABLE(Callable)
    REFLECTION_CLASS_HEADER(Callable, reflection::BaseClass);
public:
    Callable() = default;
    virtual bool CallROK(semanticTree::EvaluationInputOutput& io, Token const& iToken) = 0;
    virtual bool CallROK(semanticTree::EvaluationInputOutput& io, reflection::PrimitiveDataList const& iArgs, Token const& iToken) = 0;
    virtual bool CallROK(semanticTree::EvaluationInputOutput& io, reflection::PrimitiveDataNamedList const& iArgs, Token const& iToken) = 0;
    virtual bool CallROK(semanticTree::EvaluationInputOutput& io, reflection::IPrimitiveData const* iArg, Token const& iToken) = 0;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class IntrinsicCall : public Callable
{
    SG_NON_COPYABLE(IntrinsicCall)
    REFLECTION_CLASS_HEADER(IntrinsicCall, Callable);
public:
    IntrinsicCall() = default;
    IntrinsicCall(auto_initialized_t) : IntrinsicCall() { reflection::ObjectCreationContext context; EndCreationIFN(context); };
private:
    virtual bool CallROK(semanticTree::EvaluationInputOutput& io, Token const& iToken) override;
    virtual bool CallROK(semanticTree::EvaluationInputOutput& io, reflection::PrimitiveDataList const& iArgs, Token const& iToken) override;
    virtual bool CallROK(semanticTree::EvaluationInputOutput& io, reflection::PrimitiveDataNamedList const& iArgs, Token const& iToken) override;
    virtual bool CallROK(semanticTree::EvaluationInputOutput& io, reflection::IPrimitiveData const* iArg, Token const& iToken) override;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class Function : public Callable
{
    SG_NON_COPYABLE(Function)
    REFLECTION_CLASS_HEADER(Function, Callable);
public:
    Function();
    explicit Function(std::vector<ArgumentNameAndValue>& iArgumentsAndDefaultValuesCanBeCleared);
    bool IsPrototype() const { return m_isPrototype; }
    void SetIdentifier(reflection::Identifier const& iIdentifier) { m_identifier = iIdentifier; }
    reflection::Identifier const& Identifier() const { return m_identifier; }
    void SetBody(std::vector<refptr<semanticTree::ITreeNode> >& iInstructionsCanBeErased);
    std::vector<ArgumentNameAndValue> const& ArgumentNamesAndDefaultValues() const { return m_argumentNamesAndDefaultValues; }
    virtual bool CallROK(semanticTree::EvaluationInputOutput& io, Token const& iToken) override;
    virtual bool CallROK(semanticTree::EvaluationInputOutput& io, reflection::PrimitiveDataList const& iArgs, Token const& iToken) override;
    virtual bool CallROK(semanticTree::EvaluationInputOutput& io, reflection::PrimitiveDataNamedList const& iArgs, Token const& iToken) override;
    virtual bool CallROK(semanticTree::EvaluationInputOutput& io, reflection::IPrimitiveData const* iArg, Token const& iToken) override;
private:
    bool CallImplROK(semanticTree::EvaluationInputOutput& io, std::vector<ArgumentNameAndValue> const& iArgs, Token const& iToken);
private:
    bool m_isPrototype;
    std::vector<ArgumentNameAndValue> m_argumentNamesAndDefaultValues;
    reflection::Identifier m_identifier;
    std::vector<refptr<semanticTree::ITreeNode> > m_instructions;
};
//=============================================================================
class Template : public reflection::BaseClass
{
    SG_NON_COPYABLE(Template)
    REFLECTION_CLASS_HEADER(Template, reflection::BaseClass);
public:
    Template();
    explicit Template(std::vector<ArgumentNameAndValue>& iArgumentsAndDefaultValuesCanBeCleared);
    void SetIdentifier(reflection::Identifier const& iIdentifier) { m_identifier = iIdentifier; }
    void SetBody(std::vector<refptr<semanticTree::ITreeNode> >& iInstructionsCanBeErased);
    void SetMetaclass(reflection::Metaclass const* iMetaclass) { m_metaclass = iMetaclass; }
    bool CallROK(semanticTree::EvaluationInputOutput& io, reflection::PrimitiveDataNamedList const& iArgs, Token const& iToken);
    std::vector<ArgumentNameAndValue> const& ArgumentNamesAndDefaultValues() const { return m_argumentNamesAndDefaultValues; }
private:
    bool CallImplROK(semanticTree::EvaluationInputOutput& io, std::vector<ArgumentNameAndValue> const& iArgs, Token const& iToken);
private:
    reflection::Metaclass const* m_metaclass;
    std::vector<ArgumentNameAndValue> m_argumentNamesAndDefaultValues;
    reflection::Identifier m_identifier;
    std::vector<refptr<semanticTree::ITreeNode> > m_instructions;
};
//=============================================================================
}
}
#endif

#include "stdafx.h"

#include "SemanticTree.h"

#include "Intrinsics.h"
#include "Operators.h"
#include <Core/Log.h>
#include <Core/StringFormat.h>
#include <algorithm>
#include <sstream>

namespace sg {
namespace objectscript {
namespace {
//=============================================================================
void PushError(semanticTree::EvaluationInputOutput& io, ErrorType iErrorType, Token const& iToken, char const* msg)
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
    io.errorHandler->OnObjectScriptError(err);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void PushUnresolvedIdentifierError(semanticTree::EvaluationInputOutput& io, semanticTree::ITreeNode* iTreeNode)
{
    Token const& token = iTreeNode->GetToken();
    // TODO: print identifier if possible
    PushError(io, ErrorType::unresolved_identifier_in_expression, token, "unresolved identifier in expression");
}
//=============================================================================
struct ResolvedIdentifier
{
    enum class Type { None, Metaclass, ScriptObject, FreeVariable };
    Type type;
    reflection::Identifier fullIdentifier;
    reflection::Metaclass const* metaclass;
    safeptr<reflection::BaseClass> scriptObject;
    ResolvedIdentifier() : type(Type::None), fullIdentifier(), metaclass(), scriptObject() {}
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ResolveIdentifier(semanticTree::EvaluationInputOutput& io, reflection::Identifier const& iIdentifier, ResolvedIdentifier& oResolved)
{
    SG_ASSERT(ResolvedIdentifier::Type::None == oResolved.type);
    SG_ASSERT(oResolved.fullIdentifier.Empty());
    SG_ASSERT(nullptr == oResolved.metaclass);
    SG_ASSERT(nullptr == oResolved.scriptObject);

    std::pair<reflection::Identifier, safeptr<reflection::BaseClass> > namedScriptObject = io.scriptDatabase->GetWithIdentifierIFP(*io.pathForScript, iIdentifier);
    reflection::Metaclass const* mc = reflection::GetMetaclassIFP(*io.pathForScript, iIdentifier);

    // Note: We may like to check that there is no collision with existing
    // object name. However, this method is called when script is evaluated,
    // but object database is not ready for named queries. Maybe we need to
    // add an access for this special check.
    //std::pair<reflection::Identifier, safeptr<reflection::BaseClass> > object = io.objectDatabase->GetWithIdentifierIFP(*io.pathForObjects, iIdentifier);
    //SG_ASSERT(nullptr == object.second); // TODO: can fail, but need to add checks that script object or class name is prioritary.

    if(nullptr != mc)
    {
        SG_ASSERT(nullptr == namedScriptObject.second);
        oResolved.type = ResolvedIdentifier::Type::Metaclass;
        oResolved.fullIdentifier = mc->FullClassName();
        oResolved.metaclass = mc;
    }

    if(!namedScriptObject.first.Empty())
    {
        // NB: (This can happen only if it is possible to do "using namespace")
        SG_ASSERT(namedScriptObject.first.Size() != oResolved.fullIdentifier.Size()); // TODO: Error message "ambiguous name"?
        if(namedScriptObject.first.Size() > oResolved.fullIdentifier.Size())
        {
            if(namedScriptObject.second->GetMetaclass() == Alias::StaticGetMetaclass())
            {
                Alias const* alias = checked_cast<Alias const*>(namedScriptObject.second.get());
                ResolvedIdentifier resolvedAlias;
                ResolveIdentifier(io, alias->Value(), resolvedAlias);
                oResolved = resolvedAlias;
            }
            else if(namedScriptObject.second->GetMetaclass() == FreeVariable::StaticGetMetaclass())
            {
                oResolved.type = ResolvedIdentifier::Type::FreeVariable;
                oResolved.fullIdentifier = namedScriptObject.first;
                oResolved.scriptObject = namedScriptObject.second;
            }
            else
            {
                oResolved.type = ResolvedIdentifier::Type::ScriptObject;
                oResolved.fullIdentifier = namedScriptObject.first;
                oResolved.scriptObject = namedScriptObject.second;
            }
        }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ResolveLValueIdentifier(semanticTree::EvaluationInputOutput& io, reflection::Identifier const& iIdentifier, ResolvedIdentifier& oResolved)
{
    SG_ASSERT(!iIdentifier.IsAbsolute());
    SG_ASSERT(iIdentifier.Size() == 1);
    ResolveIdentifier(io, iIdentifier, oResolved);
    if(ResolvedIdentifier::Type::None != oResolved.type)
    {
        if(oResolved.fullIdentifier.ParentNamespace() != *io.pathForScript)
            oResolved = ResolvedIdentifier();
    }
}
//=============================================================================
bool ConvertToValueROK(semanticTree::EvaluationInputOutput& io, Token const& iToken)
{
    SG_ASSERT(io.returnIdentifier.Empty() || io.returnValue == nullptr);
    SG_ASSERT(io.qualifiers.empty());
    if(!io.returnIdentifier.Empty())
    {
        SG_ASSERT_MSG(io.qualifiers.empty(), "Qualifiers are not allowed in such expression"); // TODO: Error message
        ResolvedIdentifier resolved;
        ResolveIdentifier(io, io.returnIdentifier, resolved);
        switch(resolved.type)
        {
        case ResolvedIdentifier::Type::None:
        {
            if(io.isPreresolutionPass)
            {
                semanticTree::PreresolvedIdentifierAndQualifiers* preresolved = new semanticTree::PreresolvedIdentifierAndQualifiers(io.returnIdentifier);
                preresolved->SetToken(iToken);
                io.presesolvedNodeIFN = preresolved;
            }
            else
            {
                if(nullptr == io.objectDatabase)
                {
                    PushError(io, ErrorType::unknown_reference_in_imported_file, iToken, Format("Unknown identifier: %0", std::string(iToken.begin, iToken.end-iToken.begin)).c_str());
                    return false;
                }
                io.returnValue = new reflection::PrimitiveData<reflection::ObjectReference>(
                    reflection::ObjectReference(io.objectDatabase.get(),
                                                *io.pathForObjects,
                                                io.returnIdentifier));
                io.returnValueIsLValue = false;
                io.returnValueContainsUnresolvedIdentifier = true;
                io.returnIdentifier = reflection::Identifier();
            }
        }
            break;
        case ResolvedIdentifier::Type::Metaclass:
            PushError(io, ErrorType::use_of_class_as_value, iToken, "Class can't be used as a value");
            return false;
        case ResolvedIdentifier::Type::ScriptObject:
            {
                reflection::BaseClass* scriptObject = resolved.scriptObject.get();
                SG_ASSERT(nullptr != scriptObject);
                if(scriptObject->GetMetaclass() == Variable::StaticGetMetaclass())
                {
                    SG_ASSERT(io.enableVariableRead);
                    Variable* var = checked_cast<Variable*>(scriptObject);
                    if(var->IsConst())
                    {
                        var->Value()->CopyTo(io.returnValue);
                        SG_ASSERT(!io.returnValueIsLValue);
                        SG_ASSERT(nullptr == io.returnLValueReference);
                        io.returnIdentifier = reflection::Identifier();
                        io.returnValueContainsUnresolvedIdentifier = DoesContainObjectReference(io.returnValue.get());
                    }
                    else
                    {
                        if(io.variablesAreRestricted)
                        {
                            reflection::Identifier const& fullIdentifier = resolved.fullIdentifier;
                            SG_ASSERT(nullptr != io.restrictedVariableScope);
                            reflection::Identifier const& restrictedScope = *(io.restrictedVariableScope);
                            if(!restrictedScope.Contains(fullIdentifier))
                            {
                                PushError(io, ErrorType::read_non_const_variable_outside_function_or_template, iToken, "It is not allowed to reference non-const variables that are defined ouside the function/template scope");
                                return false;
                            }
                        }
                        io.returnValue = var->Value();
                        io.returnLValueReference = var->ValueReference();
                        io.returnValueIsLValue = true;
                        io.returnIdentifier = reflection::Identifier();
                        io.returnValueContainsUnresolvedIdentifier = DoesContainObjectReference(io.returnValue.get());
                    }
                }
                else if(scriptObject->GetMetaclass() == FreeVariable::StaticGetMetaclass())
                {
                    SG_ASSERT_NOT_REACHED();
                    SG_ASSERT(io.isPreresolutionPass);
                    semanticTree::PreresolvedIdentifierAndQualifiers* preresolved = new semanticTree::PreresolvedIdentifierAndQualifiers(io.returnIdentifier);
                    preresolved->SetToken(iToken);
                    io.presesolvedNodeIFN = preresolved;
                }
                else if(scriptObject->GetMetaclass() == Function::StaticGetMetaclass())
                {
                    Function* function = checked_cast<Function*>(scriptObject);
                    io.returnValue = new reflection::PrimitiveData<refptr<reflection::BaseClass> >(function);
                }
                else if(scriptObject->GetMetaclass() == Template::StaticGetMetaclass())
                {
                    Template* tpl = checked_cast<Template*>(scriptObject);
                    io.returnValue = new reflection::PrimitiveData<refptr<reflection::BaseClass> >(tpl);
                }
                else
                {
                    SG_ASSERT(scriptObject->GetMetaclass() == TemplateNamespace::StaticGetMetaclass());
                    TemplateNamespace* tpl = checked_cast<TemplateNamespace*>(scriptObject);
                    io.returnValue = new reflection::PrimitiveData<refptr<reflection::BaseClass> >(tpl);
                }
            }
            break;
        case ResolvedIdentifier::Type::FreeVariable:
        {
            SG_ASSERT(io.isPreresolutionPass);
            semanticTree::PreresolvedIdentifierAndQualifiers* preresolved = new semanticTree::PreresolvedIdentifierAndQualifiers(io.returnIdentifier);
            preresolved->SetToken(iToken);
            io.presesolvedNodeIFN = preresolved;
            break;
        }
        default:
            SG_ASSERT_NOT_REACHED();
        }
    }
    else
    {
        SG_ASSERT(nullptr != io.returnValue || nullptr != io.presesolvedNodeIFN);
    }
    return true;
}
//=============================================================================
bool ConvertToPreresolvedROK(semanticTree::EvaluationInputOutput& io, refptr<semanticTree::ITreeNode>& oTreeNode, Token const& iToken)
{
    SG_ASSERT(io.isPreresolutionPass);
    if(nullptr != io.presesolvedNodeIFN)
    {
        SG_ASSERT(nullptr == io.returnValue);
        oTreeNode = io.presesolvedNodeIFN.get();
    }
    else
    {
        SG_ASSERT(nullptr != io.returnValue);
        SG_ASSERT(!io.returnValueContainsUnresolvedIdentifier || io.enableObjectReferences);
        SG_ASSERT(io.returnValue->GetType() != reflection::PrimitiveDataType::Object);
        refptr<reflection::IPrimitiveData> valueCopy;
        io.returnValue->CopyTo(valueCopy);
        semanticTree::Value* preresolved = new semanticTree::Value(valueCopy.get());
        preresolved->SetToken(iToken);
        oTreeNode = preresolved;
    }
    return true;
}
//=============================================================================
bool EvaluateAndConvertToPreresolvedUsingIOROK(semanticTree::ITreeNode* iTreeNode, semanticTree::EvaluationInputOutput& io, refptr<semanticTree::ITreeNode>& oTreeNode)
{
    bool ok = iTreeNode->EvaluateROK(io);
    if(!ok)
        return false;
    ok = ConvertToValueROK(io, iTreeNode->GetToken());
    if(!ok)
        return false;
    ok = ConvertToPreresolvedROK(io, oTreeNode, iTreeNode->GetToken());
    if(!ok)
        return false;
    return true;
}
//=============================================================================
bool EvaluateAndConvertToPreresolvedROK(semanticTree::ITreeNode* iTreeNode, semanticTree::EvaluationInputOutput& io, reflection::Identifier const& iPathForScript, refptr<semanticTree::ITreeNode>& oTreeNode)
{
    semanticTree::EvaluationInputOutput evalio(io);
    evalio.pathForScript = &iPathForScript;
    bool ok = EvaluateAndConvertToPreresolvedUsingIOROK(iTreeNode, evalio, oTreeNode);
    return ok;
}
//=============================================================================
bool PreresolveInstructionROK(semanticTree::EvaluationInputOutput& io,
                              reflection::Identifier const& iPathForScript,
                              std::vector<refptr<semanticTree::ITreeNode>> const& iInstructions,
                              std::vector<refptr<semanticTree::ITreeNode>>& oPreresolvedInstructions)
{
    SG_ASSERT(io.isPreresolutionPass);
    SG_ASSERT(oPreresolvedInstructions.empty());
    size_t const instructionCount = iInstructions.size();
    oPreresolvedInstructions.reserve(instructionCount);
    for(size_t i = 0; i < instructionCount; ++i)
    {
        semanticTree::EvaluationInputOutput subio(io);
        subio.pathForScript = &iPathForScript;
        bool ok = iInstructions[i]->EvaluateROK(subio);
        if(!ok)
            return false;
        if(nullptr != subio.presesolvedNodeIFN)
            oPreresolvedInstructions.push_back(subio.presesolvedNodeIFN.get());
        if(nullptr != subio.jumpStatement)
        {
            // Do not forward jump statements in preresolution.
            subio.jumpStatement = nullptr;
        }
    }
    return true;
}
//=============================================================================
}
}
}

namespace sg {
namespace objectscript {
//=============================================================================
REFLECTION_CLASS_BEGIN((sg, objectscript), Variable)
REFLECTION_CLASS_END
//=============================================================================
REFLECTION_CLASS_BEGIN((sg, objectscript), Alias)
REFLECTION_CLASS_END
//=============================================================================
REFLECTION_ABSTRACT_CLASS_BEGIN((sg, objectscript), Callable)
REFLECTION_CLASS_END
//=============================================================================
bool IntrinsicCall::CallROK(semanticTree::EvaluationInputOutput& io, Token const& iToken)
{
    PushError(io, ErrorType::missing_argument_in_intrinsic_call, iToken, "missing arguments in intrinsic call");
    return false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool IntrinsicCall::CallROK(semanticTree::EvaluationInputOutput& io, reflection::PrimitiveDataList const& iArgs, Token const& iToken)
{
    size_t const inputArgCount = iArgs.size();
    SG_ASSERT(!iArgs.empty());

    reflection::IPrimitiveData* firstArg = iArgs[0].get();
    if(firstArg->GetType() != reflection::PrimitiveDataType::String)
    {
        PushError(io, ErrorType::unsupported_type_for_intrinsic_name, iToken, "intrinsinc name must be a string");
        return false;
    }
    std::string const name = firstArg->As<std::string>();
    IntrinsicFct intrinsic = GetIntrinsic(name.c_str());
    if(nullptr == intrinsic)
    {
        PushError(io, ErrorType::unknown_intrinsic_name, iToken, "unknown intrinsinc name");
        return false;
    }

    reflection::PrimitiveDataList args;
    //std::vector<refptr<reflection::IPrimitiveData const>> args;
    args.reserve(inputArgCount);
    for(size_t i = 1; i < inputArgCount; ++i)
    {
        args.emplace_back(iArgs[i].get());
    }

    std::string error;
    bool const ok = intrinsic(io.returnValue, args, error);
    if(!ok)
    {
        PushError(io, ErrorType::incorrect_use_of_intrinsic, iToken, error.c_str());
        return false;
    }

    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool IntrinsicCall::CallROK(semanticTree::EvaluationInputOutput& io, reflection::PrimitiveDataNamedList const& iArgs, Token const& iToken)
{
    SG_UNUSED((io, iArgs, iToken));
    SG_ASSERT_NOT_IMPLEMENTED();
    return false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool IntrinsicCall::CallROK(semanticTree::EvaluationInputOutput& io, reflection::IPrimitiveData const* iArg, Token const& iToken)
{
    SG_UNUSED((io, iArg, iToken));
    SG_ASSERT_NOT_IMPLEMENTED();
    return false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, objectscript), IntrinsicCall)
REFLECTION_CLASS_END
//=============================================================================
Function::Function()
    : m_isPrototype(true)
{
    SG_ASSERT_NOT_IMPLEMENTED();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Function::Function(std::vector<ArgumentNameAndValue>& iArgumentsAndDefaultValuesCanBeCleared)
    : m_isPrototype(true)
{
    swap(m_argumentNamesAndDefaultValues, iArgumentsAndDefaultValuesCanBeCleared);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Function::SetBody(std::vector<refptr<semanticTree::ITreeNode> >& iInstructionsCanBeErased)
{
    SG_ASSERT(m_isPrototype);
    SG_ASSERT(m_instructions.empty());
    swap(m_instructions, iInstructionsCanBeErased);
    m_isPrototype = false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Function::CallROK(semanticTree::EvaluationInputOutput& io, Token const& iToken)
{
    std::vector<ArgumentNameAndValue> args;
    args.reserve(m_argumentNamesAndDefaultValues.size());
    for(auto const& it : m_argumentNamesAndDefaultValues)
    {
        if(nullptr == it.value)
        {
            PushError(io, ErrorType::missing_argument_in_function_call, iToken, "missing arguments in function call");
            return false;
        }
        args.push_back(it);
    }
    return CallImplROK(io, args, iToken);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Function::CallROK(semanticTree::EvaluationInputOutput& io, reflection::PrimitiveDataList const& iArgs, Token const& iToken)
{
    size_t const inputArgCount = iArgs.size();
    size_t const argCount = m_argumentNamesAndDefaultValues.size();
    SG_ASSERT(argCount >= inputArgCount);
    std::vector<ArgumentNameAndValue> args;
    args.reserve(argCount);
    for(size_t i = 0; i < inputArgCount; ++i)
    {
        ArgumentNameAndValue const& arg = m_argumentNamesAndDefaultValues[i];
        args.emplace_back(arg.isConst, arg.name, iArgs[i].get());
    }
    for(size_t i = inputArgCount; i < argCount; ++i)
    {
        ArgumentNameAndValue const& arg = m_argumentNamesAndDefaultValues[i];
        SG_ASSERT(nullptr != arg.value); //TODO: Error message "missing arguments to function call"
        args.push_back(arg);
    }
    return CallImplROK(io, args, iToken);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Function::CallROK(semanticTree::EvaluationInputOutput& io, reflection::PrimitiveDataNamedList const& iArgs, Token const& iToken)
{
    SG_UNUSED((io, iArgs, iToken));
    SG_ASSERT_NOT_IMPLEMENTED();
    return false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Function::CallROK(semanticTree::EvaluationInputOutput& io, reflection::IPrimitiveData const* iArg, Token const& iToken)
{
    size_t const argCount = m_argumentNamesAndDefaultValues.size();
    SG_ASSERT(argCount >= 1);
    std::vector<ArgumentNameAndValue> args;
    args.reserve(argCount);
    args.emplace_back(m_argumentNamesAndDefaultValues[0].isConst, m_argumentNamesAndDefaultValues[0].name, iArg);
    for(size_t i = 1; i < argCount; ++i)
    {
        ArgumentNameAndValue const& arg = m_argumentNamesAndDefaultValues[i];
        SG_ASSERT(nullptr != arg.value); //TODO: Error message "missing arguments to function call"
        args.push_back(arg);
    }
    return CallImplROK(io, args, iToken);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Function::CallImplROK(semanticTree::EvaluationInputOutput& io, std::vector<ArgumentNameAndValue> const& iArgs, Token const& iToken)
{
    SG_UNUSED(iToken);
    bool ok = true;
    SG_ASSERT(!io.isPreresolutionPass);
    SG_ASSERT(!IsPrototype()); // TODO: Error message "Function must have been fully defined before use"

    reflection::ObjectDatabaseScopedTransaction scopedTransaction(io.scriptDatabase.get());
    reflection::Identifier pathForScript = reflection::Identifier(m_identifier, reflection::IdentifierNode());
    for(auto const& it : iArgs)
    {
        reflection::ObjectVisibility visibility = reflection::ObjectVisibility::Public;
        reflection::Identifier varName(pathForScript, it.name);
        Variable* var = new Variable(it.isConst, it.value.get());
        io.scriptDatabase->Add(visibility, varName, var);
    }
    size_t const instructionCount = m_instructions.size();
    for(size_t i = 0; i < instructionCount; ++i)
    {
        if(!m_instructions[i]->IsInstruction())
        {
            PushError(io, ErrorType::expression_is_not_an_instruction, m_instructions[i]->GetToken(), "expression is not an instruction");
            return false;
        }
        semanticTree::EvaluationInputOutput subio(io);
        subio.pathForScript = &pathForScript;
        subio.restrictedVariableScope = &pathForScript;
        subio.enableNamespaceDefinition = false;
        subio.enableObjectDefinitions = false;
        subio.enableObjectReferences = false;
        subio.enableScriptDefinitions = true;
        subio.variablesAreRestricted = true;
        ok = m_instructions[i]->EvaluateROK(subio);
        if(!ok)
            return false;
        if(nullptr != subio.jumpStatement)
        {
            SG_ASSERT(semanticTree::JumpStatement::Type::Return == subio.jumpStatement->GetType()); // TODO: Error message "invalid use of break/continue in function body""
            subio.jumpStatement = nullptr;
            ok = ConvertToValueROK(subio, m_instructions[i]->GetToken());
            if(!ok)
                return false;
            io.returnValue = subio.returnValue;
            break;
        }
        if(io.errorHandler->DidErrorHappen())
            break;
    }
    return ok;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, objectscript), Function)
REFLECTION_CLASS_END
//=============================================================================
Template::Template()
    : m_metaclass(nullptr)
{
    SG_ASSERT_NOT_IMPLEMENTED();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Template::Template(std::vector<ArgumentNameAndValue>& iArgumentsAndDefaultValuesCanBeCleared)
    : m_metaclass(nullptr)
{
    swap(m_argumentNamesAndDefaultValues, iArgumentsAndDefaultValuesCanBeCleared);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Template::SetBody(std::vector<refptr<semanticTree::ITreeNode> >& iInstructionsCanBeErased)
{
    SG_ASSERT(m_instructions.empty());
    swap(m_instructions, iInstructionsCanBeErased);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Template::CallROK(semanticTree::EvaluationInputOutput& io, reflection::PrimitiveDataNamedList const& iArgs, Token const& iToken)
{
    bool ok = true;
    size_t const inputArgCount = iArgs.size();
    size_t const argCount = m_argumentNamesAndDefaultValues.size();
    SG_ASSERT_AND_UNUSED(argCount >= inputArgCount);
    std::vector<ArgumentNameAndValue> args = m_argumentNamesAndDefaultValues;
    for(size_t i = 0; i < inputArgCount; ++i)
    {
        auto const& f = std::find_if(args.begin(), args.end(), [&](ArgumentNameAndValue const& a) { return a.name.Symbol().Value() == iArgs[i].first; });
        if(f == args.end())
        {
            SG_ASSERT_NOT_REACHED(); // TODO: Error message
            return false;
        }
        else
        {
            ArgumentNameAndValue& arg = *f;
            arg.value = iArgs[i].second.get();
            // TODO: How to check that it wasn't already set ?
        }
    }
    ok = CallImplROK(io, args, iToken);
    return ok;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Template::CallImplROK(semanticTree::EvaluationInputOutput& io, std::vector<ArgumentNameAndValue> const& iArgs, Token const& iToken)
{
    SG_UNUSED(iToken);
    bool ok = true;
    SG_ASSERT(nullptr != m_metaclass);

    SG_ASSERT(io.enableObjectDefinitions);
    safeptr<reflection::BaseClass> object = m_metaclass->CreateObject();

    reflection::ObjectVisibility visibility = reflection::ObjectVisibility::Protected;
    size_t visibilityQualifierCount = 0;
    for(size_t i = 0; i < io.qualifiers.size(); ++i)
    {
        switch(io.qualifiers[i])
        {
        case TokenType::keyword_export:    ++visibilityQualifierCount; visibility = reflection::ObjectVisibility::Export;    break;
        case TokenType::keyword_public:    ++visibilityQualifierCount; visibility = reflection::ObjectVisibility::Public;    break;
        case TokenType::keyword_protected: ++visibilityQualifierCount; visibility = reflection::ObjectVisibility::Protected; break;
        case TokenType::keyword_private:   ++visibilityQualifierCount; visibility = reflection::ObjectVisibility::Private;   break;
        default:
            SG_ASSERT_NOT_REACHED(); // TODO: Error message "Incorrect qualifier for object definition"
        }
    }
    SG_ASSERT(1 >= visibilityQualifierCount); // TODO: Error message "At most one visibility qualifier is allowed for object definition"
    io.qualifiers.clear();

    reflection::Identifier objectPath(*io.pathForObjects);
    reflection::Identifier scriptPath(*io.pathForScript);
    // TODO: Check if it is ok to do that? does that make referencing outside objects possible even if we don't want?
    if(nullptr != io.objectIdentifier)
    {
        SG_ASSERT(io.objectIdentifier->Size() <= 1);
        reflection::IdentifierNode const& objectIdentifierNode = (*io.objectIdentifier)[0];
        objectPath.PushBack(objectIdentifierNode);
        scriptPath.PushBack(objectIdentifierNode);
    }
    else
    {
        reflection::IdentifierNode anonymous;
        objectPath.PushBack(anonymous);
        scriptPath.PushBack(anonymous);
    }

    io.objectDatabase->Add(visibility, objectPath, object.get());
    io.returnValue = new reflection::PrimitiveData<refptr<reflection::BaseClass> >(object.get());

    reflection::ObjectDatabaseScopedTransaction scopedTransaction(io.scriptDatabase.get());
    reflection::Identifier pathForScript = reflection::Identifier(m_identifier, reflection::IdentifierNode());
    for(auto const& it : iArgs)
    {
        reflection::ObjectVisibility const argVisibility = reflection::ObjectVisibility::Protected;
        reflection::Identifier const varName(pathForScript, it.name);
        Variable* var = new Variable(it.isConst, it.value.get());
        io.scriptDatabase->Add(argVisibility, varName, var);
    }
    size_t const instructionCount = m_instructions.size();
    for(size_t i = 0; i < instructionCount; ++i)
    {
        if(!m_instructions[i]->IsInstruction())
        {
            PushError(io, ErrorType::expression_is_not_an_instruction, m_instructions[i]->GetToken(), "expression is not an instruction");
            ok = false;
            break;
        }
        semanticTree::EvaluationInputOutput subio(io);
        subio.pathForObjects = &objectPath;
        subio.pathForScript = &pathForScript;
        subio.restrictedVariableScope = &pathForScript;
        subio.enableScriptDefinitions = true;
        subio.variablesAreRestricted = true;
        subio.enableNamespaceDefinition = false;
        subio.constructionInProgress = semanticTree::Construction::Object;
        subio.returnValue = io.returnValue;
        ok = m_instructions[i]->EvaluateROK(subio);
        if(!ok)
        {
            SG_ASSERT(io.errorHandler->DidErrorHappen());
            break;
        }
        SG_ASSERT(nullptr == subio.jumpStatement); // TODO: Error message "invalid use of break/continue/return in template body"
    }
    return ok;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, objectscript), Template)
REFLECTION_CLASS_END
//=============================================================================
TemplateNamespace::TemplateNamespace()
{
    SG_ASSERT_NOT_IMPLEMENTED();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TemplateNamespace::TemplateNamespace(std::vector<ArgumentNameAndValue>& iArgumentsAndDefaultValuesCanBeCleared)
{
    swap(m_argumentNamesAndDefaultValues, iArgumentsAndDefaultValuesCanBeCleared);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TemplateNamespace::SetBody(std::vector<refptr<semanticTree::ITreeNode> >& iInstructionsCanBeErased)
{
    SG_ASSERT(m_instructions.empty());
    swap(m_instructions, iInstructionsCanBeErased);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool TemplateNamespace::CallROK(semanticTree::EvaluationInputOutput& io, reflection::PrimitiveDataNamedList const& iArgs, Token const& iToken)
{
    bool ok = true;
    size_t const inputArgCount = iArgs.size();
    size_t const argCount = m_argumentNamesAndDefaultValues.size();
    SG_ASSERT_AND_UNUSED(argCount >= inputArgCount);
    std::vector<ArgumentNameAndValue> args = m_argumentNamesAndDefaultValues;
    for(size_t i = 0; i < inputArgCount; ++i)
    {
        auto const& f = std::find_if(args.begin(), args.end(), [&](ArgumentNameAndValue const& a) { return a.name.Symbol().Value() == iArgs[i].first; });
        if(f == args.end())
        {
            SG_ASSERT_NOT_REACHED(); // TODO: Error message
            return false;
        }
        else
        {
            ArgumentNameAndValue& arg = *f;
            arg.value = iArgs[i].second.get();
            // TODO: How to check that it wasn't already set ?
        }
    }
    ok = CallImplROK(io, args, iToken);
    return ok;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool TemplateNamespace::CallImplROK(semanticTree::EvaluationInputOutput& io, std::vector<ArgumentNameAndValue> const& iArgs, Token const& iToken)
{
    SG_UNUSED(iToken);
    bool ok = true;

    SG_ASSERT(io.enableNamespaceDefinition);
    SG_ASSERT(io.qualifiers.empty());
    SG_ASSERT(*io.pathForObjects == *io.pathForScript);
    SG_ASSERT(io.objectIdentifier->Size() == 1);
    io.returnNoValueFromTemplateNamespace = true;
    reflection::IdentifierNode const& objectIdentifierNode = (*io.objectIdentifier)[0]; // TODO: rename?
    reflection::Identifier path = *io.pathForObjects;
    path.PushBack(objectIdentifierNode);

    reflection::ObjectDatabaseScopedTransaction scopedTransaction(io.scriptDatabase.get());
    reflection::Identifier pathForScript = reflection::Identifier(m_identifier, reflection::IdentifierNode());
    for(auto const& it : iArgs)
    {
        reflection::ObjectVisibility const argVisibility = reflection::ObjectVisibility::Protected;
        reflection::Identifier const varName(pathForScript, it.name);
        Variable* var = new Variable(it.isConst, it.value.get());
        io.scriptDatabase->Add(argVisibility, varName, var);
    }
    size_t const instructionCount = m_instructions.size();
    for(size_t i = 0; i < instructionCount; ++i)
    {
        if(!m_instructions[i]->IsInstruction())
        {
            PushError(io, ErrorType::expression_is_not_an_instruction, m_instructions[i]->GetToken(), "expression is not an instruction");
            ok = false;
            break;
        }
        semanticTree::EvaluationInputOutput subio(io);
        subio.pathForObjects = &path;
        subio.pathForScript = &pathForScript;
        subio.restrictedVariableScope = &pathForScript;
        subio.enableScriptDefinitions = true;
        subio.variablesAreRestricted = true;
        subio.enableNamespaceDefinition = true;
        subio.constructionInProgress = semanticTree::Construction::None;
        subio.returnValue = io.returnValue;
        ok = m_instructions[i]->EvaluateROK(subio);
        if(!ok)
        {
            SG_ASSERT(io.errorHandler->DidErrorHappen());
            break;
        }
        SG_ASSERT(nullptr == subio.jumpStatement); // TODO: Error message "invalid use of break/continue/return in template body"
    }
    return ok;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, objectscript), TemplateNamespace)
REFLECTION_CLASS_END
//=============================================================================
TemplateAlias::TemplateAlias()
{
    SG_ASSERT_NOT_IMPLEMENTED();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TemplateAlias::TemplateAlias(std::vector<ArgumentNameAndValue>& iArgumentsAndDefaultValuesCanBeCleared)
{
    swap(m_argumentNamesAndDefaultValues, iArgumentsAndDefaultValuesCanBeCleared);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TemplateAlias::SetBody(std::vector<refptr<semanticTree::ITreeNode> >& iInstructionsCanBeErased)
{
    SG_ASSERT(m_instructions.empty());
    swap(m_instructions, iInstructionsCanBeErased);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool TemplateAlias::CallROK(semanticTree::EvaluationInputOutput& io, reflection::PrimitiveDataNamedList const& iArgs, Token const& iToken)
{
    bool ok = true;
    size_t const inputArgCount = iArgs.size();
    size_t const argCount = m_argumentNamesAndDefaultValues.size();
    SG_ASSERT_AND_UNUSED(argCount >= inputArgCount);
    std::vector<ArgumentNameAndValue> args = m_argumentNamesAndDefaultValues;
    for(size_t i = 0; i < inputArgCount; ++i)
    {
        auto const& f = std::find_if(args.begin(), args.end(), [&](ArgumentNameAndValue const& a) { return a.name.Symbol().Value() == iArgs[i].first; });
        if(f == args.end())
        {
            SG_ASSERT_NOT_REACHED(); // TODO: Error message
            return false;
        }
        else
        {
            ArgumentNameAndValue& arg = *f;
            arg.value = iArgs[i].second.get();
            // TODO: How to check that it wasn't already set ?
        }
    }
    for_range(size_t, i, 0, args.size())
    {
        if(nullptr == args[i].value)
        {
            SG_ASSERT_NOT_REACHED(); // TODO: Error message, argument needs a value
            return false;
        }
    }
    ok = CallImplROK(io, args, iToken);
    return ok;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool TemplateAlias::CallImplROK(semanticTree::EvaluationInputOutput& io, std::vector<ArgumentNameAndValue> const& iArgs, Token const& iToken)
{
    SG_UNUSED(iToken);
    bool ok = true;
    SG_ASSERT(nullptr != m_alias || nullptr != m_aliasAlias || nullptr != m_aliasNamespace);

    SG_ASSERT(io.enableObjectDefinitions);

    // NB: Not doing that here as it will be done by Template
    //reflection::Identifier objectPath(*io.pathForObjects);
    //reflection::Identifier scriptPath(*io.pathForScript);
    //if(nullptr != io.objectIdentifier)
    //{
    //    SG_ASSERT(io.objectIdentifier->Size() <= 1);
    //    reflection::IdentifierNode const& objectIdentifierNode = (*io.objectIdentifier)[0];
    //    objectPath.PushBack(objectIdentifierNode);
    //    scriptPath.PushBack(objectIdentifierNode);
    //}
    //else
    //{
    //    reflection::IdentifierNode anonymous;
    //    objectPath.PushBack(anonymous);
    //    scriptPath.PushBack(anonymous);
    //}

    semanticTree::EvaluationInputOutput blocio(io);
    reflection::PrimitiveData<reflection::PrimitiveDataNamedList>* valuesData = new reflection::PrimitiveData<reflection::PrimitiveDataNamedList>();
    blocio.returnValue = valuesData;

    reflection::ObjectDatabaseScopedTransaction scopedTransaction(io.scriptDatabase.get());
    reflection::Identifier pathForScript = reflection::Identifier(m_identifier, reflection::IdentifierNode());
    for(auto const& it : iArgs)
    {
        reflection::ObjectVisibility const argVisibility = reflection::ObjectVisibility::Protected;
        reflection::Identifier const varName(pathForScript, it.name);
        Variable* var = new Variable(it.isConst, it.value.get());
        io.scriptDatabase->Add(argVisibility, varName, var);
    }
    size_t const instructionCount = m_instructions.size();
    for(size_t i = 0; i < instructionCount; ++i)
    {
        if(!m_instructions[i]->IsInstruction())
        {
            PushError(io, ErrorType::expression_is_not_an_instruction, m_instructions[i]->GetToken(), "expression is not an instruction");
            ok = false;
            break;
        }
        semanticTree::EvaluationInputOutput subio(blocio);

        //semanticTree::EvaluationInputOutput subio(io);
        //subio.pathForObjects = &pathForScript; // TODO: check that
        subio.pathForScript = &pathForScript; // TODO: check that
        subio.restrictedVariableScope = &pathForScript;
        subio.enableScriptDefinitions = true;
        subio.variablesAreRestricted = true;
        subio.enableNamespaceDefinition = false;
        subio.constructionInProgress = semanticTree::Construction::Struct;
        subio.returnValue = blocio.returnValue;
        ok = m_instructions[i]->EvaluateROK(subio);


        //subio.enableNamespaceDefinition = false;
        //subio.constructionInProgress = semanticTree::Construction::Struct;
        //subio.returnValue = blocio.returnValue;
        //ok = m_instructions[i]->EvaluateROK(subio);
        if(!ok)
        {
            SG_ASSERT(io.errorHandler->DidErrorHappen());
            break;
        }
        //if(subio.returnValueContainsUnresolvedIdentifier)
        //    SG_BREAKPOINT();
        blocio.returnValueContainsUnresolvedIdentifier |= subio.returnValueContainsUnresolvedIdentifier;
        subio.returnValueContainsUnresolvedIdentifier = false;
    }
    if(!ok)
        return false;
    if(nullptr != m_alias)
        ok = m_alias->CallROK(io, valuesData->Get(), iToken);
    else if(nullptr != m_aliasNamespace)
        ok = m_alias->CallROK(io, valuesData->Get(), iToken);
    else if(nullptr != m_aliasAlias)
        ok = m_aliasAlias->CallROK(io, valuesData->Get(), iToken);

    blocio.returnValueContainsUnresolvedIdentifier = false;
    return ok;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, objectscript), TemplateAlias)
REFLECTION_CLASS_END
//=============================================================================
REFLECTION_CLASS_BEGIN((sg, objectscript), FreeVariable)
REFLECTION_CLASS_END
//=============================================================================
}
}

namespace sg {
namespace objectscript {
namespace semanticTree {
//=============================================================================
EvaluationInputOutput::~EvaluationInputOutput()
{
    SG_ASSERT(!returnValueContainsUnresolvedIdentifier || errorHandler->DidErrorHappen());
    SG_ASSERT(qualifiers.empty() || errorHandler->DidErrorHappen());
    if(nullptr != jumpStatement)
        PushError(*this, ErrorType::unexpected_use_of_jump_statement, jumpStatement->GetToken(), "unexpected use of jump statement");
}
//=============================================================================
bool Root::EvaluateScriptROK(EvaluationInputOutput& io)
{
    SG_ASSERT(nullptr != io.objectDatabase);
    SG_ASSERT(nullptr == io.scriptDatabase);
    Token token;
    token.fileid = io.errorHandler->GetCurrentFileId();
    SetToken(token);
    reflection::ObjectDatabase scriptDatabase(reflection::ObjectDatabase::ReferencingMode::BackwardReferenceOnly);
    io.scriptDatabase = &scriptDatabase;
    io.objectDatabase->BeginTransaction();
    bool ok = EvaluateImplROK(io);
    if(ok)
    {
        auto r = io.objectDatabase->LinkTransactionReturnInvalidDeferredProperties();
        if(!r.empty())
        {
            for(auto p : r)
                PushError(io, ErrorType::invalid_value_type_for_property, GetToken(), Format("invalid value type for property '%0' of object: %1", p.property->Name(), p.objectname.AsString()).c_str());
            io.objectDatabase->AbortTransaction();
        }
        else
        {
            io.objectDatabase->CheckTransaction();
            io.objectDatabase->EndTransaction();
        }
    }
    else
    {
        io.objectDatabase->AbortTransaction();
    }
    io.scriptDatabase = nullptr;
    return ok;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Root::EvaluateImportROK(EvaluationInputOutput& io)
{
    SG_ASSERT(nullptr == io.objectDatabase);
    SG_ASSERT(nullptr != io.scriptDatabase);
    io.enableObjectDefinitions = false;
    io.enableObjectReferences = false;
    bool ok = EvaluateImplROK(io);
    return ok;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Root::EvaluateImplROK(EvaluationInputOutput& io)
{
    bool ok = true;
    io.returnValue = nullptr;
    reflection::Identifier rootPath;
    io.pathForObjects = &rootPath;
    io.pathForScript = &rootPath;
    size_t const instructionCount = m_instructions.size();
    for(size_t i = 0; i < instructionCount; ++i)
    {
        if(!m_instructions[i]->IsInstruction())
        {
            PushError(io, ErrorType::expression_is_not_an_instruction, m_instructions[i]->GetToken(), "expression is not an instruction");
            ok = false;
            break;
        }
        if(io.enableImports && !m_instructions[i]->IsImport())
        {
            // NB: We do not begin transaction before so that each import is in
            // its own transaction.
            io.scriptDatabase->BeginTransaction();
            io.enableImports = false;
        }
        EvaluationInputOutput subio(io);
        ok = m_instructions[i]->EvaluateROK(subio);
        if(!ok)
        {
            SG_ASSERT(io.errorHandler->DidErrorHappen());
            break;
        }
        subio.returnValueContainsUnresolvedIdentifier = false;
    }
    if(!io.enableImports)
        io.scriptDatabase->LinkCheckEndTransaction();
    io.pathForObjects = nullptr;
    io.pathForScript = nullptr;
    return ok;
}
//=============================================================================
bool IntrinsicKeyword::EvaluateROK(EvaluationInputOutput& io)
{
    SG_ASSERT(io.returnIdentifier.Empty());
    SG_ASSERT(nullptr == io.returnValue);
    io.returnValue = new reflection::PrimitiveData<refptr<reflection::BaseClass>>(new IntrinsicCall(auto_initialized));
    return true;
}
//=============================================================================
bool Import::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    SG_ASSERT(!io.isPreresolutionPass);
    SG_ASSERT(nullptr == io.returnValue);
    SG_ASSERT(io.enableImports); // TODO: Error message

    EvaluationInputOutput nameio(io);
    ok = m_filename->EvaluateROK(nameio);
    if(!ok)
        return false;
    SG_ASSERT(!nameio.returnValueIsLValue);
    SG_ASSERT(!nameio.returnValueIsCommaSeparatedList);
    SG_ASSERT(!nameio.returnValueContainsUnresolvedIdentifier);
    SG_ASSERT(nameio.returnIdentifier.Empty());
    SG_ASSERT(reflection::PrimitiveDataType::String == nameio.returnValue->GetType());

    std::string str;
    nameio.returnValue->As(&str);
    FilePath const filepath = FilePath::CreateFromAnyPath(str);

    ok = io.importDatabase->ImportROK(filepath, *io.scriptDatabase, *io.imported, *io.errorHandler);
    if(!ok)
        return false;

    return true;
}
//=============================================================================
bool Value::EvaluateROK(EvaluationInputOutput& io)
{
    SG_ASSERT(io.returnIdentifier.Empty());
    SG_ASSERT(nullptr == io.returnValue);
    io.returnValue = m_value;
    SG_ASSERT(!DoesContainObjectReference(io.returnValue.get()));
    return true;
}
//=============================================================================
bool Identifier::EvaluateROK(EvaluationInputOutput& io)
{
    SG_ASSERT(io.returnIdentifier.Empty());
    SG_ASSERT(nullptr == io.returnValue);
    io.returnIdentifier = reflection::Identifier(m_identifier);
    return true;
}
//=============================================================================
bool PreresolvedIdentifierAndQualifiers::EvaluateROK(EvaluationInputOutput& io)
{
    SG_ASSERT(io.returnIdentifier.Empty());
    SG_ASSERT(nullptr == io.returnValue);
    io.returnIdentifier = m_identifier;
    SG_ASSERT(io.qualifiers.empty());
    io.qualifiers = m_qualifiers;
    return true;
}
//=============================================================================
bool Qualifier::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    SG_ASSERT(io.returnIdentifier.Empty());
    SG_ASSERT(nullptr == io.returnValue);
    SG_ASSERT(io.returnIdentifier.Empty());
    ok = m_arg->EvaluateROK(io);
    if(!ok)
        return false;
    SG_ASSERT(nullptr == io.returnValue);
    SG_ASSERT(!io.returnIdentifier.Empty());
    io.qualifiers.push_back(m_token);
    return ok;
}
//=============================================================================
bool ScopeResolution::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    SG_ASSERT(io.returnIdentifier.Empty());
    SG_ASSERT(nullptr == io.returnValue);
    SG_ASSERT(nullptr != m_args[0]);
    EvaluationInputOutput arg0io(io);
    ok = m_args[0]->EvaluateROK(arg0io);
    if(!ok)
        return false;
    SG_ASSERT(arg0io.returnValue == nullptr);
    if(nullptr == m_args[1])
    {
        if(io.isPreresolutionPass && nullptr != arg0io.presesolvedNodeIFN)
        {
            SG_ASSERT_NOT_IMPLEMENTED(); // maybe not to be supported
            ScopeResolution* preresolved = new ScopeResolution;
            preresolved->SetToken(this->GetToken());
            preresolved->SetArgument(0, arg0io.presesolvedNodeIFN.get());
            io.presesolvedNodeIFN = preresolved;
            return true;
        }
        SG_ASSERT(nullptr == io.presesolvedNodeIFN);
        SG_ASSERT(arg0io.returnIdentifier.Size() == 1);
        SG_ASSERT(arg0io.qualifiers.empty());
        io.returnIdentifier = reflection::Identifier(reflection::Identifier(), arg0io.returnIdentifier[0]);
    }
    else
    {
        EvaluationInputOutput arg1io(io);
        ok = m_args[1]->EvaluateROK(arg1io);
        if(!ok)
            return false;
        SG_ASSERT(arg1io.returnValue == nullptr);
        SG_ASSERT(arg1io.qualifiers.empty());
        if(io.isPreresolutionPass && (nullptr != arg0io.presesolvedNodeIFN || nullptr != arg1io.presesolvedNodeIFN))
        {
            SG_ASSERT_NOT_IMPLEMENTED(); // maybe not to be supported
            ScopeResolution* preresolved = new ScopeResolution;
            preresolved->SetToken(this->GetToken());
            if(nullptr != arg0io.presesolvedNodeIFN)
            {
                preresolved->SetArgument(0, arg0io.presesolvedNodeIFN.get());
                if(nullptr != arg1io.presesolvedNodeIFN)
                    preresolved->SetArgument(0, arg1io.presesolvedNodeIFN.get());
                else
                {
                    SG_ASSERT(arg1io.returnIdentifier.Size() == 1);
                    preresolved->SetArgument(1, new PreresolvedIdentifierAndQualifiers(arg1io.returnIdentifier, arg1io.qualifiers));
                }
            }
            else
            {
                SG_ASSERT(nullptr != arg1io.presesolvedNodeIFN);
                preresolved->SetArgument(0, new PreresolvedIdentifierAndQualifiers(arg0io.returnIdentifier, arg0io.qualifiers));
                preresolved->SetArgument(1, arg1io.presesolvedNodeIFN.get());
            }
            io.presesolvedNodeIFN = preresolved;
            return true;
        }
        SG_ASSERT(arg1io.returnIdentifier.Size() == 1);
        io.returnIdentifier = reflection::Identifier(arg0io.returnIdentifier, arg1io.returnIdentifier[0]);
        swap(io.qualifiers, arg0io.qualifiers);
    }
    return ok;
}
//=============================================================================
bool Identifierize::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    SG_ASSERT(io.returnIdentifier.Empty());
    SG_ASSERT(nullptr == io.returnValue);
    SG_ASSERT(nullptr != m_arg);
    EvaluationInputOutput argio(io);
    ok = m_arg->EvaluateROK(argio);
    if(!ok)
        return false;
    if(io.isPreresolutionPass && nullptr != argio.presesolvedNodeIFN)
    {
        SG_ASSERT_NOT_IMPLEMENTED(); // maybe not to be supported
        Identifierize* preresolved = new Identifierize;
        preresolved->SetToken(this->GetToken());
        preresolved->SetArgument(0, argio.presesolvedNodeIFN.get());
        io.presesolvedNodeIFN = preresolved;
        return true;
    }
    SG_ASSERT(nullptr == io.presesolvedNodeIFN);
    ConvertToValueROK(argio, m_arg->GetToken());
    SG_ASSERT(nullptr != argio.returnValue);
    if(argio.returnValue->GetType() != reflection::PrimitiveDataType::String)
    {
        PushError(io, ErrorType::unsupported_type_for_identifierize_operator, m_arg->GetToken(), "unsupported type for identifierize operator");
        return false;
    }
    std::string name;
    argio.returnValue->As(&name);
    io.returnIdentifier = reflection::Identifier(name);
    return true;
}
//=============================================================================
bool ObjectDefinition::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    EvaluationInputOutput nameio(io);
    ok = m_name->EvaluateROK(nameio);
    if(!ok)
        return false;
    SG_ASSERT(nullptr == nameio.returnValue);
    SG_ASSERT(nameio.returnIdentifier.Size() == 1 || io.isPreresolutionPass); // TODO: error message
    if(nullptr == nameio.presesolvedNodeIFN)
    {
        ResolvedIdentifier resolved;
        ResolveIdentifier(io, nameio.returnIdentifier, resolved);
        if(ResolvedIdentifier::Type::None != resolved.type)
        {
            switch(resolved.type)
            {
            case ResolvedIdentifier::Type::Metaclass:
                PushError(io, ErrorType::object_name_collision_with_class, GetToken(), "object name conflicts with a class name");
                return false;
            case ResolvedIdentifier::Type::ScriptObject:
                PushError(io, ErrorType::object_name_collision_with_script_object, GetToken(), "object name conflicts with script entity");
                return false;
            default:
                SG_ASSERT_NOT_REACHED();
            }
        }
        // TODO: Error message "Object name conflicts with existing class/variable/function/..."
    }

    if(io.isPreresolutionPass)
    {
        EvaluationInputOutput objectio(io);
        // no object identifier as it may have no sense at preresolution.
        ok = m_object->EvaluateROK(objectio);
        if(!ok)
            return false;
        ObjectDefinition* preresolved = new ObjectDefinition;
        preresolved->SetToken(this->GetToken());
        if(nullptr == nameio.presesolvedNodeIFN)
            preresolved->SetArgument(0, new PreresolvedIdentifierAndQualifiers(nameio.returnIdentifier, nameio.qualifiers));
        else
            preresolved->SetArgument(0, nameio.presesolvedNodeIFN.get());
        if(nullptr != objectio.presesolvedNodeIFN)
            preresolved->SetArgument(1, objectio.presesolvedNodeIFN.get());
        else
        {
            SG_ASSERT(nullptr != objectio.returnValue);
            SG_ASSERT(reflection::PrimitiveDataType::NamedList == objectio.returnValue->GetType());
            preresolved->SetArgument(1, new Value(objectio.returnValue.get()));
        }
        io.presesolvedNodeIFN = preresolved;
        return true;
    }

    reflection::IdentifierNode const& objectIdentifierNode = nameio.returnIdentifier[0];
    reflection::Identifier objectPath(*io.pathForObjects, objectIdentifierNode);
    if(io.objectDatabase->IsIdentifierUsed(objectPath))
    {
        PushError(io, ErrorType::object_name_collision, GetToken(), "an object with same name is already defined");
        return false;
    }
    EvaluationInputOutput objectio(io);
    objectio.objectIdentifier = &nameio.returnIdentifier;
    swap(objectio.qualifiers, nameio.qualifiers);
    ok = m_object->EvaluateROK(objectio);
    if(!ok)
        return false;
    if(objectio.returnNoValueFromTemplateNamespace)
    {
        io.returnNoValueFromTemplateNamespace = true;
        return true;
    }
    if(nullptr == objectio.returnValue || objectio.returnValue->GetType() != reflection::PrimitiveDataType::Object)
    {
        PushError(io, ErrorType::object_definition_expects_an_object, GetToken(), "objects definition expects an object");
        return false;
    }
    io.returnValue = objectio.returnValue;
    SG_ASSERT(!objectio.returnValueContainsUnresolvedIdentifier);
    return ok;
}
//=============================================================================
bool Object::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    if(!io.enableObjectDefinitions)
    {
        PushError(io, ErrorType::object_definition_forbidden_in_context, GetToken(), "object definition is forbidden in this context");
        return false;
    }
    SG_ASSERT(io.enableObjectDefinitions); // TODO: Error message
    EvaluationInputOutput typeio(io);
    ok = m_type->EvaluateROK(typeio);
    if(!ok)
        return false;
    // TODO: the following assert fires when forgetting a comma in a list
    SG_ASSERT(nullptr == typeio.returnValue);
    SG_ASSERT(!typeio.returnIdentifier.Empty() || io.isPreresolutionPass);
    SG_ASSERT(typeio.qualifiers.empty() || nullptr == io.objectIdentifier);
    reflection::Metaclass const* mc = nullptr;
    bool isTemplate = false;
    if(nullptr == typeio.presesolvedNodeIFN)
    {
        ResolvedIdentifier resolved;
        ResolveIdentifier(io, typeio.returnIdentifier, resolved);
        switch(resolved.type)
        {
        case ResolvedIdentifier::Type::None:
            PushError(io, ErrorType::class_not_found, m_type->GetToken(), "class not found");
            return false;
        case ResolvedIdentifier::Type::Metaclass:
            mc = resolved.metaclass;
            break;
        case ResolvedIdentifier::Type::ScriptObject:
        {
            reflection::BaseClass* scriptObject = resolved.scriptObject.get();
            SG_ASSERT(nullptr != scriptObject);
            if(io.isPreresolutionPass)
            {
                if(scriptObject->GetMetaclass() == Template::StaticGetMetaclass() || scriptObject->GetMetaclass() == TemplateNamespace::StaticGetMetaclass() || scriptObject->GetMetaclass() == TemplateAlias::StaticGetMetaclass())
                {
                    typeio.returnIdentifier = resolved.fullIdentifier;
                    isTemplate = true;
                    break;
                }
                else if(scriptObject->GetMetaclass() == TemplateNamespace::StaticGetMetaclass())
                {
                    SG_ASSERT(false); // todo PushError();
                }
            }
            SG_ASSERT(!io.isPreresolutionPass); // not impl
            if(scriptObject->GetMetaclass() == Template::StaticGetMetaclass() || scriptObject->GetMetaclass() == TemplateNamespace::StaticGetMetaclass() || scriptObject->GetMetaclass() == TemplateAlias::StaticGetMetaclass())
            {
                EvaluationInputOutput blocio(io);
                reflection::PrimitiveData<reflection::PrimitiveDataNamedList>* valuesData = new reflection::PrimitiveData<reflection::PrimitiveDataNamedList>();
                blocio.returnValue = valuesData;

                size_t const instructionCount = m_instructions.size();
                for(size_t i = 0; i < instructionCount; ++i)
                {
                    if(!m_instructions[i]->IsInstruction())
                    {
                        PushError(io, ErrorType::expression_is_not_an_instruction, m_instructions[i]->GetToken(), "expression is not an instruction");
                        ok = false;
                        break;
                    }
                    EvaluationInputOutput subio(blocio);
                    subio.enableNamespaceDefinition = false;
                    subio.constructionInProgress = Construction::Struct;
                    subio.returnValue = blocio.returnValue;
                    ok = m_instructions[i]->EvaluateROK(subio);
                    if(!ok)
                    {
                        SG_ASSERT(io.errorHandler->DidErrorHappen());
                        break;
                    }
                    //if(subio.returnValueContainsUnresolvedIdentifier)
                    //    SG_BREAKPOINT();
                    blocio.returnValueContainsUnresolvedIdentifier |= subio.returnValueContainsUnresolvedIdentifier;
                    subio.returnValueContainsUnresolvedIdentifier = false;
                }
                if(!ok)
                    return false;
                if(scriptObject->GetMetaclass() == Template::StaticGetMetaclass())
                {
                    Template* tpl = checked_cast<Template*>(scriptObject);
                    ok = tpl->CallROK(io, valuesData->Get(), GetToken());
                }
                else if(scriptObject->GetMetaclass() == TemplateNamespace::StaticGetMetaclass())
                {
                    TemplateNamespace* tpl = checked_cast<TemplateNamespace*>(scriptObject);
                    ok = tpl->CallROK(io, valuesData->Get(), GetToken());
                }
                else
                {
                    SG_ASSERT(scriptObject->GetMetaclass() == TemplateAlias::StaticGetMetaclass());
                    TemplateAlias* tpl = checked_cast<TemplateAlias*>(scriptObject);
                    ok = tpl->CallROK(io, valuesData->Get(), GetToken());
                }
                blocio.returnValueContainsUnresolvedIdentifier = false;
                return ok;
            }
            else
            {
                SG_ASSERT_NOT_REACHED(); // TODO: Error message
            }
            break;
        }
        case ResolvedIdentifier::Type::FreeVariable:
            SG_ASSERT_NOT_REACHED();
            break;
        default:
            SG_ASSERT_NOT_REACHED();
        }
    }

    SG_ASSERT(nullptr != mc || io.isPreresolutionPass);
    SG_ASSERT(nullptr != mc || (io.isPreresolutionPass && isTemplate)); // even in preresolution pass, as class must be completely defined (no Identifierize)
    safeptr<reflection::BaseClass> object = io.isPreresolutionPass ? nullptr : mc->CreateObject();

    reflection::ObjectVisibility visibility = reflection::ObjectVisibility::Protected;
    size_t visibilityQualifierCount = 0;

    if(nullptr == io.objectIdentifier)
        swap(io.qualifiers, typeio.qualifiers);
    for(size_t i = 0; i < io.qualifiers.size(); ++i)
    {
        switch(io.qualifiers[i])
        {
        case TokenType::keyword_export:    ++visibilityQualifierCount; visibility = reflection::ObjectVisibility::Export;    break;
        case TokenType::keyword_public:    ++visibilityQualifierCount; visibility = reflection::ObjectVisibility::Public;    break;
        case TokenType::keyword_protected: ++visibilityQualifierCount; visibility = reflection::ObjectVisibility::Protected; break;
        case TokenType::keyword_private:   ++visibilityQualifierCount; visibility = reflection::ObjectVisibility::Private;   break;
        default:
            SG_ASSERT_NOT_REACHED(); // TODO: Error message "Incorrect qualifier for object definition"
        }
    }
    SG_ASSERT(1 >= visibilityQualifierCount); // TODO: Error message "At most one visibility qualifier is allowed for object definition"
    io.qualifiers.clear();

    if(io.isPreresolutionPass)
    {
        SG_ASSERT(nullptr == io.pathForObjects);
        reflection::Identifier scriptPath(*io.pathForScript);
        if(nullptr != io.objectIdentifier)
        {
            SG_ASSERT(io.objectIdentifier->Size() <= 1);
            reflection::IdentifierNode const& objectIdentifierNode = (*io.objectIdentifier)[0];
            scriptPath.PushBack(objectIdentifierNode);
        }
        else
        {
            reflection::IdentifierNode anonymous;
            scriptPath.PushBack(anonymous);
        }

        Object* preresolved = new Object;
        preresolved->SetToken(this->GetToken());

        if(nullptr != typeio.presesolvedNodeIFN)
        {
            SG_ASSERT(nullptr == typeio.returnValue);
            preresolved->SetArgument(0, typeio.presesolvedNodeIFN.get());
        }
        else
        {
            SG_ASSERT(!typeio.returnIdentifier.Empty());
            SG_ASSERT(nullptr == io.returnValue);
            SG_ASSERT(!io.returnValueContainsUnresolvedIdentifier);
            PreresolvedIdentifierAndQualifiers* preresolvedType = new PreresolvedIdentifierAndQualifiers(typeio.returnIdentifier);
            preresolved->SetArgument(0, preresolvedType);
        }

        size_t const instructionCount = m_instructions.size();
        std::vector<refptr<ITreeNode>> preresolvedSubInstructions;
        preresolvedSubInstructions.reserve(instructionCount);
        for(size_t i = 0; i < instructionCount; ++i)
        {
            if(!m_instructions[i]->IsInstruction())
            {
                PushError(io, ErrorType::expression_is_not_an_instruction, m_instructions[i]->GetToken(), "expression is not an instruction");
                ok = false;
                break;
            }
            EvaluationInputOutput subio(io);
            subio.enableNamespaceDefinition = false;
            subio.constructionInProgress = Construction::Object;
            subio.pathForObjects = nullptr; //&objectPath;
            subio.pathForScript = &scriptPath;
            ok = m_instructions[i]->EvaluateROK(subio);
            if(!ok)
            {
                SG_ASSERT(io.errorHandler->DidErrorHappen());
                break;
            }
            SG_ASSERT(nullptr != subio.presesolvedNodeIFN);
            preresolvedSubInstructions.push_back(subio.presesolvedNodeIFN.get());
        }
        preresolved->SetSubNodes(preresolvedSubInstructions, io.errorHandler.get());
        io.presesolvedNodeIFN = preresolved;
        return ok;
    }

    reflection::Identifier objectPath(*io.pathForObjects);
    reflection::Identifier scriptPath(*io.pathForScript);
    if(nullptr != io.objectIdentifier)
    {
        SG_ASSERT(io.objectIdentifier->Size() <= 1);
        reflection::IdentifierNode const& objectIdentifierNode = (*io.objectIdentifier)[0];
        objectPath.PushBack(objectIdentifierNode);
        scriptPath.PushBack(objectIdentifierNode);
    }
    else
    {
        reflection::IdentifierNode anonymous;
        objectPath.PushBack(anonymous);
        scriptPath.PushBack(anonymous);
    }


    io.objectDatabase->Add(visibility, objectPath, object.get());
    io.returnValue = new reflection::PrimitiveData<refptr<reflection::BaseClass> >(object.get());

    size_t const instructionCount = m_instructions.size();
    for(size_t i = 0; i < instructionCount; ++i)
    {
        if(!m_instructions[i]->IsInstruction())
        {
            PushError(io, ErrorType::expression_is_not_an_instruction, m_instructions[i]->GetToken(), "expression is not an instruction");
            ok = false;
            break;
        }
        EvaluationInputOutput subio(io);
        subio.enableNamespaceDefinition = false;
        subio.constructionInProgress = Construction::Object;
        subio.returnValue = io.returnValue;
        subio.pathForObjects = &objectPath;
        subio.pathForScript = &scriptPath;
        ok = m_instructions[i]->EvaluateROK(subio);
        if(!ok)
        {
            SG_ASSERT(io.errorHandler->DidErrorHappen());
            break;
        }
        SG_ASSERT(!subio.returnValueContainsUnresolvedIdentifier);
    }
    return ok;
}
//=============================================================================
bool Struct::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    SG_ASSERT(io.returnValue == nullptr);
    SG_ASSERT(io.returnIdentifier.Size() == 0);
    SG_ASSERT(io.qualifiers.empty());

    if(io.isPreresolutionPass)
    {
        EvaluationInputOutput subio(io);
        subio.enableNamespaceDefinition = false;
        Struct* preresolved = new Struct;
        preresolved->SetToken(this->GetToken());
        std::vector<refptr<ITreeNode>> preresolvedIstructions;
        ok = PreresolveInstructionROK(subio, *io.pathForScript, m_instructions, preresolvedIstructions);
        if(!ok)
            return false;
        preresolved->SetSubNodes(preresolvedIstructions, io.errorHandler.get());
        io.presesolvedNodeIFN = preresolved;
        return true;
    }

    io.returnValue = new reflection::PrimitiveData<reflection::PrimitiveDataNamedList>();

    size_t const instructionCount = m_instructions.size();
    for(size_t i = 0; i < instructionCount; ++i)
    {
        if(!m_instructions[i]->IsInstruction())
        {
            PushError(io, ErrorType::expression_is_not_an_instruction, m_instructions[i]->GetToken(), "expression is not an instruction");
            ok = false;
            break;
        }
        EvaluationInputOutput subio(io);
        subio.enableNamespaceDefinition = false;
        subio.constructionInProgress = Construction::Struct;
        subio.returnValue = io.returnValue;
        ok = m_instructions[i]->EvaluateROK(subio);
        if(!ok)
        {
            SG_ASSERT(io.errorHandler->DidErrorHappen());
            break;
        }
        io.returnValueContainsUnresolvedIdentifier |= subio.returnValueContainsUnresolvedIdentifier;
        subio.returnValueContainsUnresolvedIdentifier = false;
    }
    return ok;
}
//=============================================================================
bool List::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    SG_ASSERT(io.returnValue == nullptr);
    SG_ASSERT(io.returnIdentifier.Size() == 0);
    SG_ASSERT(io.qualifiers.empty());
    if(nullptr == m_values)
    {
        if(io.isPreresolutionPass)
            io.presesolvedNodeIFN = this;
        else
            io.returnValue = new reflection::PrimitiveData<reflection::PrimitiveDataList>();
    }
    else
    {
        EvaluationInputOutput subio(io);
        subio.enableComma = true;
        subio.enableScriptDefinitions = false;
        SG_ASSERT(nullptr == subio.returnValue);
        ok = m_values->EvaluateROK(subio);
        if(!ok)
            return false;
        ok = ConvertToValueROK(subio, m_values->GetToken());
        if(!ok)
            return false;
        if(io.isPreresolutionPass && nullptr != subio.presesolvedNodeIFN)
        {
            List* preresolved = new List;
            preresolved->SetToken(this->GetToken());
            std::vector<refptr<ITreeNode>> subNodes(1, subio.presesolvedNodeIFN.get());
            preresolved->SetSubNodes(subNodes, io.errorHandler.get());
            io.presesolvedNodeIFN = preresolved;
            return true;
        }

        if(subio.returnValueIsCommaSeparatedList)
        {
            SG_ASSERT(nullptr != subio.returnValue);
            io.returnValue = subio.returnValue;
            io.returnValueContainsUnresolvedIdentifier = subio.returnValueContainsUnresolvedIdentifier;
            subio.returnValueContainsUnresolvedIdentifier = false;
        }
        else
        {
            SG_ASSERT(nullptr != subio.returnValue);
            reflection::PrimitiveData<reflection::PrimitiveDataList>* list = new reflection::PrimitiveData<reflection::PrimitiveDataList>();
            list->GetForWriting().emplace_back(subio.returnValue.get());
            io.returnValue = list;
            io.returnValueContainsUnresolvedIdentifier = subio.returnValueContainsUnresolvedIdentifier;
            subio.returnValueContainsUnresolvedIdentifier = false;
        }
    }
    return ok;
}
//=============================================================================
bool Parenthesis::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    if(io.isPreresolutionPass) { SG_ASSERT_NOT_IMPLEMENTED(); }
    SG_ASSERT(io.returnValue == nullptr);
    SG_ASSERT(io.returnIdentifier.Size() == 0);
    SG_ASSERT(io.qualifiers.empty());

    EvaluationInputOutput subio(io);
    subio.enableComma = false;
    subio.enableScriptDefinitions = false;
    SG_ASSERT(nullptr == subio.returnValue);
    ok = m_expression->EvaluateROK(subio);
    if(!ok)
        return false;
    ok = ConvertToValueROK(subio, m_expression->GetToken());
    if(!ok)
        return false;

    SG_ASSERT(nullptr != subio.returnValue);
    SG_ASSERT(!subio.returnValueIsCommaSeparatedList);
    io.returnValue = subio.returnValue;
    io.returnValueContainsUnresolvedIdentifier =  subio.returnValueContainsUnresolvedIdentifier;
    subio.returnValueContainsUnresolvedIdentifier = false;
    return ok;
}
//=============================================================================
bool Indexing::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    SG_ASSERT(io.returnValue == nullptr);
    SG_ASSERT(io.returnIdentifier.Size() == 0);
    SG_ASSERT(io.qualifiers.empty());

    EvaluationInputOutput indexedio(io);
    indexedio.enableComma = false;
    indexedio.enableScriptDefinitions = false;
    ok = m_indexed->EvaluateROK(indexedio);
    if(!ok)
        return false;
    ok = ConvertToValueROK(indexedio, m_indexed->GetToken());
    if(!ok)
        return false;
    EvaluationInputOutput indexio(io);
    indexio.enableComma = false;
    indexio.enableScriptDefinitions = false;
    SG_ASSERT(nullptr == indexio.returnValue);
    ok = m_index->EvaluateROK(indexio);
    if(!ok)
        return false;
    ok = ConvertToValueROK(indexio, m_index->GetToken());
    if(!ok)
        return false;

    if(io.isPreresolutionPass && (nullptr != indexedio.presesolvedNodeIFN || nullptr != indexio.presesolvedNodeIFN))
    {
        Indexing* preresolved = new Indexing();
        preresolved->SetToken(this->GetToken());
        {
            refptr<ITreeNode> indexed;
            ok = ConvertToPreresolvedROK(indexedio, indexed, m_indexed->GetToken());
            if(!ok)
                return false;
            preresolved->SetArgument(0, indexed.get());
        }
        {
            refptr<ITreeNode> index;
            ok = ConvertToPreresolvedROK(indexio, index, m_index->GetToken());
            if(!ok)
                return false;
            preresolved->SetIndex(index.get());
        }
        io.presesolvedNodeIFN = preresolved;
        return true;
    }

    switch(indexedio.returnValue->GetType())
    {
    case reflection::PrimitiveDataType::List:
        {
            size_t index = all_ones;
            switch(indexio.returnValue->GetType())
            {
            case reflection::PrimitiveDataType::Int32:
            case reflection::PrimitiveDataType::UInt32:
                index = indexio.returnValue->As<u32>();
                break;
            default:
                SG_ASSERT_NOT_REACHED(); // TODO: Error message
            }
            reflection::PrimitiveData<reflection::PrimitiveDataList>* datalist = checked_cast<reflection::PrimitiveData<reflection::PrimitiveDataList>* >(indexedio.returnValue.get());
            reflection::PrimitiveDataList& list = datalist->GetForWriting();
            SG_ASSERT_MSG(index < list.size(), "index out of range");
            io.returnValue = list[index];
            io.returnValueIsLValue = indexedio.returnValueIsLValue;
            if(indexedio.returnValueIsLValue)
                io.returnLValueReference = &(list[index]);
            io.returnValueContainsUnresolvedIdentifier = indexedio.returnValueContainsUnresolvedIdentifier;
            indexedio.returnValueContainsUnresolvedIdentifier = false;
        }
        break;
    default:
        SG_ASSERT_NOT_REACHED(); // TODO: Error message
    }
    return ok;
}
//=============================================================================
bool PropertyAffectation::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    SG_ASSERT(nullptr != io.returnValue || io.isPreresolutionPass);
    EvaluationInputOutput leftio(io);
    ok = m_left->EvaluateROK(leftio);
    if(!ok)
        return false;
    SG_ASSERT(leftio.returnIdentifier.Size() == 1);
    SG_ASSERT(leftio.qualifiers.empty());
    std::string const propertyName = leftio.returnIdentifier[0].Symbol().Value();
    EvaluationInputOutput rightio(io);
    ok = m_right->EvaluateROK(rightio);
    if(!ok)
        return false;
    ok = ConvertToValueROK(rightio, m_right->GetToken());
    if(!ok)
        return false;

    if(io.isPreresolutionPass)
    {
        PropertyAffectation* preresolved = new PropertyAffectation();
        preresolved->SetToken(this->GetToken());
        if(nullptr != leftio.presesolvedNodeIFN)
            preresolved->SetArgument(0, leftio.presesolvedNodeIFN.get());
        else
            preresolved->SetArgument(0, new PreresolvedIdentifierAndQualifiers(leftio.returnIdentifier, leftio.qualifiers));
        refptr<ITreeNode> rightPreresolved;
        ok = ConvertToPreresolvedROK(rightio, rightPreresolved, m_right->GetToken());
        if(!ok)
            return false;
        preresolved->SetArgument(1, rightPreresolved.get());
        io.presesolvedNodeIFN = preresolved;
        return ok;
    }

    switch(io.constructionInProgress)
    {
    case Construction::Object:
    {
        refptr<reflection::BaseClass> object;
        io.returnValue->As<refptr<reflection::BaseClass> >(&object);
        SG_ASSERT(nullptr != object);
        reflection::IProperty const* property = object->GetMetaclass()->GetPropertyIFP(propertyName.c_str());
        if(nullptr == property)
        {
            PushError(rightio, ErrorType::unknown_property_name, m_left->GetToken(), Format("unknown property name \"%0\"", propertyName).c_str());
            return false;
        }
        if(!rightio.returnValueContainsUnresolvedIdentifier)
        {
            ok = property->SetROK(object.get(), rightio.returnValue.get());
            SG_ASSERT_AND_UNUSED(ok); // TODO: Error message "invalid type for property"
        }
        else
        {
            io.objectDatabase->AddDeferredProperty(*io.pathForObjects, property, rightio.returnValue.get());
            rightio.returnValueContainsUnresolvedIdentifier = false;
        }
        break;
    }
    case Construction::Struct:
    {
        reflection::PrimitiveData<reflection::PrimitiveDataNamedList>* data = checked_cast<reflection::PrimitiveData<reflection::PrimitiveDataNamedList>*>(io.returnValue.get());
        reflection::PrimitiveDataNamedList& strct = data->GetForWriting();
        strct.emplace_back(propertyName, rightio.returnValue.get());
        io.returnValueContainsUnresolvedIdentifier = io.returnValueContainsUnresolvedIdentifier || rightio.returnValueContainsUnresolvedIdentifier;
        rightio.returnValueContainsUnresolvedIdentifier = false;
        break;
    }
    default:
        SG_ASSERT_NOT_REACHED(); // TODO: Error message
    }
    return ok;
}
//=============================================================================
bool Assignment::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    EvaluationInputOutput leftio(io);
    ok = m_left->EvaluateROK(leftio);
    if(!ok)
        return false;
    EvaluationInputOutput rightio(io);
    ok = m_right->EvaluateROK(rightio);
    if(!ok)
        return false;
    ok = ConvertToValueROK(rightio, m_right->GetToken());
    if(!ok)
        return false;

    bool isDeclaration = false;
    bool isConstDeclaration = false;
    if(nullptr == leftio.presesolvedNodeIFN && !leftio.returnIdentifier.Empty())
    {
        ResolvedIdentifier resolved;
        ResolveIdentifier(io, leftio.returnIdentifier, resolved);
        switch(resolved.type)
        {
        case ResolvedIdentifier::Type::None:
        {
            SG_ASSERT(leftio.returnIdentifier.Size() == 1); // TODO: Error message "Can not declare variable outside of current namespace"
            isDeclaration = true;
            size_t constnessQualifierCount = 0;
            for(size_t i = 0; i < leftio.qualifiers.size(); ++i)
            {
                switch(leftio.qualifiers[i])
                {
                case TokenType::keyword_const: ++constnessQualifierCount; isConstDeclaration = true; break;
                case TokenType::keyword_var: ++constnessQualifierCount; isConstDeclaration = false; break;
                default:
                    PushError(io, ErrorType::incorrect_use_of_qualifier_for_variable_definition, GetToken(), "incorrect use of qualifier for variable defintion");
                    break;
                }
            }
            if(0 == constnessQualifierCount)
            {
                PushError(io, ErrorType::variable_declaration_needs_constness_qualifier, GetToken(), "variable declaration needs either const or var qualifier");
                return false;
            }
            if(1 < constnessQualifierCount)
            {
                PushError(io, ErrorType::incorrect_use_of_qualifier_for_variable_definition, GetToken(), "too many qualifier for variable definition");
                return false;
            }
            SG_ASSERT(1 >= constnessQualifierCount); // TODO: Error message "At most one constness qualifier is allowed for variable definition"
            if(!isConstDeclaration && !io.enableNonConstVariableDefinitions)
            {
                PushError(io, ErrorType::non_const_variable_definition_in_imported_file, GetToken(), "can not declare non-const variable in imported file");
                return false;
            }
            break;
        }
        case ResolvedIdentifier::Type::Metaclass:
            PushError(io, ErrorType::assign_to_class, GetToken(), "can not assign to a class");
            return false;
        case ResolvedIdentifier::Type::ScriptObject:
        {
            reflection::BaseClass* object = resolved.scriptObject.get();
            SG_ASSERT(nullptr != object);
            if(!leftio.qualifiers.empty())
            {
                if(resolved.fullIdentifier.ParentNamespace() == *io.pathForScript)
                {
                    PushError(io, ErrorType::name_already_defined, GetToken(), "this name conflicts with a previous definition");
                    leftio.qualifiers.clear();
                    return false;
                }
                else
                {
                    PushError(io, ErrorType::name_already_defined_in_outer_scope, GetToken(), "this name conflicts with a definition in an outer scope or namespace");
                    leftio.qualifiers.clear();
                    return false;
                }
            }
            SG_ASSERT(object->GetMetaclass() == Variable::StaticGetMetaclass());
            Variable* var = checked_cast<Variable*>(object);
            SG_ASSERT_MSG_AND_UNUSED(!var->IsConst(), "Can not modify const variable");
            SG_ASSERT(!rightio.returnValueIsCommaSeparatedList);
            SG_ASSERT(!rightio.returnValueContainsUnresolvedIdentifier); // TODO
            break;
        }
        case ResolvedIdentifier::Type::FreeVariable:
        {
            reflection::BaseClass* object = resolved.scriptObject.get();
            SG_ASSERT(nullptr != object);
            SG_ASSERT_MSG(leftio.qualifiers.empty(), "Qualifiers are not allowed on already declared variables");
            SG_ASSERT(object->GetMetaclass() == FreeVariable::StaticGetMetaclass());
            FreeVariable* var = checked_cast<FreeVariable*>(object);
            SG_ASSERT_MSG_AND_UNUSED(!var->IsConst(), "Can not modify const variable");
            SG_ASSERT(!rightio.returnValueIsCommaSeparatedList);
            SG_ASSERT(!rightio.returnValueContainsUnresolvedIdentifier); // TODO
            break;
        }
        default:
            SG_ASSERT_NOT_REACHED();
        }
    }

    if(io.isPreresolutionPass)
    {
        if(nullptr == leftio.presesolvedNodeIFN && nullptr == rightio.presesolvedNodeIFN && isConstDeclaration)
        {
            // Let's use standard path. This will replace the const variable by
            // its value at each use location.
        }
        else
        {
            if(isDeclaration)
            {
                reflection::ObjectVisibility visibility = reflection::ObjectVisibility::Public;
                SG_ASSERT(leftio.returnIdentifier.Size() == 1);
                SG_ASSERT(!leftio.returnIdentifier.IsAbsolute());
                reflection::Identifier varName(*io.pathForScript, leftio.returnIdentifier[0]);
                FreeVariable* var = new FreeVariable(isConstDeclaration);
                io.scriptDatabase->Add(visibility, varName, var);
            }
            Assignment* preresolved = new Assignment();
            preresolved->SetToken(this->GetToken());
            if(nullptr != leftio.presesolvedNodeIFN)
                preresolved->SetArgument(0, leftio.presesolvedNodeIFN.get());
            else
                preresolved->SetArgument(0, new PreresolvedIdentifierAndQualifiers(leftio.returnIdentifier, leftio.qualifiers));
            refptr<ITreeNode> rightPreresolved;
            ok = ConvertToPreresolvedROK(rightio, rightPreresolved, m_right->GetToken());
            if(!ok)
                return false;
            preresolved->SetArgument(1, rightPreresolved.get());
            io.presesolvedNodeIFN = preresolved;
            return ok;
        }
    }

    if(rightio.returnValue->GetType() == reflection::PrimitiveDataType::Object)
    {
        // TODO: How this case should be handled? Is it like assigning a
        // variable on a reference? Is the object created in database? What if
        // it is anonymous? What if it has a name?
        SG_ASSERT_NOT_IMPLEMENTED();
    }
    refptr<reflection::IPrimitiveData> valueCopy;
    rightio.returnValue->CopyTo(valueCopy);

    if(!leftio.returnIdentifier.Empty())
    {
        ResolvedIdentifier resolved;
        ResolveLValueIdentifier(io, leftio.returnIdentifier, resolved);
        switch(resolved.type)
        {
        case ResolvedIdentifier::Type::None:
            {
                reflection::ObjectVisibility visibility = reflection::ObjectVisibility::Public;
                reflection::Identifier varName(*io.pathForScript, leftio.returnIdentifier[0]);
                Variable* var = new Variable(isConstDeclaration, valueCopy.get());
                leftio.qualifiers.clear();
                io.scriptDatabase->Add(visibility, varName, var);
            }
            break;
        case ResolvedIdentifier::Type::Metaclass: SG_ASSUME_NOT_REACHED();
        case ResolvedIdentifier::Type::ScriptObject:
            {
                reflection::BaseClass* object = resolved.scriptObject.get();
                SG_ASSERT(nullptr != object);
                SG_ASSERT_MSG(leftio.qualifiers.empty(), "Qualifiers are not allowed on already declared variables");
                SG_ASSERT(object->GetMetaclass() == Variable::StaticGetMetaclass());
                Variable* var = checked_cast<Variable*>(object);
                SG_ASSERT_MSG(!var->IsConst(), "Can not modify const variable");
                SG_ASSERT(!rightio.returnValueIsCommaSeparatedList);
                SG_ASSERT(!rightio.returnValueContainsUnresolvedIdentifier); // TODO
                var->SetValue(valueCopy.get());

                io.returnValue = var->Value();
                io.returnValueIsLValue = true;
                io.returnLValueReference = var->ValueReference();
            }
            break;
        case ResolvedIdentifier::Type::FreeVariable: SG_ASSUME_NOT_REACHED();
        default:
            SG_ASSERT_NOT_REACHED();
        }
    }
    else
    {
        ok = ConvertToValueROK(leftio, m_left->GetToken());
        if(!ok)
            return false;
        SG_ASSERT(leftio.returnValueIsLValue); // TODO: Error message
        SG_ASSERT(nullptr != leftio.returnLValueReference);
        *(leftio.returnLValueReference) = valueCopy;

        io.returnValue = leftio.returnValue;
        io.returnValueIsLValue = leftio.returnValueIsLValue;
        io.returnLValueReference = leftio.returnLValueReference;
    }
    rightio.returnValueContainsUnresolvedIdentifier = false;
    return ok;
}
//=============================================================================
IncDecRement::IncDecRement(OperatorTraits const& iTraits)
    : m_arg()
    , m_token(iTraits.op)
    , m_isSuffix(Arrity::SuffixUnary == iTraits.arrity)
{}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool IncDecRement::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    SG_ASSERT(nullptr == io.returnValue);
    EvaluationInputOutput argio(io);
    ok = m_arg->EvaluateROK(argio);
    if(!ok)
        return false;
    ok = ConvertToValueROK(argio, m_arg->GetToken());
    if(!ok)
        return false;
    if(io.isPreresolutionPass)
    {
        IncDecRement* preresolved = new IncDecRement(m_token, m_isSuffix);
        preresolved->SetToken(this->GetToken());
        refptr<ITreeNode> arg;
        ok = ConvertToPreresolvedROK(argio, arg, m_arg->GetToken());
        if(!ok)
            return false;
        preresolved->SetArgument(0, arg.get());
        io.presesolvedNodeIFN = preresolved;
        return true;
    }
    SG_ASSERT(argio.returnValueIsLValue);
    switch(argio.returnValue->GetType())
    {
    case reflection::PrimitiveDataType::Null:
    case reflection::PrimitiveDataType::Boolean:
        SG_ASSERT_NOT_IMPLEMENTED();
    case reflection::PrimitiveDataType::Int32:
        {
            if(m_isSuffix)
                argio.returnValue->CopyTo(io.returnValue);
            reflection::PrimitiveData<i32>* data = checked_cast<reflection::PrimitiveData<i32>*>(argio.returnValue.get());
            switch(m_token)
            {
            case TokenType::operator_plus_plus:
                ++(data->GetForWriting());
                break;
            case TokenType::operator_minus_minus:
                --(data->GetForWriting());
                break;
            default:
                SG_ASSERT_NOT_REACHED();
            }
            if(!m_isSuffix)
                argio.returnValue->CopyTo(io.returnValue);
        }
        break;
    case reflection::PrimitiveDataType::UInt32:
        SG_ASSERT_NOT_REACHED();
        break;
    case reflection::PrimitiveDataType::Float:
    case reflection::PrimitiveDataType::String:
    case reflection::PrimitiveDataType::List:
    case reflection::PrimitiveDataType::NamedList:
    case reflection::PrimitiveDataType::Object:
        PushError(io, ErrorType::unsupported_types_for_increment_operator, GetToken(), "operator does not support operand type");
        return false;
    case reflection::PrimitiveDataType::ObjectReference:
        SG_ASSERT_NOT_REACHED();
        return false;
    default:
        SG_ASSERT_NOT_REACHED();
    }
    return ok;
}
//=============================================================================
bool Comma::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    if(nullptr == m_args[1])
    {
        //trailing comma
        ok = m_args[0]->EvaluateROK(io);
        if(!ok)
            return false;
        return ok;
    }
    SG_ASSERT(io.enableComma); // TODO: Error message
    EvaluationInputOutput arg0io(io);
    ok = m_args[0]->EvaluateROK(arg0io);
    if(!ok)
        return false;
    ok = ConvertToValueROK(arg0io, m_args[0]->GetToken());
    if(!ok)
        return false;
    EvaluationInputOutput arg1io(io);
    ok = m_args[1]->EvaluateROK(arg1io);
    if(!ok)
        return false;
    ok = ConvertToValueROK(arg1io, m_args[1]->GetToken());
    if(!ok)
        return false;

    if(io.isPreresolutionPass)
    {
        Comma* preresolved = new Comma();
        preresolved->SetToken(this->GetToken());
        {
            refptr<ITreeNode> arg0;
            ok = ConvertToPreresolvedROK(arg0io, arg0, m_args[0]->GetToken());
            if(!ok)
                return false;
            preresolved->SetArgument(0, arg0.get());
        }
        {
            refptr<ITreeNode> arg1;
            ok = ConvertToPreresolvedROK(arg1io, arg1, m_args[1]->GetToken());
            if(!ok)
                return false;
            preresolved->SetArgument(1, arg1.get());
        }
        io.presesolvedNodeIFN = preresolved;
        return ok;
    }

    SG_ASSERT(nullptr != arg1io.returnValue);
    SG_ASSERT(arg1io.returnIdentifier.Empty());
    SG_ASSERT(arg1io.qualifiers.empty());

    SG_ASSERT(!arg1io.returnValueIsCommaSeparatedList);
    reflection::PrimitiveData<reflection::PrimitiveDataList>* list;
    if(arg0io.returnValueIsCommaSeparatedList)
    {
        SG_ASSERT(nullptr != arg0io.returnValue);
        SG_ASSERT(arg0io.returnIdentifier.Empty());
        SG_ASSERT(arg0io.qualifiers.empty());
        list = checked_cast<reflection::PrimitiveData<reflection::PrimitiveDataList>*>(arg0io.returnValue.get());
    }
    else
    {
        SG_ASSERT(nullptr != arg0io.returnValue);
        SG_ASSERT(arg0io.returnIdentifier.Empty());
        SG_ASSERT(arg0io.qualifiers.empty());
        list = new reflection::PrimitiveData<reflection::PrimitiveDataList>();
        list->GetForWriting().emplace_back(arg0io.returnValue.get());
    }
    list->GetForWriting().emplace_back(arg1io.returnValue.get());
    io.returnValue = list;
    io.returnValueIsCommaSeparatedList = true;
    io.returnValueContainsUnresolvedIdentifier = arg0io.returnValueContainsUnresolvedIdentifier || arg1io.returnValueContainsUnresolvedIdentifier;
    arg0io.returnValueContainsUnresolvedIdentifier = false;
    arg1io.returnValueContainsUnresolvedIdentifier = false;
    return ok;
}
//=============================================================================
bool Namespace::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    SG_ASSERT(!io.isPreresolutionPass);
    SG_ASSERT(nullptr == io.returnValue);
    SG_ASSERT(io.enableNamespaceDefinition); // TODO: Error message
    SG_ASSERT(io.enableScriptDefinitions); // TODO: Error message

    EvaluationInputOutput nameio(io);
    ok = m_name->EvaluateROK(nameio);
    if(!ok)
        return false;
    SG_ASSERT(nameio.returnIdentifier.Size() == 1);
    SG_ASSERT(nameio.qualifiers.empty());

    ResolvedIdentifier resolved;
    ResolveLValueIdentifier(io, nameio.returnIdentifier, resolved);
    if(ResolvedIdentifier::Type::None != resolved.type)
    {
        switch(resolved.type)
        {
        case ResolvedIdentifier::Type::Metaclass:
            PushError(io, ErrorType::namespace_name_collision_with_class, m_name->GetToken(), "Namespace name conflicts with class name");
            return false;
        case ResolvedIdentifier::Type::ScriptObject:
            // TODO: add line of script object declaration
            PushError(io, ErrorType::namespace_name_collision_with_script_object, m_name->GetToken(), "Namespace name conflicts with another script entity");
            return false;
        default:
            SG_ASSERT_NOT_REACHED();
        }
    }

    // Note: Here, we can't keep a reference on propertyName as future methods can add FastSymbols, thus invalidating reference.
    std::string const name = nameio.returnIdentifier[0].Symbol().Value();

    SG_ASSERT(*io.pathForObjects == *io.pathForScript);
    reflection::Identifier path = reflection::Identifier(*io.pathForObjects, name);

    size_t const instructionCount = m_instructions.size();
    for(size_t i = 0; i < instructionCount; ++i)
    {
        if(!m_instructions[i]->IsInstruction())
        {
            PushError(io, ErrorType::expression_is_not_an_instruction, m_instructions[i]->GetToken(), "expression is not an instruction");
            return false;
        }
        EvaluationInputOutput subio(io);
        subio.pathForObjects = &path;
        subio.pathForScript = &path;
        ok = m_instructions[i]->EvaluateROK(subio);
        if(!ok)
        {
            SG_ASSERT(io.errorHandler->DidErrorHappen());
            return false;
        }
    }
    return ok;
}
//=============================================================================
bool If::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    SG_ASSERT(io.enableScriptDefinitions); // TODO: Error message

    reflection::ObjectDatabaseScopedTransaction scopedTransaction(io.scriptDatabase.get());
    reflection::Identifier pathForScript = reflection::Identifier(*io.pathForScript, reflection::IdentifierNode());

    EvaluationInputOutput exprio(io);
    exprio.pathForScript = &pathForScript;
    ok = m_expr->EvaluateROK(exprio);
    if(!ok)
        return false;
    ok = ConvertToValueROK(exprio, m_expr->GetToken());
    if(!ok)
        return false;

    bool cond = true;
    if(!io.isPreresolutionPass || nullptr == exprio.presesolvedNodeIFN)
    {
        SG_ASSERT(nullptr != exprio.returnValue);
        SG_ASSERT(exprio.returnIdentifier.Empty());
        SG_ASSERT(exprio.qualifiers.empty());

        bool isCondOk = exprio.returnValue->AsROK<bool>(&cond);
        if(!isCondOk)
        {
            PushError(io, ErrorType::unexpected_type_for_condition, m_expr->GetToken(), "unexpected type for condition. should be boolean.");
            return false;
        }
    }

    if(io.isPreresolutionPass)
    {
        if(nullptr != exprio.presesolvedNodeIFN)
        {
            If* preresolved = new If;
            preresolved->SetToken(this->GetToken());
            refptr<ITreeNode> expr;
            ok = ConvertToPreresolvedROK(exprio, expr, m_expr->GetToken());
            if(!ok)
                return false;
            preresolved->SetExpr(expr.get());
            {
                reflection::ObjectDatabaseScopedTransaction scopedTransaction2(io.scriptDatabase.get());
                reflection::Identifier pathForScript2 = reflection::Identifier(pathForScript, reflection::IdentifierNode());
                std::vector<refptr<ITreeNode>> preresolvedIstructions;
                ok = PreresolveInstructionROK(io, pathForScript2, m_instructions, preresolvedIstructions);
                if(!ok)
                    return false;
                preresolved->SetInstructions(preresolvedIstructions);
            }
            {
                reflection::ObjectDatabaseScopedTransaction scopedTransaction2(io.scriptDatabase.get());
                reflection::Identifier pathForScript2 = reflection::Identifier(pathForScript, reflection::IdentifierNode());
                std::vector<refptr<ITreeNode>> preresolvedElseIstructions;
                ok = PreresolveInstructionROK(io, pathForScript2, m_elseinstructions, preresolvedElseIstructions);
                if(!ok)
                    return false;
                preresolved->SetElseInstructions(preresolvedElseIstructions);
            }
            io.presesolvedNodeIFN = preresolved;
            return true;
        }
        else
        {
            SG_ASSERT_NOT_IMPLEMENTED();
            // TODO: replace by a InstructionBloc node.
            Struct* preresolved = new Struct;
            preresolved->SetToken(this->GetToken());
            std::vector<refptr<ITreeNode>> preresolvedIstructions;
            ok = PreresolveInstructionROK(io, pathForScript, cond ? m_instructions : m_elseinstructions, preresolvedIstructions);
            if(!ok)
                return false;
            preresolved->SetSubNodes(preresolvedIstructions, io.errorHandler.get());
            io.presesolvedNodeIFN = preresolved;
            return true;
        }
    }

    SG_ASSERT(nullptr == exprio.presesolvedNodeIFN);

    if(cond)
    {
        size_t const instructionCount = m_instructions.size();
        for(size_t i = 0; i < instructionCount; ++i)
        {
            if(!m_instructions[i]->IsInstruction())
            {
                PushError(io, ErrorType::expression_is_not_an_instruction, m_instructions[i]->GetToken(), "expression is not an instruction");
                return false;
            }
            EvaluationInputOutput subio(io);
            subio.pathForScript = &pathForScript;
            ok = m_instructions[i]->EvaluateROK(subio);
            if(!ok)
                return false;
            if(nullptr != subio.jumpStatement)
                subio.ForwardJumpStatement(io);
            subio.returnValueContainsUnresolvedIdentifier = false;
        }
    }
    else
    {
        size_t const instructionCount = m_elseinstructions.size();
        for(size_t i = 0; i < instructionCount; ++i)
        {
            EvaluationInputOutput subio(io);
            subio.pathForScript = &pathForScript;
            ok = m_elseinstructions[i]->EvaluateROK(subio);
            if(!ok)
                return false;
            if(nullptr != subio.jumpStatement)
                subio.ForwardJumpStatement(io);
            subio.returnValueContainsUnresolvedIdentifier = false;
        }
    }
    return ok;
}
//=============================================================================
bool While::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    SG_ASSERT(nullptr == io.returnValue);
    SG_ASSERT(io.enableScriptDefinitions); // TODO: Error message

    reflection::ObjectDatabaseScopedTransaction scopedTransaction(io.scriptDatabase.get());

    bool isFirstIteration = true;
    while(!io.errorHandler->DidErrorHappen())
    {
        reflection::Identifier pathForScript = reflection::Identifier(*io.pathForScript, reflection::IdentifierNode());

        if(io.isPreresolutionPass)
        {
            While* preresolved = new While;
            preresolved->SetToken(this->GetToken());
            preresolved->SetAlwaysPerformFirstIteration(m_alwaysPerformFirstIteration);
            {
                refptr<ITreeNode> expr;
                ok = EvaluateAndConvertToPreresolvedROK(m_expr.get(), io, pathForScript, expr);
                if(!ok)
                    return false;
                preresolved->SetExpr(expr.get());
            }
            {
                std::vector<refptr<ITreeNode>> preresolvedIstructions;
                ok = PreresolveInstructionROK(io, pathForScript, m_instructions, preresolvedIstructions);
                if(!ok)
                    return false;
                preresolved->SetInstructions(preresolvedIstructions);
            }
            io.presesolvedNodeIFN = preresolved;
            return true;
        }

        if(!m_alwaysPerformFirstIteration)
        {
            isFirstIteration = false;
            EvaluationInputOutput exprio(io);
            exprio.pathForScript = &pathForScript;
            ok = m_expr->EvaluateROK(exprio);
            if(!ok)
                return false;
            ok = ConvertToValueROK(exprio, m_expr->GetToken());
            if(!ok)
                return false;
            SG_ASSERT(nullptr != exprio.returnValue);
            SG_ASSERT(exprio.returnIdentifier.Empty());
            SG_ASSERT(exprio.qualifiers.empty());
            bool cond = true;
            bool const isCondOk = exprio.returnValue->AsROK<bool>(&cond);
            if(!isCondOk)
            {
                PushError(io, ErrorType::unexpected_type_for_condition, m_expr->GetToken(), "unexpected type for condition. should be boolean.");
                return false;
            }
            if(!cond)
                break;
        }

        {
            reflection::ObjectDatabaseScopedTransaction scopedTransactionBloc(io.scriptDatabase.get());

            size_t const instructionCount = m_instructions.size();
            for(size_t i = 0; i < instructionCount; ++i)
            {
                if(!m_instructions[i]->IsInstruction())
                {
                    PushError(io, ErrorType::expression_is_not_an_instruction, m_instructions[i]->GetToken(), "expression is not an instruction");
                    return false;
                }
                EvaluationInputOutput subio(io);
                subio.pathForScript = &pathForScript;
                ok = m_instructions[i]->EvaluateROK(subio);
                if(!ok)
                {
                    SG_ASSERT(io.errorHandler->DidErrorHappen());
                    return false;
                }
                if(nullptr != subio.jumpStatement)
                {
                    switch(subio.jumpStatement->GetType())
                    {
                    case semanticTree::JumpStatement::Type::Return:
                        subio.ForwardJumpStatement(io);
                        break;
                    case semanticTree::JumpStatement::Type::Break:
                        subio.jumpStatement = nullptr;
                        return ok;
                    case semanticTree::JumpStatement::Type::Continue:
                        subio.jumpStatement = nullptr;
                        i = instructionCount;
                        break;
                    default:
                        SG_ASSERT_NOT_REACHED();
                    }
                }
                subio.returnValueContainsUnresolvedIdentifier = false;
            }
        }

        if(m_alwaysPerformFirstIteration)
        {
            isFirstIteration = false;
            EvaluationInputOutput exprio(io);
            exprio.pathForScript = &pathForScript;
            ok = m_expr->EvaluateROK(exprio);
            if(!ok)
                return false;
            ok = ConvertToValueROK(exprio, m_expr->GetToken());
            if(!ok)
                return false;
            SG_ASSERT(nullptr != exprio.returnValue);
            SG_ASSERT(exprio.returnIdentifier.Empty());
            SG_ASSERT(exprio.qualifiers.empty());
            bool cond = true;
            bool const isCondOk = exprio.returnValue->AsROK<bool>(&cond);
            SG_ASSERT_AND_UNUSED(isCondOk); // TODO: Error message
            if(!cond)
                break;
        }
    }
    return ok;
}
//=============================================================================
bool NoOp::EvaluateROK(EvaluationInputOutput& io)
{
    SG_UNUSED(io);
    return true;
}
//=============================================================================
bool For::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    SG_ASSERT(nullptr == io.returnValue);
    SG_ASSERT(io.returnIdentifier.Size() == 0);
    SG_ASSERT(io.qualifiers.empty());
    SG_ASSERT(io.enableScriptDefinitions); // TODO: Error message
    SG_ASSERT(ConstructProgress::End == GetConstructProgress());

    reflection::ObjectDatabaseScopedTransaction scopedTransaction(io.scriptDatabase.get());
    reflection::Identifier pathForScript = reflection::Identifier(*io.pathForScript, reflection::IdentifierNode());
    if(nullptr == m_var)
    {
        SG_ASSERT(nullptr == m_list);
        SG_ASSERT(!m_init.empty());
        SG_ASSERT(nullptr != m_cond);
        SG_ASSERT(!m_incr.empty());

        if(io.isPreresolutionPass)
        {
            For* preresolved = new For;
            preresolved->SetToken(this->GetToken());
            {
                std::vector<refptr<ITreeNode>> init;
                for(auto const& it : m_init)
                {
                    refptr<ITreeNode> preit;
                    ok = EvaluateAndConvertToPreresolvedROK(it.get(), io, pathForScript, preit);
                    if(!ok)
                        return false;
                    init.push_back(preit);
                }
                preresolved->SetInit(AsArrayView(init));
            }
            {
                refptr<ITreeNode> cond;
                ok = EvaluateAndConvertToPreresolvedROK(m_cond.get(), io, pathForScript, cond);
                if(!ok)
                    return false;
                preresolved->SetCond(cond.get());
            }
            {
                std::vector<refptr<ITreeNode>> incr;
                for(auto const& it : m_incr)
                {
                    refptr<ITreeNode> preit;
                    ok = EvaluateAndConvertToPreresolvedROK(it.get(), io, pathForScript, preit);
                    if(!ok)
                        return false;
                    incr.push_back(preit);
                }
                preresolved->SetIncr(AsArrayView(incr));
            }
            {
                std::vector<refptr<ITreeNode>> preresolvedIstructions;
                ok = PreresolveInstructionROK(io, pathForScript, m_instructions, preresolvedIstructions);
                if(!ok)
                    return false;
                preresolved->SetInstructions(preresolvedIstructions);
            }
            io.presesolvedNodeIFN = preresolved;
            return true;
        }

        for(auto const& it : m_init)
        {
            EvaluationInputOutput initio(io);
            initio.pathForScript = &pathForScript;
            ok = it->EvaluateROK(initio);
            if(!ok)
                return false;
        }

        while(!io.errorHandler->DidErrorHappen())
        {
            {
                EvaluationInputOutput condio(io);
                condio.pathForScript = &pathForScript;
                ok = m_cond->EvaluateROK(condio);
                if(!ok)
                    return false;
                ok = ConvertToValueROK(condio, m_cond->GetToken());
                if(!ok)
                    return false;
                SG_ASSERT(nullptr != condio.returnValue);
                SG_ASSERT(condio.returnIdentifier.Empty());
                SG_ASSERT(condio.qualifiers.empty());
                bool cond = true;
                bool const isCondOk = condio.returnValue->AsROK<bool>(&cond);if(!isCondOk)
                {
                    PushError(io, ErrorType::unexpected_type_for_condition, m_cond->GetToken(), "unexpected type for condition. should be boolean.");
                    return false;
                }
                if(!cond)
                    break;
            }

            {
                reflection::ObjectDatabaseScopedTransaction scopedTransactionBloc(io.scriptDatabase.get());

                size_t const instructionCount = m_instructions.size();
                for(size_t i = 0; i < instructionCount; ++i)
                {
                    if(!m_instructions[i]->IsInstruction())
                    {
                        PushError(io, ErrorType::expression_is_not_an_instruction, m_instructions[i]->GetToken(), "expression is not an instruction");
                        return false;
                    }
                    EvaluationInputOutput subio(io);
                    subio.pathForScript = &pathForScript;
                    ok = m_instructions[i]->EvaluateROK(subio);
                    if(!ok)
                    {
                        SG_ASSERT(io.errorHandler->DidErrorHappen());
                        return false;
                    }
                    if(nullptr != subio.jumpStatement)
                    {
                        switch(subio.jumpStatement->GetType())
                        {
                        case semanticTree::JumpStatement::Type::Return:
                            subio.ForwardJumpStatement(io);
                            return ok;
                        case semanticTree::JumpStatement::Type::Break:
                            subio.jumpStatement = nullptr;
                            return ok;
                        case semanticTree::JumpStatement::Type::Continue:
                            subio.jumpStatement = nullptr;
                            i = instructionCount;
                            break;
                        default:
                            SG_ASSERT_NOT_REACHED();
                        }
                    }
                    subio.returnValueContainsUnresolvedIdentifier = false;
                }
            }

            for(auto const& it : m_incr)
            {
                EvaluationInputOutput incrio(io);
                incrio.pathForScript = &pathForScript;
                ok = it->EvaluateROK(incrio);
                if(!ok)
                    return false;
            }
        }
    }
    else
    {
        SG_ASSERT(nullptr != m_list);
        SG_ASSERT(m_init.empty());
        SG_ASSERT(nullptr == m_cond);
        SG_ASSERT(m_incr.empty());

        EvaluationInputOutput vario(io);
        vario.pathForScript = &pathForScript;
        ok = m_var->EvaluateROK(vario);
        if(!ok)
            return false;
        SG_ASSERT(!vario.returnIdentifier.Empty()); // TODO: Error message

        reflection::BaseClass* object = io.scriptDatabase->GetIFP(*io.pathForScript, vario.returnIdentifier);
        SG_ASSERT_AND_UNUSED(nullptr == object); // TODO: Error message "a variable with same name already exists"
        SG_ASSERT(vario.returnIdentifier.Size() == 1); // TODO: Error message "Can not declare variable outside of current namespace"
        reflection::ObjectVisibility const visibility = reflection::ObjectVisibility::Protected;
#if 1
        bool const isConst = true;
        SG_ASSERT(vario.qualifiers.empty()); // TODO: Error message "unexpected qualifier in for range"
#else
        bool isConst = false;
        size_t constnessQualifierCount = 0;
        for(size_t i = 0; i < vario.qualifiers.size(); ++i)
        {
            switch(vario.qualifiers[i])
            {
            case TokenType::keyword_const: ++constnessQualifierCount; isConst = true; break;
            case TokenType::keyword_var: ++constnessQualifierCount; isConst = false; break;
            default:
                SG_ASSERT_NOT_REACHED(); // TODO: Error message "Incorrect qualifier for object definition"
            }
        }
        SG_ASSERT(1 >= constnessQualifierCount); // TODO: Error message "At most one const qualifier is allowed for object definition"
#endif
        reflection::Identifier const varName(pathForScript, vario.returnIdentifier[0]);

        if(io.isPreresolutionPass)
        {
            For* preresolved = new For;
            preresolved->SetToken(this->GetToken());
            {
                PreresolvedIdentifierAndQualifiers* preresolvedVar = new PreresolvedIdentifierAndQualifiers(vario.returnIdentifier, vario.qualifiers);
                preresolvedVar->SetToken(m_var->GetToken());
                preresolved->SetVar(preresolvedVar);
                FreeVariable* var = new FreeVariable(isConst);
                io.scriptDatabase->Add(visibility, varName, var);
            }
            {
                refptr<ITreeNode> list;
                ok = EvaluateAndConvertToPreresolvedROK(m_list.get(), io, pathForScript, list);
                if(!ok)
                    return false;
                preresolved->SetList(list.get());
            }
            {
                std::vector<refptr<ITreeNode>> preresolvedIstructions;
                ok = PreresolveInstructionROK(io, pathForScript, m_instructions, preresolvedIstructions);
                if(!ok)
                    return false;
                preresolved->SetInstructions(preresolvedIstructions);
            }
            io.presesolvedNodeIFN = preresolved;
            return true;
        }

        Variable* var = new Variable(isConst);
        io.scriptDatabase->Add(visibility, varName, var);

        EvaluationInputOutput listio(io);
        listio.pathForScript = &pathForScript;
        listio.enableComma = false;
        listio.enableScriptDefinitions = false;
        ok = m_list->EvaluateROK(listio);
        if(!ok)
            return false;
        ok = ConvertToValueROK(listio, m_list->GetToken());
        if(!ok)
            return false;
        switch(listio.returnValue->GetType())
        {
        case reflection::PrimitiveDataType::List:
            {
                reflection::PrimitiveData<reflection::PrimitiveDataList>* datalist = checked_cast<reflection::PrimitiveData<reflection::PrimitiveDataList>* >(listio.returnValue.get());
                reflection::PrimitiveDataList const& list = datalist->Get();
                size_t const size = list.size();
                for(size_t i = 0; i < size; ++i)
                {
                    reflection::IPrimitiveData* it = list[i].get();
                    var->SetValue(it);

                    reflection::ObjectDatabaseScopedTransaction scopedTransactionBloc(io.scriptDatabase.get());

                    size_t const instructionCount = m_instructions.size();
                    for(size_t j = 0; j < instructionCount; ++j)
                    {
                        if(!m_instructions[j]->IsInstruction())
                        {
                            PushError(io, ErrorType::expression_is_not_an_instruction, m_instructions[j]->GetToken(), "expression is not an instruction");
                            return false;
                        }
                        EvaluationInputOutput subio(io);
                        subio.pathForScript = &pathForScript;
                        ok = m_instructions[j]->EvaluateROK(subio);
                        if(!ok)
                        {
                            SG_ASSERT(io.errorHandler->DidErrorHappen());
                            return false;
                        }
                        if(nullptr != subio.jumpStatement)
                        {
                            switch(subio.jumpStatement->GetType())
                            {
                            case semanticTree::JumpStatement::Type::Return:
                                subio.ForwardJumpStatement(io);
                                break;
                            case semanticTree::JumpStatement::Type::Break:
                                subio.jumpStatement = nullptr;
                                return ok;
                            case semanticTree::JumpStatement::Type::Continue:
                                subio.jumpStatement = nullptr;
                                j = instructionCount;
                                break;
                            default:
                                SG_ASSERT_NOT_REACHED();
                            }
                        }
                    }
                }
            }
            break;
        default:
            SG_ASSERT_NOT_REACHED(); // TODO: Error message "type can't be iterated over"
        }
    }
    return ok;
}
//=============================================================================
bool Assert::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    EvaluationInputOutput subio(io);
    subio.enableNamespaceDefinition = false;
    subio.enableObjectDefinitions = false;
    subio.enableObjectReferences = false;
    subio.enableComma = true;
    ok = m_arg->EvaluateROK(subio);
    if(!ok)
        return false;
    ok = ConvertToValueROK(subio, m_arg->GetToken());
    if(!ok)
        return false;
    if(io.isPreresolutionPass)
    {
        Assert* preresolved = new Assert();
        preresolved->SetToken(this->GetToken());
        refptr<ITreeNode> arg;
        ok = ConvertToPreresolvedROK(subio, arg, m_arg->GetToken());
        if(!ok)
            return false;
        preresolved->SetArgument(0, arg.get());
        io.presesolvedNodeIFN = preresolved;
        return true;
    }
    if(subio.returnValueIsCommaSeparatedList)
    {
        reflection::PrimitiveDataList list;
        ok = subio.returnValue->AsROK(&list);
        SG_ASSERT(list.size() == 2); // TODO: Error message
        bool expr = false;
        ok = list[0]->AsROK(&expr);
        if(!ok)
        {
            SG_ASSERT_NOT_REACHED(); //TODO: Error message "assert expression must be boolean"
            return false;
        }
        std::string msg;
        ok = list[1]->AsROK(&msg);
        if(!ok)
        {
            SG_ASSERT_NOT_REACHED(); //TODO: Error message "assert expression must be boolean"
            return false;
        }
        if(!expr)
        {
            std::ostringstream oss;
            oss << "Assert failed: " << msg;
            PushError(io, ErrorType::assert_failed, GetToken(), oss.str().c_str());
            return false; // or true ?
        }
    }
    else
    {
        bool expr = false;
        ok = subio.returnValue->AsROK(&expr);
        if(!ok)
        {
            SG_ASSERT_NOT_REACHED(); //TODO: Error message "assert expression must be boolean"
            return false;
        }
        if(!expr)
        {
            PushError(io, ErrorType::assert_failed, GetToken(), "Assert failed!");
            return false; // or true ?
        }
    }
    return ok;
}
//=============================================================================
bool Continue::EvaluateROK(EvaluationInputOutput& io)
{
    if(io.isPreresolutionPass) { io.presesolvedNodeIFN = this; }
    io.jumpStatement = this;
    return true;
}
//=============================================================================
bool Break::EvaluateROK(EvaluationInputOutput& io)
{
    if(io.isPreresolutionPass) { io.presesolvedNodeIFN = this; }
    io.jumpStatement = this;
    return true;
}
//=============================================================================
bool Return::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    ok = m_arg->EvaluateROK(io);
    if(!ok)
        return false;
    ok = ConvertToValueROK(io, m_arg->GetToken());
    if(!ok)
        return false;
    if(io.isPreresolutionPass)
    {
        Return* preresolved = new Return();
        preresolved->SetToken(this->GetToken());
        if(nullptr == io.presesolvedNodeIFN)
        {
            SG_ASSERT(nullptr != io.returnValue);
            refptr<ITreeNode> arg;
            ok = ConvertToPreresolvedROK(io, arg, m_arg->GetToken());
            if(!ok)
                return false;
            preresolved->SetArgument(0, arg.get());
        }
        else
            preresolved->SetArgument(0, io.presesolvedNodeIFN.get());
        io.presesolvedNodeIFN = preresolved;
        return true;
    }
    io.jumpStatement = this;
    return ok;
}
//=============================================================================
bool FunctionCall::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    SG_ASSERT(io.returnValue == nullptr);
    SG_ASSERT(io.returnIdentifier.Size() == 0);
    SG_ASSERT(io.qualifiers.empty());

    EvaluationInputOutput callableio(io);
    callableio.enableComma = false;
    callableio.enableScriptDefinitions = false;
    ok = m_callable->EvaluateROK(callableio);
    if(!ok)
        return false;

    Callable* callable = nullptr;
    if(nullptr == callableio.presesolvedNodeIFN)
    {
        ok = ConvertToValueROK(callableio, m_callable->GetToken());
        if(!ok)
            return false;

        if(nullptr == callableio.returnValue || callableio.returnValue->GetType() == reflection::PrimitiveDataType::ObjectReference)
        {
            PushError(io, ErrorType::callable_not_found, m_callable->GetToken(), "callable not found!");
            return false;
        }
        if(callableio.returnValue->GetType() != reflection::PrimitiveDataType::Object)
        {
            PushError(io, ErrorType::expression_is_not_callable, m_callable->GetToken(), "expression is not callable!");
            return false;
        }
        refptr<reflection::BaseClass> callableObject;
        callableio.returnValue->As<refptr<reflection::BaseClass> >(&callableObject);
        if(!Callable::StaticGetMetaclass()->IsBaseOf(callableObject->GetMetaclass()))
        {
            PushError(io, ErrorType::expression_is_not_callable, GetToken(), "expression is not callable");
            return false;
        }
        callable = checked_cast<Callable*>(callableObject.get());
    }

    if(io.isPreresolutionPass)
    {
        // TODO: if the function has no side effect (ie. no object creation),
        // and that all arguments are constants, the function can be called
        // here.

        FunctionCall* preresolved = new FunctionCall;
        preresolved->SetToken(this->GetToken());
        SG_ASSERT(nullptr != callableio.presesolvedNodeIFN || nullptr != callable);
        if(nullptr != callable)
            preresolved->SetArgument(0, new Value(new reflection::PrimitiveData<refptr<reflection::BaseClass> >(callable)));
        else
            preresolved->SetArgument(0, callableio.presesolvedNodeIFN.get());

        io.presesolvedNodeIFN = preresolved;
        if(nullptr == m_args)
            return true;
        else
        {
            EvaluationInputOutput argsio(io);
            argsio.enableComma = true;
            argsio.enableScriptDefinitions = false;
            SG_ASSERT(nullptr == argsio.returnValue);
            // TODO: Accept arguments as list of named values.
            refptr<ITreeNode> args;
            ok = EvaluateAndConvertToPreresolvedUsingIOROK(m_args.get(), argsio, args);
            if(!ok)
                return false;
            preresolved->SetArguments(args.get());
            return true;
        }
    }

    SG_ASSERT(!io.isPreresolutionPass);
    SG_ASSERT(nullptr != callable);

    if(nullptr == m_args)
    {
        ok = callable->CallROK(io, GetToken());
        if(!ok)
            return false;
    }
    else
    {
        EvaluationInputOutput argsio(io);
        argsio.enableComma = true;
        argsio.enableScriptDefinitions = false;
        SG_ASSERT(nullptr == argsio.returnValue);
        // TODO: Accept arguments as list of named values.
        ok = m_args->EvaluateROK(argsio);
        if(!ok)
            return false;
        ok = ConvertToValueROK(argsio, m_args->GetToken());
        if(!ok)
            return false;
        if(argsio.returnValueIsCommaSeparatedList)
        {
            reflection::PrimitiveData<reflection::PrimitiveDataList>* datalist =
                checked_cast<reflection::PrimitiveData<reflection::PrimitiveDataList>* >(argsio.returnValue.get());
            reflection::PrimitiveDataList const& list = datalist->Get();
            ok = callable->CallROK(io, list, GetToken());
            if(!ok)
                return false;
        }
        else
        {
            ok = callable->CallROK(io, argsio.returnValue.get(), GetToken());
            if(!ok)
                return false;
        }
        SG_ASSERT(!argsio.returnValueContainsUnresolvedIdentifier); // TODO: Not implemented yet.
    }
    return ok;
}
//=============================================================================
namespace {
bool ParseArgsROK(EvaluationInputOutput& io, std::vector<ArgumentNameAndValue>& oArgs, std::vector<std::pair<refptr<ITreeNode>, refptr<ITreeNode> > > const& iArgumentNamesAndDefaultValues, bool requiresTrailingDefaultValues)
{
    SG_ASSERT(oArgs.empty());
    oArgs.reserve(iArgumentNamesAndDefaultValues.size());
    bool defaultValueWasEncountered = false;
    for(auto const& it : iArgumentNamesAndDefaultValues)
    {
        ITreeNode* arg = it.first.get();
        ITreeNode* value = it.second.get();

        SG_ASSERT(nullptr != arg);
        EvaluationInputOutput argio(io);
        argio.enableVariableRead = false;
        argio.enableScriptDefinitions = false;
        bool ok = arg->EvaluateROK(argio);
        if(!ok)
             return false;
        SG_ASSERT(nullptr == argio.presesolvedNodeIFN);
        SG_ASSERT(!argio.returnIdentifier.Empty());
        SG_ASSERT(argio.returnIdentifier.Size() == 1);
        reflection::IdentifierNode argName = argio.returnIdentifier[0];
#if 1
        bool const isConst = true;
        SG_ASSERT(argio.qualifiers.empty()); // TODO: Error message "unexpected qualifier in function arguments"
#else
        bool isConst = false;
        size_t constnessQualifierCount = 0;
        for(size_t i = 0; i < argio.qualifiers.size(); ++i)
        {
            switch(argio.qualifiers[i])
            {
            case TokenType::keyword_const: ++constnessQualifierCount; isConst = true; break;
            case TokenType::keyword_var: ++constnessQualifierCount; isConst = false; break;
            default:
                SG_ASSERT_NOT_REACHED(); // TODO: Error message "Incorrect qualifier for argument"
            }
        }
        SG_ASSERT(1 >= constnessQualifierCount); // TODO: Error message "At most one const qualifier is allowed for argument"
#endif
        refptr<reflection::IPrimitiveData> defaultValue = nullptr;
        if(nullptr != value)
        {
            defaultValueWasEncountered = true;
            EvaluationInputOutput valueio(io);
            ok = value->EvaluateROK(valueio);
            if(!ok)
                return false;
            SG_ASSERT(nullptr == valueio.presesolvedNodeIFN);
            ok = ConvertToValueROK(valueio, value->GetToken());
            if(!ok)
                return false;
            SG_ASSERT(nullptr != valueio.returnValue);
            SG_ASSERT(valueio.returnIdentifier.Empty());
            SG_ASSERT(valueio.qualifiers.empty());
            defaultValue = valueio.returnValue;
        }
        else
        {
            SG_ASSERT_AND_UNUSED(!defaultValueWasEncountered || !requiresTrailingDefaultValues); // TODO: Error message "missing default value for argument. default values must be for the last arguments"
        }

        oArgs.emplace_back(isConst, argName, defaultValue.get());
    }
    return true;
}
}
//=============================================================================
bool FunctionDeclaration::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    if(io.isPreresolutionPass) { SG_ASSERT_NOT_IMPLEMENTED(); }
    EvaluationInputOutput nameio(io);
    ok = m_name->EvaluateROK(nameio);
    if(!ok)
        return false;
    SG_ASSERT(nullptr == nameio.presesolvedNodeIFN);

    std::vector<ArgumentNameAndValue> args;
    ok = ParseArgsROK(io, args, m_argumentNamesAndDefaultValues, true);
    if(!ok)
        return false;

    Function* function = nullptr;
    if(!nameio.returnIdentifier.Empty())
    {
        ResolvedIdentifier resolved;
        ResolveLValueIdentifier(io, nameio.returnIdentifier, resolved);
        switch(resolved.type)
        {
        case ResolvedIdentifier::Type::None:
            {
                SG_ASSERT(nameio.returnIdentifier.Size() == 1); // TODO: Error message "Can not declare function outside of current namespace"
                reflection::ObjectVisibility visibility = reflection::ObjectVisibility::Public;

                reflection::Identifier functionName(*io.pathForScript, nameio.returnIdentifier[0]);
                function = new Function(args);
                function->SetIdentifier(functionName);
                io.scriptDatabase->Add(visibility, functionName, function);
            }
            break;
        case ResolvedIdentifier::Type::Metaclass:
            PushError(io, ErrorType::function_name_collision_with_class, m_name->GetToken(), "Function name conflicts with a class");
            return false;
        case ResolvedIdentifier::Type::ScriptObject:
            {
                reflection::BaseClass* object = resolved.scriptObject.get();
                SG_ASSERT(nullptr != object);
                if(object->GetMetaclass() == Variable::StaticGetMetaclass())
                    SG_ASSERT_MSG(false, "a variable with same name already exists!");
                else if(object->GetMetaclass() == Template::StaticGetMetaclass())
                    SG_ASSERT_MSG(false, "a template with same name already exists!");
                else
                {
                    SG_ASSERT(object->GetMetaclass() == Function::StaticGetMetaclass());
                    function = checked_cast<Function*>(object);
                    SG_ASSERT(function->IsPrototype()); // TODO: Error message "function already defined"
                    SG_ASSERT(!m_isPrototype); // TODO: Error message "function already declared"
                    std::vector<ArgumentNameAndValue> const& declaredArgs = function->ArgumentNamesAndDefaultValues();
                    SG_ASSERT_AND_UNUSED(declaredArgs.size() == args.size()); // TODO: Error message "function arguments do not match previous declaration"
                    SG_ASSERT(nullptr == args.back().value); // TODO: Error message "default value for function arguments must be given only in its first declaration"
                    size_t const argCount = args.size();
                    for(size_t i = 0; i < argCount; ++i)
                    {
                        SG_ASSERT(args[i].name == declaredArgs[i].name); // TODO: Error message "argument does not match previous declaration"
                        SG_ASSERT(args[i].isConst == declaredArgs[i].isConst); // TODO: Error message "argument does not match previous declaration"
                        SG_ASSERT(nullptr == args[i].value);
                    }
                }
            }
            break;
        default:
            SG_ASSERT_NOT_REACHED();
        }
    }
    else
    {
        SG_ASSERT_NOT_IMPLEMENTED(); // anonymous function
        //ConvertToValue(leftio);
        //SG_ASSERT(leftio.returnValueIsLValue); // TODO: Error message
        //SG_ASSERT(nullptr != leftio.returnLValueReference);
        //*(leftio.returnLValueReference) = valueCopy;

        //io.returnValue = leftio.returnValue;
        //io.returnValueIsLValue = leftio.returnValueIsLValue;
        //io.returnLValueReference = leftio.returnLValueReference;
    }

    if(!m_isPrototype)
    {
        reflection::ObjectDatabaseScopedTransaction scopedTransaction(io.scriptDatabase.get());
        reflection::Identifier pathForScript = reflection::Identifier(function->Identifier(), reflection::IdentifierNode());
        for(auto const& it : function->ArgumentNamesAndDefaultValues())
        {
            reflection::ObjectVisibility visibility = reflection::ObjectVisibility::Public;
            reflection::Identifier varName(pathForScript, it.name);
            FreeVariable* var = new FreeVariable(it.isConst);
            io.scriptDatabase->Add(visibility, varName, var);
        }
        size_t const instructionCount = m_instructions.size();
        std::vector<refptr<ITreeNode>> preresolvedInstructions;
        preresolvedInstructions.reserve(instructionCount);
        for(size_t i = 0; i < instructionCount; ++i)
        {
            if(!m_instructions[i]->IsInstruction())
            {
                PushError(io, ErrorType::expression_is_not_an_instruction, m_instructions[i]->GetToken(), "expression is not an instruction");
                return false;
            }
            semanticTree::EvaluationInputOutput subio(io);
            subio.pathForScript = &pathForScript;
            subio.restrictedVariableScope = &pathForScript;
            subio.enableNamespaceDefinition = false;
            subio.enableObjectDefinitions = false;
            subio.enableObjectReferences = false;
            subio.enableScriptDefinitions = true;
            subio.variablesAreRestricted = true;
            subio.isPreresolutionPass = true;
            ok = m_instructions[i]->EvaluateROK(subio);
            if(!ok)
                return false;
            if(nullptr != subio.presesolvedNodeIFN)
                preresolvedInstructions.push_back(subio.presesolvedNodeIFN.get());
        }
        function->SetBody(preresolvedInstructions);
    }
    else
    {
        SG_ASSERT(m_instructions.empty());
    }
    return ok;
}
//=============================================================================
bool TemplateDeclaration::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    if(io.isPreresolutionPass) { SG_ASSERT_NOT_IMPLEMENTED(); }
    EvaluationInputOutput nameio(io);
    ok = m_name->EvaluateROK(nameio);
    if(!ok)
        return false;

    reflection::Metaclass const* mc = nullptr;
    refptr<reflection::BaseClass> alias = nullptr;
    SG_ASSERT(m_isNamespace == (m_type == nullptr));
    if(!m_isNamespace)
    {
        EvaluationInputOutput typeio(io);
        ok = m_type->EvaluateROK(typeio);
        if(!ok)
            return false;
        SG_ASSERT(nullptr == typeio.returnValue);
        SG_ASSERT(typeio.qualifiers.empty());

        ResolvedIdentifier resolvedType;
        ResolveIdentifier(io, typeio.returnIdentifier, resolvedType);
        switch(resolvedType.type)
        {
        case ResolvedIdentifier::Type::None:
            PushError(io, ErrorType::class_not_found, m_type->GetToken(), "class not found");
            return false;
        case ResolvedIdentifier::Type::Metaclass:
            mc = resolvedType.metaclass;
            break;
        case ResolvedIdentifier::Type::ScriptObject:
        {
            reflection::BaseClass* object = resolvedType.scriptObject.get();
            SG_ASSERT(nullptr != object);
            if(object->GetMetaclass() == Variable::StaticGetMetaclass())
                SG_ASSERT_MSG(false, "invalid type for template");
            else if(object->GetMetaclass() == Function::StaticGetMetaclass())
                SG_ASSERT_MSG(false, "invalid type for template");
            else
            {
                SG_ASSERT(object->GetMetaclass() == Template::StaticGetMetaclass() || object->GetMetaclass() == TemplateNamespace::StaticGetMetaclass() || object->GetMetaclass() == TemplateAlias::StaticGetMetaclass());
                alias = object;
            }
            break;
        }
        default:
            SG_ASSERT_NOT_REACHED();
        }
        SG_ASSERT(nullptr != mc || nullptr != alias);
    }

    std::vector<ArgumentNameAndValue> args;
    ok = ParseArgsROK(io, args, m_argumentNamesAndDefaultValues, false);
    if(!ok)
        return false;

    if(!nameio.returnIdentifier.Empty())
    {
        ResolvedIdentifier resolved;
        ResolveLValueIdentifier(io, nameio.returnIdentifier, resolved);
        switch(resolved.type)
        {
        case ResolvedIdentifier::Type::None:
            {
                SG_ASSERT(nameio.returnIdentifier.Size() == 1); // TODO: Error message "Can not declare template outside of current namespace"
                reflection::ObjectVisibility const visibility = reflection::ObjectVisibility::Public;

                reflection::Identifier const templateName(*io.pathForScript, nameio.returnIdentifier[0]);

                std::vector<refptr<ITreeNode>> preresolvedInstructions;
                {
                    reflection::ObjectDatabaseScopedTransaction scopedTransaction(io.scriptDatabase.get());

                    reflection::Identifier objectPath;
                    objectPath.PushBack(reflection::IdentifierNode());
                    reflection::Identifier pathForScript = reflection::Identifier(templateName, reflection::IdentifierNode());

                    for(auto const& it : args)
                    {
                        reflection::ObjectVisibility const argVisibility = reflection::ObjectVisibility::Public;
                        reflection::Identifier const varName(pathForScript, it.name);
                        FreeVariable* var = new FreeVariable(it.isConst);
                        io.scriptDatabase->Add(argVisibility, varName, var);
                    }
                    size_t const instructionCount = m_instructions.size();
                    preresolvedInstructions.reserve(instructionCount);
                    for(size_t i = 0; i < instructionCount; ++i)
                    {
                        if(!m_instructions[i]->IsInstruction())
                        {
                            PushError(io, ErrorType::expression_is_not_an_instruction, m_instructions[i]->GetToken(), "expression is not an instruction");
                            ok = false;
                            break;
                        }
                        semanticTree::EvaluationInputOutput subio(io);
                        subio.pathForObjects = nullptr; // &objectPath;
                        subio.pathForScript = &pathForScript;
                        subio.restrictedVariableScope = &pathForScript;
                        subio.enableScriptDefinitions = true;
                        subio.enableObjectDefinitions = true;
                        subio.enableObjectReferences = true;
                        subio.variablesAreRestricted = true;
                        subio.enableNamespaceDefinition = false;
                        subio.isPreresolutionPass = true;
                        subio.constructionInProgress = semanticTree::Construction::Object;
                        subio.returnValue = io.returnValue;
                        ok = m_instructions[i]->EvaluateROK(subio);
                        if(!ok)
                        {
                            SG_ASSERT(io.errorHandler->DidErrorHappen());
                            break;
                        }
                        if(nullptr != subio.presesolvedNodeIFN)
                            preresolvedInstructions.push_back(subio.presesolvedNodeIFN.get());
                        if(nullptr != subio.jumpStatement)
                        {
                            PushError(subio, ErrorType::unexpected_use_of_jump_statement, m_instructions[i]->GetToken(), "unexpected use of jump statement");
                            return false;
                        }
                    }
                }
                if(m_isNamespace)
                {
                    TemplateNamespace* tmplt = new TemplateNamespace(args);
                    tmplt->SetIdentifier(templateName);
                    tmplt->SetBody(preresolvedInstructions);
                    io.scriptDatabase->Add(visibility, templateName, tmplt);
                }
                else if (nullptr != mc)
                {
                    Template* tmplt = new Template(args);
                    tmplt->SetIdentifier(templateName);
                    tmplt->SetMetaclass(mc);
                    tmplt->SetBody(preresolvedInstructions);
                    io.scriptDatabase->Add(visibility, templateName, tmplt);
                }
                else
                {
                    SG_ASSERT(nullptr != alias);
                    TemplateAlias* tmplt = new TemplateAlias(args);
                    tmplt->SetIdentifier(templateName);
                    if(alias->GetMetaclass() == Template::StaticGetMetaclass())
                        tmplt->SetType(checked_cast<Template*>(alias.get()));
                    else if(alias->GetMetaclass() == TemplateNamespace::StaticGetMetaclass())
                        tmplt->SetType(checked_cast<TemplateNamespace*>(alias.get()));
                    else if(alias->GetMetaclass() == TemplateAlias::StaticGetMetaclass())
                        tmplt->SetType(checked_cast<TemplateAlias*>(alias.get()));
                    else
                        SG_ASSERT_NOT_REACHED();
                    tmplt->SetBody(preresolvedInstructions);
                    io.scriptDatabase->Add(visibility, templateName, tmplt);
                }
            }
            break;
        case ResolvedIdentifier::Type::Metaclass:
            PushError(io, ErrorType::template_name_collision_with_class, m_name->GetToken(), "Template name conflicts with a class");
            return false;
        case ResolvedIdentifier::Type::ScriptObject:
            {
                reflection::BaseClass* object = resolved.scriptObject.get();
                SG_ASSERT(nullptr != object);
                if(object->GetMetaclass() == Variable::StaticGetMetaclass())
                    SG_ASSERT_MSG(false, "a variable with same name already exists!");
                else if(object->GetMetaclass() == Function::StaticGetMetaclass())
                    SG_ASSERT_MSG(false, "a function with same name already exists!");
                else
                {
                    SG_ASSERT(object->GetMetaclass() == Template::StaticGetMetaclass());
                    SG_ASSERT_NOT_REACHED(); // TODO: Error message "a template with same name already exists!"
                }
            }
            break;
        default:
            SG_ASSERT_NOT_REACHED();
        }
    }
    else
    {
        SG_ASSERT_NOT_REACHED(); // anonymous template ?
    }

    return ok;
}
//=============================================================================
bool TypedefDeclaration::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    if(io.isPreresolutionPass) { SG_ASSERT_NOT_IMPLEMENTED(); }
    SG_ASSERT(io.enableScriptDefinitions); // TODO: Error message
    EvaluationInputOutput typeio(io);
    ok = m_type->EvaluateROK(typeio);
    if(!ok)
        return false;
    SG_ASSERT(nullptr == typeio.returnValue);
    SG_ASSERT(!typeio.returnIdentifier.Empty());

    EvaluationInputOutput aliasio(io);
    ok = m_alias->EvaluateROK(aliasio);
    if(!ok)
        return false;
    SG_ASSERT(nullptr == aliasio.returnValue);
    SG_ASSERT(!aliasio.returnIdentifier.Empty());

    reflection::ObjectVisibility const visibility = reflection::ObjectVisibility::Public;
    ResolvedIdentifier resolvedTypeName;
    ResolveIdentifier(io, typeio.returnIdentifier, resolvedTypeName);
    reflection::Identifier const& fullTypeName = resolvedTypeName.fullIdentifier;
    SG_ASSERT(!fullTypeName.Empty()); // TODO: Error message "Unknown type name"
    SG_ASSERT(fullTypeName.IsAbsolute());
    SG_ASSERT(aliasio.returnIdentifier.Size() == 1); // TODO: Error message "Can not declare typedef outside of current namespace"
    reflection::Identifier const aliasName(*io.pathForScript, aliasio.returnIdentifier[0]);
    Alias* alias = new Alias(fullTypeName);
    io.scriptDatabase->Add(visibility, aliasName, alias);

    return true;
}
//=============================================================================
typedef bool (*UnaryExpressionImplROKFct) (EvaluationInputOutput& io, Token const& iToken, refptr<reflection::IPrimitiveData>&, reflection::IPrimitiveData* iData);
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct UnaryExpressionEntry
{
    TokenType op;
    reflection::PrimitiveDataType type;
    UnaryExpressionImplROKFct impl;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <TokenType OP, typename T> bool UnaryExpressionImplROK(EvaluationInputOutput& io, Token const& iToken, refptr<reflection::IPrimitiveData>& oData, reflection::IPrimitiveData* iData) { return nullptr; };
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define APPLY_MACRO_TO_UNARY_EXPRESSION_IMPL(MACRO) \
    MACRO(TokenType::operator_not,    !v,      bool,  bool)    \
    MACRO(TokenType::operator_bitnot, ~v,      i32,   i32)     \
    MACRO(TokenType::operator_bitnot, ~v,      u32,   u32)     \
    MACRO(TokenType::operator_plus,   +v,      i32,   i32)     \
    MACRO(TokenType::operator_plus,   +v,      u32,   u32)     \
    MACRO(TokenType::operator_plus,   +v,      float, float)   \
    MACRO(TokenType::operator_minus,  -v,      i32,   i32)     \
    MACRO(TokenType::operator_minus,  -(i32)v, u32,   i32)     \
    MACRO(TokenType::operator_minus,  -v,      float, float)
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define DEFINE_UNARY_EXPRESSION(TOKEN, EXPR, INPUT_TYPE, OUTPUT_TYPE) \
    template <> bool UnaryExpressionImplROK<TOKEN, INPUT_TYPE>(EvaluationInputOutput& io, Token const& iToken, refptr<reflection::IPrimitiveData>& oData, reflection::IPrimitiveData* iData) \
    { \
        SG_UNUSED((io, iToken)); \
        INPUT_TYPE v; \
        iData->As<INPUT_TYPE>(&v); \
        oData = new reflection::PrimitiveData<OUTPUT_TYPE>(EXPR); \
        return true; \
    };
APPLY_MACRO_TO_UNARY_EXPRESSION_IMPL(DEFINE_UNARY_EXPRESSION)
#undef DEFINE_UNARY_EXPRESSION
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
UnaryExpressionEntry const unaryExpressionEntries[] =
{
#define DECLARE_UNARY_EXPRESSION_ENTRY(TOKEN, OP, INPUT_TYPE, OUTPUT_TYPE) \
    { TOKEN, reflection::PrimitiveDataTraits<INPUT_TYPE>::primitive_data_type, UnaryExpressionImplROK<TOKEN, INPUT_TYPE> },
APPLY_MACRO_TO_UNARY_EXPRESSION_IMPL(DECLARE_UNARY_EXPRESSION_ENTRY)
#undef DECLARE_UNARY_EXPRESSION_ENTRY
};
size_t const unaryExpressionEntriesCount = SG_ARRAYSIZE(unaryExpressionEntries);
//=============================================================================
bool UnaryExpression::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    EvaluationInputOutput argio(io);
    ok = m_arg->EvaluateROK(argio);
    if(!ok)
        return false;
    ok = ConvertToValueROK(argio, m_arg->GetToken());
    if(!ok)
        return false;
    if(io.isPreresolutionPass && nullptr != argio.presesolvedNodeIFN)
    {
        UnaryExpression* preresolved = new UnaryExpression(m_op);
        preresolved->SetToken(this->GetToken());
        preresolved->SetArgument(0, argio.presesolvedNodeIFN.get());
        io.presesolvedNodeIFN = preresolved;
        return true;
    }
    SG_ASSERT(nullptr != argio.returnValue);
    SG_ASSERT(argio.returnIdentifier.Empty());
    SG_ASSERT(argio.qualifiers.empty());
    if(argio.returnValueContainsUnresolvedIdentifier)
    {
        PushUnresolvedIdentifierError(argio, m_arg.get());
        return false;
    }

    io.returnValueContainsUnresolvedIdentifier = argio.returnValueContainsUnresolvedIdentifier;
    argio.returnValueContainsUnresolvedIdentifier = false;
    reflection::PrimitiveDataType type = argio.returnValue->GetType();
    for(size_t i = 0; i < unaryExpressionEntriesCount; ++i)
    {
        if(unaryExpressionEntries[i].op == m_op && unaryExpressionEntries[i].type == type)
        {
            ok = unaryExpressionEntries[i].impl(io, GetToken(), io.returnValue, argio.returnValue.get());
            return ok;
        }
    }
    PushError(io, ErrorType::unsupported_type_for_unary_operator, GetToken(), "unsupported type for operation");
    return false;
}
//=============================================================================
namespace {
    template<typename T> void StrConcat(std::string& o, std::string const& l, T const& r)
    {
        std::ostringstream oss;
        oss << l << r;
        o = oss.str();
    }
    template<typename T> void StrConcat(std::string& l, T const& r)
    {
        std::ostringstream oss;
        oss << l << r;
        l = oss.str();
    }
    void ListConcat(reflection::PrimitiveDataList& o, reflection::PrimitiveDataList const& l, reflection::PrimitiveDataList const& r)
    {
        size_t const sizel = l.size();
        size_t const sizer = r.size();
        o.reserve(sizel + sizer);
        for(size_t i = 0; i < sizel; ++i)
            o.push_back(l[i]);
        for(size_t i = 0; i < sizer; ++i)
            o.push_back(r[i]);
    }
    void ListConcat(reflection::PrimitiveDataList& l, reflection::PrimitiveDataList const& r)
    {
        size_t const sizel = l.size();
        size_t const sizer = r.size();
        l.reserve(sizel + sizer);
        for(size_t i = 0; i < sizer; ++i)
            l.push_back(r[i]);
    }
    template<typename T>
    bool CheckDivideByZeroROK(EvaluationInputOutput& io, Token const& iToken, T const& iRight)
    {
        if(T(0) == iRight)
        {
            PushError(io, ErrorType::divide_by_zero, iToken, "divide by zero");
            return false;
        }
        return true;
    }
}
//=============================================================================
typedef bool (*BinaryExpressionImplROKFct) (EvaluationInputOutput& io, Token const& iToken, refptr<reflection::IPrimitiveData>& oData, reflection::IPrimitiveData const* iLeft, reflection::IPrimitiveData const* iRight);
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct BinaryExpressionEntry
{
    TokenType op;
    reflection::PrimitiveDataType leftType;
    reflection::PrimitiveDataType rightType;
    BinaryExpressionImplROKFct impl;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <TokenType OP, typename T1, typename T2> bool BinaryExpressionImplROK(EvaluationInputOutput& io, Token const& iToken, refptr<reflection::IPrimitiveData>& oData, reflection::IPrimitiveData const* iLeft, reflection::IPrimitiveData const* iRight) { return nullptr; };
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
// Note: all integer computations return i32 (instead of u32, like c++) to ease comparisons in script, as the user can't cast himself.
#define APPLY_MACRO_TO_BINARY_EXPRESSION_IMPL_FLOAT_INT(MACRO, TOKEN, EXPR) \
    MACRO(TOKEN, EXPR, i32,     i32,    i32) \
    MACRO(TOKEN, EXPR, u32,     i32,    i32) \
    MACRO(TOKEN, EXPR, i32,     u32,    i32) \
    MACRO(TOKEN, EXPR, u32,     u32,    i32) \
    MACRO(TOKEN, EXPR, float,   i32,    float) \
    MACRO(TOKEN, EXPR, float,   u32,    float) \
    MACRO(TOKEN, EXPR, i32,     float,  float) \
    MACRO(TOKEN, EXPR, u32,     float,  float) \
    MACRO(TOKEN, EXPR, float,   float,  float)
#define APPLY_MACRO_TO_BINARY_EXPRESSION_IMPL_FLOAT_INT_COMPARISON(MACRO, TOKEN, OP) \
    MACRO(TOKEN, o=(l OP r),            i32,     i32,    bool) \
    MACRO(TOKEN, o=((i32)l OP r),       u32,     i32,    bool) \
    MACRO(TOKEN, o=(l OP (i32)r),       i32,     u32,    bool) \
    MACRO(TOKEN, o=((i32)l OP (i32)r),  u32,     u32,    bool) \
    MACRO(TOKEN, o=(l OP r),            float,   i32,    bool) \
    MACRO(TOKEN, o=(l OP r),            float,   u32,    bool) \
    MACRO(TOKEN, o=(l OP r),            i32,     float,  bool) \
    MACRO(TOKEN, o=(l OP r),            u32,     float,  bool) \
    MACRO(TOKEN, o=(l OP r),            float,   float,  bool)
#define APPLY_MACRO_TO_BINARY_EXPRESSION_IMPL_INT(MACRO, TOKEN, EXPR) \
    MACRO(TOKEN, EXPR, i32,     i32,    i32) \
    MACRO(TOKEN, EXPR, u32,     i32,    i32) \
    MACRO(TOKEN, EXPR, i32,     u32,    i32) \
    MACRO(TOKEN, EXPR, u32,     u32,    i32) \
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define APPLY_MACRO_TO_BINARY_EXPRESSION_IMPL(MACRO) \
    APPLY_MACRO_TO_BINARY_EXPRESSION_IMPL_FLOAT_INT(MACRO, TokenType::operator_plus,       o=(l+r)) \
    APPLY_MACRO_TO_BINARY_EXPRESSION_IMPL_FLOAT_INT(MACRO, TokenType::operator_minus,      o=(l-r)) \
    APPLY_MACRO_TO_BINARY_EXPRESSION_IMPL_FLOAT_INT(MACRO, TokenType::operator_multiply,   o=(l*r)) \
    APPLY_MACRO_TO_BINARY_EXPRESSION_IMPL_FLOAT_INT(MACRO, TokenType::operator_divide,     if(!CheckDivideByZeroROK(io, iToken, r)) return false; o=(l/r)) \
    APPLY_MACRO_TO_BINARY_EXPRESSION_IMPL_FLOAT_INT_COMPARISON(MACRO, TokenType::operator_less,          < ) \
    APPLY_MACRO_TO_BINARY_EXPRESSION_IMPL_FLOAT_INT_COMPARISON(MACRO, TokenType::operator_less_equal,    <=) \
    APPLY_MACRO_TO_BINARY_EXPRESSION_IMPL_FLOAT_INT_COMPARISON(MACRO, TokenType::operator_greater,       > ) \
    APPLY_MACRO_TO_BINARY_EXPRESSION_IMPL_FLOAT_INT_COMPARISON(MACRO, TokenType::operator_greater_equal, >=) \
    APPLY_MACRO_TO_BINARY_EXPRESSION_IMPL_FLOAT_INT_COMPARISON(MACRO, TokenType::operator_equal_equal,   ==) \
    APPLY_MACRO_TO_BINARY_EXPRESSION_IMPL_FLOAT_INT_COMPARISON(MACRO, TokenType::operator_not_equal,     !=) \
    APPLY_MACRO_TO_BINARY_EXPRESSION_IMPL_INT(MACRO, TokenType::operator_bitand, o=(l&r)) \
    APPLY_MACRO_TO_BINARY_EXPRESSION_IMPL_INT(MACRO, TokenType::operator_bitxor, o=(l^r)) \
    APPLY_MACRO_TO_BINARY_EXPRESSION_IMPL_INT(MACRO, TokenType::operator_bitor,  o=(l|r)) \
    APPLY_MACRO_TO_BINARY_EXPRESSION_IMPL_INT(MACRO, TokenType::operator_modulo, if(!CheckDivideByZeroROK(io, iToken, r)) return false; o=(l%r)) \
    APPLY_MACRO_TO_BINARY_EXPRESSION_IMPL_INT(MACRO, TokenType::operator_shift_left,  o=(l<<r)) \
    APPLY_MACRO_TO_BINARY_EXPRESSION_IMPL_INT(MACRO, TokenType::operator_shift_right, o=(l>>r)) \
    MACRO(TokenType::operator_and,          o=(l&&r), bool, bool, bool) \
    MACRO(TokenType::operator_or,           o=(l||r), bool, bool, bool) \
    MACRO(TokenType::operator_equal_equal,  o=(l==r), bool, bool, bool) \
    MACRO(TokenType::operator_not_equal,    o=(l!=r), bool, bool, bool) \
    MACRO(TokenType::operator_plus, StrConcat(o,l,r), std::string, std::string, std::string) \
    MACRO(TokenType::operator_plus, StrConcat(o,l,r), std::string, i32,         std::string) \
    MACRO(TokenType::operator_plus, StrConcat(o,l,r), std::string, u32,         std::string) \
    MACRO(TokenType::operator_plus, StrConcat(o,l,r), std::string, float,       std::string) \
    MACRO(TokenType::operator_plus, StrConcat(o,l,r), std::string, bool,        std::string) \
    MACRO(TokenType::operator_plus, o=l,              std::string, nullptr_t,   std::string) \
    MACRO(TokenType::operator_plus, ListConcat(o,l,r), reflection::PrimitiveDataList, reflection::PrimitiveDataList, reflection::PrimitiveDataList) \
    MACRO(TokenType::operator_equal_equal,  o=(l==r), std::string, std::string, bool) \
/* end of macro */
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define DEFINE_BINARY_EXPRESSION(TOKEN, EXPR, LEFT_TYPE, RIGHT_TYPE, OUTPUT_TYPE) \
    template <> bool BinaryExpressionImplROK<TOKEN, LEFT_TYPE, RIGHT_TYPE>(EvaluationInputOutput& io, Token const& iToken, refptr<reflection::IPrimitiveData>& oData, reflection::IPrimitiveData const* iLeft, reflection::IPrimitiveData const* iRight) \
    { \
        SG_UNUSED((io, iToken)); \
        LEFT_TYPE l; \
        iLeft->As<LEFT_TYPE>(&l); \
        RIGHT_TYPE r; \
        iRight->As<RIGHT_TYPE>(&r); \
        reflection::PrimitiveData<OUTPUT_TYPE>* output = new reflection::PrimitiveData<OUTPUT_TYPE>; \
        OUTPUT_TYPE& o = output->GetForWriting(); \
        EXPR; \
        oData = output; \
        return true; \
    }; \
/* end of macro */
APPLY_MACRO_TO_BINARY_EXPRESSION_IMPL(DEFINE_BINARY_EXPRESSION)
#undef DEFINE_BINARY_EXPRESSION
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BinaryExpressionEntry const binaryExpressionEntries[] =
{
#define DECLARE_BINARY_EXPRESSION_ENTRY(TOKEN, OP, LEFT_TYPE, RIGHT_TYPE, OUTPUT_TYPE) \
    { TOKEN, reflection::PrimitiveDataTraits<LEFT_TYPE>::primitive_data_type, reflection::PrimitiveDataTraits<RIGHT_TYPE>::primitive_data_type, BinaryExpressionImplROK<TOKEN, LEFT_TYPE, RIGHT_TYPE> },
APPLY_MACRO_TO_BINARY_EXPRESSION_IMPL(DECLARE_BINARY_EXPRESSION_ENTRY)
#undef DECLARE_BINARY_EXPRESSION_ENTRY
};
size_t const binaryExpressionEntryCount = SG_ARRAYSIZE(binaryExpressionEntries);
//=============================================================================
bool BinaryExpression::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    EvaluationInputOutput arg0io(io);
    ok = m_args[0]->EvaluateROK(arg0io);
    if(!ok)
        return false;
    ok = ConvertToValueROK(arg0io, m_args[0]->GetToken());
    if(!ok)
        return false;
    EvaluationInputOutput arg1io(io);
    ok = m_args[1]->EvaluateROK(arg1io);
    if(!ok)
        return false;
    ok = ConvertToValueROK(arg1io, m_args[1]->GetToken());
    if(!ok)
        return false;

    if(io.isPreresolutionPass && (nullptr != arg0io.presesolvedNodeIFN || nullptr != arg1io.presesolvedNodeIFN))
    {
        BinaryExpression* preresolved = new BinaryExpression(m_op);
        preresolved->SetToken(this->GetToken());
        {
            refptr<ITreeNode> arg0;
            ok = ConvertToPreresolvedROK(arg0io, arg0, m_args[0]->GetToken());
            if(!ok)
                return false;
            preresolved->SetArgument(0, arg0.get());
        }
        {
            refptr<ITreeNode> arg1;
            ok = ConvertToPreresolvedROK(arg1io, arg1, m_args[1]->GetToken());
            if(!ok)
                return false;
            preresolved->SetArgument(1, arg1.get());
        }
        io.presesolvedNodeIFN = preresolved;
        return true;
    }

    SG_ASSERT(nullptr != arg0io.returnValue);
    SG_ASSERT(nullptr != arg1io.returnValue);
    if(arg0io.returnValueContainsUnresolvedIdentifier)
    {
        PushUnresolvedIdentifierError(arg0io, m_args[0].get());
        return false;
    }
    if(arg1io.returnValueContainsUnresolvedIdentifier)
    {
        PushUnresolvedIdentifierError(arg1io, m_args[1].get());
        return false;
    }

    io.returnValueContainsUnresolvedIdentifier = arg0io.returnValueContainsUnresolvedIdentifier || arg1io.returnValueContainsUnresolvedIdentifier;
    arg0io.returnValueContainsUnresolvedIdentifier = false;
    arg1io.returnValueContainsUnresolvedIdentifier = false;
    reflection::PrimitiveDataType leftType = arg0io.returnValue->GetType();
    reflection::PrimitiveDataType rightType = arg1io.returnValue->GetType();
    for(size_t i = 0; i < binaryExpressionEntryCount; ++i)
    {
        if(binaryExpressionEntries[i].op == m_op && binaryExpressionEntries[i].leftType == leftType && binaryExpressionEntries[i].rightType == rightType)
        {
            ok = binaryExpressionEntries[i].impl(io, GetToken(), io.returnValue, arg0io.returnValue.get(), arg1io.returnValue.get());
            return ok;
        }
    }
    PushError(io, ErrorType::unsupported_types_for_binary_operator, GetToken(), "operator does not support operand types");
    return false;
}
//=============================================================================
typedef bool (*CompoundAssignmentImplROKFct) (EvaluationInputOutput& io, Token const& iToken, reflection::IPrimitiveData* iLeft, reflection::IPrimitiveData const* iRight);
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct CompoundAssignmentEntry
{
    TokenType op;
    reflection::PrimitiveDataType leftType;
    reflection::PrimitiveDataType rightType;
    CompoundAssignmentImplROKFct impl;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <TokenType OP, typename T1, typename T2> bool CompoundAssignmentImplROK(EvaluationInputOutput& io, Token const& iToken, reflection::IPrimitiveData* iLeft, reflection::IPrimitiveData const* iRight) { return nullptr; };
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define APPLY_MACRO_TO_COMPOUND_ASSIGNMENT_IMPL_FLOAT_INT(MACRO, TOKEN, EXPR) \
    MACRO(TOKEN, EXPR, i32,     i32) \
    MACRO(TOKEN, EXPR, u32,     i32) \
    MACRO(TOKEN, EXPR, i32,     u32) \
    MACRO(TOKEN, EXPR, u32,     u32) \
    MACRO(TOKEN, EXPR, float,   i32) \
    MACRO(TOKEN, EXPR, float,   u32) \
    MACRO(TOKEN, EXPR, float,   float)
#define APPLY_MACRO_TO_COMPOUND_ASSIGNMENT_IMPL_INT(MACRO, TOKEN, EXPR) \
    MACRO(TOKEN, EXPR, i32,     i32) \
    MACRO(TOKEN, EXPR, u32,     i32) \
    MACRO(TOKEN, EXPR, i32,     u32) \
    MACRO(TOKEN, EXPR, u32,     u32) \
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define APPLY_MACRO_TO_COMPOUND_ASSIGNMENT_IMPL(MACRO) \
    APPLY_MACRO_TO_COMPOUND_ASSIGNMENT_IMPL_FLOAT_INT(MACRO, TokenType::operator_plus_equal,     l+=r) \
    APPLY_MACRO_TO_COMPOUND_ASSIGNMENT_IMPL_FLOAT_INT(MACRO, TokenType::operator_minus_equal,    l-=r) \
    APPLY_MACRO_TO_COMPOUND_ASSIGNMENT_IMPL_FLOAT_INT(MACRO, TokenType::operator_multiply_equal, l*=r) \
    APPLY_MACRO_TO_COMPOUND_ASSIGNMENT_IMPL_FLOAT_INT(MACRO, TokenType::operator_divide_equal,   if(!CheckDivideByZeroROK(io, iToken, r)) return false; l/=r) \
    APPLY_MACRO_TO_COMPOUND_ASSIGNMENT_IMPL_INT(MACRO, TokenType::operator_modulo_equal,         if(!CheckDivideByZeroROK(io, iToken, r)) return false; l%=r) \
    APPLY_MACRO_TO_COMPOUND_ASSIGNMENT_IMPL_INT(MACRO, TokenType::operator_shift_left_equal,  l<<=r) \
    APPLY_MACRO_TO_COMPOUND_ASSIGNMENT_IMPL_INT(MACRO, TokenType::operator_shift_right_equal, l>>=r) \
    MACRO(TokenType::operator_plus_equal, StrConcat(l,r), std::string, std::string) \
    MACRO(TokenType::operator_plus_equal, StrConcat(l,r), std::string, i32) \
    MACRO(TokenType::operator_plus_equal, StrConcat(l,r), std::string, u32) \
    MACRO(TokenType::operator_plus_equal, StrConcat(l,r), std::string, float) \
    MACRO(TokenType::operator_plus_equal, StrConcat(l,r), std::string, bool) \
    MACRO(TokenType::operator_plus_equal, ListConcat(l,r), reflection::PrimitiveDataList, reflection::PrimitiveDataList)
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define DEFINE_COMPOUND_ASSIGNMENT(TOKEN, EXPR, LEFT_TYPE, RIGHT_TYPE) \
    template <> bool CompoundAssignmentImplROK<TOKEN, LEFT_TYPE, RIGHT_TYPE>(EvaluationInputOutput& io, Token const& iToken, reflection::IPrimitiveData* iLeft, reflection::IPrimitiveData const* iRight) \
    { \
        SG_UNUSED((io, iToken)); \
        reflection::PrimitiveData<LEFT_TYPE>* leftData = checked_cast<reflection::PrimitiveData<LEFT_TYPE>*>(iLeft); \
        LEFT_TYPE& l = leftData->GetForWriting(); \
        RIGHT_TYPE r; \
        iRight->As<RIGHT_TYPE>(&r); \
        EXPR; \
        return true; \
    };
APPLY_MACRO_TO_COMPOUND_ASSIGNMENT_IMPL(DEFINE_COMPOUND_ASSIGNMENT)
#undef DEFINE_COMPOUND_ASSIGNMENT
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
CompoundAssignmentEntry const compoundAssignmentEntries[] =
{
#define DECLARE_COMPOUND_ASSIGNMENT_ENTRY(TOKEN, OP, LEFT_TYPE, RIGHT_TYPE) \
    { TOKEN, reflection::PrimitiveDataTraits<LEFT_TYPE>::primitive_data_type, reflection::PrimitiveDataTraits<RIGHT_TYPE>::primitive_data_type, CompoundAssignmentImplROK<TOKEN, LEFT_TYPE, RIGHT_TYPE> },
APPLY_MACRO_TO_COMPOUND_ASSIGNMENT_IMPL(DECLARE_COMPOUND_ASSIGNMENT_ENTRY)
#undef DECLARE_COMPOUND_ASSIGNMENT_ENTRY
};
size_t const compoundAssignmentEntryCount = SG_ARRAYSIZE(compoundAssignmentEntries);
//=============================================================================
bool CompoundAssignment::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    EvaluationInputOutput arg0io(io);
    ok = m_args[0]->EvaluateROK(arg0io);
    if(!ok)
        return false;
    ok = ConvertToValueROK(arg0io, m_args[0]->GetToken());
    if(!ok)
        return false;
    EvaluationInputOutput arg1io(io);
    ok = m_args[1]->EvaluateROK(arg1io);
    if(!ok)
        return false;
    ok = ConvertToValueROK(arg1io, m_args[1]->GetToken());
    if(!ok)
        return false;

    if(io.isPreresolutionPass && (nullptr != arg0io.presesolvedNodeIFN || nullptr != arg1io.presesolvedNodeIFN))
    {
        CompoundAssignment* preresolved = new CompoundAssignment(m_op);
        preresolved->SetToken(this->GetToken());
        {
            refptr<ITreeNode> arg0;
            ok = ConvertToPreresolvedROK(arg0io, arg0, m_args[0]->GetToken());
            if(!ok)
                return false;
            preresolved->SetArgument(0, arg0.get());
        }
        {
            refptr<ITreeNode> arg1;
            ok = ConvertToPreresolvedROK(arg1io, arg1, m_args[1]->GetToken());
            if(!ok)
                return false;
            preresolved->SetArgument(1, arg1.get());
        }
        io.presesolvedNodeIFN = preresolved;
        return true;
    }

    SG_ASSERT(nullptr != arg0io.returnValue);
    SG_ASSERT(arg0io.returnIdentifier.Empty());
    SG_ASSERT(arg0io.qualifiers.empty());
    SG_ASSERT(arg0io.returnValueIsLValue); // TODO: Error message

    SG_ASSERT(nullptr != arg1io.returnValue);
    SG_ASSERT(arg1io.returnIdentifier.Empty());
    SG_ASSERT(arg1io.qualifiers.empty());

    io.returnValueContainsUnresolvedIdentifier = arg0io.returnValueContainsUnresolvedIdentifier || arg1io.returnValueContainsUnresolvedIdentifier;
    arg0io.returnValueContainsUnresolvedIdentifier = false;
    arg1io.returnValueContainsUnresolvedIdentifier = false;
    reflection::PrimitiveDataType leftType = arg0io.returnValue->GetType();
    reflection::PrimitiveDataType rightType = arg1io.returnValue->GetType();
    for(size_t i = 0; i < compoundAssignmentEntryCount; ++i)
    {
        if(compoundAssignmentEntries[i].op == m_op && compoundAssignmentEntries[i].leftType == leftType && compoundAssignmentEntries[i].rightType == rightType)
        {
            ok = compoundAssignmentEntries[i].impl(io, GetToken(), arg0io.returnValue.get(), arg1io.returnValue.get());
            io.returnValue = arg0io.returnValue;
            io.returnValueIsLValue = arg0io.returnValueIsLValue;
            io.returnLValueReference = arg0io.returnLValueReference;
            return ok;
        }
    }
    PushError(io, ErrorType::unsupported_type_for_compound_assignment, GetToken(), "unsupported type for operation");
    return false;
}
//=============================================================================
bool TernaryExpression::EvaluateROK(EvaluationInputOutput& io)
{
    bool ok = true;
    EvaluationInputOutput arg0io(io);
    ok = m_args[0]->EvaluateROK(arg0io);
    if(!ok)
        return false;
    ok = ConvertToValueROK(arg0io, m_args[0]->GetToken());
    if(!ok)
        return false;
    EvaluationInputOutput arg1io(io);
    ok = m_args[1]->EvaluateROK(arg1io);
    if(!ok)
        return false;
    ok = ConvertToValueROK(arg1io, m_args[1]->GetToken());
    if(!ok)
        return false;
    EvaluationInputOutput arg2io(io);
    ok = m_args[2]->EvaluateROK(arg2io);
    if(!ok)
        return false;
    ok = ConvertToValueROK(arg2io, m_args[2]->GetToken());
    if(!ok)
        return false;

    if(io.isPreresolutionPass && (nullptr != arg0io.presesolvedNodeIFN || nullptr != arg1io.presesolvedNodeIFN || nullptr != arg2io.presesolvedNodeIFN))
    {
        TernaryExpression* preresolved = new TernaryExpression(TokenType::operator_interrogation);
        preresolved->SetToken(this->GetToken());
        {
            refptr<ITreeNode> arg0;
            ok = ConvertToPreresolvedROK(arg0io, arg0, m_args[0]->GetToken());
            if(!ok)
                return false;
            preresolved->SetArgument(0, arg0.get());
        }
        {
            refptr<ITreeNode> arg1;
            ok = ConvertToPreresolvedROK(arg1io, arg1, m_args[1]->GetToken());
            if(!ok)
                return false;
            preresolved->SetCenterNode(arg1.get());
        }
        {
            refptr<ITreeNode> arg2;
            ok = ConvertToPreresolvedROK(arg2io, arg2, m_args[2]->GetToken());
            if(!ok)
                return false;
            preresolved->SetArgument(1, arg2.get());
        }
        io.presesolvedNodeIFN = preresolved;
        return true;
    }

    SG_ASSERT(nullptr != arg0io.returnValue);
    SG_ASSERT(arg0io.returnIdentifier.Empty());
    SG_ASSERT(arg0io.qualifiers.empty());
    SG_ASSERT(nullptr != arg1io.returnValue);
    SG_ASSERT(arg1io.returnIdentifier.Empty());
    SG_ASSERT(arg1io.qualifiers.empty());
    SG_ASSERT(nullptr != arg2io.returnValue);
    SG_ASSERT(arg2io.returnIdentifier.Empty());
    SG_ASSERT(arg2io.qualifiers.empty());

    SG_ASSERT(!arg0io.returnValueContainsUnresolvedIdentifier);
    reflection::PrimitiveDataType type0 = arg0io.returnValue->GetType();
    reflection::PrimitiveDataType type1 = arg1io.returnValue->GetType();
    reflection::PrimitiveDataType type2 = arg2io.returnValue->GetType();
    SG_UNUSED((type0, type1, type2));
    bool cond;
    bool isCondOk = arg0io.returnValue->AsROK<bool>(&cond);
    if(!isCondOk)
    {
        PushError(io, ErrorType::incorrect_type_for_ternary_operator_condition, m_args[0]->GetToken(), "incorrect type for ternary operator condition");
        return false;
    }
    if(cond)
    {
        io.returnValue = arg1io.returnValue;
        io.returnValueContainsUnresolvedIdentifier = arg1io.returnValueContainsUnresolvedIdentifier;
        arg1io.returnValueContainsUnresolvedIdentifier = false;
    }
    else
    {
        io.returnValue = arg2io.returnValue;
        io.returnValueContainsUnresolvedIdentifier = arg2io.returnValueContainsUnresolvedIdentifier;
        arg2io.returnValueContainsUnresolvedIdentifier = false;
    }
    return ok;
}
//=============================================================================
}
}
}

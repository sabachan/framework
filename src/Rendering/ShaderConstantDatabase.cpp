#include "stdafx.h"

#include "ShaderConstantDatabase.h"

namespace sg {
namespace rendering {
//=============================================================================
FAST_SYMBOL_TYPE_IMPL(ShaderConstantName)
//=============================================================================
IShaderVariable const* IShaderConstantDatabase::GetConstant(ShaderConstantName iName) const
{
    IShaderVariable const* var = GetConstantIFP(iName);
    if(nullptr == var)
    {
        SG_CODE_FOR_ASSERT(std::string nameAsString = iName.Value());
        SG_ASSERT_MSG(false, "variable not in database");
        return nullptr;
    }
    else
        return var;
}
//=============================================================================
ShaderConstantDatabase::ShaderConstantDatabase()
    : m_constants()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ShaderConstantDatabase::~ShaderConstantDatabase()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ShaderConstantDatabase::AddVariable(ShaderConstantName iName, IShaderVariable* iValue)
{
    auto r = m_constants.insert(std::make_pair(iName, iValue));
    SG_ASSERT_MSG(r.second, "already in database");
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
IShaderVariable* ShaderConstantDatabase::GetConstantForWriting(ShaderConstantName iName)
{
    auto f = m_constants.find(iName);
    SG_ASSERT_MSG(m_constants.end() != f, "variable not in database");
    if(m_constants.end() != f)
        return f->second.get();
    else
        return nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
IShaderVariable const* ShaderConstantDatabase::GetConstantIFP(ShaderConstantName iName) const
{
    auto f = m_constants.find(iName);
    if(m_constants.end() != f)
        return f->second.get();
    else
        return nullptr;
}
//=============================================================================
template <bool keepRefOnFirst, bool keepRefOnSecond>
ShaderConstantDatabasePair<keepRefOnFirst, keepRefOnSecond>::ShaderConstantDatabasePair(IShaderConstantDatabase const* iFirstDatabase, IShaderConstantDatabase const* iSecondDatabase)
: m_firstDatabase(iFirstDatabase)
, m_secondDatabase(iSecondDatabase)
{
    SG_ASSERT(nullptr != iFirstDatabase);
    SG_ASSERT(nullptr != iSecondDatabase);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <bool keepRefOnFirst, bool keepRefOnSecond>
ShaderConstantDatabasePair<keepRefOnFirst, keepRefOnSecond>::~ShaderConstantDatabasePair()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <bool keepRefOnFirst, bool keepRefOnSecond>
IShaderVariable const* ShaderConstantDatabasePair<keepRefOnFirst, keepRefOnSecond>::GetConstantIFP(ShaderConstantName iName) const
{
    SG_ASSERT(nullptr != m_firstDatabase);
    SG_ASSERT(nullptr != m_secondDatabase);

    IShaderVariable const* var =  m_firstDatabase->GetConstantIFP(iName);
    if(nullptr != var)
        return var;
    return m_secondDatabase->GetConstantIFP(iName);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template class ShaderConstantDatabasePair<true, false>;
template class ShaderConstantDatabasePair<false, true>;
template class ShaderConstantDatabasePair<false, false>;
//=============================================================================
}
}

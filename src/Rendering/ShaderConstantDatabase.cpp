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
void ShaderConstantDatabase::AddReference(ShaderConstantName iName, IShaderVariable const* iValue)
{
    Entry entry;
    entry.reference = iValue;
    auto r = m_constants.insert(std::make_pair(iName, entry));
    SG_ASSERT_MSG(r.second, "already in database");
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ShaderConstantDatabase::AddVariable(ShaderConstantName iName, IShaderVariable* iValue)
{
    Entry entry;
    entry.reference = iValue;
    entry.variable = iValue;
    auto r = m_constants.insert(std::make_pair(iName, entry));
    SG_ASSERT_MSG(r.second, "already in database");
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ShaderConstantDatabase::RemoveVariable(ShaderConstantName iName)
{
    auto f = m_constants.find(iName);
    SG_ASSERT_MSG(m_constants.end() != f, "variable not in database");
    if(m_constants.end() != f)
        m_constants.erase(f);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ShaderConstantDatabase::Clear()
{
    m_constants.clear();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
IShaderVariable* ShaderConstantDatabase::GetConstantForWriting(ShaderConstantName iName)
{
    auto f = m_constants.find(iName);
    SG_ASSERT_MSG(m_constants.end() != f, "variable not in database");
    if(m_constants.end() != f)
    {
        SG_ASSERT_MSG(nullptr != f->second.variable, "variable is not writable");
        return f->second.variable.get();
    }
    else
        return nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
IShaderVariable const* ShaderConstantDatabase::GetConstantIFP(ShaderConstantName iName) const
{
    auto f = m_constants.find(iName);
    if(m_constants.end() != f)
        return f->second.reference.get();
    else
        return nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
size_t ShaderConstantDatabase::ComputeHash() const
{
    size_t const rot = 7;
    size_t h = m_constants.size();
    for(auto const& it : m_constants)
    {
        BitRotate<rot>(h);
        h ^= it.first.Hash();
        BitRotate<rot>(h);
        h ^= ptrdiff_t(it.second.reference.get());
    }
    return h;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool ShaderConstantDatabase::IsSameAs(ShaderConstantDatabase const& other) const
{
    auto const& variables1 = m_constants;
    auto const& variables2 = other.m_constants;
    if(variables1.size() != variables2.size())
        return false;
    {
        auto begin1 = variables1.begin();
        auto begin2 = variables2.begin();
        auto end1 = variables1.end();
        auto end2 = variables2.end();
        for(auto it1 = begin1, it2 = begin2; it1 != end1; ++it1, ++it2)
        {
            SG_ASSERT(it2 != end2);
            if(it1->first != it2->first)
                return false;
            if(it1->second.reference != it2->second.reference)
                return false;
        }
    }
    return true;
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

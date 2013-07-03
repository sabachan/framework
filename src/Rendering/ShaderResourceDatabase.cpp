#include "stdafx.h"

#include "ShaderResourceDatabase.h"

#include "ShaderResource.h"
#include <d3d11.h>

namespace sg {
namespace rendering {
//=============================================================================
FAST_SYMBOL_TYPE_IMPL(ShaderResourceName)
//=============================================================================
IShaderResource const* IShaderResourceDatabase::GetResource(ShaderResourceName iName) const
{
    IShaderResource const* shaderResource = GetResourceIFP(iName);
    if(nullptr == shaderResource)
    {
        SG_CODE_FOR_ASSERT(std::string nameAsString = iName.Value());
        SG_ASSERT_MSG(false, "resource not in database");
        return nullptr;
    }
    else
        return shaderResource;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ID3D11SamplerState* IShaderResourceDatabase::GetSampler(ShaderResourceName iName) const
{
    ID3D11SamplerState* sampler = GetSamplerIFP(iName);
    if(nullptr == sampler)
    {
        SG_CODE_FOR_ASSERT(std::string nameAsString = iName.Value());
        SG_ASSERT_MSG(false, "sampler not in database");
        return nullptr;
    }
    else
        return sampler;
}
//=============================================================================
ShaderResourceDatabase::ShaderResourceDatabase()
    : m_resources()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ShaderResourceDatabase::~ShaderResourceDatabase()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ShaderResourceDatabase::AddResource(ShaderResourceName iName, IShaderResource const* iResource)
{
    SG_ASSERT(nullptr != iResource);
    auto const r = m_resources.insert(std::make_pair(iName, iResource));
    SG_ASSERT_MSG(r.second, "already in database");
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ShaderResourceDatabase::RemoveResource(ShaderResourceName iName, IShaderResource const* iResource)
{
    SG_ASSERT_AND_UNUSED(nullptr != iResource);
    auto const f = m_resources.find(iName);
    SG_ASSERT_MSG(m_resources.end() != f, "resource not in database");
    SG_ASSERT(f->second == iResource);
    auto const next = m_resources.erase(f);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
IShaderResource const* ShaderResourceDatabase::GetResourceIFP(ShaderResourceName iName) const
{
    auto f = m_resources.find(iName);
    if(m_resources.end() != f)
        return f->second.get();
    else
        return nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ShaderResourceDatabase::AddSampler(ShaderResourceName iName, ID3D11SamplerState* iSampler)
{
    SG_ASSERT(nullptr != iSampler);
    auto const r = m_samplers.emplace(iName, iSampler);
    SG_ASSERT_MSG(r.second, "already in database");
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ShaderResourceDatabase::RemoveSampler(ShaderResourceName iName, ID3D11SamplerState* iSampler)
{
    SG_ASSERT_AND_UNUSED(nullptr != iSampler);
    auto const f = m_samplers.find(iName);
    SG_ASSERT_MSG(m_samplers.end() != f, "sampler not in database");
    SG_ASSERT(f->second == iSampler);
    auto const next = m_samplers.erase(f);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ID3D11SamplerState* ShaderResourceDatabase::GetSamplerIFP(ShaderResourceName iName) const
{
    auto f = m_samplers.find(iName);
    if(m_samplers.end() != f)
        return f->second.get();
    else
        return nullptr;
}
//=============================================================================
template <bool keepRefOnFirst, bool keepRefOnSecond>
ShaderResourceDatabasePair<keepRefOnFirst, keepRefOnSecond>::ShaderResourceDatabasePair(IShaderResourceDatabase const* iFirstDatabase, IShaderResourceDatabase const* iSecondDatabase)
: m_firstDatabase(iFirstDatabase)
, m_secondDatabase(iSecondDatabase)
{
    SG_ASSERT(nullptr != iFirstDatabase);
    SG_ASSERT(nullptr != iSecondDatabase);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <bool keepRefOnFirst, bool keepRefOnSecond>
ShaderResourceDatabasePair<keepRefOnFirst, keepRefOnSecond>::~ShaderResourceDatabasePair()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <bool keepRefOnFirst, bool keepRefOnSecond>
IShaderResource const* ShaderResourceDatabasePair<keepRefOnFirst, keepRefOnSecond>::GetResourceIFP(ShaderResourceName iName) const
{
    SG_ASSERT(nullptr != m_firstDatabase);
    SG_ASSERT(nullptr != m_secondDatabase);
    IShaderResource const* resource =  m_firstDatabase->GetResourceIFP(iName);
    if(nullptr != resource)
        return resource;

    return m_secondDatabase->GetResourceIFP(iName);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <bool keepRefOnFirst, bool keepRefOnSecond>
ID3D11SamplerState* ShaderResourceDatabasePair<keepRefOnFirst, keepRefOnSecond>::GetSamplerIFP(ShaderResourceName iName) const
{
    SG_ASSERT(nullptr != m_firstDatabase);
    SG_ASSERT(nullptr != m_secondDatabase);

    ID3D11SamplerState* sampler =  m_firstDatabase->GetSamplerIFP(iName);
    if(nullptr != sampler)
        return sampler;
    return m_secondDatabase->GetSamplerIFP(iName);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template class ShaderResourceDatabasePair<true, false>;
template class ShaderResourceDatabasePair<false, true>;
template class ShaderResourceDatabasePair<false, false>;
//=============================================================================
}
}

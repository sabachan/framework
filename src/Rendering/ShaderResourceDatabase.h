#ifndef Rendering_ShaderResourceDatabase_H
#define Rendering_ShaderResourceDatabase_H

#include <Core/Assert.h>
#include <Core/ComPtr.h>
#include <Core/FastSymbol.h>
#include <Core/IntTypes.h>
#include <Core/SmartPtr.h>
#include "ShaderResource.h"
#include <unordered_map>
#include <d3d11.h>

struct ID3D11SamplerState;

namespace sg {
namespace rendering {
//=============================================================================
class IShaderResource;
//=============================================================================
FAST_SYMBOL_TYPE_HEADER(ShaderResourceName)
//=============================================================================
class IShaderResourceDatabase : public RefAndSafeCountable
{
public:
    virtual ~IShaderResourceDatabase() {}
    IShaderResource const* GetResource(ShaderResourceName iName) const;
    virtual IShaderResource const* GetResourceIFP(ShaderResourceName iName) const = 0;
    ID3D11SamplerState* GetSampler(ShaderResourceName iName) const;
    virtual ID3D11SamplerState* GetSamplerIFP(ShaderResourceName iName) const = 0;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class ShaderResourceDatabase : public IShaderResourceDatabase
{
    ShaderResourceDatabase& operator=(ShaderResourceDatabase const&) = delete;
public:
    ShaderResourceDatabase();
    virtual ~ShaderResourceDatabase() override;
    ShaderResourceDatabase(ShaderResourceDatabase const&) = default;
    void AddResource(ShaderResourceName iName, IShaderResource const* iResource);
    void RemoveResource(ShaderResourceName iName, IShaderResource const* iResource);
    virtual IShaderResource const* GetResourceIFP(ShaderResourceName iName) const override;
    std::unordered_map<ShaderResourceName, refptr<IShaderResource const> > const& Resources() const { return m_resources; }

    void AddSampler(ShaderResourceName iName, ID3D11SamplerState* iSampler);
    void RemoveSampler(ShaderResourceName iName, ID3D11SamplerState* iSampler);
    virtual ID3D11SamplerState* GetSamplerIFP(ShaderResourceName iName) const override;
    std::unordered_map<ShaderResourceName, comptr<ID3D11SamplerState> > const& Samplers() const { return m_samplers; }
private:
    std::unordered_map<ShaderResourceName, refptr<IShaderResource const> > m_resources;
    std::unordered_map<ShaderResourceName, comptr<ID3D11SamplerState> > m_samplers;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <bool keepRefOnFirst, bool keepRefOnSecond>
class ShaderResourceDatabasePair : public IShaderResourceDatabase
{
    SG_NON_COPYABLE(ShaderResourceDatabasePair)
public:
    ShaderResourceDatabasePair(IShaderResourceDatabase const* iFirstDatabase = nullptr, IShaderResourceDatabase const* iSecondDatabase = nullptr);
    virtual ~ShaderResourceDatabasePair() override;
    virtual IShaderResource const* GetResourceIFP(ShaderResourceName iName) const override;
    virtual ID3D11SamplerState* GetSamplerIFP(ShaderResourceName iName) const override;
private:
    std::unordered_map<ShaderResourceName, refptr<IShaderResource const> > m_resources;
    std::unordered_map<ShaderResourceName, comptr<ID3D11SamplerState> > m_samplers;
    reforsafeptr<IShaderResourceDatabase const, keepRefOnFirst> const m_firstDatabase;
    reforsafeptr<IShaderResourceDatabase const, keepRefOnSecond> const m_secondDatabase;
};
//=============================================================================
}
}

#endif

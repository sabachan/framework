#ifndef RenderEngine_MaterialDescriptor_H
#define RenderEngine_MaterialDescriptor_H

//#include "ShaderResourceDescriptor.h"
#include <Reflection/BaseClass.h>
#include <Rendering/Material.h>
#include <Rendering/RenderStateDico.h>
#include <Rendering/Shader.h>
#include <Rendering/ShaderResourceDatabase.h>
#include <Core/ArrayList.h>
#include <Core/ComPtr.h>
#include <Core/FilePath.h>
#include <Core/HashMap.h>
#include <Core/SmartPtr.h>

namespace sg {
namespace renderengine {
//=============================================================================
class VertexShaderDescriptor;
class PixelShaderDescriptor;
//=============================================================================
class MaterialDescriptorCache
{
    typedef rendering::Material::ConstantsView ConstantsView;
    typedef rendering::Material::ResourcesView ResourcesView;
    typedef rendering::Material::SamplersView SamplersView;
public:
    rendering::Material const* GetMaterial(
        ArrayView<D3D11_INPUT_ELEMENT_DESC const> iInputLayout,
        rendering::VertexShaderProxy iVertexShader,
        rendering::PixelShaderProxy iPixelShader,
        rendering::RenderStateName iBlendState,
        ConstantsView iConstants,
        ResourcesView iResources,
        SamplersView iSamplers,
        int iPriority);
private:
    struct LightKey
    {
        rendering::ShaderInputLayoutProxy inputLayout;
        //ConstantsView constants;
        ResourcesView resources;
        //SamplersView samplers;
    };
    struct Key
    {
        rendering::ShaderInputLayoutProxy inputLayout;
        //ConstantsView constants;
        ArrayList<std::pair<rendering::ShaderResourceName, void const*> > resources;
        //SamplersView samplers;

        Key() {}
        Key(LightKey const& iOther) : inputLayout(iOther.inputLayout) { resources.Reserve(iOther.resources.Size()); for(auto const& it : iOther.resources) { resources.EmplaceBack_AssumeCapacity(it.first, it.second); }}
    };
    struct HasherComparer
    {
        template <typename RV>
        static size_t Hash(
            rendering::ShaderInputLayoutProxy iInputLayout,
            //ConstantsView iConstants,
            RV iResources)
            //SamplersView iSamplers)
        {
            size_t h = 0x13;
            h ^= iInputLayout.Hash();
            h *= 0x69ab1337;
            h ^= iResources.Size();
            h *= 0x69ab1337;
            for(auto& it : iResources)
            {
                size_t h2 = it.first.Hash();
                h2 *= 0xa2743533;
                h2 ^= size_t(it.second);
                h2 *= 0xa2743533;
                h ^= h2;
            }
            h ^= h >> 15;
            return h;
        }
        size_t operator()(LightKey const& k) const { return Hash(k.inputLayout, k.resources); }
        size_t operator()(Key const& k) const { return Hash(k.inputLayout, k.resources.View()); }
        template <typename A, typename B>
        static bool Compare(A const& a, B const& b)
        {
            if(a.inputLayout != b.inputLayout)
                return false;
            size_t const asize = a.resources.Size();
            size_t const bsize = b.resources.Size();
            if(asize != bsize)
                return false;
            SG_ASSERT(asize <= 4); // else, maybe we need better complexity?
            u32 found = 0;
            for_range(size_t, i, 0, asize)
            {
                for_range(size_t, j, 0, bsize)
                {
                    if((0 == ((found >> j) & 1)) && a.resources[i].first == b.resources[j].first && a.resources[i].second == b.resources[j].second)
                    {
                        found |= 1 << j;
                        break;
                    }
                }
            }
            if(found != ((u32(1) << asize) - 1))
                return false;
            return true;
        }
        bool operator()(LightKey const& a, LightKey const& b) const { return Compare(a, b); }
        bool operator()(LightKey const& a, Key const& b) const { return Compare(a, b); }
        bool operator()(Key const& a, LightKey const& b) const { return Compare(a, b); }
        bool operator()(Key const& a, Key const& b) const { return Compare(a, b); }
    };

    // TODO: This dico should be emptied once in a while to prevent java leak.
    // It should be great not to prevent the release of the IShaderResource,
    // however, that means we need a weak reference or something like that...
    // Note that the material also keeps a reference on resource.
    HashMap<Key, refptr<rendering::Material const>, HasherComparer, HasherComparer> m_materials;
};
//=============================================================================
class MaterialDescriptor : public reflection::BaseClass
{
    REFLECTION_CLASS_HEADER(MaterialDescriptor, reflection::BaseClass)
public:
    typedef rendering::Material::ConstantsView ConstantsView;
    typedef rendering::Material::ResourcesView ResourcesView;
    typedef rendering::Material::SamplersView SamplersView;

    bool IsValid() const; // { return !m_file.Empty(); }
protected:
    MaterialDescriptor();
public:
    virtual ~MaterialDescriptor() override;
    rendering::Material const* GetMaterial(
        ArrayView<D3D11_INPUT_ELEMENT_DESC const> iInputLayout,
        ConstantsView iConstants = nullptr,
        ResourcesView iResources = nullptr,
        SamplersView iSamplers = nullptr);
private:
    void CreateCache();
    void OnCreatedOrModified();
protected:
    virtual void VirtualOnCreated(reflection::ObjectCreationContext& iContext) { reflection_parent_type::VirtualOnCreated(iContext); OnCreatedOrModified(); }
    virtual void VirtualOnModified(reflection::ObjectModificationContext& iContext) { reflection_parent_type::VirtualOnModified(iContext); OnCreatedOrModified(); }
#if ENABLE_REFLECTION_PROPERTY_CHECK
    virtual void VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const override;
#endif
private:
    scopedptr<MaterialDescriptorCache> m_cache;

    refptr<VertexShaderDescriptor const> m_vertexShader;
    refptr<PixelShaderDescriptor const> m_pixelShader;
    rendering::RenderStateName m_blendState;
    i32 m_priority {};

    // TODO:
    // vertex descriptor
    // textures
    // constants
};
//=============================================================================
}
}

#endif

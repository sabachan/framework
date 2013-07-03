#ifndef RenderEngine_SamplerDescriptors_H
#define RenderEngine_SamplerDescriptors_H

#include <Core/ComPtr.h>
#include <Math/Vector.h>
#include <Reflection/BaseClass.h>

struct ID3D11SamplerState;

namespace sg {
namespace rendering {
    class RenderDevice;
}
}

namespace sg {
namespace renderengine {
//=============================================================================
class SamplerDescriptor : public reflection::BaseClass
{
    REFLECTION_CLASS_HEADER(SamplerDescriptor, reflection::BaseClass)
public:
    enum class Filter
    {
        Point,
        Linear,
        Anisotropic,
    };
    enum class AdressMode
    {
        Wrap,
        Mirror,
        Clamp,
        Border,
        MirrorOnce,
    };
public:
    SamplerDescriptor();
    virtual ~SamplerDescriptor() override;
    void CreateSamplerState(rendering::RenderDevice const* iRenderDevice, comptr<ID3D11SamplerState>& oSampler) const;
protected:
#if ENABLE_REFLECTION_PROPERTY_CHECK
    virtual void VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const override;
#endif
private:
    Filter m_filter;
    AdressMode m_adressMode;
    u32 m_maxAnisotropy;
    float4 m_borderColor;
};
REFLECTION_ENUM_HEADER(SamplerDescriptor::Filter)
REFLECTION_ENUM_HEADER(SamplerDescriptor::AdressMode)
//=============================================================================
}
}

#endif

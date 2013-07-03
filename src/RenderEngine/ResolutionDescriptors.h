#ifndef RenderEngine_ResolutionDescriptors_H
#define RenderEngine_ResolutionDescriptors_H

#include <Core/SmartPtr.h>
#include <Reflection/BaseClass.h>
#include <Rendering/ResolutionServer.h>

namespace sg {
namespace rendering {
    class IRenderTarget;
    class IShaderResource;
    class Surface;
}
}

namespace sg {
namespace renderengine {
//=============================================================================
class AbstractResolutionDescriptor;
class AbstractSurfaceDescriptor;
class Compositing;
class InputSurfaceDescriptor;
class OutputSurfaceDescriptor;
class TextureSurfaceDescriptor;
//=============================================================================
class ResolutionInstance: public RefCountable
                        , public rendering::ResolutionServer
                        , private Observer<rendering::ResolutionServer>
{
    PARENT_SAFE_COUNTABLE(rendering::ResolutionServer)
public:
    static const size_t MAX_PARENT_COUNT = 2;
    ResolutionInstance(AbstractResolutionDescriptor const* iDescriptor, rendering::ResolutionServer const*const (&iParent)[MAX_PARENT_COUNT]);
    virtual ~ResolutionInstance() override;
private:
    virtual void VirtualOnNotified(ResolutionServer const* iResolutionServer) override;
    void Update();
private:
    safeptr<AbstractResolutionDescriptor const> m_descriptor;
    safeptr<rendering::ResolutionServer const> m_parents[MAX_PARENT_COUNT];
};
//=============================================================================
class AbstractResolutionDescriptor : public reflection::BaseClass
{
    REFLECTION_CLASS_HEADER(AbstractResolutionDescriptor, reflection::BaseClass)
public:
    AbstractResolutionDescriptor();
    virtual ~AbstractResolutionDescriptor() override;
    virtual rendering::ResolutionServer const* CreateInstance(Compositing* iCompositing) const = 0;
protected:
    friend class ResolutionInstance;
    virtual uint2 ComputeResolution(uint2 const (&iParentResolutions)[ResolutionInstance::MAX_PARENT_COUNT]) const = 0;

};
//=============================================================================
class ResolutionFromInputSurfaceDescriptor : public AbstractResolutionDescriptor
{
    REFLECTION_CLASS_HEADER(ResolutionFromInputSurfaceDescriptor, AbstractResolutionDescriptor)
public:
    ResolutionFromInputSurfaceDescriptor();
    virtual ~ResolutionFromInputSurfaceDescriptor() override;
    virtual rendering::ResolutionServer const* CreateInstance(Compositing* iCompositing) const override;
private:
    virtual uint2 ComputeResolution(uint2 const (&iParentResolutions)[ResolutionInstance::MAX_PARENT_COUNT]) const override;
private:
    refptr<InputSurfaceDescriptor> m_surface;
};
//=============================================================================
class ResolutionFromOutputSurfaceDescriptor : public AbstractResolutionDescriptor
{
    REFLECTION_CLASS_HEADER(ResolutionFromOutputSurfaceDescriptor, AbstractResolutionDescriptor)
public:
    ResolutionFromOutputSurfaceDescriptor();
    virtual ~ResolutionFromOutputSurfaceDescriptor() override;
    virtual rendering::ResolutionServer const* CreateInstance(Compositing* iCompositing) const override;
private:
    virtual uint2 ComputeResolution(uint2 const (&iParentResolutions)[ResolutionInstance::MAX_PARENT_COUNT]) const override;
private:
    refptr<OutputSurfaceDescriptor> m_surface;
};
//=============================================================================
class ResolutionFromTextureSurfaceDescriptor : public AbstractResolutionDescriptor
{
    REFLECTION_CLASS_HEADER(ResolutionFromTextureSurfaceDescriptor, AbstractResolutionDescriptor)
public:
    ResolutionFromTextureSurfaceDescriptor();
    virtual ~ResolutionFromTextureSurfaceDescriptor() override;
    virtual rendering::ResolutionServer const* CreateInstance(Compositing* iCompositing) const override;
private:
    virtual uint2 ComputeResolution(uint2 const (&iParentResolutions)[ResolutionInstance::MAX_PARENT_COUNT]) const override;
private:
    refptr<TextureSurfaceDescriptor> m_surface;
};
//=============================================================================
class ResolutionDescriptor : public AbstractResolutionDescriptor
{
    REFLECTION_CLASS_HEADER(ResolutionDescriptor, AbstractResolutionDescriptor)
public:
    enum class RoundingMode { Floor, Round, Ceil };
public:
    ResolutionDescriptor();
    virtual ~ResolutionDescriptor() override;
    virtual rendering::ResolutionServer const* CreateInstance(Compositing* iCompositing) const override;
private:
    virtual uint2 ComputeResolution(uint2 const (&iParentResolutions)[ResolutionInstance::MAX_PARENT_COUNT]) const override;
#if ENABLE_REFLECTION_PROPERTY_CHECK
    virtual void VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const override;
#endif
private:
    refptr<AbstractResolutionDescriptor> m_parentx;
    refptr<AbstractResolutionDescriptor> m_parenty;
    float2 m_scale;
    RoundingMode m_roundingMode;
    uint2 m_additiveTerm;
    SG_CODE_FOR_ASSERT(mutable bool m_isInCreateInstance;)
};
REFLECTION_ENUM_HEADER(ResolutionDescriptor::RoundingMode)
//=============================================================================
}
}

#endif

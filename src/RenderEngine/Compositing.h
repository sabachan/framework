#ifndef RenderEngine_Compositing_H
#define RenderEngine_Compositing_H

#include <Core/SmartPtr.h>
#include <Core/ArrayView.h>
#include <Reflection/BaseClass.h>
#include <Rendering/RenderTarget.h>
#include <Rendering/ShaderConstantDatabase.h>
#include <Rendering/ShaderResource.h>
#include <Rendering/ShaderResourceDatabase.h>

namespace sg {
namespace rendering {
    class BaseSurface;
    class BCSurface;
    class DepthStencilSurface;
    class RenderDevice;
    class Surface;
    class TextureFromFile;
}
}

namespace sg {
namespace renderengine {
//=============================================================================
class AbstractCompositingInstructionDescriptor;
class AbstractConstantDatabaseDescriptor;
class AbstractResolutionDescriptor;
class AbstractShaderResourceDatabaseDescriptor;
class AbstractSurfaceDescriptor;
class BCSurfaceDescriptor;
class CompositingDescriptor;
class CompositingLayer;
class CompositingLayerDescriptor;
class ConstantDatabaseDescriptor;
class DepthStencilSurfaceDescriptor;
class ICompositingInstruction;
class InputConstantDatabaseDescriptor;
class InputShaderResourceDatabaseDescriptor;
class InputSurfaceDescriptor;
class OutputSurfaceDescriptor;
class QuadForShaderPass;
class RenderBatch;
class ResolutionInstance;
class ShaderResourceDatabaseDescriptor;
class SurfaceDescriptor;
class TextureSurfaceDescriptor;
//=============================================================================
class ICompositing : public RefAndSafeCountable
{
public:
    virtual ~ICompositing() {};
    virtual bool HasInstructionBlock(FastSymbol iInstructionBlockName) = 0;
    virtual void Execute(FastSymbol iInstructionBlockName) = 0;
    virtual CompositingLayer* GetLayer(FastSymbol iInstructionBlockName, ArrayView<FastSymbol const> const& iSpecRequest) = 0;
    virtual rendering::RenderDevice const* RenderDevice() const = 0;
    CompositingLayer* GetLayer(FastSymbol iInstructionBlockName, FastSymbol iSpecRequest)
    {
        return GetLayer(iInstructionBlockName, ArrayView<FastSymbol const>(&iSpecRequest, 1));
    }
    CompositingLayer* GetLayer(FastSymbol iInstructionBlockName, FastSymbol iSpecRequest0, FastSymbol iSpecRequest1)
    {
        FastSymbol const specRequest[] = { iSpecRequest0, iSpecRequest1 };
        return GetLayer(iInstructionBlockName, AsArrayView(specRequest));
    }
};
//=============================================================================
class Compositing
    : public ICompositing
{
    friend class CompositingDescriptor;
private:
    Compositing(CompositingDescriptor const* iDescriptor,
                rendering::RenderDevice const* iRenderDevice,
                ArrayView<rendering::IShaderResource*> const& iInputSurfaces,
                ArrayView<rendering::IRenderTarget*> const& iOutputSurfaces,
                ArrayView<rendering::IShaderConstantDatabase*> const& iConstantDatabases,
                ArrayView<rendering::IShaderResourceDatabase*> const& iShaderResourceDatabases);
public:
    ~Compositing();
    virtual bool HasInstructionBlock(FastSymbol iInstructionBlockName) override;
    virtual void Execute(FastSymbol iInstructionBlockName) override;
    virtual CompositingLayer* GetLayer(FastSymbol iInstructionBlockName, ArrayView<FastSymbol const> const& iSpecRequest) override;
    virtual rendering::RenderDevice const* RenderDevice() const override { return m_renderDevice.get(); }

    rendering::ResolutionServer const* GetResolutionServer(AbstractResolutionDescriptor const* iResolutionDescriptor);
    rendering::IRenderTarget const* GetRenderTarget(OutputSurfaceDescriptor const* iSurfaceDescriptor);
    rendering::IShaderResource const* GetShaderResource(InputSurfaceDescriptor const* iSurfaceDescriptor);
    rendering::IShaderResource const* GetShaderResource(TextureSurfaceDescriptor const* iSurfaceDescriptor);
    rendering::Surface const* GetSurface(SurfaceDescriptor const* iSurfaceDescriptor);
    rendering::BCSurface const* GetBCSurface(BCSurfaceDescriptor const* iSurfaceDescriptor);
    rendering::DepthStencilSurface const* GetDepthStencilSurface(DepthStencilSurfaceDescriptor const* iSurfaceDescriptor);
    rendering::IShaderConstantDatabase const* GetShaderConstantDatabase(AbstractConstantDatabaseDescriptor const* iDescriptor);
    rendering::IShaderResourceDatabase const* GetShaderResourceDatabase(AbstractShaderResourceDatabaseDescriptor const* iDescriptor);
    rendering::IShaderConstantDatabase const* GetCurrentShaderConstantDatabase() const { return m_currentShaderConstantDatabase.get(); }
    rendering::IShaderResourceDatabase const* GetCurrentShaderResourceDatabase() const { return m_currentShaderResourceDatabase.get(); }
    void SetCurrentShaderConstantDatabase(rendering::IShaderConstantDatabase const* iDatabase);
    void SetCurrentShaderResourceDatabase(rendering::IShaderResourceDatabase const* iDatabase);
    QuadForShaderPass* GetQuadForShaderPass();

    void RegisterResolutionInstanceForHandling(ResolutionInstance* iInstance);
private:
    rendering::BaseSurface const* GetBaseSurface(AbstractSurfaceDescriptor const* iSurfaceDescriptor);
private:
    safeptr<CompositingDescriptor const> m_descriptor;
    safeptr<rendering::RenderDevice const> m_renderDevice;
    std::unordered_map<safeptr<TextureSurfaceDescriptor const>, refptr<rendering::TextureFromFile const> > m_textures;
    std::vector<refptr<ResolutionInstance> > m_resolutionInstanceHandles;
    std::unordered_map<safeptr<AbstractResolutionDescriptor const>, safeptr<rendering::ResolutionServer const> > m_resolutionServers;
    std::vector<safeptr<rendering::IShaderResource> > m_inputSurfaces;
    std::vector<safeptr<rendering::IRenderTarget> > m_outputSurfaces;
    std::unordered_map<safeptr<AbstractSurfaceDescriptor const>, refptr<rendering::BaseSurface> > m_surfaces;
    std::unordered_map<safeptr<AbstractConstantDatabaseDescriptor const>, refptr<rendering::IShaderConstantDatabase> > m_constantDatabases;
    std::unordered_map<safeptr<AbstractShaderResourceDatabaseDescriptor const>, refptr<rendering::IShaderResourceDatabase> > m_shaderResourceDatabases;
    safeptr<rendering::IShaderConstantDatabase const> m_currentShaderConstantDatabase;
    safeptr<rendering::IShaderResourceDatabase const> m_currentShaderResourceDatabase;
    std::unordered_map<FastSymbol, std::vector<refptr<ICompositingInstruction> > > m_instructions;
    scopedptr<QuadForShaderPass> m_quadForShaderPass;
#if SG_ENABLE_ASSERT
    bool m_isInExecute;
#endif
};
//=============================================================================
class CompositingDescriptor : public reflection::BaseClass
{
    REFLECTION_CLASS_HEADER(CompositingDescriptor, reflection::BaseClass)
    friend class Compositing;
public:
    CompositingDescriptor();
    virtual ~CompositingDescriptor() override;
    ICompositing* CreateInstance(rendering::RenderDevice const* iRenderDevice,
                                 ArrayView<rendering::IShaderResource*> const& iInputSurfaces,
                                 ArrayView<rendering::IRenderTarget*> const& iOutputSurfaces,
                                 ArrayView<rendering::IShaderConstantDatabase*> const& iConstantDatabases,
                                 ArrayView<rendering::IShaderResourceDatabase*> const& iShaderResourceDatabases) const;
private:
#if ENABLE_REFLECTION_PROPERTY_CHECK
    virtual void VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const override;
#endif
private:
    std::unordered_map<FastSymbol, std::vector<refptr<AbstractCompositingInstructionDescriptor const> > > m_instructions;
    std::vector<std::pair<std::string, refptr<InputSurfaceDescriptor const> > > m_inputSurfaces;
    std::vector<std::pair<std::string, refptr<OutputSurfaceDescriptor const> > > m_outputSurfaces;
    std::vector<std::pair<std::string, refptr<InputConstantDatabaseDescriptor const> > > m_inputConstantDatabases;
    std::vector<std::pair<std::string, refptr<InputShaderResourceDatabaseDescriptor const> > > m_inputShaderResourcesDatabases;
};
//=============================================================================
}
}

#endif

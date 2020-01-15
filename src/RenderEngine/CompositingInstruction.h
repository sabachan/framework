#ifndef RenderEngine_CompositingInstruction_H
#define RenderEngine_CompositingInstruction_H

#include <Core/ComPtr.h>
#include <Core/SmartPtr.h>
#include <Math/Vector.h>
#include <Reflection/BaseClass.h>
#include <Rendering/RenderStateDico.h>
#include <Rendering/Shader.h>
#include <Rendering/ShaderResourceDatabase.h>
#include <Rendering/ShaderResourceBuffer.h>
#include <memory>

struct ID3D11BlendState;
struct ID3D11Buffer;

namespace sg {
namespace rendering {
    class CPUSurfaceReader;
    class DepthStencilSurface;
    class IRenderTarget;
    class IShaderConstantDatabase;
    class IShaderResource;
    class IShaderResourceDatabase;
    class RenderDevice;
    class ShaderConstantBuffers;
    class ShaderConstantDatabase;
    class Surface;
}
}

namespace sg {
namespace renderengine {
//=============================================================================
class AbstractSurfaceDescriptor;
class Compositing;
class DepthStencilSurfaceDescriptor;
class PixelShaderDescriptor;
class RenderBatch;
class ShaderResourceBuffer;
class SurfaceDescriptor;
class VertexShaderDescriptor;
//=============================================================================
class QuadForShaderPass
{
    SG_NON_COPYABLE(QuadForShaderPass)
public:
    QuadForShaderPass(rendering::RenderDevice const* iRenderDevice);
    ~QuadForShaderPass();
    void Execute(Compositing* iCompositing);
    rendering::VertexShaderProxy DefaultVertexShader() const { return m_defaultVertexShader; }
private:
    rendering::ShaderInputLayoutProxy m_inputLayout;
    rendering::VertexShaderProxy m_defaultVertexShader;
    comptr<ID3D11Buffer> m_vertexBuffer;
    scopedptr<rendering::ShaderConstantBuffers> m_vsConstantBuffers;
};
//=============================================================================
class ICompositingInstruction : public RefAndSafeCountable
{
    SG_NON_COPYABLE(ICompositingInstruction)
    friend class Compositing;
public:
    ICompositingInstruction() {}
    virtual ~ICompositingInstruction() {}
private:
    virtual void Execute(Compositing* iCompositing) { SG_UNUSED(iCompositing); } //= 0;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class AbstractCompositingInstructionDescriptor : public reflection::BaseClass
{
    SG_NON_COPYABLE(AbstractCompositingInstructionDescriptor)
    REFLECTION_CLASS_HEADER(AbstractCompositingInstructionDescriptor, reflection::BaseClass)
public:
    AbstractCompositingInstructionDescriptor();
    virtual ~AbstractCompositingInstructionDescriptor() override;

    virtual ICompositingInstruction* CreateInstance(Compositing* iCompositing) const { SG_UNUSED(iCompositing); return nullptr; } //= 0;
};
//=============================================================================
class SetBlendStateDescriptor;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class SetBlendState : public ICompositingInstruction
{
    SG_NON_COPYABLE(SetBlendState)
    friend class SetBlendStateDescriptor;
private:
    virtual ~SetBlendState() override;
    virtual void Execute(Compositing* iCompositing) override;
    SetBlendState(SetBlendStateDescriptor const* iDescriptor, Compositing* iCompositing);
private:
    safeptr<SetBlendStateDescriptor const> m_descriptor;
    comptr<ID3D11BlendState> m_blendState;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class SetBlendStateDescriptor : public AbstractCompositingInstructionDescriptor
{
    SG_NON_COPYABLE(SetBlendStateDescriptor)
    REFLECTION_CLASS_HEADER(SetBlendStateDescriptor, AbstractCompositingInstructionDescriptor)
    friend class SetBlendState;
public:
    SetBlendStateDescriptor();
    virtual ~SetBlendStateDescriptor() override;
    virtual SetBlendState* CreateInstance(Compositing* iCompositing) const override;
private:
    rendering::RenderStateName m_preregisteredName;
};
//=============================================================================
class ClearSurfacesDescriptor;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class ClearSurfaces : public ICompositingInstruction
{
    SG_NON_COPYABLE(ClearSurfaces)
    friend class ClearSurfacesDescriptor;
private:
    virtual ~ClearSurfaces() override;
    virtual void Execute(Compositing* iCompositing) override;
    ClearSurfaces(ClearSurfacesDescriptor const* iDescriptor, Compositing* iCompositing);
private:
    safeptr<ClearSurfacesDescriptor const> m_descriptor;
    std::vector<refptr<rendering::IRenderTarget const> > m_renderTargets;
    std::vector<refptr<rendering::DepthStencilSurface const> > m_depthStencils;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class ClearSurfacesDescriptor : public AbstractCompositingInstructionDescriptor
{
    SG_NON_COPYABLE(ClearSurfacesDescriptor)
    REFLECTION_CLASS_HEADER(ClearSurfacesDescriptor, AbstractCompositingInstructionDescriptor)
    friend class ClearSurfaces;
public:
    ClearSurfacesDescriptor();
    virtual ~ClearSurfacesDescriptor() override;
    virtual ClearSurfaces* CreateInstance(Compositing* iCompositing) const override;
private:
    std::vector<refptr<AbstractSurfaceDescriptor> > m_renderTargets;
    float4 m_color;
    // TO SEPARATE ?
    std::vector<refptr<DepthStencilSurfaceDescriptor> > m_depthStencils;
    float m_depth;
    u8 m_stencil;
};
//=============================================================================
class SwapSurfacesDescriptor;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class SwapSurfaces : public ICompositingInstruction
{
    SG_NON_COPYABLE(SwapSurfaces)
    friend class SwapSurfacesDescriptor;
private:
    SwapSurfaces(SwapSurfacesDescriptor const* iDescriptor, Compositing* iCompositing);
    virtual ~SwapSurfaces() override;
    virtual void Execute(Compositing* iCompositing) override;
private:
    safeptr<SwapSurfacesDescriptor const> m_descriptor;
    ArrayList<std::pair<refptr<rendering::Surface>, refptr<rendering::Surface>>> m_surfaces;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class SwapSurfacesDescriptor : public AbstractCompositingInstructionDescriptor
{
    SG_NON_COPYABLE(SwapSurfacesDescriptor)
    REFLECTION_CLASS_HEADER(SwapSurfacesDescriptor, AbstractCompositingInstructionDescriptor)
    friend class SwapSurfaces;
public:
    SwapSurfacesDescriptor();
    virtual ~SwapSurfacesDescriptor() override;
    virtual SwapSurfaces* CreateInstance(Compositing* iCompositing) const override;
private:
#if ENABLE_REFLECTION_PROPERTY_CHECK
    virtual void VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const override;
#endif
private:
    ArrayList<std::pair<refptr<SurfaceDescriptor const>, refptr<SurfaceDescriptor const>>> m_surfaces;
};
//=============================================================================
class GenerateMipmapDescriptor;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class GenerateMipmap : public ICompositingInstruction
{
    SG_NON_COPYABLE(GenerateMipmap)
    friend class GenerateMipmapDescriptor;
private:
    GenerateMipmap(GenerateMipmapDescriptor const* iDescriptor, Compositing* iCompositing);
    virtual ~GenerateMipmap() override;
    virtual void Execute(Compositing* iCompositing) override;
private:
    safeptr<GenerateMipmapDescriptor const> m_descriptor;
    refptr<rendering::Surface const> m_surface;

    comptr<ID3D11BlendState> m_blendState;
    comptr<ID3D11SamplerState> m_samplerState;

    rendering::PixelShaderProxy m_pixelShader;
    rendering::VertexShaderProxy m_vertexShader;
    scopedptr<rendering::ShaderConstantBuffers> m_psConstantBuffers;
    scopedptr<rendering::ShaderConstantBuffers> m_vsConstantBuffers;
    scopedptr<rendering::ShaderResourceBuffer> m_psResources;
    scopedptr<rendering::ShaderResourceBuffer> m_vsResources;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class GenerateMipmapDescriptor : public AbstractCompositingInstructionDescriptor
{
    SG_NON_COPYABLE(GenerateMipmapDescriptor)
    friend class GenerateMipmap;
public:
    GenerateMipmapDescriptor();
    virtual ~GenerateMipmapDescriptor() override;
    virtual GenerateMipmap* CreateInstance(Compositing* iCompositing) const override;
protected:
#if ENABLE_REFLECTION_PROPERTY_CHECK
    virtual void VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const override;
#endif
private:
    refptr<SurfaceDescriptor> m_surface;
    rendering::ShaderResourceName m_bindingPoint;
    refptr<PixelShaderDescriptor const> m_pixelShader;
    refptr<VertexShaderDescriptor const> m_vertexShader;
    REFLECTION_CLASS_HEADER(GenerateMipmapDescriptor, AbstractCompositingInstructionDescriptor)
};
//=============================================================================
class GenerateBCTextureDescriptor : public AbstractCompositingInstructionDescriptor
{
    SG_NON_COPYABLE(GenerateBCTextureDescriptor)
    REFLECTION_CLASS_HEADER(GenerateBCTextureDescriptor, AbstractCompositingInstructionDescriptor)
public:
    GenerateBCTextureDescriptor();
    virtual ~GenerateBCTextureDescriptor() override;
};
//=============================================================================
class SetRenderTargetsDescriptor;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class SetRenderTargets : public ICompositingInstruction
{
    SG_NON_COPYABLE(SetRenderTargets)
    friend class SetRenderTargetsDescriptor;
private:
    SetRenderTargets(SetRenderTargetsDescriptor const* iDescriptor, Compositing* iCompositing);
    virtual ~SetRenderTargets() override;
    virtual void Execute(Compositing* iCompositing) override;
private:
    safeptr<SetRenderTargetsDescriptor const> m_descriptor;
    std::vector<refptr<rendering::IRenderTarget const> > m_renderTargets;
    refptr<rendering::DepthStencilSurface const> m_depthStencil;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class SetRenderTargetsDescriptor : public AbstractCompositingInstructionDescriptor
{
    SG_NON_COPYABLE(SetRenderTargetsDescriptor)
    REFLECTION_CLASS_HEADER(SetRenderTargetsDescriptor, AbstractCompositingInstructionDescriptor)
    friend class SetRenderTargets;
public:
    SetRenderTargetsDescriptor();
    virtual ~SetRenderTargetsDescriptor() override;
    virtual SetRenderTargets* CreateInstance(Compositing* iCompositing) const override;
private:
    std::vector<refptr<AbstractSurfaceDescriptor> > m_renderTargets;
    refptr<DepthStencilSurfaceDescriptor> m_depthStencil;
};
//=============================================================================
class ShaderPassDescriptor;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class ShaderPass : public ICompositingInstruction
{
    SG_NON_COPYABLE(ShaderPass)
    friend class ShaderPassDescriptor;
private:
    ShaderPass(ShaderPassDescriptor const* iDescriptor, Compositing* iCompositing);
    virtual ~ShaderPass() override;
    virtual void Execute(Compositing* iCompositing) override;
private:
    safeptr<ShaderPassDescriptor const> m_descriptor;
    rendering::PixelShaderProxy m_pixelShader;
    rendering::VertexShaderProxy m_vertexShader;
    scopedptr<rendering::ShaderConstantBuffers> m_psConstantBuffers;
    scopedptr<rendering::ShaderConstantBuffers> m_vsConstantBuffers;
    scopedptr<rendering::ShaderResourceBuffer> m_psResources;
    scopedptr<rendering::ShaderResourceBuffer> m_vsResources;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class ShaderPassDescriptor : public AbstractCompositingInstructionDescriptor
{
    SG_NON_COPYABLE(ShaderPassDescriptor)
    REFLECTION_CLASS_HEADER(ShaderPassDescriptor, AbstractCompositingInstructionDescriptor)
    friend class ShaderPass;
public:
    ShaderPassDescriptor();
    virtual ~ShaderPassDescriptor() override;
    virtual ShaderPass* CreateInstance(Compositing* iCompositing) const override;
protected:
#if ENABLE_REFLECTION_PROPERTY_CHECK
    virtual void VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const override;
#endif
private:
    refptr<PixelShaderDescriptor const> m_pixelShader;
    refptr<VertexShaderDescriptor const> m_vertexShader;
};
//=============================================================================
class AbstractConstantDatabaseDescriptor;
class SetConstantDatabaseDescriptor;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class SetConstantDatabase : public ICompositingInstruction
{
    SG_NON_COPYABLE(SetConstantDatabase)
    friend class SetConstantDatabaseDescriptor;
private:
    SetConstantDatabase(SetConstantDatabaseDescriptor const* iDescriptor, Compositing* iCompositing);
    virtual ~SetConstantDatabase() override;
    virtual void Execute(Compositing* iCompositing) override;
private:
    safeptr<SetConstantDatabaseDescriptor const> m_descriptor;
    safeptr<rendering::IShaderConstantDatabase const> m_database;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class SetConstantDatabaseDescriptor : public AbstractCompositingInstructionDescriptor
{
    SG_NON_COPYABLE(SetConstantDatabaseDescriptor)
    REFLECTION_CLASS_HEADER(SetConstantDatabaseDescriptor, AbstractCompositingInstructionDescriptor)
    friend class SetConstantDatabase;
public:
    SetConstantDatabaseDescriptor();
    virtual ~SetConstantDatabaseDescriptor() override;
    virtual SetConstantDatabase* CreateInstance(Compositing* iCompositing) const override;
private:
#if ENABLE_REFLECTION_PROPERTY_CHECK
    virtual void VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const override;
#endif
private:
    refptr<AbstractConstantDatabaseDescriptor> m_database;
};
//=============================================================================
class SetShaderResourceDatabaseDescriptor;
class ShaderResourceDatabaseDescriptor;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class SetShaderResourceDatabase : public ICompositingInstruction
{
    SG_NON_COPYABLE(SetShaderResourceDatabase)
    friend class SetShaderResourceDatabaseDescriptor;
private:
    SetShaderResourceDatabase(SetShaderResourceDatabaseDescriptor const* iDescriptor, Compositing* iCompositing);
    virtual ~SetShaderResourceDatabase() override;
    virtual void Execute(Compositing* iCompositing) override;
private:
    safeptr<SetShaderResourceDatabaseDescriptor const> m_descriptor;
    safeptr<rendering::IShaderResourceDatabase const> m_database;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class SetShaderResourceDatabaseDescriptor : public AbstractCompositingInstructionDescriptor
{
    SG_NON_COPYABLE(SetShaderResourceDatabaseDescriptor)
    REFLECTION_CLASS_HEADER(SetShaderResourceDatabaseDescriptor, AbstractCompositingInstructionDescriptor)
    friend class SetShaderResourceDatabase;
public:
    SetShaderResourceDatabaseDescriptor();
    virtual ~SetShaderResourceDatabaseDescriptor() override;
    virtual SetShaderResourceDatabase* CreateInstance(Compositing* iCompositing) const override;
private:
#if ENABLE_REFLECTION_PROPERTY_CHECK
    virtual void VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const override;
#endif
private:
    refptr<ShaderResourceDatabaseDescriptor> m_database;
};
//=============================================================================
class ConstantDatabaseDescriptor;
class SetConstantDatabaseDescriptor;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class SetAliasesInConstantDatabase : public ICompositingInstruction
{
    SG_NON_COPYABLE(SetAliasesInConstantDatabase)
    friend class SetAliasesInConstantDatabaseDescriptor;
private:
    SetAliasesInConstantDatabase(SetAliasesInConstantDatabaseDescriptor const* iDescriptor, Compositing* iCompositing);
    virtual ~SetAliasesInConstantDatabase() override;
    virtual void Execute(Compositing* iCompositing) override;
private:
    safeptr<SetAliasesInConstantDatabaseDescriptor const> m_descriptor;
    safeptr<rendering::IShaderConstantDatabase const> m_database;
    safeptr<rendering::ShaderConstantDatabase> m_writableDatabase;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class SetAliasesInConstantDatabaseDescriptor : public AbstractCompositingInstructionDescriptor
{
    SG_NON_COPYABLE(SetAliasesInConstantDatabaseDescriptor)
    REFLECTION_CLASS_HEADER(SetAliasesInConstantDatabaseDescriptor, AbstractCompositingInstructionDescriptor)
    friend class SetAliasesInConstantDatabase;
public:
    SetAliasesInConstantDatabaseDescriptor();
    virtual ~SetAliasesInConstantDatabaseDescriptor() override;
    virtual SetAliasesInConstantDatabase* CreateInstance(Compositing* iCompositing) const override;
private:
#if ENABLE_REFLECTION_PROPERTY_CHECK
    virtual void VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const override;
#endif
private:
    ArrayList<std::pair<std::string, std::string>> m_aliases;
    refptr<ConstantDatabaseDescriptor> m_database;
};
//=============================================================================
class DebugDumpSurfacesDescriptor;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class DebugDumpSurfaces : public ICompositingInstruction
{
    SG_NON_COPYABLE(DebugDumpSurfaces)
    friend class DebugDumpSurfacesDescriptor;
private:
    DebugDumpSurfaces(DebugDumpSurfacesDescriptor const* iDescriptor, Compositing* iCompositing);
    virtual ~DebugDumpSurfaces() override;
    virtual void Execute(Compositing* iCompositing) override;
private:
    safeptr<DebugDumpSurfacesDescriptor const> m_descriptor;
    ArrayList<refptr<rendering::CPUSurfaceReader>> m_surfaceReaders;
    u32 m_iter = 0;
    u32 m_count = 0;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class DebugDumpSurfacesDescriptor : public AbstractCompositingInstructionDescriptor
{
    SG_NON_COPYABLE(DebugDumpSurfacesDescriptor)
    REFLECTION_CLASS_HEADER(DebugDumpSurfacesDescriptor, AbstractCompositingInstructionDescriptor)
    friend class DebugDumpSurfaces;
public:
    DebugDumpSurfacesDescriptor();
    virtual ~DebugDumpSurfacesDescriptor() override;
    virtual DebugDumpSurfaces* CreateInstance(Compositing* iCompositing) const override;
protected:
#if ENABLE_REFLECTION_PROPERTY_CHECK
    virtual void VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const override;
#endif
private:
    FilePath m_directory;
    ArrayList<std::pair<std::string, refptr<SurfaceDescriptor const>>> m_surfaces;
    u32 m_start = 0;
    u32 m_skip  = 0;
    u32 m_stop  = 0;
};
//=============================================================================
// TODO: SetAliasesInResourceDatabase
//=============================================================================
//class ViewportDescriptor : public AbstractCompositingInstructionDescriptor
//{
//    REFLECTION_CLASS_HEADER(ViewportDescriptor, AbstractCompositingInstructionDescriptor)
//public:
//    ViewportDescriptor();
//    virtual ~ViewportDescriptor() override;
//};
//=============================================================================
}
}

#endif

#ifndef RenderEngine_CompositingInstruction_H
#define RenderEngine_CompositingInstruction_H

#include <Core/ComPtr.h>
#include <Core/SmartPtr.h>
#include <Math/Vector.h>
#include <Reflection/BaseClass.h>
#include <Rendering/Shader.h>
#include <Rendering/ShaderResourceDatabase.h>
#include <Rendering/ShaderResourceBuffer.h>
#include <memory>

struct ID3D11BlendState;
struct ID3D11Buffer;

namespace sg {
namespace rendering {
    class DepthStencilSurface;
    class IRenderTarget;
    class IShaderConstantDatabase;
    class IShaderResource;
    class IShaderResourceDatabase;
    class RenderDevice;
    class ShaderConstantBuffers;
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

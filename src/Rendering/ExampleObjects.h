#ifndef Rendering_ExampleObjects_H
#define Rendering_ExampleObjects_H

#include "MipmapGenerator.h"
#include "Shader.h"
#include "ShaderConstantDatabase.h"
#include "Surface.h"
#include <Core/ComPtr.h>
#include <Core/Observer.h>
#include <Math/Matrix.h>
#include <vector>

struct ID3D11Buffer;
struct ID3D11SamplerState;

namespace sg {
namespace rendering {
//=============================================================================
class DepthStencilSurface;
class IRenderTarget;
class IShaderResource;
class RenderDevice;
class ResolutionServer;
class TextureFromFile;
class ShaderConstantBuffers;
class Surface;
//=============================================================================
namespace exampleobjects {
//=============================================================================
class IExampleObject : public RefAndSafeCountable
{
public:
    virtual ~IExampleObject() {}
    virtual void Draw(RenderDevice const* iRenderDevice) = 0;
};
//=============================================================================
class Triangle : public IExampleObject
{
public:
    Triangle(RenderDevice const* iRenderDevice);
    virtual ~Triangle() override;
    virtual void Draw(RenderDevice const* iRenderDevice) override;
private:
    ShaderInputLayoutProxy m_inputLayout;
    VertexShaderProxy m_vertexShader;
    PixelShaderProxy m_pixelShader;
    comptr<ID3D11Buffer> m_vertexBuffer;
};
//=============================================================================
class ClearRenderTarget : public IExampleObject
{
public:
    ClearRenderTarget(IRenderTarget* iRenderTarget, u32 iColor) : m_renderTarget(iRenderTarget), m_color(iColor) {}
    virtual void Draw(RenderDevice const* iRenderDevice) override;
private:
    safeptr<IRenderTarget> m_renderTarget;
    u32 m_color;
};
//=============================================================================
class ClearDepthStencil : public IExampleObject
{
public:
    ClearDepthStencil(DepthStencilSurface* iDepthStencilSurface) : m_depthStencilSurface(iDepthStencilSurface) {}
    virtual void Draw(RenderDevice const* iRenderDevice) override;
private:
    safeptr<DepthStencilSurface> m_depthStencilSurface;
};
//=============================================================================
class SetRenderTargets : public IExampleObject
{
public:
    SetRenderTargets(IRenderTarget* iRenderTaret, DepthStencilSurface* iDepthStencilSurface) : m_renderTargets(), m_depthStencilSurface(iDepthStencilSurface) { m_renderTargets.push_back(iRenderTaret); }
    virtual void Draw(RenderDevice const* iRenderDevice) override;
private:
    std::vector<safeptr<IRenderTarget> > m_renderTargets;
    safeptr<DepthStencilSurface> m_depthStencilSurface;
};
//=============================================================================
class GenerateMipmap : public IExampleObject
{
public:
    GenerateMipmap(RenderDevice const* iRenderDevice, Surface* iSurface);
    virtual void Draw(RenderDevice const* iRenderDevice) override;
private:
    safeptr<Surface> m_surface;
    SimpleMipmapGenerator m_mipmapGenerator;
    PixelShaderProxy m_pixelShader;
};
//=============================================================================
class TexturedQuad : public IExampleObject
{
public:
    TexturedQuad(RenderDevice const* iRenderDevice, ShaderConstantDatabase const* iDatabase);
    TexturedQuad(RenderDevice const* iRenderDevice, ShaderConstantDatabase const* iDatabase, FilePath const& iPixelShader);
    ~TexturedQuad() override;
    void SetShaderResource(IShaderResource* iShaderResource) { SG_ASSERT(nullptr != iShaderResource); m_shaderResource = iShaderResource; }
    void Prepare(RenderDevice const* iRenderDevice, FilePath const& iPixelShader);
    virtual void Draw(RenderDevice const* iRenderDevice) override;
private:
    ShaderInputLayoutProxy m_inputLayout;
    VertexShaderProxy m_vertexShader;
    PixelShaderProxy m_pixelShader;
    comptr<ID3D11Buffer> m_vertexBuffer;
    refptr<IShaderResource> m_shaderResource;
    comptr<ID3D11SamplerState> m_samplerState;
    safeptr<ShaderConstantDatabase const> m_database;
    scopedptr<ShaderConstantBuffers> m_psConstantBuffers;
    scopedptr<ShaderConstantBuffers> m_vsConstantBuffers;
};
//=============================================================================
class Camera : private Observer<ResolutionServer>
{
    PARENT_SAFE_COUNTABLE(Observer<ResolutionServer>)
public:
    Camera(RenderDevice const* iRenderDevice, ResolutionServer const* iResolutionServer, ShaderConstantDatabase* iDatabase);
    virtual ~Camera() override;

    float4x4 const& GetCameraMatrix() { return m_camera; }
    float4x4_colmajor const& GetColumnMajorCameraMatrix() { return m_columnMajorCamera; }
    void DummyEvolve(float2 const& value);
private:
    void ComputeProjMatrix();
    void UpdateFullMatrix();
    virtual void VirtualOnNotified(ResolutionServer const* iResolutionServer) override;
private:
    safeptr<RenderDevice const> m_renderDevice;
    safeptr<ResolutionServer const> m_resolution;
    float4x4 m_view;
    float4x4 m_proj;
    float4x4 m_camera;
    float4x4_colmajor m_columnMajorCamera;
    safeptr<ShaderConstantDatabase> m_database;
    ShaderConstantName m_constantName;
};
//=============================================================================
class Cube : public IExampleObject
{
public:
    Cube(RenderDevice const* iRenderDevice, Camera* iCamera, ShaderConstantDatabase const* iDatabase, float3 const& iCenter);
    virtual ~Cube() override;
    virtual void Draw(RenderDevice const* iRenderDevice) override;
private:
    ShaderInputLayoutProxy m_inputLayout;
    VertexShaderProxy m_vertexShader;
    PixelShaderProxy m_pixelShader;
    comptr<ID3D11Buffer> m_vertexBuffer;
    comptr<ID3D11Buffer> m_indexBuffer;
    //std::vector<comptr<ID3D11Buffer> > m_constantBuffers;
    safeptr<Camera> m_camera;
    safeptr<ShaderConstantDatabase const> m_database;
    scopedptr<ShaderConstantBuffers> m_psConstantBuffers;
    scopedptr<ShaderConstantBuffers> m_vsConstantBuffers;
};
//=============================================================================
}
}
}

#endif

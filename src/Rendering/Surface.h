#ifndef Rendering_Surface_H
#define Rendering_Surface_H

#include "IRenderTarget.h"
#include "IShaderResource.h"
#include "ISurface.h"
#include "ResolutionServer.h"
#include "SurfaceFormat.h"
#include <Core/ComPtr.h>
#include <Core/Observer.h>
#include <Image/ImageView.h>
#include <dxgiformat.h>

struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;

namespace sg {
namespace rendering {
//=============================================================================
class RenderDevice;
//=============================================================================
struct SurfaceProperties
{
    size_t mipLevels;
    SurfaceFormat baseFormat;
    SurfaceFormat readFormat;
    SurfaceFormat writeFormat;

    SurfaceProperties()
        : mipLevels(0)
        , baseFormat(SurfaceFormat::UNKNOWN)
        , readFormat(SurfaceFormat::UNKNOWN)
        , writeFormat(SurfaceFormat::UNKNOWN)
    {}
    SurfaceProperties(
        size_t iMipLevels,
        SurfaceFormat iBaseFormat,
        SurfaceFormat iReadFormat,
        SurfaceFormat iWriteFormat)
        : mipLevels(iMipLevels)
        , baseFormat(iBaseFormat)
        , readFormat(iReadFormat)
        , writeFormat(iWriteFormat)
    {}
    friend bool operator==(SurfaceProperties const& a, SurfaceProperties const& b)
    {
        return a.mipLevels == b.mipLevels
            && a.baseFormat == b.baseFormat
            && a.readFormat == b.readFormat
            && a.writeFormat == b.writeFormat;
    }
};
//=============================================================================
class BaseSurface : public RefAndSafeCountable
                  , public ISurface
                  , public Observable<BaseSurface>
                  , private Observer<ResolutionServer>
{
    PARENT_REF_COUNTABLE(RefAndSafeCountable)
    PARENT_SAFE_COUNTABLE(RefAndSafeCountable)
    SG_NON_COPYABLE(BaseSurface)
    friend class SurfaceView;
    friend class CPUSurfaceReader;
public:
    virtual ~BaseSurface() override;
    void Terminate();
#if SG_ENABLE_ASSERT
    bool IsValid_ForAssert() const { return nullptr != m_resolutionServer; }
#endif
    virtual ID3D11Texture2D* GetSurfaceTexture() const override { return Texture2D(); }
    virtual ResolutionServer const* GetSurfaceResolution() const override { SG_ASSERT(IsValid_ForAssert()); return m_resolutionServer.get(); }
    virtual SurfaceFormat GetSurfaceFormat() const override { return m_properties.baseFormat; }
protected:
    BaseSurface(RenderDevice const* iRenderDevice, ResolutionServer const* iResolutionServer, SurfaceProperties const& iProperties, bool iIsBC, bool iIsDepthStencil);
    ID3D11Texture2D* Texture2D() const { SG_ASSERT(IsValid_ForAssert()); return m_texture2D.get(); }
    ID3D11ShaderResourceView* ShaderResourceView() const { SG_ASSERT(IsValid_ForAssert()); return m_shaderResourceView.get(); }
    ID3D11RenderTargetView* RenderTargetView() const { SG_ASSERT(IsValid_ForAssert()); return GetMipLevelRenderTargetView(0); }
    ID3D11DepthStencilView* GetDepthStencilView() const { SG_ASSERT(IsValid_ForAssert()); return m_depthStencilView.get(); }
    ResolutionServer const* Resolution() const { SG_ASSERT(IsValid_ForAssert()); return m_resolutionServer.get(); }
    ResolutionServer const* BlocResolution() const { SG_ASSERT(IsValid_ForAssert()); return &m_blocResolutionServer; }
    size_t MipCount() const { SG_ASSERT(IsValid_ForAssert()); return m_mipShaderResourceViews.size(); }
    ID3D11ShaderResourceView* GetMipLevelShaderResourceView(size_t iMipLevel) const;
    ID3D11RenderTargetView* GetMipLevelRenderTargetView(size_t iMipLevel) const;
    SurfaceProperties const& Properties() const { return m_properties; }

    friend void swap(BaseSurface& a, BaseSurface& b);
private:
    virtual void VirtualOnNotified(ResolutionServer const* iResolutionServer) override;
    void ReleaseObjectsIFN();
    void CreateObjectsIFN();
private:
    safeptr<RenderDevice const> const m_renderDevice;
    safeptr<ResolutionServer const> m_resolutionServer;
    ResolutionServer m_blocResolutionServer;
    SurfaceProperties const m_properties;
    comptr<ID3D11Texture2D> m_texture2D;
    comptr<ID3D11ShaderResourceView> m_shaderResourceView;
    comptr<ID3D11DepthStencilView> m_depthStencilView;
    std::vector<comptr<ID3D11ShaderResourceView> > m_mipShaderResourceViews;
    std::vector<comptr<ID3D11RenderTargetView> > m_mipRenderTargetViews;
    bool m_isBC;
    bool m_isDepthStencil;
    SG_CODE_FOR_ASSERT(mutable int m_generatedMip;)
};
void swap(BaseSurface& a, BaseSurface& b);
//=============================================================================
class Surface : public BaseSurface
              , public IRenderTarget
              , public IShaderResource
{
    PARENT_REF_COUNTABLE(BaseSurface)
    PARENT_SAFE_COUNTABLE(BaseSurface)
public:
    Surface(RenderDevice const* iRenderDevice, ResolutionServer const* iResolutionServer, SurfaceProperties const* iOptionnalProperties = nullptr);
    virtual ID3D11ShaderResourceView* GetShaderResourceView() const override { return ShaderResourceView(); }
    virtual ResolutionServer const* ShaderResourceResolution() const override { return Resolution(); }
    virtual ID3D11RenderTargetView* GetRenderTargetView() const override { return RenderTargetView(); }
    virtual ResolutionServer const* RenderTargetResolution() const override { return Resolution(); }
    using BaseSurface::Resolution;
    using BaseSurface::MipCount;
    using BaseSurface::GetMipLevelShaderResourceView;
    using BaseSurface::GetMipLevelRenderTargetView;
    friend void swap(Surface& a, Surface& b) { swap(static_cast<BaseSurface&>(a), static_cast<BaseSurface&>(b)); }
};
//=============================================================================
class BCSurface : public BaseSurface
                , public IRenderTarget
                , public IShaderResource
{
    PARENT_REF_COUNTABLE(BaseSurface)
    PARENT_SAFE_COUNTABLE(BaseSurface)
public:
    BCSurface(RenderDevice const* iRenderDevice, ResolutionServer const* iResolutionServer, SurfaceProperties const* iOptionnalProperties = nullptr);
    virtual ID3D11ShaderResourceView* GetShaderResourceView() const override { return ShaderResourceView(); }
    virtual ResolutionServer const* ShaderResourceResolution() const override { return Resolution(); }
    virtual ID3D11RenderTargetView* GetRenderTargetView() const override { return RenderTargetView(); }
    virtual ResolutionServer const* RenderTargetResolution() const override { return BlocResolution(); }
    using BaseSurface::MipCount;
    using BaseSurface::GetMipLevelShaderResourceView;
    using BaseSurface::GetMipLevelRenderTargetView;
};
//=============================================================================
class DepthStencilSurface : public BaseSurface
                          , public IShaderResource
{
    PARENT_REF_COUNTABLE(BaseSurface)
    PARENT_SAFE_COUNTABLE(BaseSurface)
public:
    DepthStencilSurface(RenderDevice const* iRenderDevice, ResolutionServer const* iResolutionServer, SurfaceProperties const* iOptionnalProperties = nullptr);
    virtual ID3D11ShaderResourceView* GetShaderResourceView() const override { return ShaderResourceView(); }
    virtual ResolutionServer const* ShaderResourceResolution() const override { return Resolution(); }
    using BaseSurface::GetDepthStencilView;
    using BaseSurface::Resolution;
};
//=============================================================================
// TODO: test this class
class SurfaceView : public RefAndSafeCountable
                  , private Observer<BaseSurface>
{
    PARENT_SAFE_COUNTABLE(RefAndSafeCountable)
    SG_NON_COPYABLE(SurfaceView)
public:
    virtual ~SurfaceView() override;
#if SG_ENABLE_ASSERT
    bool IsValid_ForAssert() const { return nullptr != m_baseSurface; }
#endif
protected:
    SurfaceView(RenderDevice const* iRenderDevice, BaseSurface const* iBaseSurface, SurfaceProperties const& iProperties);
    ID3D11ShaderResourceView* ShaderResourceView() const { SG_ASSERT(IsValid_ForAssert()); return m_shaderResourceView.get(); }
    ID3D11RenderTargetView* RenderTargetView() const { SG_ASSERT(IsValid_ForAssert()); return m_renderTargetView.get(); }
    ResolutionServer const* Resolution() const { SG_ASSERT(IsValid_ForAssert()); return m_baseSurface->Resolution(); }
private:
    void Terminate();
    virtual void VirtualOnNotified(BaseSurface const* iBaseSurface) override;
    void ReleaseObjectsIFN();
    void CreateObjectsIFN();
private:
    safeptr<RenderDevice const> const m_renderDevice;
    safeptr<BaseSurface const> m_baseSurface;
    SurfaceProperties const m_properties;
    comptr<ID3D11ShaderResourceView> m_shaderResourceView;
    comptr<ID3D11RenderTargetView> m_renderTargetView;
};
//=============================================================================
// A CPUSurface is writable by the CPU and readable by the GPU
class CPUSurface : public RefAndSafeCountable
                 , public IShaderResource
                 , private Observer<ResolutionServer>
{
    PARENT_REF_COUNTABLE(RefAndSafeCountable)
    PARENT_SAFE_COUNTABLE(RefAndSafeCountable)
    SG_NON_COPYABLE(CPUSurface)
public:
    CPUSurface(RenderDevice const* iRenderDevice, ResolutionServer const* iResolutionServer, SurfaceProperties const* iOptionnalProperties = nullptr);
    virtual ~CPUSurface() override;
    virtual ID3D11ShaderResourceView* GetShaderResourceView() const override { SG_ASSERT(IsValid_ForAssert()); return m_shaderResourceView.get(); }
    virtual ResolutionServer const* ShaderResourceResolution() const override { SG_ASSERT(IsValid_ForAssert()); return m_resolutionServer.get(); }
    void Terminate();
#if SG_ENABLE_ASSERT
    bool IsValid_ForAssert() const { return nullptr != m_resolutionServer; }
#endif
    bool IsMapped() const;
    void MapAndDiscard();
    void Unmap();
    template <typename T> void GetImage(image::ImageView<T>& oImage)
    {
        SG_ASSERT(sizeof(T) == GetProperties(m_properties.writeFormat).byteCountPerPixel);
        SG_ASSERT(IsMapped());
        oImage = image::ImageView<T>(static_cast<T*>(
            m_mappedData),
            m_resolutionServer->Get(),
            m_mappedStrideInBytes
            SG_CODE_FOR_ASSERT(SG_COMMA m_checkerForImageView.GetSafeCountable()));
    }
private:
    virtual void VirtualOnNotified(ResolutionServer const* iResolutionServer) override;
    void ReleaseObjectsIFN();
    void CreateObjectsIFN();
private:
    safeptr<RenderDevice const> const m_renderDevice;
    safeptr<ResolutionServer const> m_resolutionServer;
    SurfaceProperties const m_properties;
    comptr<ID3D11Texture2D> m_texture2D;
    comptr<ID3D11ShaderResourceView> m_shaderResourceView;
    void* m_mappedData;
    size_t m_mappedStrideInBytes;
#if SG_ENABLE_ASSERT
    image::CheckerForImageView m_checkerForImageView;
#endif
};
//=============================================================================
class CPUSurfaceWithResolution : private ResolutionServer
                               , public CPUSurface
{
    PARENT_REF_COUNTABLE(CPUSurface)
    PARENT_SAFE_COUNTABLE(CPUSurface)
public:
    CPUSurfaceWithResolution(RenderDevice const* iRenderDevice, uint2 const& iResolution, SurfaceProperties const* iOptionnalProperties = nullptr)
        : ResolutionServer(iResolution)
        , CPUSurface(iRenderDevice, this, iOptionnalProperties)
    {
    }
    ResolutionServer* Resolution() { SG_ASSERT(IsValid_ForAssert()); return this; }
    ResolutionServer const* Resolution() const { SG_ASSERT(IsValid_ForAssert()); return this; }
};
//=============================================================================
class CPUSurfaceReader : public RefAndSafeCountable
                       , private Observer<ResolutionServer>
{
    PARENT_SAFE_COUNTABLE(RefAndSafeCountable)
    SG_NON_COPYABLE(CPUSurfaceReader)
public:
    CPUSurfaceReader(RenderDevice const* iRenderDevice, ISurface const* iSurface);
    virtual ~CPUSurfaceReader() override;
    ResolutionServer const* Resolution() const { SG_ASSERT(IsValid_ForAssert()); return m_surface->GetSurfaceResolution(); }
    void Terminate();
#if SG_ENABLE_ASSERT
    bool IsValid_ForAssert() const { return nullptr != m_surface; }
#endif
    ISurface const* GetBaseSurface() const { return m_surface.get(); }
    void Update();
    bool IsMapped() const;
    void Map();
    void Unmap();
    template <typename T> void GetImage(image::ImageView<T const>& oImage) const
    {
        SG_ASSERT(sizeof(T) == GetProperties(m_surface->GetSurfaceFormat()).byteCountPerPixel);
        SG_ASSERT(IsMapped());
        oImage = image::ImageView<T const>(static_cast<T const*>(
            m_mappedData),
            m_surface->GetSurfaceResolution()->Get(),
            m_mappedStrideInBytes
            SG_CODE_FOR_ASSERT(SG_COMMA m_checkerForImageView.GetSafeCountable()));
    }
private:
    virtual void VirtualOnNotified(ResolutionServer const* iResolutionServer) override;
    void ReleaseObjectsIFN();
    void CreateObjectsIFN();
private:
    safeptr<RenderDevice const> const m_renderDevice;
    safeptr<ISurface const> m_surface;
    comptr<ID3D11Texture2D> m_texture2D;
    void const* m_mappedData {nullptr};
    size_t m_mappedStrideInBytes {0};
#if SG_ENABLE_ASSERT
    image::CheckerForImageView m_checkerForImageView;
#endif
};
//=============================================================================
}
}

#endif

#include "stdafx.h"

#include "RenderDevice.h"

#include "RenderState.h"
#include "RenderStateDico.h"
#include "ShaderCache.h"
#include "TextureCache.h"
#include <Math/Vector.h>
#include <algorithm>

#if SG_PLATFORM_IS_WIN
#include <dxgi.h>
#include <d3d11.h>
#include <Core/WindowsH.h>
#endif

namespace sg {
namespace rendering {
//=============================================================================
#if SG_ENABLE_ASSERT
void SetDebugName(ID3D11DeviceChild* child, std::string const& name)
{
  if (nullptr != child && !name.empty())
    child->SetPrivateData(WKPDID_D3DDebugObjectName, checked_numcastable(name.size()), name.c_str());
}
#endif
//=============================================================================
RenderDevice::RenderDevice()
    : m_d3dDevice()
    , m_immediateContext()
#if SG_ENABLE_ASSERT
    , m_isInRender(false)
#endif
{
    D3D_FEATURE_LEVEL featureLevelsRequested[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };
    UINT featureLevelsRequestedCount = SG_ARRAYSIZE(featureLevelsRequested);
    D3D_FEATURE_LEVEL featureLevelsSupported;
#if SG_ENABLE_ASSERT
    UINT flags = D3D11_CREATE_DEVICE_DEBUG;
#else
    UINT flags = 0;
#endif

    HRESULT hr = D3D11CreateDevice(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        flags,
        featureLevelsRequested,
        featureLevelsRequestedCount,
        D3D11_SDK_VERSION,
        m_d3dDevice.GetPointerForInitialisation(),
        &featureLevelsSupported,
        m_immediateContext.GetPointerForInitialisation()
    );
    SG_ASSERT(SUCCEEDED(hr));

    renderstatedico::SetRenderDevice(this);
    shadercache::SetRenderDevice(this);
    texturecache::SetRenderDevice(this);

    // TODO: migrate on renderstatedico
    // Create a rasterizer state
    D3D11_RASTERIZER_DESC rasterizerDesc;
    ZeroMemory( &rasterizerDesc, sizeof( rasterizerDesc ) );
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    //rasterizerDesc.CullMode = D3D11_CULL_NONE; // To uncomment for debug
    rasterizerDesc.FrontCounterClockwise = true;
    rasterizerDesc.DepthBias = false;
    rasterizerDesc.DepthBiasClamp = 0;
    rasterizerDesc.SlopeScaledDepthBias = 0;
    rasterizerDesc.DepthClipEnable = false;
    rasterizerDesc.ScissorEnable = false; //true; // Check perf
    rasterizerDesc.MultisampleEnable = false; // has no effect if target is not multisample ?
    rasterizerDesc.AntialiasedLineEnable = false;
    hr = m_d3dDevice->CreateRasterizerState(&rasterizerDesc, m_rasterizerState.GetPointerForInitialisation());
    SG_ASSERT(SUCCEEDED(hr));

    {
        // Create a blend state
        BlendStateDescriptor blendStateDesc;
        D3D11_BLEND_DESC& desc = blendStateDesc.GetWritableDesc();
        desc.AlphaToCoverageEnable = false;
        desc.IndependentBlendEnable = false;
        desc.RenderTarget[0].BlendEnable = true;
        desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
        desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE; //D3D11_BLEND_SRC_ALPHA;
        desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        m_blendState = renderstatedico::GetBlendState(RenderStateName("Premultiplied Alpha Blending"), &blendStateDesc);
        SG_ASSERT(nullptr != m_blendState);
    }

    // TODO: migrate on renderstatedico
    // Create a depth stencil state
    D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
    depthStencilStateDesc.DepthEnable = true;
    depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilStateDesc.StencilEnable = false;
    depthStencilStateDesc.StencilReadMask = 0;
    depthStencilStateDesc.StencilWriteMask = 0;
    depthStencilStateDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilStateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilStateDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilStateDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NEVER;
    depthStencilStateDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilStateDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilStateDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilStateDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;
    hr = m_d3dDevice->CreateDepthStencilState(&depthStencilStateDesc, m_depthStencilState.GetPointerForInitialisation());
    SG_ASSERT(SUCCEEDED(hr));

#if SG_ENABLE_ASSERT
    SetDebugName(m_depthStencilState.get(), "depthStencilState");
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
RenderDevice::~RenderDevice()
{
    SG_ASSERT(!m_isInRender);
    texturecache::SetRenderDevice(nullptr);
    shadercache::SetRenderDevice(nullptr);
    renderstatedico::SetRenderDevice(nullptr);

    m_depthStencilState = nullptr;
    m_blendState = nullptr;
    m_rasterizerState = nullptr;
    m_immediateContext = nullptr;

#if 0 // #if SG_ENABLE_ASSERT // To activate to find leaks of d3d objects.
    comptr<ID3D11Debug> d3dDebug;
    HRESULT hr = m_d3dDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)d3dDebug.GetPointerForInitialisation());
    d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RenderDevice::BeginRender()
{
    SG_ASSERT(!m_isInRender);
    SG_CODE_FOR_ASSERT(m_isInRender = true);
    m_immediateContext->RSSetState( m_rasterizerState.get() );
    m_immediateContext->OMSetBlendState( m_blendState.get(), NULL, 0xFFFFFFFF );
    m_immediateContext->OMSetDepthStencilState( m_depthStencilState.get(), 0 );
    m_immediateContext->OMSetRenderTargets( 0, NULL, NULL );
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RenderDevice::EndRender()
{
    SG_ASSERT(m_isInRender);
    m_immediateContext->OMSetRenderTargets( 0, 0, 0 );
    m_presentFrame.NotifyObservers();
    SG_CODE_FOR_ASSERT(m_isInRender = false);
}
//=============================================================================
}
}


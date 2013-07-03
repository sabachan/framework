#include "stdafx.h"

#include "ExampleObjects.h"

#include "RenderDevice.h"
#include "RenderTarget.h"
#include "ShaderConstantBuffers.h"
#include "ShaderResource.h"
#include "TextureFromFile.h"
#include "VertexTypes.h"
#include <Core/Cast.h>
#include <cstring>
#include <d3d11.h>
#include <d3d11shader.h>


namespace sg {
namespace rendering {
namespace exampleobjects {
//=============================================================================
namespace {
    typedef Vertex_Pos3f_Tex2f TriangleVertex;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Triangle::Triangle(RenderDevice const* iRenderDevice)
{
    ID3D11Device* device = iRenderDevice->D3DDevice();
    SG_ASSERT(nullptr != device);

    refptr<VertexShaderDescriptor> vsDesc = new VertexShaderDescriptor(FilePath("src:/Rendering/Shaders/ExampleObjects/Triangle.hlsl"), "vmain");
    m_vertexShader = vsDesc->GetProxy();
    refptr<PixelShaderDescriptor> psDesc = new PixelShaderDescriptor(FilePath("src:/Rendering/Shaders/ExampleObjects/Triangle.hlsl"), "pmain");
    m_pixelShader = psDesc->GetProxy();
    m_inputLayout = ShaderInputLayoutProxy(TriangleVertex::InputEltDesc(), m_vertexShader);

    TriangleVertex vertices[] =
    {
        { float3( 0.f,   0.5f, 1.f), float2(0.5f, 0.f) },
        { float3(-0.5f, -0.5f, 1.f), float2(0.f,  1.f) },
        { float3( 0.5f, -0.5f, 1.f), float2(1.f,  1.f) },
    };
    size_t vertexCount = SG_ARRAYSIZE(vertices);

    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage               = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth           = num_cast<UINT>(sizeof( TriangleVertex ) * vertexCount);
    bufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags      = 0;
    bufferDesc.MiscFlags           = 0;
    bufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = vertices;
    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;
    HRESULT hr = device->CreateBuffer( &bufferDesc, &InitData, m_vertexBuffer.GetPointerForInitialisation() );
    SG_ASSERT_AND_UNUSED(SUCCEEDED(hr));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Triangle::~Triangle()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Triangle::Draw(RenderDevice const* iRenderDevice)
{
    ID3D11DeviceContext* context = iRenderDevice->ImmediateContext();
    SG_ASSERT(nullptr != context);

    UINT const stride = sizeof( TriangleVertex );
    UINT const offset = 0;

    ID3D11Buffer* vertexBuffers[] =
    {
        m_vertexBuffer.get()
    };
    UINT const vertexBufferCount = SG_ARRAYSIZE(vertexBuffers);

    context->IASetVertexBuffers( 
        0,                // the first input slot for binding
        vertexBufferCount,// the number of buffers in the array
        vertexBuffers,    // the array of vertex buffers
        &stride,          // array of stride values, one for each buffer
        &offset );        // array of offset values, one for each buffer

    // Set the input layout
    context->IASetInputLayout( m_inputLayout.GetInputLayout() );
    context->VSSetShader( m_vertexShader.GetShader(), NULL, 0 );
    context->PSSetShader( m_pixelShader.GetShader(), NULL, 0 );

    context->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    context->Draw(3, 0);
}
//=============================================================================
void ClearRenderTarget::Draw(RenderDevice const* iRenderDevice)
{
    SG_ASSERT(nullptr != m_renderTarget->GetRenderTargetView());
    float const oo255 = 1.f/255.f;
    float color[] = { ((m_color>>24)&0xFF)*oo255, ((m_color>>16)&0xFF)*oo255, ((m_color>>8)&0xFF)*oo255, (m_color&0xFF)*oo255 };
    iRenderDevice->ImmediateContext()->ClearRenderTargetView(m_renderTarget->GetRenderTargetView(), color);
}
//=============================================================================
void ClearDepthStencil::Draw(RenderDevice const* iRenderDevice)
{
    SG_ASSERT(nullptr != m_depthStencilSurface->GetDepthStencilView());
    iRenderDevice->ImmediateContext()->ClearDepthStencilView(m_depthStencilSurface->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.f, 0);
}
//=============================================================================
void SetRenderTargets::Draw(RenderDevice const* iRenderDevice)
{
    size_t const MAX_RTV_COUNT = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
    ID3D11RenderTargetView* rtvs[MAX_RTV_COUNT];
    size_t const rtvCount = m_renderTargets.size();
    SG_ASSERT(rtvCount < MAX_RTV_COUNT);
    uint2 const resolution = rtvCount > 0 ? m_renderTargets[0]->RenderTargetResolution()->Get() : uint2(0);
    for(size_t i = 0; i < rtvCount; ++i)
    {
        rtvs[i] = m_renderTargets[i]->GetRenderTargetView();
        SG_ASSERT(m_renderTargets[i]->RenderTargetResolution()->Get() == resolution);
    }
    ID3D11DepthStencilView* dsv = nullptr == m_depthStencilSurface ? NULL : m_depthStencilSurface->GetDepthStencilView();
    SG_ASSERT(nullptr == m_depthStencilSurface || m_depthStencilSurface->Resolution()->Get() == resolution);

    ID3D11DeviceContext* d3dContext = iRenderDevice->ImmediateContext();
    d3dContext->OMSetRenderTargets( (UINT)rtvCount, rtvs, dsv );
    if(rtvCount > 0)
    {
        // Setup the viewport
        D3D11_VIEWPORT vp;
        vp.Width = (FLOAT)resolution.x();
        vp.Height = (FLOAT)resolution.y();
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        d3dContext->RSSetViewports( 1, &vp );

        // Setup a scissor rect
        D3D11_RECT rect;
        rect.left = LONG(vp.TopLeftX);
        rect.top = LONG(vp.TopLeftY);
        rect.right = LONG(vp.Width - vp.TopLeftX);
        rect.bottom = LONG(vp.Height - vp.TopLeftY);
        //rect.left = int(vp.TopLeftX + 60);
        //rect.top = int(vp.TopLeftY + 30);
        //rect.right = std::max(int(vp.Width - vp.TopLeftX - 10), int(rect.left));
        //rect.bottom = std::max(int(vp.Height - vp.TopLeftY - 30), int(rect.top));
        d3dContext->RSSetScissorRects( 1, &rect );
    }
}
//=============================================================================
GenerateMipmap::GenerateMipmap(RenderDevice const* iRenderDevice, Surface* iSurface)
    : m_surface(iSurface)
    , m_mipmapGenerator(iRenderDevice)
{
    refptr<PixelShaderDescriptor> psDesc = new PixelShaderDescriptor(FilePath("src:/Rendering/Shaders/ExampleObjects/Mipmap_Pixel.hlsl"), "pmain");
    m_pixelShader = psDesc->GetProxy();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void GenerateMipmap::Draw(RenderDevice const* iRenderDevice)
{
    m_mipmapGenerator.GenerateMipmap(iRenderDevice, m_surface.get(), m_pixelShader);
}
//=============================================================================
namespace {
    typedef Vertex_Pos2f_Tex2f TexturedQuadVertex;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TexturedQuad::TexturedQuad(RenderDevice const* iRenderDevice, ShaderConstantDatabase const* iDatabase)
    : m_database(iDatabase)
    , m_psConstantBuffers(new ShaderConstantBuffers())
    , m_vsConstantBuffers(new ShaderConstantBuffers())
{
    Prepare(iRenderDevice, FilePath("src:/Rendering/Shaders/ExampleObjects/TexturedQuad.hlsl"));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TexturedQuad::TexturedQuad(RenderDevice const* iRenderDevice, ShaderConstantDatabase const* iDatabase, FilePath const& iPixelShader)
    : m_database(iDatabase)
    , m_psConstantBuffers(new ShaderConstantBuffers())
    , m_vsConstantBuffers(new ShaderConstantBuffers())
{
    Prepare(iRenderDevice, iPixelShader);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TexturedQuad::Prepare(RenderDevice const* iRenderDevice, FilePath const& iPixelShader)
{
    ID3D11Device* device = iRenderDevice->D3DDevice();
    SG_ASSERT(nullptr != device);

    refptr<VertexShaderDescriptor> vsDesc = new VertexShaderDescriptor(FilePath("src:/Rendering/Shaders/ExampleObjects/TexturedQuad.hlsl"), "vmain");
    m_vertexShader = vsDesc->GetProxy();
    refptr<PixelShaderDescriptor> psDesc = new PixelShaderDescriptor(iPixelShader, "pmain");
    m_pixelShader = psDesc->GetProxy();
    m_inputLayout = ShaderInputLayoutProxy(TexturedQuadVertex::InputEltDesc(), m_vertexShader);

    TexturedQuadVertex vertices[] =
    {
        { float2(-1,  1), float2(0, 0) },
        { float2(-1, -1), float2(0, 1) },
        { float2( 1,  1), float2(1, 0) },
        { float2( 1, -1), float2(1, 1) },
    };
    size_t vertexCount = SG_ARRAYSIZE(vertices);

    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage               = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth           = num_cast<UINT>( sizeof( TexturedQuadVertex ) * vertexCount );
    bufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags      = 0;
    bufferDesc.MiscFlags           = 0;
    bufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = vertices;
    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;
    HRESULT hr = device->CreateBuffer( &bufferDesc, &InitData, m_vertexBuffer.GetPointerForInitialisation() );
    SG_ASSERT(SUCCEEDED(hr));

    m_shaderResource = new TextureFromFile(iRenderDevice, FilePath("src:/Rendering/Data/ExampleObjects/pixels.png"));

    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 0;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0.f;
    samplerDesc.BorderColor[1] = 0.f;
    samplerDesc.BorderColor[2] = 0.f;
    samplerDesc.BorderColor[3] = 0.f;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = device->CreateSamplerState(&samplerDesc, m_samplerState.GetPointerForInitialisation());
    SG_ASSERT(SUCCEEDED(hr));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TexturedQuad::~TexturedQuad()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TexturedQuad::Draw(RenderDevice const* iRenderDevice)
{
    ID3D11DeviceContext* context = iRenderDevice->ImmediateContext();
    SG_ASSERT(nullptr != context);

    {
        ID3D11ShaderReflection* reflection = m_pixelShader.GetReflection();
        m_psConstantBuffers->UpdateIFN(iRenderDevice, reflection, m_database.get());
        ID3D11Buffer** buffers = m_psConstantBuffers->Buffers();
        UINT bufferCount = (UINT)m_psConstantBuffers->BufferCount();
        context->PSSetConstantBuffers(
            0,
            bufferCount,
            buffers);
    }

    {
        ID3D11ShaderReflection* reflection = m_vertexShader.GetReflection();
        m_vsConstantBuffers->UpdateIFN(iRenderDevice, reflection, m_database.get());
        ID3D11Buffer** buffers = m_vsConstantBuffers->Buffers();
        UINT bufferCount = (UINT)m_vsConstantBuffers->BufferCount();
        context->VSSetConstantBuffers(
            0,
            bufferCount,
            buffers);
    }

    UINT const stride = sizeof( TexturedQuadVertex );
    UINT const offset = 0;

    ID3D11Buffer* vertexBuffers[] =
    {
        m_vertexBuffer.get()
    };
    UINT const vertexBufferCount = SG_ARRAYSIZE(vertexBuffers);

    context->IASetVertexBuffers(
        0,                // the first input slot for binding
        vertexBufferCount,// the number of buffers in the array
        vertexBuffers,    // the array of vertex buffers
        &stride,          // array of stride values, one for each buffer
        &offset );        // array of offset values, one for each buffer

    // Set the input layout
    context->IASetInputLayout( m_inputLayout.GetInputLayout() );
    context->VSSetShader( m_vertexShader.GetShader(), NULL, 0 );
    context->PSSetShader( m_pixelShader.GetShader(), NULL, 0 );

    ID3D11ShaderResourceView* srvs[] = {
        m_shaderResource->GetShaderResourceView(),
    };
    size_t const srvCount = SG_ARRAYSIZE(srvs);
    context->PSSetShaderResources( 0, srvCount, srvs );

    ID3D11SamplerState* sss[] = {
        m_samplerState.get(),
    };
    size_t const ssCount = SG_ARRAYSIZE(sss);
    context->PSSetSamplers( 0, ssCount, sss );

    context->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

    context->Draw(4, 0);

    ID3D11ShaderResourceView* clearsrvs[] = {
        nullptr,
    };
    size_t const clearsrvCount = SG_ARRAYSIZE(clearsrvs);
    context->PSSetShaderResources( 0, clearsrvCount, clearsrvs );
}
//=============================================================================
// Note about orientations:
// I try to work with all frames direct oriented, and front faces that are
// counter clockwise.
// Render target should have its x axis to the right and z pointing front, which
// should implies a y pointing down.
// However, as DirectX uses a y pointing up, it imposes an indirect frame for
// render target.
// We will work as if y were downside, and flip it as the last step.
Camera::Camera(RenderDevice const* iRenderDevice, ResolutionServer const* iResolution, ShaderConstantDatabase* iDatabase)
    : m_renderDevice(iRenderDevice)
    , m_resolution(iResolution)
    , m_proj(uninitialized)
    , m_database(iDatabase)
    , m_constantName("camera_matrix")
{
    SG_ASSERT(nullptr != m_database);
    m_resolution->RegisterObserver(this);

    ShaderVariable<float4x4_colmajor>* variable =  new ShaderVariable<float4x4_colmajor>();
    m_database->AddVariable(m_constantName, variable);

    //m_view = float4x4::Identity();
    m_view = matrix::HomogeneousTranslation(float3(0, 0, 5));
    ComputeProjMatrix();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Camera::~Camera()
{
    m_resolution->UnregisterObserver(this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Camera::DummyEvolve(float2 const& value)
{
    // rotate
    {
        float3 target = float3(0,0,5);
        float3 dir = float3(value.x(), value.y(), 0.f);
        float3 z = float3(0,0,1);
        float3 axis = cross(z, dir);
        float4x4 R = matrix::HomogeneousRotation(axis, sqrt(value.LengthSq()) * 0.1f);
        float4x4 T = matrix::HomogeneousTranslation(target);
        float4x4 mT = matrix::HomogeneousTranslation(-target);
        m_view = T * R * mT * m_view;
    }
    // translate
    //{
    //    float3 dir = float3(value.x(), value.y(), 0.f);
    //    float4x4 T = matrix::HomogeneousTranslation(dir);
    //    m_view = T * m_view;
    //}

    UpdateFullMatrix();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Camera::ComputeProjMatrix()
{
    uint2 vpwh = m_resolution->Get();
    float const nearPlane = 0.1f;
    float const farPlane = 10.f;
    float2 const viewportWH = float2(vpwh);
    float const aspectRatio = viewportWH.x() / viewportWH.y();

    float const fovYMin = -0.6f;
    float const fovYMax = 0.6f;
    float const fovXMin = fovYMin * aspectRatio;
    float const fovXMax = fovYMax * aspectRatio;

    //float const fovY = 0.5f; // y / z at the boundary of viewport = tan(a/2), where a is the angle of vision
    //float const fovX = fovY * aspectRatio; // x / z at the boundary of viewport

    // Assuming a viewing directon along z, x to the right, y to the bottom, with
    // the center of projection in (0,0).

    // We look for matrix Proj such that:
    // X' = Proj . X
    // if X E Frustum, X' E [-1,1] x [-1,1] x [0,1]

    // The matrix:
    //              1   0   0   0
    //    Proj =    0   1   0   0
    //              0   0   1   0
    //              0   0   1   0
    // gives X' = (x/z, y/z, z/z) = (x/z, y/z, 1)
    // The matrix:
    //              1   0   0   0
    //    Proj =    0   1   0   0
    //              0   0   0   1
    //              0   0   1   0
    // gives X' = (x/z, y/z, 1/z)

    float const np = nearPlane;
    float const fp = farPlane;

    float const ooAvgFovX = 2.f/(fovXMax-fovXMin);
    float const ooAvgFovY = 2.f/(fovYMax-fovYMin);
    float const fovOffsetX = -(fovXMax+fovXMin)/(fovXMax-fovXMin);
    float const fovOffsetY = -(fovYMax+fovYMin)/(fovYMax-fovYMin);

    //float d2i


    float values[] = {
        ooAvgFovX,  0,         0,           fovOffsetX,
        0,          ooAvgFovY, 0,           fovOffsetY,
        0,          0,         fp/(fp-np),  -fp*np/(fp-np),
        0,          0,         1.f,         0,
    };
    /*
    float values[] = {
        1,          0,         0,           0,
        0,          1,         0,           0,
        0,          0,         1,           0,
        0,          0,         1,           0,
    };
    */
    m_proj = float4x4(values);

    float revertYValues[] = {
        1,  0,  0,  0,
        0, -1,  0,  0,
        0,  0,  1,  0,
        0,  0,  0,  1,
    };
    float4x4 revertY = float4x4(revertYValues);
    m_proj = revertY * m_proj;

    UpdateFullMatrix();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Camera::UpdateFullMatrix()
{
    m_camera = m_proj * m_view;

    m_columnMajorCamera = m_camera;
    //DummyEvolve(float2(2,0));

    IShaderVariable* ivar = m_database->GetConstantForWriting(m_constantName);
    ShaderVariable<float4x4_colmajor>* var = checked_cast<ShaderVariable<float4x4_colmajor>*>(ivar);
    var->Set(m_columnMajorCamera);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Camera::VirtualOnNotified(ResolutionServer const* iResolutionServer)
{
    SG_UNUSED(iResolutionServer);
    ComputeProjMatrix();
}
//=============================================================================
namespace {
    typedef Vertex_Pos3f_Normal3f_Tex2f_Col4f CubeVertex;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Cube::Cube(RenderDevice const* iRenderDevice, Camera* iCamera, ShaderConstantDatabase const* iDatabase, float3 const& iCenter)
    : m_inputLayout()
    , m_vertexShader()
    , m_pixelShader()
    , m_vertexBuffer()
    , m_indexBuffer()
    , m_camera(iCamera)
    , m_database(iDatabase)
    , m_psConstantBuffers(new ShaderConstantBuffers())
    , m_vsConstantBuffers(new ShaderConstantBuffers())
{
    ID3D11Device* device = iRenderDevice->D3DDevice();
    SG_ASSERT(nullptr != device);

    SG_ASSERT(nullptr != iCamera);

    refptr<VertexShaderDescriptor> vsDesc = new VertexShaderDescriptor(FilePath("src:/Rendering/Shaders/ExampleObjects/Cube.hlsl"), "vmain");
    m_vertexShader = vsDesc->GetProxy();
    refptr<PixelShaderDescriptor> psDesc = new PixelShaderDescriptor(FilePath("src:/Rendering/Shaders/ExampleObjects/Cube.hlsl"), "pmain");
    m_pixelShader = psDesc->GetProxy();
    m_inputLayout = ShaderInputLayoutProxy(CubeVertex::InputEltDesc(), m_vertexShader);

    {
        float3 const& center = iCenter;
        CubeVertex vertices[] =
        {
            { center + float3(-1.f, -1.f, -1.f), float3( 0.f,  0.f, -1.f), float2(0,0), float4(0,0,0,1) },
            { center + float3(-1.f,  1.f, -1.f), float3( 0.f,  0.f, -1.f), float2(0,1), float4(0,1,0,1) },
            { center + float3( 1.f, -1.f, -1.f), float3( 0.f,  0.f, -1.f), float2(1,0), float4(1,0,0,1) },
            { center + float3( 1.f,  1.f, -1.f), float3( 0.f,  0.f, -1.f), float2(1,1), float4(1,1,0,1) },

            { center + float3(-1.f, -1.f, -1.f), float3(-1.f,  0.f,  0.f), float2(0,0), float4(1,0,0,1) },
            { center + float3(-1.f, -1.f,  1.f), float3(-1.f,  0.f,  0.f), float2(0,1), float4(1,0,0,1) },
            { center + float3(-1.f,  1.f, -1.f), float3(-1.f,  0.f,  0.f), float2(1,0), float4(1,0,0,1) },
            { center + float3(-1.f,  1.f,  1.f), float3(-1.f,  0.f,  0.f), float2(1,1), float4(1,0,0,1) },

            { center + float3(-1.f, -1.f, -1.f), float3( 0.f, -1.f,  0.f), float2(0,0), float4(0,1,0,1) },
            { center + float3( 1.f, -1.f, -1.f), float3( 0.f, -1.f,  0.f), float2(0,1), float4(0,1,0,1) },
            { center + float3(-1.f, -1.f,  1.f), float3( 0.f, -1.f,  0.f), float2(1,0), float4(0,1,0,1) },
            { center + float3( 1.f, -1.f,  1.f), float3( 0.f, -1.f,  0.f), float2(1,1), float4(0,1,0,1) },

            { center + float3( 1.f, -1.f, -1.f), float3( 1.f,  0.f,  0.f), float2(0,0), float4(1,1,0,1) },
            { center + float3( 1.f,  1.f, -1.f), float3( 1.f,  0.f,  0.f), float2(0,1), float4(1,1,0,1) },
            { center + float3( 1.f, -1.f,  1.f), float3( 1.f,  0.f,  0.f), float2(1,0), float4(1,1,0,1) },
            { center + float3( 1.f,  1.f,  1.f), float3( 1.f,  0.f,  0.f), float2(1,1), float4(1,1,0,1) },

            { center + float3(-1.f,  1.f, -1.f), float3( 0.f,  1.f,  0.f), float2(0,0), float4(0,1,1,1) },
            { center + float3(-1.f,  1.f,  1.f), float3( 0.f,  1.f,  0.f), float2(0,1), float4(0,1,1,1) },
            { center + float3( 1.f,  1.f, -1.f), float3( 0.f,  1.f,  0.f), float2(1,0), float4(0,1,1,1) },
            { center + float3( 1.f,  1.f,  1.f), float3( 0.f,  1.f,  0.f), float2(1,1), float4(0,1,1,1) },

            { center + float3(-1.f, -1.f,  1.f), float3( 0.f,  0.f,  1.f), float2(0,0), float4(1,1,1,1) },
            { center + float3( 1.f, -1.f,  1.f), float3( 0.f,  0.f,  1.f), float2(0,1), float4(1,1,1,1) },
            { center + float3(-1.f,  1.f,  1.f), float3( 0.f,  0.f,  1.f), float2(1,0), float4(1,1,1,1) },
            { center + float3( 1.f,  1.f,  1.f), float3( 0.f,  0.f,  1.f), float2(1,1), float4(1,1,1,1) },
        };
        size_t const vertexCount = SG_ARRAYSIZE(vertices);

        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.Usage               = D3D11_USAGE_IMMUTABLE;
        bufferDesc.ByteWidth           = sizeof( CubeVertex ) * vertexCount;
        bufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags      = 0;
        bufferDesc.MiscFlags           = 0;
        bufferDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = vertices;
        InitData.SysMemPitch = 0;
        InitData.SysMemSlicePitch = 0;
        HRESULT hr = device->CreateBuffer( &bufferDesc, &InitData, m_vertexBuffer.GetPointerForInitialisation() );
        SG_ASSERT_AND_UNUSED(SUCCEEDED(hr));

#if 0 // DEBUG
        float4x4 m = m_camera->GetCameraMatrix();
        float4 p[vertexCount];
        for(size_t i = 0; i < vertexCount; ++i)
        {
            p[i] = m * vertices[i].pos.xyz1();
            p[i] /= p[i].w();
        }
        int a = 0;
#endif
    }

    {
        u16 indices[] =
        {
             0, 1, 2,  2, 1, 3,
             4, 5, 6,  6, 5, 7,
             8, 9,10, 10, 9,11,
            12,13,14, 14,13,15,
            16,17,18, 18,17,19,
            20,21,22, 22,21,23,

            24,25,26,
        };
        size_t indexCount = SG_ARRAYSIZE(indices);

        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.Usage               = D3D11_USAGE_IMMUTABLE;
        bufferDesc.ByteWidth           = num_cast<UINT>( sizeof( u16 ) * indexCount );
        bufferDesc.BindFlags           = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.CPUAccessFlags      = 0;
        bufferDesc.MiscFlags           = 0;
        bufferDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = indices;
        InitData.SysMemPitch = 0;
        InitData.SysMemSlicePitch = 0;
        HRESULT hr = device->CreateBuffer( &bufferDesc, &InitData, m_indexBuffer.GetPointerForInitialisation() );
        SG_ASSERT_AND_UNUSED(SUCCEEDED(hr));
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Cube::~Cube()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Cube::Draw(RenderDevice const* iRenderDevice)
{
    ID3D11DeviceContext* context = iRenderDevice->ImmediateContext();
    SG_ASSERT(nullptr != context);

    {
        ID3D11ShaderReflection* reflection = m_pixelShader.GetReflection();
        m_psConstantBuffers->UpdateIFN(iRenderDevice, reflection, m_database.get());
        ID3D11Buffer** buffers = m_psConstantBuffers->Buffers();
        UINT bufferCount = (UINT)m_psConstantBuffers->BufferCount();
        context->PSSetConstantBuffers(
            0,
            bufferCount,
            buffers);
    }

    {
        ID3D11ShaderReflection* reflection = m_vertexShader.GetReflection();
        m_vsConstantBuffers->UpdateIFN(iRenderDevice, reflection, m_database.get());
        ID3D11Buffer** buffers = m_vsConstantBuffers->Buffers();
        UINT bufferCount = (UINT)m_vsConstantBuffers->BufferCount();
        context->VSSetConstantBuffers(
            0,
            bufferCount,
            buffers);
    }

    UINT const stride = sizeof( CubeVertex );
    UINT const offset = 0;
    ID3D11Buffer* vertexBuffer = m_vertexBuffer.get();
    size_t const vertexBufferCount = 1;
    context->IASetVertexBuffers(
        0,
        vertexBufferCount,
        &vertexBuffer,
        &stride,
        &offset );

    context->IASetIndexBuffer(
        m_indexBuffer.get(),
        DXGI_FORMAT_R16_UINT,
        0 );

    context->IASetInputLayout( m_inputLayout.GetInputLayout() );
    context->VSSetShader( m_vertexShader.GetShader(), NULL, 0 );
    context->PSSetShader( m_pixelShader.GetShader(), NULL, 0 );

    context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    context->DrawIndexed(6 * 6, 0, 0);
    //context->DrawIndexed(2 * 6, 4 * 6, 0);
}
//=============================================================================
}
}
}

#include "stdafx.h"

#include "TransientRenderBatch.h"

#include "IShaderResource.h"
#include "RenderDevice.h"
#include "Material.h"
#include "ShaderConstantBuffers.h"
#include "ShaderResourceBuffer.h"
#include <Core/Assert.h>
#include <Core/Log.h>
#include <Core/PerfLog.h>
#include <Core/ArrayView.h>
#include <algorithm>
#include "WTF/IncludeD3D11.h"
#if SG_ENABLE_UNIT_TESTS
#include "InitShutdown.h"
#include "VertexTypes.h"
#include <Core/FileSystem.h>
#include <Core/TestFramework.h>
#include <random>
#endif

namespace sg {
namespace rendering {
//=============================================================================
TransientRenderBatch::TransientRenderBatch(rendering::RenderDevice const* iRenderDevice, Material const* iMaterial, Properties const& iProperties)
: m_properties(iProperties)
, m_material(iMaterial)
, m_vertexData()
, m_indexData()
, m_layerData()
, m_layerDataIndex(0)
, m_layerEnd(0)
, m_bufferIndex(0)
, m_psConstantBuffers(new ShaderConstantBuffers())
, m_vsConstantBuffers(new ShaderConstantBuffers())
, m_psResources(new ShaderResourceBuffer())
, m_vsResources(new ShaderResourceBuffer())
#if SG_ENABLE_ASSERT
, m_reservedVertexCount(0)
, m_reservedIndexCount(0)
#endif
{
    SG_UNUSED(iRenderDevice);
#if SG_ENABLE_ASSERT
    SG_ASSERT(nullptr != m_material);
    ArrayView<D3D11_INPUT_ELEMENT_DESC const> inputLayoutDescriptor = m_material->InputLayout().GetDescriptor();
    for(size_t i = 0; i < inputLayoutDescriptor.size(); ++i)
    {
        SG_ASSERT(0 == inputLayoutDescriptor[i].InputSlot);
    }
    // TODO: check vertex size ?
#endif

#if 1==TRANSIENT_RENDER_BATCH_USE_DOUBLE_BUFFER
    for(size_t i = 0; i < BUFFER_COUNT; ++i)
    {
        m_vertexBuffersCapacity[i] = 0;
        m_indexBufferCapacity[i] = 0;
    }
#endif
    //SG_LOG_DEBUG("Rendering", Format("TransientRenderBatch[%0]::Ctor(%1, %2, %3, %4)", ptrdiff_t(this), ptrdiff_t(iRenderDevice), ptrdiff_t(iMaterial), int(iProperties.indexSize), int(iProperties.vertexSize)));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TransientRenderBatch::~TransientRenderBatch()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TransientRenderBatch::ReserveVertexAndIndex(size_t iVertexCount, size_t iIndexCount)
{
    m_vertexData.reserve((m_vertexCount + iVertexCount) * m_properties.vertexSize);
    m_indexData.reserve((m_indexCount + iIndexCount) * m_properties.indexSize);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
int TransientRenderBatch::GetPriority(RenderBatchPassId iPassId)
{
    SG_ASSERT_AND_UNUSED(iPassId == RenderBatchPassId());
    return m_material->Priority();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TransientRenderBatch::PreExecute(RenderBatchPassId iPassId,
                                      RenderDevice const* iRenderDevice,
                                      IShaderConstantDatabase const* iShaderConstantDatabase,
                                      IShaderResourceDatabase const* iShaderResourceDatabase)
{
    SG_ASSERT_AND_UNUSED(iPassId == RenderBatchPassId());
    UpdateBuffers(iRenderDevice);

    ShaderConstantDatabasePair<false, false> constantDB(&m_material->Constants(), iShaderConstantDatabase);
    ShaderResourceDatabasePair<false, false> resourceDB(&m_material->Resources(), iShaderResourceDatabase);

    ID3D11ShaderReflection* psReflection = m_material->PixelShader().GetReflection();
    m_psConstantBuffers->UpdateIFN(iRenderDevice, psReflection, &constantDB);
    m_psResources->UpdateIFN(psReflection, &resourceDB);

    ID3D11ShaderReflection* vsReflection = m_material->VertexShader().GetReflection();
    m_vsConstantBuffers->UpdateIFN(iRenderDevice, vsReflection, &constantDB);
    m_vsResources->UpdateIFN(vsReflection, &resourceDB);

    SG_ASSERT(!constantDB.IsRefCounted_ForAssert());
    SG_ASSERT(!resourceDB.IsRefCounted_ForAssert());

    m_layerDataIndex = 0;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
size_t TransientRenderBatch::GetSubLayerEnd(RenderBatchPassId iPassId)
{
    SG_ASSERT_AND_UNUSED(iPassId == RenderBatchPassId());
    //SG_LOG_DEBUG("Rendering", Format("TransientRenderBatch[%0]::GetSubLayerEnd() -> %1", ptrdiff_t(this), m_layerEnd));
    return m_layerEnd;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TransientRenderBatch::Execute(RenderBatchPassId iPassId,
                                   RenderDevice const* iRenderDevice,
                                   IShaderConstantDatabase const* iShaderConstantDatabase,
                                   IShaderResourceDatabase const* iShaderResourceDatabase,
                                   size_t iSubLayer,
                                   size_t& ioNextSubLayer)
{
    SG_ASSERT_AND_UNUSED(iPassId == RenderBatchPassId());
    SG_UNUSED((iShaderConstantDatabase, iShaderResourceDatabase));
    //SG_LOG_DEBUG("Rendering", Format("TransientRenderBatch[%0]::Execute(%1, _, _, %2, %3)", ptrdiff_t(this), ptrdiff_t(iRenderDevice), iSubLayer, ioNextSubLayer));

    while(m_layerDataIndex < m_layerData.size() && m_layerData[m_layerDataIndex].layer < iSubLayer)
        ++m_layerDataIndex;
    SG_ASSERT(m_layerDataIndex < m_layerData.size());
    if(m_layerDataIndex >= m_layerData.size())
    {
        ioNextSubLayer = all_ones;
        return;
    }
    size_t const layerIndex = m_layerData[m_layerDataIndex].layer;
    if(layerIndex > iSubLayer)
    {
        ioNextSubLayer = layerIndex;
        return;
    }
    LayerData const& layerData = m_layerData[m_layerDataIndex];
    SG_ASSERT(0 != layerData.indexCount);

    ++m_layerDataIndex;
    if(m_layerDataIndex >= m_layerData.size())
        ioNextSubLayer = all_ones;
    else
        ioNextSubLayer = m_layerData[m_layerDataIndex].layer;

    ID3D11DeviceContext* context = iRenderDevice->ImmediateContext();
    SG_ASSERT(nullptr != context);

    {
        ID3D11Buffer** buffers = m_psConstantBuffers->Buffers();
        UINT bufferCount = (UINT)m_psConstantBuffers->BufferCount();
        context->PSSetConstantBuffers(
            0,
            bufferCount,
            buffers);

        ID3D11ShaderResourceView* srvs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
        size_t srvCount = SG_ARRAYSIZE(srvs);
        m_psResources->GetShaderResourceViews(srvCount, srvs);
        context->PSSetShaderResources( 0, checked_numcastable(srvCount), srvs );

        ID3D11SamplerState* sss[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
        size_t ssCount = SG_ARRAYSIZE(sss);
        m_psResources->GetSamplers(ssCount, sss);
        context->PSSetSamplers( 0, checked_numcastable(ssCount), sss );
    }

    {
        ID3D11Buffer** buffers = m_vsConstantBuffers->Buffers();
        UINT bufferCount = (UINT)m_vsConstantBuffers->BufferCount();
        context->VSSetConstantBuffers(
            0,
            bufferCount,
            buffers);

        ID3D11ShaderResourceView* srvs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
        size_t srvCount = SG_ARRAYSIZE(srvs);
        m_vsResources->GetShaderResourceViews(srvCount, srvs);
        context->VSSetShaderResources( 0, checked_numcastable(srvCount), srvs );

        ID3D11SamplerState* sss[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
        size_t ssCount = SG_ARRAYSIZE(sss);
        m_vsResources->GetSamplers(ssCount, sss);
        context->VSSetSamplers( 0, checked_numcastable(ssCount), sss );
    }

    UINT stride = m_properties.vertexSize;
    UINT offset = 0;
    ID3D11Buffer* vertexBuffer = m_vertexBuffer[m_bufferIndex].get();

    size_t const vertexBufferCount = 1;
    context->IASetVertexBuffers(
        0,
        vertexBufferCount,
        &vertexBuffer,
        &stride,
        &offset );

    DXGI_FORMAT indexFormat = DXGI_FORMAT_UNKNOWN;
    switch(m_properties.indexSize)
    {
        case sizeof(u16): indexFormat = DXGI_FORMAT_R16_UINT; break;
        case sizeof(u32): indexFormat = DXGI_FORMAT_R32_UINT; break;
        default:
            SG_ASSERT_NOT_REACHED();
    }
    ID3D11Buffer* indexBuffer = m_indexBuffer[m_bufferIndex].get();
    context->IASetIndexBuffer(
        indexBuffer,
        indexFormat,
        0 );

    context->IASetInputLayout( m_material->InputLayout().GetInputLayout() );
    context->VSSetShader( m_material->VertexShader().GetShader(), NULL, 0 );
    context->PSSetShader( m_material->PixelShader().GetShader(), NULL, 0 );

    context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    context->DrawIndexed(layerData.indexCount, layerData.indexBegin, 0);

    // This may not be done that often, maybe. If a CompositingLayer has
    // exclusivity over rendering, it can be done at the end of the layer,
    // as not doing this can be problematic only if a shader resource is
    // used as a render target while still bound.
    ID3D11ShaderResourceView* clearsrvs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
    context->PSSetShaderResources( 0, checked_numcastable(D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT), clearsrvs );
    context->VSSetShaderResources( 0, checked_numcastable(D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT), clearsrvs );
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TransientRenderBatch::PostExecute(RenderBatchPassId iPassId)
{
    SG_ASSERT_AND_UNUSED(iPassId == RenderBatchPassId());
    m_bufferIndex = (m_bufferIndex + 1) % BUFFER_COUNT;
    m_vertexCount = 0;
    m_indexCount = 0;
    m_layerDataIndex = 0;
    m_layerData.clear();
    m_layerEnd = 0;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void* TransientRenderBatch::GetVertexPointerForWritingImpl(size_t iSizeofVertex, size_t iMaxVertexCount)
{
    SG_ASSERT(0 < iMaxVertexCount);
    SG_ASSERT(0 == m_reservedVertexCount);
    SG_ASSERT(m_properties.vertexSize == iSizeofVertex);
    size_t const vertexCount = m_vertexCount;
    size_t const newVertexCount = vertexCount + iMaxVertexCount;
#if SG_ENABLE_ASSERT
    size_t const newSize = (newVertexCount + 1) * iSizeofVertex;
#else
    size_t const newSize = newVertexCount * iSizeofVertex;
#endif
    if(m_vertexData.size() < newSize)
        m_vertexData.resize(newSize);
#if SG_ENABLE_ASSERT
    u8 const guard = 0xDD;
    u8*const buffer = m_vertexData.data();
    u8*const begin = buffer + vertexCount * iSizeofVertex;
    u8*const end = buffer + m_vertexData.size();
    memset(begin, guard, end-begin);
#endif
    SG_CODE_FOR_ASSERT(m_reservedVertexCount = newVertexCount);
    SG_ASSERT(0 < m_properties.vertexSize);
    return m_vertexData.data() + vertexCount * iSizeofVertex;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TransientRenderBatch::FinishWritingVertex(size_t iVertexCount)
{
    SG_ASSERT(0 != m_reservedVertexCount);
    size_t const vertexCount = m_vertexCount;
    size_t const newVertexCount = vertexCount + iVertexCount;
    SG_ASSERT(newVertexCount <= m_reservedVertexCount);
    m_vertexCount = newVertexCount;
#if SG_ENABLE_ASSERT
    u8 const guard = 0xDD;
    size_t const sizeofWord= sizeof(uintptr_t);
    u8*const buffer = m_vertexData.data();
    u8*const begin = buffer + newVertexCount * m_properties.vertexSize;
#if 0 // This is the full check
    u8*const end = buffer + m_vertexData.size();
#elif 1 // This is a reduced check, only few bytes after end of buffer
    u8*const end = buffer + std::min(size_t(newVertexCount * m_properties.vertexSize + 8), m_vertexData.size());
#else // no check
    u8*const end = begin;
#endif
#if 0
    for(u8* p = begin; p < end; ++p)
        SG_ASSERT_MSG(guard == *p, "It seems that client has writen beyond buffer end");
#else
    uintptr_t const guardWord = broadcast_byte<uintptr_t, guard>::value;
    u8*const beginWord = reinterpret_cast<u8*>(((ptrdiff_t(begin)+sizeofWord-1)/sizeofWord)*sizeofWord);
    u8*const endWord = reinterpret_cast<u8*>((ptrdiff_t(end)/sizeofWord)*sizeofWord);
    for(u8* p = begin; p < beginWord; ++p)
        SG_ASSERT_MSG(guard == *p, "It seems that client has writen beyond buffer end");
    for(uintptr_t* p = reinterpret_cast<uintptr_t*>(beginWord); p < reinterpret_cast<uintptr_t*>(endWord); ++p)
        SG_ASSERT_MSG(guardWord == *p, "It seems that client has writen beyond buffer end");
    for(u8* p = endWord; p < end; ++p)
        SG_ASSERT_MSG(guard == *p, "It seems that client has writen beyond buffer end");
#endif
#endif
    SG_CODE_FOR_ASSERT(m_reservedVertexCount = 0);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void* TransientRenderBatch::GetIndexPointerForWritingImpl(size_t iSizeofIndex, size_t iMaxIndexCount, size_t& oFirstVertexIndex)
{
    SG_ASSERT(0 < iMaxIndexCount);
    SG_ASSERT(0 == iMaxIndexCount % 3);
    SG_ASSERT(0 == m_reservedIndexCount);
    SG_ASSERT(0 != m_reservedVertexCount);
    size_t const firstVertexIndex = m_vertexCount;
    oFirstVertexIndex = firstVertexIndex;
    size_t const indexCount = m_indexCount;
    size_t const newIndexCount = indexCount + iMaxIndexCount;
    size_t const newSize = newIndexCount * iSizeofIndex;
    if(m_indexData.size() < newSize)
        m_indexData.resize(newSize);
    SG_CODE_FOR_ASSERT(m_reservedIndexCount = newIndexCount);
    return m_indexData.data() + indexCount * iSizeofIndex;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TransientRenderBatch::FinishWritingIndex(size_t iIndexCount, size_t iLayer)
{
    SG_ASSERT(0 <= iIndexCount);
    SG_ASSERT(0 == iIndexCount % 3);
    SG_ASSERT(0 != m_reservedIndexCount);
    if(0 != iIndexCount)
    {
        size_t const indexCount = m_indexCount;

#if 1 // Can be enabled or disabled depending on performances
        if(!m_layerData.empty() && m_layerData.back().layer == iLayer)
        {
            m_layerData.back().indexCount = checked_numcastable(m_layerData.back().indexCount + iIndexCount);
        }
        else
#endif
        {
            m_layerData.emplace_back();
            m_layerData.back().layer = checked_numcastable(iLayer);
            m_layerData.back().indexBegin = checked_numcastable(indexCount);
            m_layerData.back().indexCount = checked_numcastable(iIndexCount);
        }
        m_layerEnd = std::max(m_layerEnd, iLayer+1);

        size_t const newIndexCount = indexCount + iIndexCount;
#if SG_ENABLE_ASSERT
        switch(m_properties.indexSize)
        {
        case sizeof(u16):
            {
                u16* indices = static_cast<u16*>((void*)m_indexData.data());
                SG_UNUSED(indices);
                for(size_t i = indexCount; i < newIndexCount; ++i)
                {
                    SG_ASSERT(indices[i] < m_reservedVertexCount || 0 == m_reservedVertexCount);
                    SG_ASSERT(indices[i] < m_vertexCount || 0 != m_reservedVertexCount);
                }
            }
            break;
        case sizeof(u32):
            {
                u32* indices = static_cast<u32*>((void*)m_indexData.data());
                SG_UNUSED(indices);
                for(size_t i = indexCount; i < newIndexCount; ++i)
                {
                    SG_ASSERT(indices[i] < m_reservedVertexCount || 0 == m_reservedVertexCount);
                    SG_ASSERT(indices[i] < m_vertexCount || 0 != m_reservedVertexCount);
                }
            }
            break;
        default:
            SG_ASSERT_NOT_IMPLEMENTED();
        }
#endif
        SG_ASSERT(newIndexCount <= m_reservedIndexCount);
        m_indexCount = newIndexCount;
    }
    SG_CODE_FOR_ASSERT(m_reservedIndexCount = 0);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TransientRenderBatch::UpdateBuffers(rendering::RenderDevice const* iRenderDevice)
{
    size_t const vertexCount = m_vertexCount;
    size_t const indexCount = m_indexCount;

    if(0 == vertexCount || 0 == indexCount)
        return;
#if SG_ENABLE_ASSERT
    SG_ASSERT(vertexCount * m_properties.vertexSize <= m_vertexData.size());
#endif
    if(m_vertexBuffersCapacity[m_bufferIndex] < vertexCount)
    {
        ID3D11Device* device = iRenderDevice->D3DDevice();
        SG_ASSERT(nullptr != device);

        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.Usage               = D3D11_USAGE_DEFAULT; // for UpdateSubResource
        bufferDesc.ByteWidth           = checked_numcastable(m_properties.vertexSize * vertexCount);
        bufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags      = 0;
        bufferDesc.MiscFlags           = 0;
        bufferDesc.StructureByteStride = 0;
        SG_ASSERT(bufferDesc.ByteWidth  <= m_vertexData.size());

        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = m_vertexData.data();
        InitData.SysMemPitch = 0;
        InitData.SysMemSlicePitch = 0;

        HRESULT hr = device->CreateBuffer( &bufferDesc, &InitData, m_vertexBuffer[m_bufferIndex].GetPointerForInitialisation() );
        SG_ASSERT_AND_UNUSED(SUCCEEDED(hr));

        m_vertexBuffersCapacity[m_bufferIndex] = vertexCount;
    }
    else
    {
        SG_ASSERT(vertexCount > 0);
        ID3D11DeviceContext* context = iRenderDevice->ImmediateContext();
        SG_ASSERT(nullptr != context);

        D3D11_BOX box;
        box.left = 0;
        box.top = 0;
        box.front = 0;
        box.right = checked_numcastable(m_properties.vertexSize * vertexCount);
        box.bottom = 1;
        box.back = 1;
        context->UpdateSubresource(m_vertexBuffer[m_bufferIndex].get(), 0, &box, m_vertexData.data(), 0, 0);
    }

    SortIndexData();

    if(m_indexBufferCapacity[m_bufferIndex] < indexCount)
    {
        ID3D11Device* device = iRenderDevice->D3DDevice();
        SG_ASSERT(nullptr != device);

        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.Usage               = D3D11_USAGE_DEFAULT; // for UpdateSubResource
        bufferDesc.ByteWidth           = checked_numcastable( static_cast<size_t>(m_properties.indexSize) * indexCount );
        bufferDesc.BindFlags           = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.CPUAccessFlags      = 0;
        bufferDesc.MiscFlags           = 0;
        bufferDesc.StructureByteStride = 0;
        SG_ASSERT(bufferDesc.ByteWidth  <= m_indexData.size());

        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = m_indexData.data();
        InitData.SysMemPitch = 0;
        InitData.SysMemSlicePitch = 0;
        HRESULT hr = device->CreateBuffer( &bufferDesc, &InitData, m_indexBuffer[m_bufferIndex].GetPointerForInitialisation() );
        SG_ASSERT_AND_UNUSED(SUCCEEDED(hr));

        m_indexBufferCapacity[m_bufferIndex] = indexCount;
    }
    else
    {
        SG_ASSERT(indexCount > 0);
        ID3D11DeviceContext* context = iRenderDevice->ImmediateContext();
        SG_ASSERT(nullptr != context);
        D3D11_BOX box;
        box.left = 0;
        box.top = 0;
        box.front = 0;
        box.right = checked_numcastable(m_properties.indexSize * indexCount);
        box.bottom = 1;
        box.back = 1;
        context->UpdateSubresource(m_indexBuffer[m_bufferIndex].get(), 0, &box, m_indexData.data(), 0, 0);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TransientRenderBatch::SortIndexData()
{
    SG_ASSERT(!m_layerData.empty());
    SG_ASSERT(!m_indexData.empty());
    using std::sort;
    sort(m_layerData.begin(), m_layerData.end(),
         [](LayerData const& a, LayerData const& b) { return a.layer < b.layer || (a.layer == b.layer && a.indexBegin < b.indexBegin); });

#define USE_IN_PLACE_SORT 0 // 0: sort indices to new buffer, 1: use in-place sort

#if 0==USE_IN_PLACE_SORT || SG_ENABLE_ASSERT || SG_ENABLE_PERF_LOG
    SG_CODE_FOR_ASSERT(std::vector<u8> prevIndexData = m_indexData;)
    std::vector<u8> sortedIndexData;
    {
        //SIMPLE_CPU_PERF_LOG_SCOPE("MultiLayerTransientRenderBatch::SortIndexData - simple method");
        sortedIndexData.resize(m_indexData.size());
        size_t layer = all_ones;
        std::vector<LayerData> layerData;
        u8 const*const indexData = m_indexData.data();
        u8*const sortedData = sortedIndexData.data();
        u8* dst = sortedData;
        for(auto it : m_layerData)
        {
            if(it.layer != layer)
            {
                SG_ASSERT(0 == ((dst-sortedData) % m_properties.indexSize));
                size_t const curIndex = (dst-sortedData) / m_properties.indexSize;
                if(-1 != layer)
                {
                    size_t const indexCount = curIndex - layerData.back().indexBegin;
                    SG_ASSERT(-1 == layerData.back().indexCount);
                    layerData.back().indexCount = checked_numcastable(indexCount);
                }
                layer = it.layer;
                layerData.emplace_back();
                layerData.back().layer = checked_numcastable(layer);
                layerData.back().indexBegin = checked_numcastable(curIndex);
                SG_CODE_FOR_ASSERT(layerData.back().indexCount = all_ones;)
            }
            size_t const srcBegin = it.indexBegin * m_properties.indexSize;
            size_t const srcSize = it.indexCount * m_properties.indexSize;
            SG_ASSERT(0 == srcBegin % 2);
#if 0
            size_t const srcEnd = srcBegin + srcSize;
            SG_ASSERT(0 == srcEnd % 2);
            for(size_t i = srcBegin; i < srcEnd; i += 2, dst += 2)
            {
                *reinterpret_cast<u16*>(dst) = *reinterpret_cast<u16 const*>(indexData + i);
            }
#else
            memcpy(dst, indexData + srcBegin, srcSize);
            dst += srcSize;
#endif
        }
        {
            size_t const curIndex = (dst-sortedData) / m_properties.indexSize;
            size_t const indexCount = curIndex - layerData.back().indexBegin;
            SG_ASSERT(-1 == layerData.back().indexCount);
            layerData.back().indexCount = checked_numcastable(indexCount);
        }
#if 0==USE_IN_PLACE_SORT
        {
            using std::swap;
            swap(m_indexData, sortedIndexData);
            swap(m_layerData, layerData);
        }
#endif
    }
#endif

#if 1==USE_IN_PLACE_SORT
    SG_CODE_FOR_ASSERT(u16 const*const prevu16 = reinterpret_cast<u16 const*>(prevIndexData.data());)
    SG_CODE_FOR_ASSERT(u16 const*const sorted16 = reinterpret_cast<u16 const*>(sortedIndexData.data());)
    SG_CODE_FOR_ASSERT(u16 const*const index16 = reinterpret_cast<u16 const*>(m_indexData.data());)
    {
        SIMPLE_CPU_PERF_LOG_SCOPE("MultiLayerTransientRenderBatch::SortIndexData - in-place sort");
        struct Redirect
        {
            size_t prevBegin;
            size_t newBegin;
            size_t count;
        };
        std::vector<Redirect> redirects;
        redirects.reserve(m_layerData.size() * 10);
        struct Range
        {
            size_t begin;
            size_t count;
            Range(size_t iBegin, size_t iCount) : begin(iBegin), count(iCount) {}
        };
        std::vector<Range> srcRanges;
        srcRanges.reserve(100);

        size_t layer = -1;
        std::vector<LayerData> layerData;

        u8* const indexData = m_indexData.data();

        size_t dst = 0;
        for(auto it : m_layerData)
        {
            if(it.layer != layer)
            {
                size_t const curIndex = dst;
                if(-1 != layer)
                {
                    size_t const indexCount = curIndex - layerData.back().indexBegin;
                    SG_ASSERT(all_ones == layerData.back().indexCount);
                    layerData.back().indexCount = checked_numcastable(indexCount);
                }
                layer = it.layer;
                layerData.emplace_back();
                layerData.back().layer = layer;
                layerData.back().indexBegin = checked_numcastable(curIndex);
                SG_CODE_FOR_ASSERT(layerData.back().indexCount = -1;)
            }
            srcRanges.emplace_back(it.indexBegin, it.indexCount);
            while(!srcRanges.empty())
            {
                size_t const srcBegin = srcRanges.back().begin;
                size_t const srcCount = srcRanges.back().count;
                srcRanges.pop_back();
                if(srcBegin < dst)
                {
                    struct CompRedirect
                    {
                        bool operator() (Redirect const& a, size_t val) { SG_ASSERT(0 != a.count); return a.prevBegin + a.count <= val; }
                        bool operator() (size_t val, Redirect const& a) { SG_ASSERT(0 != a.count); return val < a.prevBegin; }
                    };
                    auto f = std::lower_bound(redirects.begin(), redirects.end(), srcBegin, CompRedirect());
                    SG_ASSERT(f != redirects.end());
                    size_t const srcInPrev = srcBegin - f->prevBegin;
                    if(srcInPrev + srcCount <= f->count)
                    {
                        srcRanges.emplace_back(f->newBegin + srcInPrev, srcCount);
                    }
                    else
                    {
                        size_t const countInPrev = f->count - srcInPrev;
                        srcRanges.emplace_back(srcBegin + countInPrev, srcCount - countInPrev);
                        SG_ASSERT(0 != srcRanges.back().count);
                        srcRanges.emplace_back(f->newBegin + srcInPrev, countInPrev);
                        SG_ASSERT(0 != srcRanges.back().count);
                    }
                }
                else if(srcBegin == dst)
                {
                    // do nothing
                    dst += srcCount;
                }
                else if(srcBegin < dst + srcCount)
                {
                    // swapping will displace [dst, dst+count] after src, but
                    // it may have been rotated in the process.
                    // => there can be 2 indirections.
                    size_t const displacedCount = srcBegin-dst;
                    size_t const rotation = srcCount % displacedCount;
                    if(0 != rotation)
                    {
                        redirects.emplace_back();
                        redirects.back().prevBegin = dst;
                        redirects.back().newBegin = dst+srcCount+displacedCount-rotation;
                        redirects.back().count = rotation;
                    }
                    redirects.emplace_back();
                    redirects.back().prevBegin = dst+rotation;
                    redirects.back().newBegin = dst+srcCount;
                    redirects.back().count = srcBegin-(dst+rotation);
                    SG_ASSERT(0 != redirects.back().count);
                    size_t const srcEnd = srcBegin + srcCount;
                    for(size_t src = srcBegin; src < srcEnd; ++src, ++dst)
                    {
                        using std::swap;
                        for(size_t o = 0; o < m_properties.indexSize; ++o)
                            swap(indexData[dst * m_properties.indexSize + o], indexData[src * m_properties.indexSize + o]);
                    }
                }
                else
                {
                    redirects.emplace_back();
                    redirects.back().prevBegin = dst;
                    redirects.back().newBegin = srcBegin;
                    redirects.back().count = srcCount;
                    SG_ASSERT(0 != redirects.back().count);
                    size_t const srcEnd = srcBegin + srcCount;
                    for(size_t i = srcBegin; i < srcEnd; ++i, ++dst)
                    {
                        using std::swap;
                        for(size_t o = 0; o < m_properties.indexSize; ++o)
                            swap(indexData[dst * m_properties.indexSize + o], indexData[i * m_properties.indexSize + o]);
                    }
                }
            }
        }
        {
            size_t const curIndex = dst;
            size_t const indexCount = curIndex - layerData.back().indexBegin;
            SG_ASSERT(all_ones == layerData.back().indexCount);
            layerData.back().indexCount = checked_numcastable(indexCount);
        }
        {
            using std::swap;
            swap(m_layerData, layerData);
        }
    }

#if SG_ENABLE_ASSERT
    size_t const indexSize = m_indexData.size();
    SG_ASSERT(sortedIndexData.size() == indexSize);
    for(size_t i = 0; i < indexSize; ++i)
    {
        SG_ASSERT(sortedIndexData[i] == m_indexData[i]);
    }
#endif

#endif

#undef USE_IN_PLACE_SORT
}
//=============================================================================
#if SG_ENABLE_UNIT_TESTS
namespace {
    typedef rendering::Vertex_Pos2f_Tex2f_Col4f UIVertex;
}
void TransientRenderBatch::Test()
{
    perflog::Init();
    filesystem::Init();
    filesystem::MountDeclaredMountingPoints();
    rendering::Init();

    {
        scopedptr<RenderDevice> renderDevice(new rendering::RenderDevice());
        typedef u32 IndexType;
        TransientRenderBatch::Properties prop;
        prop.indexSize = sizeof(IndexType);
        prop.vertexSize = 1;
        refptr<rendering::VertexShaderDescriptor> vsDesc = new rendering::VertexShaderDescriptor(FilePath("src:/Rendering/UnitTests/Shaders/V_dummy.hlsl"), "vmain");
        rendering::VertexShaderProxy vs = vsDesc->GetProxy();
        refptr<rendering::PixelShaderDescriptor> psDesc = new rendering::PixelShaderDescriptor(FilePath("src:/Rendering/UnitTests/Shaders/P_dummy.hlsl"), "pmain");
        rendering::PixelShaderProxy ps = psDesc->GetProxy();
        rendering::ShaderInputLayoutProxy layout(UIVertex::InputEltDesc(), vs);
        RenderStateName blendState("Premultiplied Alpha Blending");
        refptr<rendering::Material> material = new rendering::Material(layout, vs, ps, blendState);

        size_t const drawCount = 1000;
        size_t const minTriangleCountPerDraw = 1;
        size_t const maxTriangleCountPerDraw = 50;
        size_t const layerCount = 30;
        size_t const maxIndex = drawCount * maxTriangleCountPerDraw * 3;

        unsigned int const seed = 0x563A7D64; //std::chrono::system_clock::now().time_since_epoch().count();
        std::mt19937 generator(seed);
        std::uniform_int_distribution<size_t> randLayer(0, layerCount-1);
        std::uniform_int_distribution<size_t> randTriangleCount(minTriangleCountPerDraw, maxTriangleCountPerDraw);

        for(size_t kk = 0; kk < 1; ++kk)
        {
            TransientRenderBatch batch(renderDevice.get(), material.get(), prop);

            batch.GetVertexPointerForWritingImpl(prop.vertexSize, maxIndex);

            for(size_t i = 0; i < drawCount; ++i)
            {
                size_t const triangleCount = randTriangleCount(generator);
                size_t const layer = randLayer(generator);
                size_t firstVertexIndex = 0;
                IndexType* indices = static_cast<IndexType*>(batch.GetIndexPointerForWritingImpl(sizeof(IndexType), triangleCount*3, firstVertexIndex));
                SG_ASSERT(0 == firstVertexIndex);
                for(size_t j = 0; j < triangleCount; ++j, indices += 3)
                {
                    indices[0] = checked_numcastable(batch.m_indexCount+j*3);
                    indices[1] = checked_numcastable(layer);
                    indices[2] = checked_numcastable(i);
                }
                batch.FinishWritingIndex(triangleCount*3, layer);
            }
            batch.FinishWritingVertex(maxIndex);

            batch.SortIndexData();
        }
    }

    rendering::Shutdown();
    filesystem::Shutdown();
    perflog::Shutdown();
}
SG_TEST((sg, rendering), TransientRenderBatch, (Rendering, slow))
{
    TransientRenderBatch::Test();
}
#endif
//=============================================================================
}
}

#include "stdafx.h"

#include "RenderBatch.h"

#include <Rendering/RenderDevice.h>
#include <Rendering/ShaderConstantBuffers.h>

namespace sg {
namespace renderengine {
//=============================================================================
RenderBatch::RenderBatch(rendering::RenderDevice const* iRenderDevice, RenderBatchDescriptor const& iDescriptor)
: m_descriptor(iDescriptor)
, m_vertexCount()
, m_indexCount(0)
, m_vertexData()
, m_indexData()
, m_vertexWriteOffset(0)
, m_bufferIndex(0)
, m_vertexBuffers()
, m_indexBuffer()
, m_psConstantBuffers(new rendering::ShaderConstantBuffers())
, m_vsConstantBuffers(new rendering::ShaderConstantBuffers())
, m_psResources(new rendering::ShaderResourceBuffer())
, m_vsResources(new rendering::ShaderResourceBuffer())
#if SG_ENABLE_ASSERT
, m_reservedVertexCount()
, m_reservedIndexCount(0)
#endif
{
    SG_UNUSED(iRenderDevice);
    for(size_t i = 0; i < BUFFER_COUNT; ++i)
    {
        m_vertexBuffersCapacity[i] = 0;
        m_indexBufferCapacity[i] = 0;
    }

#if SG_ENABLE_ASSERT
    SG_ASSERT(m_descriptor.inputSlotCount > 0);
    ArrayView<D3D11_INPUT_ELEMENT_DESC const> inputLayoutDescriptor = m_descriptor.inputLayout.GetDescriptor();
    // TODO: check coherence
#endif

    m_vertexData.resize(m_descriptor.inputSlotCount);
    m_vertexCount.resize(m_descriptor.inputSlotCount, 0);
    for(size_t b = 0; b < BUFFER_COUNT; ++b)
        m_vertexBuffers[b].resize(m_descriptor.inputSlotCount);
    SG_CODE_FOR_ASSERT(m_reservedVertexCount.resize(m_descriptor.inputSlotCount, 0));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
RenderBatch::~RenderBatch()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void* RenderBatch::GetVertexPointerForWritingImpl(size_t iSizeofVertex, size_t iMaxVertexCount, size_t iInputSlot)
{
    SG_ASSERT(0 < iMaxVertexCount);
    SG_ASSERT(iInputSlot < m_descriptor.inputSlotCount);
    SG_ASSERT(0 == m_reservedVertexCount[iInputSlot]);
    SG_ASSERT(m_vertexData.size() == m_descriptor.inputSlotCount);
    SG_ASSERT(m_vertexCount.size() == m_descriptor.inputSlotCount);
    SG_ASSERT(m_descriptor.vertexSize[iInputSlot] == iSizeofVertex);
    size_t const vertexCount = m_vertexCount[iInputSlot];
    size_t const newVertexCount = vertexCount + iMaxVertexCount;
#if SG_ENABLE_ASSERT
    size_t const newSize = (newVertexCount + 1) * iSizeofVertex;
#else
    size_t const newSize = newVertexCount * iSizeofVertex;
#endif
    if(m_vertexData[iInputSlot].size() < newSize)
        m_vertexData[iInputSlot].resize(newSize);
#if SG_ENABLE_ASSERT
    u8 const guard = 0xDD;
    u8*const buffer = m_vertexData[iInputSlot].data();
    u8*const begin = buffer + vertexCount * iSizeofVertex;
    u8*const end = buffer + m_vertexData[iInputSlot].size();
    memset(begin, guard, end-begin);
#endif
    SG_CODE_FOR_ASSERT(m_reservedVertexCount[iInputSlot] = newVertexCount);
    SG_ASSERT(0 < m_descriptor.vertexSize[iInputSlot]);
    return m_vertexData[iInputSlot].data() + vertexCount * iSizeofVertex;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RenderBatch::FinishWritingVertex(size_t iVertexCount, size_t iInputSlot)
{
    SG_ASSERT(0 != m_reservedVertexCount[iInputSlot]);
    size_t const vertexCount = m_vertexCount[iInputSlot];
    size_t const newVertexCount = vertexCount + iVertexCount;
    SG_ASSERT(newVertexCount <= m_reservedVertexCount[iInputSlot]);
    m_vertexCount[iInputSlot] = newVertexCount;
#if SG_ENABLE_ASSERT
    u8 const guard = 0xDD;
    size_t sizeofWord= sizeof(uintptr_t);
    u8*const buffer = m_vertexData[iInputSlot].data();
    u8*const begin = buffer + newVertexCount * m_descriptor.vertexSize[iInputSlot];
    u8*const end = buffer + m_vertexData[iInputSlot].size();
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
    SG_CODE_FOR_ASSERT(m_reservedVertexCount[iInputSlot] = 0);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void* RenderBatch::GetIndexPointerForWritingImpl(size_t iSizeofIndex, size_t iMaxIndexCount)
{
    SG_ASSERT(0 < iMaxIndexCount);
    SG_ASSERT(0 == m_reservedIndexCount);

    SG_ASSERT(0 != m_reservedVertexCount[0]);
    size_t const firstVertexIndex = m_vertexCount[0];
    m_vertexWriteOffset = firstVertexIndex;
#if SG_ENABLE_ASSERT
    for(size_t i = 1; i < m_descriptor.inputSlotCount; ++i)
        SG_ASSERT(firstVertexIndex == m_vertexCount[i]);
#endif
    size_t const indexCount = m_indexCount;
    size_t const newIndexCount = indexCount + iMaxIndexCount;
    size_t const newSize = newIndexCount * iSizeofIndex;
    if(m_indexData.size() < newSize)
        m_indexData.resize(newSize);
    SG_CODE_FOR_ASSERT(m_reservedIndexCount = newIndexCount);
    return m_indexData.data() + indexCount * iSizeofIndex;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RenderBatch::FinishWritingIndex(size_t iIndexCount)
{
    SG_ASSERT(0 != m_reservedIndexCount);
    size_t const indexCount = m_indexCount;
    size_t const newIndexCount = indexCount + iIndexCount;
    switch(m_descriptor.indexSize)
    {
    case RenderBatchDescriptor::IndexSize::u16:
        {
            u16* indices = static_cast<u16*>((void*)m_indexData.data());
            for(size_t i = indexCount; i < newIndexCount; ++i)
            {
                indices[i] = checked_numcastable(indices[i] + m_vertexWriteOffset);
                SG_ASSERT(indices[i] < m_reservedVertexCount[0] || 0 == m_reservedVertexCount[0]);
                SG_ASSERT(indices[i] < m_vertexCount[0] || 0 != m_reservedVertexCount[0]);
            }
        }
        break;
    case RenderBatchDescriptor::IndexSize::u32:
        {
            u32* indices = static_cast<u32*>((void*)m_indexData.data());
            for(size_t i = indexCount; i < newIndexCount; ++i)
            {
                indices[i] = checked_numcastable(indices[i] + m_vertexWriteOffset);
                SG_ASSERT(indices[i] < m_reservedVertexCount[0] || 0 == m_reservedVertexCount[0]);
                SG_ASSERT(indices[i] < m_vertexCount[0] || 0 != m_reservedVertexCount[0]);
            }
        }
        break;
    default:
        SG_ASSERT_NOT_IMPLEMENTED();
    }
    SG_ASSERT(newIndexCount <= m_reservedIndexCount);
    m_indexCount = newIndexCount;
    SG_CODE_FOR_ASSERT(m_reservedIndexCount = 0);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RenderBatch::UpdateBuffers(rendering::RenderDevice const* iRenderDevice)
{
    size_t const vertexCount = m_vertexCount[0];
    size_t const indexCount = m_indexCount;
    size_t const inputSlotCount = m_descriptor.inputSlotCount;
#if SG_ENABLE_ASSERT
    for(size_t i = 0; i < inputSlotCount; ++i)
    {
        SG_ASSERT(vertexCount == m_vertexCount[i]);
        SG_ASSERT(vertexCount * m_descriptor.vertexSize[i] <= m_vertexData[i].size());
    }
#endif
    if(m_vertexBuffersCapacity[m_bufferIndex] < vertexCount)
    {
        ID3D11Device* device = iRenderDevice->D3DDevice();
        SG_ASSERT(nullptr != device);

        for(size_t i = 0; i < inputSlotCount; ++i)
        {
            D3D11_BUFFER_DESC bufferDesc;
            bufferDesc.Usage               = D3D11_USAGE_DEFAULT; // for UpdateSubResource
            bufferDesc.ByteWidth           = checked_numcastable(m_descriptor.vertexSize[i] * vertexCount);
            bufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
            bufferDesc.CPUAccessFlags      = 0;
            bufferDesc.MiscFlags           = 0;
            bufferDesc.StructureByteStride = 0;
            SG_ASSERT(bufferDesc.ByteWidth  <= m_vertexData[i].size());

            D3D11_SUBRESOURCE_DATA InitData;
            InitData.pSysMem = m_vertexData[i].data();
            InitData.SysMemPitch = 0;
            InitData.SysMemSlicePitch = 0;

            HRESULT hr = device->CreateBuffer( &bufferDesc, &InitData, m_vertexBuffers[m_bufferIndex][i].GetPointerForInitialisation() );
            SG_ASSERT_AND_UNUSED(SUCCEEDED(hr));
        }

        m_vertexBuffersCapacity[m_bufferIndex] = vertexCount;
    }
    else
    {
        ID3D11DeviceContext* context = iRenderDevice->ImmediateContext();
        SG_ASSERT(nullptr != context);
        for(size_t i = 0; i < inputSlotCount; ++i)
        {
            D3D11_BOX box;
            box.left = 0;
            box.top = 0;
            box.front = 0;
            box.right = checked_numcastable(m_descriptor.vertexSize[i] * vertexCount);
            box.bottom = 1;
            box.back = 1;
            context->UpdateSubresource(m_vertexBuffers[m_bufferIndex][i].get(), 0, &box, m_vertexData[i].data(), 0, 0);
        }
    }
    if(m_indexBufferCapacity[m_bufferIndex] < indexCount)
    {
        ID3D11Device* device = iRenderDevice->D3DDevice();
        SG_ASSERT(nullptr != device);

        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.Usage               = D3D11_USAGE_DEFAULT; // for UpdateSubResource
        bufferDesc.ByteWidth           = checked_numcastable( static_cast<size_t>(m_descriptor.indexSize) * indexCount );
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
        ID3D11DeviceContext* context = iRenderDevice->ImmediateContext();
        SG_ASSERT(nullptr != context);
        D3D11_BOX box;
        box.left = 0;
        box.top = 0;
        box.front = 0;
        box.right = checked_numcastable(static_cast<int>(m_descriptor.indexSize) * vertexCount);
        box.bottom = 1;
        box.back = 1;
        context->UpdateSubresource(m_indexBuffer[m_bufferIndex].get(), 0, &box, m_indexData.data(), 0, 0);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
int RenderBatch::GetPriority(rendering::RenderBatchPassId iPassId)
{
    SG_ASSERT_AND_UNUSED(iPassId == rendering::RenderBatchPassId());
    return 0;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
size_t RenderBatch::GetSubLayerEnd(rendering::RenderBatchPassId iPassId)
{
    SG_ASSERT_AND_UNUSED(iPassId == rendering::RenderBatchPassId());
    return 1;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RenderBatch::Execute(rendering::RenderBatchPassId iPassId,
                          rendering::RenderDevice const* iRenderDevice,
                          rendering::IShaderConstantDatabase const* iShaderConstantDatabase,
                          rendering::IShaderResourceDatabase const* iShaderResourceDatabase,
                          size_t iSubLayer,
                          size_t& ioNextSubLayer)
{
    SG_ASSERT_AND_UNUSED(iPassId == rendering::RenderBatchPassId());
    SG_UNUSED((ioNextSubLayer));
    ID3D11DeviceContext* context = iRenderDevice->ImmediateContext();
    SG_ASSERT(nullptr != context);
    SG_ASSERT_AND_UNUSED(0 == iSubLayer);

    UpdateBuffers(iRenderDevice);

    {
        ID3D11ShaderReflection* reflection = m_descriptor.pixelShader.GetReflection();
        m_psConstantBuffers->UpdateIFN(iRenderDevice, reflection, iShaderConstantDatabase);
        ID3D11Buffer** buffers = m_psConstantBuffers->Buffers();
        UINT bufferCount = (UINT)m_psConstantBuffers->BufferCount();
        context->PSSetConstantBuffers(
            0,
            bufferCount,
            buffers);

        m_psResources->UpdateIFN(reflection, iShaderResourceDatabase);
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
        ID3D11ShaderReflection* reflection = m_descriptor.vertexShader.GetReflection();
        m_vsConstantBuffers->UpdateIFN(iRenderDevice, reflection, iShaderConstantDatabase);
        ID3D11Buffer** buffers = m_vsConstantBuffers->Buffers();
        UINT bufferCount = (UINT)m_vsConstantBuffers->BufferCount();
        context->VSSetConstantBuffers(
            0,
            bufferCount,
            buffers);

        m_vsResources->UpdateIFN(reflection, iShaderResourceDatabase);
        ID3D11ShaderResourceView* srvs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
        size_t srvCount = SG_ARRAYSIZE(srvs);
        m_psResources->GetShaderResourceViews(srvCount, srvs);
        context->VSSetShaderResources( 0, checked_numcastable(srvCount), srvs );

        ID3D11SamplerState* sss[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
        size_t ssCount = SG_ARRAYSIZE(sss);
        m_vsResources->GetSamplers(ssCount, sss);
        context->VSSetSamplers( 0, checked_numcastable(ssCount), sss );
    }

    UINT strides[RenderBatchDescriptor::MAX_INPUT_SLOT_COUNT];
    UINT offsets[RenderBatchDescriptor::MAX_INPUT_SLOT_COUNT];
    ID3D11Buffer* vertexBuffer[RenderBatchDescriptor::MAX_INPUT_SLOT_COUNT];
    for(size_t i = 0; i < m_descriptor.inputSlotCount; ++i)
    {
        strides[i] = m_descriptor.vertexSize[i];
        offsets[i] = 0;
        vertexBuffer[i] = m_vertexBuffers[m_bufferIndex][i].get();
    }
    size_t const vertexBufferCount = m_descriptor.inputSlotCount;
    context->IASetVertexBuffers(
        0,
        checked_numcastable(vertexBufferCount),
        vertexBuffer,
        strides,
        offsets );

    DXGI_FORMAT indexFormat = DXGI_FORMAT_UNKNOWN;
        switch(m_descriptor.indexSize)
    {
        case RenderBatchDescriptor::IndexSize::u16: indexFormat = DXGI_FORMAT_R16_UINT; break;
        case RenderBatchDescriptor::IndexSize::u32: indexFormat = DXGI_FORMAT_R32_UINT; break;
        default:
            SG_ASSERT_NOT_REACHED();
    }
    context->IASetIndexBuffer(
        m_indexBuffer[m_bufferIndex].get(),
        indexFormat,
        0 );

    context->IASetInputLayout( m_descriptor.inputLayout.GetInputLayout() );
    context->VSSetShader( m_descriptor.vertexShader.GetShader(), NULL, 0 );
    context->PSSetShader( m_descriptor.pixelShader.GetShader(), NULL, 0 );

    context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    context->DrawIndexed(checked_numcastable(m_indexCount), 0, 0);

    {
        m_bufferIndex = (m_bufferIndex + 1) % BUFFER_COUNT;
        size_t const inputSlotCount = m_descriptor.inputSlotCount;
        for(size_t i = 0; i < inputSlotCount; ++i)
        {
            m_vertexCount[i] = 0;
        }
        m_indexCount = 0;
    }
}
//=============================================================================
}
}

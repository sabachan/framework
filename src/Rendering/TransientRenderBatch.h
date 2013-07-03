#ifndef Rendering_TransientRenderBatch_H
#define Rendering_TransientRenderBatch_H

#include "RenderBatch.h"
#include <Core/ComPtr.h>
#include <Core/IntTypes.h>
#include <Core/SmartPtr.h>
#include <algorithm>
#include <vector>

struct ID3D11Buffer;

namespace sg {
namespace rendering {
//=============================================================================
class IShaderConstantDatabase;
class IShaderResourceDatabase;
class Material;
class RenderDevice;
class ShaderConstantBuffers;
class ShaderResourceBuffer;
//=============================================================================
// TODO: Rename to TransientRenderBatch
// A transient render batch is used to display a geometry for one frame only.
class TransientRenderBatch : public IRenderBatch
{
public:
    struct Properties
    {
        u8 indexSize;
        u8 vertexSize;

        friend bool operator== (Properties const& a, Properties const& b)
        {
            return a.indexSize == b.indexSize
                && a.vertexSize == b.vertexSize;
        }
    };
    TransientRenderBatch(rendering::RenderDevice const* iRenderDevice, Material const* iMaterial, Properties const& iProperties);
    ~TransientRenderBatch();

    Material const* GetMaterial() const { return m_material.get(); }
    Properties const& GetProperties() const { return m_properties; }
    void ReserveVertexAndIndex(size_t iVertexCount, size_t iIndexCount);

    // NB: WriteAccess can be requested for exact type (which is recommended) or
    // for void type. In that case, it is the client code that is responsible for
    // correctly checking filling of vertex data.
    template<typename VertexType>
    class WriteAccess
    {
        SG_NON_COPYABLE(WriteAccess)
        void* operator new(size_t);
    public:
        WriteAccess(TransientRenderBatch& iBatch, size_t iMaxVertexCount, size_t iMaxIndexCount, size_t iLayer)
            : batch(&iBatch)
            , vertexBuffer(nullptr)
            , indexBuffer(nullptr)
            , layer(iLayer)
            , indexSize(iBatch.m_properties.indexSize)
            , vertexCount(iMaxVertexCount)
            , firstIndex(0)
            , indexCount(0)
#if SG_ENABLE_ASSERT
            , reservedVertexCount(iMaxVertexCount)
            , reservedIndexCount(iMaxIndexCount)
            , maxIndexedVertex(0)
#endif
        {
            size_t const vertexSize = sizeof(VertexType);
            vertexBuffer = static_cast<VertexType*>(batch->GetVertexPointerForWritingImpl(vertexSize, vertexCount));
            indexBuffer = batch->GetIndexPointerForWritingImpl(indexSize, iMaxIndexCount, firstIndex);
        }
        ~WriteAccess()
        {
            SG_ASSERT(0 < vertexCount);
            SG_ASSERT(maxIndexedVertex < vertexCount);
#if SG_ENABLE_ASSERT
            if(sizeof(u16) == indexSize)
            {
                u16 const checkConversion = checked_numcastable(maxIndexedVertex + firstIndex);;
            }
#endif
            batch->FinishWritingVertex(vertexCount);
            batch->FinishWritingIndex(indexCount, layer);
        }
        VertexType* GetVertexPointerForWriting() const { return vertexBuffer; }
        template<size_t assumedIndexSize>
        void PushIndices_AssumeIndexSize(size_t a, size_t b, size_t c)
        {
#if SG_ENABLE_ASSERT
            SG_ASSERT(assumedIndexSize == indexSize);
            SG_ASSERT(indexCount+3 <= reservedIndexCount);
            maxIndexedVertex = std::max(a, maxIndexedVertex);
            maxIndexedVertex = std::max(b, maxIndexedVertex);
            maxIndexedVertex = std::max(c, maxIndexedVertex);
#endif
            switch(assumedIndexSize)
            {
            case sizeof(u16):
                {
                    u16* ib = static_cast<u16*>(indexBuffer) + indexCount;
                    ib[0] = checked_numcastable(a + firstIndex);
                    ib[1] = checked_numcastable(b + firstIndex);
                    ib[2] = checked_numcastable(c + firstIndex);
                    indexCount += 3;
                }
                break;
            case sizeof(u32):
                {
                    u32* ib = static_cast<u32*>(indexBuffer) + indexCount;
                    ib[0] = checked_numcastable(a + firstIndex);
                    ib[1] = checked_numcastable(b + firstIndex);
                    ib[2] = checked_numcastable(c + firstIndex);
                    indexCount += 3;
                }
                break;
            default:
                SG_ASSERT_NOT_REACHED();
            }
            SG_ASSERT(indexCount <= reservedIndexCount);
        }
        void PushIndices(size_t a, size_t b, size_t c)
        {
            if(sizeof(u16) == indexSize)
                PushIndices_AssumeIndexSize<sizeof(u16)>(a,b,c);
            else
            {
                SG_ASSERT(sizeof(u32) == indexSize);
                PushIndices_AssumeIndexSize<sizeof(u32)>(a,b,c);
            }
        }
        template<size_t assumedIndexSize, typename IT>
        void PushIndices_AssumeIndexSize(IT begin, IT end)
        {
#if SG_ENABLE_ASSERT
            SG_ASSERT(assumedIndexSize == indexSize);
            SG_ASSERT(indexCount+(end-begin) <= reservedIndexCount);
#endif
            IT it = begin;
            size_t i = 0;
            switch(assumedIndexSize)
            {
            case sizeof(u16):
                {
                    u16* ib = static_cast<u16*>(indexBuffer) + indexCount;
                    while(it != end)
                    {
                        size_t const index = size_t(*it);
                        SG_CODE_FOR_ASSERT(if(maxIndexedVertex < index) { maxIndexedVertex = index; });
#if defined(COMPILATION_CONFIG_IS_FAST_DEBUG)
                        ib[i] = checked_numcastable(index + firstIndex);
#else // For perf
                        ib[i] = u16(index + firstIndex);
#endif
                        ++i;
                        ++it;
                    }
                }
                break;
            case sizeof(u32):
                {
                    u32* ib = static_cast<u32*>(indexBuffer) + indexCount;
                    while(it != end)
                    {
                        size_t const index = size_t(*it);
                        SG_CODE_FOR_ASSERT(if(maxIndexedVertex < index) { maxIndexedVertex = index; });
#if defined(COMPILATION_CONFIG_IS_FAST_DEBUG)
                        ib[i] = checked_numcastable(index + firstIndex);
#else // For perf
                        ib[i] = u32(index + firstIndex);
#endif
                        ++i;
                        ++it;
                    }
                }
                break;
            default:
                SG_ASSERT_NOT_REACHED();
            }
            indexCount += i;
            SG_ASSERT(indexCount <= reservedIndexCount);
        }
        template<typename IT>
        void PushIndices(IT begin, IT end)
        {
            if(sizeof(u16) == indexSize)
                PushIndices_AssumeIndexSize<sizeof(u16)>(begin, end);
            else
            {
                SG_ASSERT(sizeof(u32) == indexSize);
                PushIndices_AssumeIndexSize<sizeof(u32)>(begin, end);
            }
        }
        void FinishWritingVertex(size_t iVertexCount)
        {
            SG_ASSERT(iVertexCount <= reservedVertexCount);
            SG_ASSERT_MSG(vertexCount == reservedVertexCount, "FinishWritingVertex must be call 0 or 1 time, no more.");
            vertexCount = iVertexCount;
        }
    private:
        safeptr<TransientRenderBatch> batch;
        VertexType* vertexBuffer;
        void* indexBuffer;
        u8 indexSize;
        size_t layer;
        size_t vertexCount;
        size_t firstIndex;
        size_t indexCount;
#if SG_ENABLE_ASSERT
        size_t reservedVertexCount;
        size_t reservedIndexCount;
        size_t maxIndexedVertex;
#endif
    };

    virtual int GetPriority() override;
    virtual void PreExecute(RenderDevice const* iRenderDevice,
                            IShaderConstantDatabase const* iShaderConstantDatabase,
                            IShaderResourceDatabase const* iShaderResourceDatabase) override;
    virtual size_t GetSubLayerCount() override;
    virtual void Execute(RenderDevice const* iRenderDevice,
                         IShaderConstantDatabase const* iShaderConstantDatabase,
                         IShaderResourceDatabase const* iShaderResourceDatabase,
                         size_t iSubLayer,
                         size_t& ioNextSubLayer) override;
    virtual void PostExecute() override;

#if SG_ENABLE_UNIT_TESTS
    static void Test();
#endif
private:
    void* GetVertexPointerForWritingImpl(size_t iSizeofVertex, size_t iMaxVertexCount);
    void* GetIndexPointerForWritingImpl(size_t iSizeofIndex, size_t iMaxIndexCount, size_t& oFirstVertexIndex);
    void FinishWritingVertex(size_t iVertexCount);
    void FinishWritingIndex(size_t iIndexCount, size_t iLayer);
    void UpdateBuffers(rendering::RenderDevice const* iRenderDevice);
    void SortIndexData();
private:
    Properties m_properties;
    refptr<Material const> m_material;
    size_t m_vertexCount;
    size_t m_indexCount;
    std::vector<u8> m_vertexData;
    std::vector<u8> m_indexData;
    struct LayerData
    {
        u32 layer;
        u32 indexCount;
        u32 indexBegin;
    };
    std::vector<LayerData> m_layerData;
    size_t m_layerDataIndex;
    size_t m_layerCount;
    static size_t const BUFFER_COUNT = 2;
    size_t m_bufferIndex;
    // TODO: vertexBuffer can be implemented only by one circular buffer on a ID3D11Buffer
    // (cf. https://developer.nvidia.com/sites/default/files/akamai/gamedev/files/gdc12/Efficient_Buffer_Management_McDonald.pdf).
    size_t m_vertexBuffersCapacity[BUFFER_COUNT];
    comptr<ID3D11Buffer> m_vertexBuffer[BUFFER_COUNT];
    // TODO: double-buffering of index buffer can be removed as it is updated very locally in the frame.
    // It can also be implemented on a circular buffer if more buffering is needed.
    size_t m_indexBufferCapacity[BUFFER_COUNT];
    comptr<ID3D11Buffer> m_indexBuffer[BUFFER_COUNT];
    // Note: These constant buffers can lead to a lock if still in use by GPU.
    scopedptr<ShaderConstantBuffers> m_psConstantBuffers;
    scopedptr<ShaderConstantBuffers> m_vsConstantBuffers;
    scopedptr<ShaderResourceBuffer> m_psResources;
    scopedptr<ShaderResourceBuffer> m_vsResources;
#if SG_ENABLE_ASSERT
    size_t m_reservedVertexCount;
    size_t m_reservedIndexCount;
#endif
};
//=============================================================================
}
}

#endif

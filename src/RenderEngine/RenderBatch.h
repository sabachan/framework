#ifndef RenderEngine_RenderBatch_H
#define RenderEngine_RenderBatch_H

#include <Core/ComPtr.h>
#include <Core/SmartPtr.h>
#include <Rendering/RenderBatch.h>
#include <Rendering/Shader.h>
#include <Rendering/VertexTypes.h>
#include <vector>
#include "CompositingLayer.h"

struct ID3D11BlendState;
struct ID3D11Buffer;

namespace sg {
namespace rendering {
    class IShaderResource;
    class RenderDevice;
    class ShaderConstantBuffers;
    class ShaderConstantDatabase;
    class ShaderResourceBuffer;
    class ShaderResourceDatabase;
    class Surface;
}
}

// TO REVIEW

// WARNING: This class is certainly deprecated until review and refactoring.

namespace sg {
namespace renderengine {
//=============================================================================
class Compositing;
//=============================================================================
struct RenderBatchDescriptor
{
    enum class IndexSize : u8 { u16 = sizeof(u16), u32 = sizeof(u32) };
    enum class UpdateMode : u8 { UpdateSubresource, Map, Immutable };
    enum class Sorting : u8 { NoSort, KeepOrder, FrontToBack, BackToFront };
    static size_t const MAX_INPUT_SLOT_COUNT = 16;

    u8 inputSlotCount;
    IndexSize indexSize;
    UpdateMode indexUpdateMode;
    u8 vertexSize[MAX_INPUT_SLOT_COUNT];
    UpdateMode vertexUpdateMode[MAX_INPUT_SLOT_COUNT];
    Sorting sorting;

    //rendering::Material material;
    rendering::ShaderInputLayoutProxy inputLayout;
    rendering::VertexShaderProxy vertexShader;
    rendering::PixelShaderProxy pixelShader;
    // blend mode
    // z mode

    size_t Hash() const
    {
        size_t h = 0;
        size_t t = 0;
        h ^= inputLayout.Hash();
        t = vertexShader.Hash();
        h ^= (t << 5) ^ (t >> (sizeof(size_t)*8-5));
        t = pixelShader.Hash();
        h ^= (t << 13) ^ (t >> (sizeof(size_t)*8-13));
        return h;
    }
    RenderBatchDescriptor()
        : inputSlotCount(0)
        , indexSize(IndexSize::u16)
        , inputLayout()
        , vertexShader()
        , pixelShader()
        , indexUpdateMode(UpdateMode::UpdateSubresource)
        , sorting(Sorting::NoSort)
    {
        for(size_t i = 0; i < MAX_INPUT_SLOT_COUNT; ++i)
        {
            vertexSize[i] = 0;
            vertexUpdateMode[i] = UpdateMode::UpdateSubresource;
        }
    }
};
inline bool operator==(RenderBatchDescriptor const& a, RenderBatchDescriptor const& b)
{
    if(a.pixelShader != b.pixelShader)
        return false;
    if(a.vertexShader != b.vertexShader)
        return false;
    if(a.inputLayout != b.inputLayout)
        return false;
    return true;
}
inline bool operator!=(RenderBatchDescriptor const& a, RenderBatchDescriptor const& b)
{
    return !(a == b);
}
//=============================================================================
class RenderBatch : public rendering::IRenderBatch
{
public:
    RenderBatch(rendering::RenderDevice const* iRenderDevice, RenderBatchDescriptor const& iDescriptor);
    ~RenderBatch();

    //template<typename VertexType>
    //class WriteAccess
    //{
    //    VertexType* GetVertexPointerForWriting(size_t iInputSlot = 0);
    //    void PushIndices(size_t a, size_t b, size_t c);
    //    void FinishWriting(size_t iVertexCount);
    //};

    template<typename VertexType>
    VertexType* GetVertexPointerForWriting(size_t iMaxVertexCount, size_t iInputSlot = 0)
    {
        return static_cast<VertexType*>(GetVertexPointerForWritingImpl(sizeof(VertexType), iMaxVertexCount, iInputSlot));
    }
    void FinishWritingVertex(size_t iVertexCount, size_t iInputSlot = 0);

    // TODO: Indices must be offseted by first vertex index => a WriteAccess class should provide vertices and indices.
    u16* GetIndex16PointerForWriting(size_t iMaxIndexCount) { return static_cast<u16*>(GetIndexPointerForWritingImpl(sizeof(u16), iMaxIndexCount)); }
    u32* GetIndex32PointerForWriting(size_t iMaxIndexCount) { return static_cast<u32*>(GetIndexPointerForWritingImpl(sizeof(u32), iMaxIndexCount)); }
    void FinishWritingIndex(size_t iIndicesCount);

    virtual int GetPriority(rendering::RenderBatchPassId iPassId) override;
    virtual void PreExecute(rendering::RenderBatchPassId iPassId,
                            rendering::RenderDevice const* iRenderDevice,
                            rendering::IShaderConstantDatabase const* iShaderConstantDatabase,
                            rendering::IShaderResourceDatabase const* iShaderResourceDatabase) override { SG_ASSERT_AND_UNUSED(iPassId == rendering::RenderBatchPassId()); SG_UNUSED((iRenderDevice, iShaderConstantDatabase, iShaderResourceDatabase)); }
    virtual size_t GetSubLayerEnd(rendering::RenderBatchPassId iPassId) override;
    virtual void Execute(rendering::RenderBatchPassId iPassId,
                         rendering::RenderDevice const* iRenderDevice,
                         rendering::IShaderConstantDatabase const* iShaderConstantDatabase,
                         rendering::IShaderResourceDatabase const* iShaderResourceDatabase,
                         size_t iSubLayer,
                         size_t& ioNextSubLayer) override;
    virtual void PostExecute(rendering::RenderBatchPassId iPassId) override { SG_ASSERT_AND_UNUSED(iPassId == rendering::RenderBatchPassId()); }
private:
    void* GetVertexPointerForWritingImpl(size_t iSizeofVertex, size_t iMaxVertexCount, size_t iInputSlot);
    void* GetIndexPointerForWritingImpl(size_t iSizeofIndex, size_t iMaxIndexCount);
    void UpdateBuffers(rendering::RenderDevice const* iRenderDevice);
private:
    RenderBatchDescriptor m_descriptor;
    std::vector<size_t> m_vertexCount;
    size_t m_indexCount;
    // TODO: remove posibility to declare multiple vertex buffers as this is of no use if they have the same lifetime policy...
    std::vector<std::vector<u8> > m_vertexData;
    std::vector<u8> m_indexData;
    size_t m_vertexWriteOffset;
    static size_t const BUFFER_COUNT = 2;
    size_t m_bufferIndex;
    // TODO: create class rendering::RenderBuffer ?
    size_t m_vertexBuffersCapacity[BUFFER_COUNT];
    std::vector<comptr<ID3D11Buffer> > m_vertexBuffers[BUFFER_COUNT];
    size_t m_indexBufferCapacity[BUFFER_COUNT];
    comptr<ID3D11Buffer> m_indexBuffer[BUFFER_COUNT];
    scopedptr<rendering::ShaderConstantBuffers> m_psConstantBuffers;
    scopedptr<rendering::ShaderConstantBuffers> m_vsConstantBuffers;
    scopedptr<rendering::ShaderResourceBuffer> m_psResources;
    scopedptr<rendering::ShaderResourceBuffer> m_vsResources;
#if SG_ENABLE_ASSERT
    std::vector<size_t> m_reservedVertexCount;
    size_t m_reservedIndexCount;
#endif
};
//=============================================================================
class RenderBatchDico
{
public:
    RenderBatch* GetRenderBatch(RenderBatchDescriptor const& iDescriptor);
private:

};
//=============================================================================
}
}
#endif

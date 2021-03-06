#ifndef Core_IndexedBuffer_H
#define Core_IndexedBuffer_H

#include "ArrayView.h"
#include "Assert.h"
#include "Cast.h"
#include "SmartPtr.h"
#include <algorithm>
#include <vector>

namespace sg {
//=============================================================================
template <typename T, typename I = size_t>
class IndexedBuffer : public SafeCountable
{
    typedef T data_type;
    typedef I index_type;
public:
    IndexedBuffer();
    ~IndexedBuffer();
    void Clear();
    ArrayView<data_type> Data() { return AsArrayView(m_data); }
    ArrayView<data_type const> Data() const { return AsArrayView(m_data); }
    ArrayView<index_type> Indices() { return AsArrayView(m_indices); }
    ArrayView<index_type const> Indices() const { return AsArrayView(m_indices); }
    void Reserve(size_t iDataCount, size_t iIndexCount);
    void IncrementalReserve(size_t iDataCount, size_t iIndexCount);
    void PushData(data_type const& iData) { m_data.push_back(iData); }
    void PushIndex(index_type const& iIndex) { m_indices.push_back(iIndex); }

    class WriteAccess
    {
        SG_NON_NEWABLE
        SG_NON_COPYABLE(WriteAccess)
    public:
        WriteAccess(IndexedBuffer& iBuffer, size_t iMaxDataCount, size_t iMaxIndexCount);
        ~WriteAccess();
        data_type* GetDataPointerForWriting() const { return dataBuffer; }
        void PushIndices(index_type i);
        template<typename... Is> void PushIndices(index_type i, Is... iIndices);
        template<typename IT> void PushIndicesRange(IT begin, IT end);
        void FinishWritingData(size_t iDataCount);
    private:
        safeptr<IndexedBuffer> buffer;
        data_type* dataBuffer;
        index_type* indexBuffer;
        size_t dataCount;
        size_t firstIndex;
        size_t indexCount;
#if SG_ENABLE_ASSERT
        size_t reservedDataCount;
        size_t reservedIndexCount;
        size_t maxIndexedData;
#endif
    };

private:
    data_type* GetDataPointerForWritingImpl(size_t iMaxDataCount);
    index_type* GetIndexPointerForWritingImpl(size_t iMaxIndexCount, size_t& oFirstIndex);
    void FinishWritingData(size_t iDataCount);
    void FinishWritingIndex(size_t iIndexCount);
private:
    std::vector<T> m_data;
    std::vector<I> m_indices;
    size_t m_reservedDataCount;
    size_t m_reservedIndexCount;
};
//=============================================================================
template <typename T, typename I>
IndexedBuffer<T, I>::WriteAccess::WriteAccess(IndexedBuffer& iBuffer, size_t iMaxDataCount, size_t iMaxIndexCount)
    : buffer(&iBuffer)
    , dataBuffer(nullptr)
    , indexBuffer(nullptr)
    , dataCount(iMaxDataCount)
    , firstIndex(0)
    , indexCount(0)
#if SG_ENABLE_ASSERT
    , reservedDataCount(iMaxDataCount)
    , reservedIndexCount(iMaxIndexCount)
    , maxIndexedData(0)
#endif
{
    dataBuffer = buffer->GetDataPointerForWritingImpl(dataCount);
    indexBuffer = buffer->GetIndexPointerForWritingImpl(iMaxIndexCount, firstIndex);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, typename I>
IndexedBuffer<T, I>::WriteAccess::~WriteAccess()
{
    SG_ASSERT(0 < dataCount);
    SG_ASSERT(maxIndexedData < dataCount);
    buffer->FinishWritingData(dataCount);
    buffer->FinishWritingIndex(indexCount);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, typename I>
void IndexedBuffer<T, I>::WriteAccess::PushIndices(index_type i)
{
    SG_ASSERT(indexCount < reservedIndexCount);
    SG_CODE_FOR_ASSERT(maxIndexedData = std::max(size_t(i), maxIndexedData);)
#if defined(COMPILATION_CONFIG_IS_FAST_DEBUG)
    indexBuffer[indexCount] = checked_numcastable(i + firstIndex);
#else // For perf
    indexBuffer[indexCount] = index_type(i + firstIndex);
#endif
    ++indexCount;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, typename I>
template<typename... Is>
void IndexedBuffer<T, I>::WriteAccess::PushIndices(index_type i, Is... iIndices)
{
    PushIndices(i);
    PushIndices(iIndices...);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, typename I>
template<typename IT>
void IndexedBuffer<T, I>::WriteAccess::PushIndicesRange(IT begin, IT end)
{
    SG_ASSERT(indexCount+(end-begin) <= reservedIndexCount);
    IT it = begin;
    size_t i = 0;
    index_type* ib = indexBuffer + indexCount;
    while(it != end)
    {
        auto const index = *it;
        SG_CODE_FOR_ASSERT(maxIndexedData = std::max(size_t(index), maxIndexedData);)
#if defined(COMPILATION_CONFIG_IS_FAST_DEBUG)
            ib[i] = checked_numcastable(index + firstIndex);
#else // For perf
            ib[i] = index_type(index + firstIndex);
#endif
        ++i;
        ++it;
    }
    indexCount += i;
    SG_ASSERT(indexCount <= reservedIndexCount);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, typename I>
void IndexedBuffer<T, I>::WriteAccess::FinishWritingData(size_t iDataCount)
{
    SG_ASSERT(iDataCount <= reservedDataCount);
    SG_ASSERT_MSG(dataCount == reservedDataCount, "FinishWritingData must be call 0 or 1 time, no more.");
    dataCount = iDataCount;
}
//=============================================================================
template <typename T, typename I>
IndexedBuffer<T, I>::IndexedBuffer()
: m_data()
, m_indices()
, m_reservedDataCount(0)
, m_reservedIndexCount(0)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, typename I>
IndexedBuffer<T, I>::~IndexedBuffer()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, typename I>
void IndexedBuffer<T, I>::Clear()
{
    m_data.clear();
    m_indices.clear();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, typename I>
void IndexedBuffer<T, I>::Reserve(size_t iDataCount, size_t iIndexCount)
{
    m_data.reserve(iDataCount);
    m_indices.reserve(iIndexCount);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, typename I>
void IndexedBuffer<T, I>::IncrementalReserve(size_t iDataCount, size_t iIndexCount)
{
    m_data.reserve(m_data.size() + iDataCount);
    m_indices.reserve(m_indices.size() + iIndexCount);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, typename I>
typename IndexedBuffer<T, I>::data_type* IndexedBuffer<T, I>::GetDataPointerForWritingImpl(size_t iMaxDataCount)
{
    SG_ASSERT(0 < iMaxDataCount);
    SG_ASSERT(0 == m_reservedDataCount);
    size_t const dataCount = m_data.size();
    size_t const newDataCount = dataCount + iMaxDataCount;
    m_data.resize(newDataCount);
    m_reservedDataCount = iMaxDataCount;
    return m_data.data() + dataCount;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, typename I>
typename IndexedBuffer<T, I>::index_type* IndexedBuffer<T, I>::GetIndexPointerForWritingImpl(size_t iMaxIndexCount, size_t& oFirstDataIndex)
{
    SG_ASSERT(0 < iMaxIndexCount);
    SG_ASSERT(0 == m_reservedIndexCount);
    SG_ASSERT(0 != m_reservedDataCount);
    size_t const firstDataIndex = m_data.size() - m_reservedDataCount;
    oFirstDataIndex = firstDataIndex;
    size_t const indexCount = m_indices.size();
    size_t const newIndexCount = indexCount + iMaxIndexCount;
    m_indices.resize(newIndexCount);
    m_reservedIndexCount = iMaxIndexCount;
    return m_indices.data() + indexCount;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, typename I>
void IndexedBuffer<T, I>::FinishWritingData(size_t iDataCount)
{
    SG_ASSERT(0 != m_reservedDataCount);
    size_t const dataCapacity = m_data.size();
    size_t const prevDataCount = dataCapacity - m_reservedDataCount;
    size_t const newDataCount = prevDataCount + iDataCount;
    SG_ASSERT(newDataCount <= dataCapacity);
    m_data.resize(newDataCount);
    m_reservedDataCount = 0;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, typename I>
void IndexedBuffer<T, I>::FinishWritingIndex(size_t iIndexCount)
{
    SG_ASSERT(0 != m_reservedIndexCount);
    size_t const indexCapacity = m_indices.size();
    size_t const prevIndexCount = indexCapacity - m_reservedIndexCount;
    size_t const newIndexCount = prevIndexCount + iIndexCount;
    SG_ASSERT(newIndexCount <= indexCapacity);
    m_indices.resize(newIndexCount);
    m_reservedIndexCount = 0;
}
//=============================================================================
}

#endif

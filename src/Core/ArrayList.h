#ifndef Core_ArrayList_H
#define Core_ArrayList_H

#include "Allocators.h"
#include "ArrayView.h"
#include "Assert.h"
#include "Config.h"
#include "For.h"
#include "TemplateUtils.h"

namespace sg {
//=============================================================================
namespace internal {
template<typename T, typename S>
class ArrayListBase : public internal::ArrayBase<T, true, S>
{
    typedef S size_type;
public:
    size_t Capacity() const { return m_capacity; }
    T& PushBack_AssumeCapacity(T const& iVal);
    template<typename... Args> T& EmplaceBack_AssumeCapacity(Args&&... iArgs);
    //void Resize_AssumeCapacity(size_t iSize);
    template <typename... Args> void Resize_AssumeCapacity(size_t iSize, Args&&... iArgs);
    void PopBack();
    void SizeDown(size_t iSize);
    void Clear();
#if SG_CONTAINERS_EXPOSE_STD_NAMES
    void pop_back() { return PopBack(); }
    void clear() { return Clear(); }
#endif
protected:
    ArrayListBase(T* data, size_type size, size_t capacity) : ArrayBase(data, size), m_capacity(capacity) {}
protected:
    size_t m_capacity;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename S>
T& ArrayListBase<T, S>::PushBack_AssumeCapacity(T const& iVal)
{
    size_t const index = m_size;
    SG_ASSERT(index < m_capacity);
    new(m_data + index) T(iVal);
    m_size = index+1;
    return m_data[index];
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename S>
template<typename... Args>
T& ArrayListBase<T, S>::EmplaceBack_AssumeCapacity(Args&&... iArgs)
{
    size_t const index = m_size;
    SG_ASSERT(index < m_capacity);
    new(m_data + index) T(std::forward<Args>(iArgs)...);
    m_size = index+1;
    return m_data[index];
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename S>
template<typename... Args>
void ArrayListBase<T, S>::Resize_AssumeCapacity(size_t iSize, Args&&... iArgs)
{
    SG_ASSERT(iSize <= m_capacity);
    size_t const size = m_size;
    if(size > iSize)
        SizeDown(iSize);
    else if(size < iSize)
    {
        T* data = m_data;
        for_range(size_t, i, size, iSize)
            new(data + i) T(std::forward<Args>(iArgs)...);
        m_data = data;
        m_size = iSize;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename S>
void ArrayListBase<T, S>::PopBack()
{
    size_t const size = m_size;
    SG_ASSERT(0 < size);
    size_t const index = size - 1;
    m_data[index].~T();
    m_size = index;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename S>
void ArrayListBase<T, S>::SizeDown(size_t iSize)
{
    size_t const size = m_size;
    SG_ASSERT(iSize <= size);
    size_t const count = size - iSize;
    for(T& it : AsArrayView(m_data + iSize, count))
        it.~T();
    m_size = iSize;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename S>
void ArrayListBase<T, S>::Clear()
{
    for(T& it : AsArrayView(m_data, m_size))
    {
        SG_UNUSED(it); // VC2015 rises a warning when T is trivially default destructible
        it.~T();
    }
    m_size = 0;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename A>
class ArrayListImpl : public ArrayListBase<T, size_t>
{
    typedef internal::ArrayListBase<T, size_t> parent_type;
    typedef typename ConstPassing<value_type>::type value_type_for_const_passing;
    typedef typename A::template Rebind<T>::type allocator_type;
public:
    ArrayListImpl();
    template <typename... Args> ArrayListImpl(size_t iCount, Args&&... iArgs);
    ArrayListImpl(ArrayListImpl const& other);
    ArrayListImpl(ArrayListImpl&& other);
    ArrayListImpl& operator=(ArrayListImpl const& other);
    ArrayListImpl& operator=(ArrayListImpl&& other);
    ~ArrayListImpl() { Clear(); m_allocator.Deallocate(m_data, m_capacity); }

    void Reserve(size_t iCapacity);

    T& PushBack(T const& iVal) { Reserve(m_size + 1); return PushBack_AssumeCapacity(iVal); }
    template<typename... Args> T& EmplaceBack(Args&&... iArgs) { Reserve(m_size + 1); return EmplaceBack_AssumeCapacity(std::forward<Args>(iArgs)...); }

    template <typename... Args> void Resize(size_t iSize, Args&&... iArgs) { Reserve(iSize); Resize_AssumeCapacity(iSize, std::forward<Args>(iArgs)...); }

#if SG_CONTAINERS_EXPOSE_STD_NAMES
    void reserve(size_t iCapacity) { return Reserve(iCapacity); }
    void push_back(T const& iVal) { PushBack(iVal); }
    template<typename... Args> void emplace_back(Args&&... iArgs) { EmplaceBack(iArgs...); }
    template <typename... Args> void resize(size_t iSize, Args&&... iArgs) { Resize(iSize, iArgs...); }
#endif
private:
    void ChangeCapacity(size_t iCapacity);
private:
    allocator_type m_allocator;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename A>
ArrayListImpl<T, A>::ArrayListImpl()
    : parent_type(nullptr, 0, 0), m_allocator()
{
    // In order to get capacity for fix size allocator:
    Reserve(0);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename A>
template <typename... Args>
ArrayListImpl<T, A>::ArrayListImpl(size_t iCount, Args&&... iArgs)
    : ArrayListImpl()
{
    Resize(iCount, iArgs...);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename A>
ArrayListImpl<T, A>::ArrayListImpl(ArrayListImpl const& other)
    : ArrayListImpl()
{
    *this = other;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename A>
ArrayListImpl<T, A>& ArrayListImpl<T, A>::operator=(ArrayListImpl const& other)
{
    size_t const size = other.m_size;
    Reserve(size);
    T* data = m_data;
    T const* src = other.m_data;
    for_range(size_t, i, 0, size)
        new(data + i) T(src[i]);
    m_size = size;
    return *this;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename A>
ArrayListImpl<T, A>::ArrayListImpl(ArrayListImpl&& other)
    : ArrayListImpl()
{
    *this = std::move(other);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename A>
ArrayListImpl<T, A>& ArrayListImpl<T, A>::operator=(ArrayListImpl&& other)
{
    bool const isSwappable = allocator_type::is_always_equal
        || m_allocator == other.m_allocator
        || allocator_type::AreBuffersSwappable(m_allocator, m_data, other.m_allocator, other.m_data);

    if(SG_POTENTIAL_CONSTANT_CONDITION(isSwappable))
    {
        std::swap(m_data, other.m_data);
        std::swap(m_size, other.m_size);
        std::swap(m_capacity, other.m_capacity);
    }
    else
    {
        size_t const size = other.m_size;
        Clear();
        Reserve(size);
        T* data = m_data;
        T* src = other.m_data;
        for_range(size_t, i, 0, size)
            new(data + i) T(std::move(src[i]));
        m_size = size;
    }
    return *this;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename A>
void ArrayListImpl<T, A>::Reserve(size_t iCapacity)
{
    size_t const prevCapacity = m_capacity;
    if(prevCapacity < iCapacity)
        ChangeCapacity(iCapacity);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename A>
void ArrayListImpl<T, A>::ChangeCapacity(size_t iCapacity)
{
    size_t newCapacity;
    void* newBuffer = m_allocator.AllocateAtLeast(iCapacity, newCapacity);
    T* newData = reinterpret_cast<T*>(newBuffer);
    T* data = m_data;
    size_t const size = m_size;
    if(0 != size)
    {
        Relocate_AssumeDisjoint(newData, data, size);
    }
    m_allocator.Deallocate(data, m_capacity);
    m_data = newData;
    m_size = size;
    m_capacity = newCapacity;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
template<typename T>
class MaxSizeArrayListView : public internal::ArrayListBase<T, size_t&>
{
public:
    MaxSizeArrayListView(T* iBuffer, size_t& ioSize, size_t iCapacity) : ArrayListBase(iBuffer, ioSize, iCapacity) {}
    MaxSizeArrayListView(MaxSizeArrayListView const&) = default;
    MaxSizeArrayListView operator=(MaxSizeArrayListView const&) = delete; // impossible due to size as reference
};
//=============================================================================
template<typename T, typename A = StandardAllocator<void>, bool IsCopyable = std::is_copy_constructible<T>::value >
class ArrayList : public internal::ArrayListImpl<T, A>
{
public:
    ArrayList() {}
    template <typename... Args> ArrayList(size_t iCount, Args&&... iArgs) : ArrayListImpl(iCount, iArgs...) {}
    ArrayList(ArrayList const& other) = default;
    ArrayList& operator=(ArrayList const& other) = default;
    ArrayList(ArrayList&& other) : ArrayListImpl(std::move(other)) { }
    ArrayList& operator=(ArrayList&& other) { *static_cast<ArrayListImpl*>(this) = std::move(other); return *this; }
};
//=============================================================================
template<typename T, typename A>
class ArrayList<T, A, false> : public internal::ArrayListImpl<T, A>
{
public:
    ArrayList() {}
    template <typename... Args> ArrayList(size_t iCount, Args&&... iArgs) : ArrayListImpl(iCount, iArgs...) {}
    ArrayList(ArrayList const& other) = delete;
    ArrayList& operator=(ArrayList const& other) = delete;
    ArrayList(ArrayList&& other) : ArrayListImpl(std::move(other)) { }
    ArrayList& operator=(ArrayList&& other) { *static_cast<ArrayListImpl*>(this) = std::move(other); return *this; }
};
//=============================================================================
}

#endif

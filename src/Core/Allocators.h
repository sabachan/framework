#ifndef Core_Allocators_H
#define Core_Allocators_H

#include "Assert.h"
#include "For.h"
#include "Preprocessor.h"
#include <type_traits>
#include <utility>


namespace sg {
//=============================================================================
#define DECLARE_REBIND(TYPE) \
    template <typename U> \
    struct Rebind \
    { \
        /*static_assert(std::is_same<T, U>::value || std::is_same<T, void>::value,*/ \
            /*"One should use correct typed allocator or use void alocator");*/ \
        typedef STRIP_PARENTHESIS(TYPE) type; \
    };
//=============================================================================
template <typename T = void>
struct StandardAllocator
{
    static_assert(!std::is_const<T>::value, "Containers of const elements are forbidden.");
    typedef T value_type;
    static bool const is_always_equal = true;
    friend bool operator == (StandardAllocator const&, StandardAllocator const&) { return true; }
    friend bool operator != (StandardAllocator const&, StandardAllocator const&) { return false; }
    static bool AreBuffersSwappable(StandardAllocator const&, void const*, StandardAllocator const&, void const*) { return true; }

    void* AllocateAtLeast(size_t const iSizeInElements, size_t& oAllocatedSizeInElements)
    {
        oAllocatedSizeInElements = iSizeInElements;
#ifdef SG_ENABLE_ASSERT
        void* buffer = malloc((oAllocatedSizeInElements + 2) * sizeof(T));
        u8* p = reinterpret_cast<u8*>(buffer);
        for_range(int, i, 0, sizeof(T))
            p[i] = 0xAB;
        p = p + (oAllocatedSizeInElements + 1) * sizeof(T);
        for_range(int, i, 0, sizeof(T))
            p[i] = 0xAB;
        return reinterpret_cast<u8*>(buffer) + sizeof(T);
#else
        void* buffer = malloc(iSizeInElements * sizeof(T));
        return buffer;
#endif
    }
    void Deallocate(void* iBuffer, size_t const iAllocatedSizeInElements)
    {
#ifdef SG_ENABLE_ASSERT
        SG_ASSERT((nullptr == iBuffer) == (0 == iAllocatedSizeInElements));
        if(nullptr != iBuffer)
        {
            u8* p = reinterpret_cast<u8*>(iBuffer);
            p = p - sizeof(T);
            void* buffer = p;
            for_range(int, i, 0, sizeof(T))
                SG_ASSERT(0xAB == p[i]);
            p = p + (iAllocatedSizeInElements + 1) * sizeof(T);
            for_range(int, i, 0, sizeof(T))
                SG_ASSERT(0xAB == p[i]);
            free(buffer);
        }
        else
        {
            free(iBuffer);
        }
#else
        free(iBuffer);
#endif
    }
};
template<>
struct StandardAllocator<void>
{
    typedef void value_type;
    DECLARE_REBIND(StandardAllocator<U>)
};
//=============================================================================
template <size_t Size, class ForwardingAllocator = StandardAllocator<>, typename T = void>
struct InPlaceAllocator
{
    static_assert(!std::is_const<T>::value, "Containers of const elements are forbidden.");
    typedef T value_type;
    static bool const is_always_equal = false;
    friend bool operator == (InPlaceAllocator const& a, InPlaceAllocator const& b) { return &a == &b; }
    friend bool operator != (InPlaceAllocator const& a, InPlaceAllocator const& b) { return &a != &b; }

    static bool AreBuffersSwappable(InPlaceAllocator const& a, void const* abuffer, InPlaceAllocator const& b, void const* bbuffer)
    {
        return abuffer != reinterpret_cast<void const*>(a.m_buffer) && bbuffer != reinterpret_cast<void const*>(b.m_buffer);
    }
    void* AllocateAtLeast(size_t const iSizeInElements, size_t& oAllocatedSizeInElements)
    {
        if(iSizeInElements <= Size && !m_isBufferUsed)
        {
            oAllocatedSizeInElements = Size;
            m_isBufferUsed = true;
            return reinterpret_cast<void*>(m_buffer);
        }
        else
        {
            return m_forwardingallocator.AllocateAtLeast(iSizeInElements, oAllocatedSizeInElements);
        }
    }
    void Deallocate(void* iBuffer, size_t const iAllocatedSizeInElements)
    {
        if(iBuffer == reinterpret_cast<void*>(m_buffer))
        {
            SG_ASSERT(m_isBufferUsed);
            SG_ASSERT(Size == iAllocatedSizeInElements);
            m_isBufferUsed = false;
        }
        else
        {
            SG_ASSERT(reinterpret_cast<u8 const*>(iBuffer) - m_buffer < 0 || reinterpret_cast<u8 const*>(iBuffer) - m_buffer > Size * sizeof(T));
            m_forwardingallocator.Deallocate(iBuffer, iAllocatedSizeInElements);
        }
    }

private:
    alignas(T) u8 m_buffer[Size * sizeof(T)];
    bool m_isBufferUsed {false};
    typename ForwardingAllocator m_forwardingallocator;
};
template <size_t Size, class ForwardingAllocator>
struct InPlaceAllocator<Size, ForwardingAllocator, void>
{
    typedef void value_type;
    DECLARE_REBIND((InPlaceAllocator<Size, typename ForwardingAllocator::template Rebind<U>::type, U>))
};
//=============================================================================
template <typename T = void>
struct NullAllocator
{
    static_assert(!std::is_const<T>::value, "Containers of const elements are forbidden.");
    typedef T value_type;
    static bool const is_always_equal = true;
    friend bool operator == (NullAllocator const&, NullAllocator const&) { return true; }
    friend bool operator != (NullAllocator const&, NullAllocator const&) { return false; }
    static bool AreBuffersSwappable(NullAllocator const&, void const* abuffer, NullAllocator const&, void const* bbuffer) { SG_ASSERT(nullptr == abuffer); SG_ASSERT(nullptr == bbuffer); return true; }

    void* AllocateAtLeast(size_t const iSizeInElements, size_t& oAllocatedSizeInElements)
    {
        SG_ASSERT_AND_UNUSED(0 == iSizeInElements);
        oAllocatedSizeInElements = 0;
        return nullptr;
    }
    void Deallocate(void* iBuffer, size_t const iAllocatedSizeInElements)
    {
        SG_ASSERT_AND_UNUSED(nullptr == iBuffer);
        SG_ASSERT_AND_UNUSED(0 == iAllocatedSizeInElements);
    }
};
template <>
struct NullAllocator<void>
{
    typedef void value_type;
    DECLARE_REBIND(NullAllocator<U>)
};
//=============================================================================
#undef DECLARE_REBIND
//=============================================================================
namespace relocate_internal {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
namespace is_swappable_impl {
struct not_swappable_t {};
template <typename U, typename V>
not_swappable_t swap(U&, V&);
template <typename T>
struct IsDedicatedlySwappable
{
    static const bool value = !std::is_same<decltype(swap(std::declval<T&>(), std::declval<T&>())), not_swappable_t>::value;
};
}
template <typename T>
struct IsDedicatedlySwappable : public is_swappable_impl::IsDedicatedlySwappable<T> {};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
struct RelocationPolicy
{
    static const bool use_memcpy = std::is_trivially_copyable<T>::value;
    // TODO: how to know if a move constructor has been implemented?
    static const bool use_move_ctor = !use_memcpy && std::is_move_constructible<T>::value && !std::is_copy_constructible<T>::value;
    static const bool use_swap = !use_memcpy && !use_move_ctor && std::is_default_constructible<T>::value && IsDedicatedlySwappable<T>::value;
    static const bool use_ctor = !use_memcpy && !use_move_ctor && !use_swap;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
typename std::enable_if<RelocationPolicy<T>::use_memcpy, void>::type Relocate_Forward(T* SG_RESTRICT dst, T* SG_RESTRICT src, size_t count)
{
    static_assert(std::is_trivially_destructible<T>::value, "");
    std::memmove(dst, src, count * sizeof(T));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
typename std::enable_if<RelocationPolicy<T>::use_move_ctor, void>::type Relocate_Forward(T* SG_RESTRICT dst, T* SG_RESTRICT src, size_t count)
{
    for_range(size_t, i, 0, count)
    {
        T& srci = src[i];
        new(dst + i) T(std::move(srci));
        srci.~T();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
typename std::enable_if<RelocationPolicy<T>::use_swap, void>::type Relocate_Forward(T* SG_RESTRICT dst, T* SG_RESTRICT src, size_t count)
{
    for_range(size_t, i, 0, count)
    {
        new(dst + i) T();
        T& srci = src[i];
        using std::swap;
        swap(dst[i], srci);
        srci.~T();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
typename std::enable_if<RelocationPolicy<T>::use_ctor, void>::type Relocate_Forward(T* SG_RESTRICT dst, T* SG_RESTRICT src, size_t count)
{
    for_range(size_t, i, 0, count)
    {
        T const& srci = src[i];
        new(dst + i) T(srci);
        srci.~T();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
typename std::enable_if<RelocationPolicy<T>::use_memcpy, void>::type Relocate_Backward(T* SG_RESTRICT dst, T* SG_RESTRICT src, size_t count)
{
    static_assert(std::is_trivially_destructible<T>::value, "");
    std::memmove(dst, src, count * sizeof(T));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
typename std::enable_if<RelocationPolicy<T>::use_move_ctor, void>::type Relocate_Backward(T* SG_RESTRICT dst, T* SG_RESTRICT src, size_t count)
{
    reverse_for_range(size_t, i, 0, count)
    {
        T& srci = src[i];
        new(dst + i) T(std::move(srci));
        srci.~T();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
typename std::enable_if<RelocationPolicy<T>::use_swap, void>::type Relocate_Backward(T* SG_RESTRICT dst, T* SG_RESTRICT src, size_t count)
{
    reverse_for_range(size_t, i, 0, count)
    {
        new(dst + i) T();
        T& srci = src[i];
        using std::swap;
        swap(dst[i], srci);
        srci.~T();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
typename std::enable_if<RelocationPolicy<T>::use_ctor, void>::type Relocate_Backward(T* SG_RESTRICT dst, T* SG_RESTRICT src, size_t count)
{
    reverse_for_range(size_t, i, 0, count)
    {
        T const& srci = src[i];
        new(dst + i) T(srci);
        srci.~T();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
typename std::enable_if<RelocationPolicy<T>::use_memcpy, void>::type Relocate_AssumeDisjoint(T* SG_RESTRICT dst, T* SG_RESTRICT src, size_t count)
{
    static_assert(std::is_trivially_destructible<T>::value, "");
    std::memcpy(dst, src, count * sizeof(T));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
typename std::enable_if<!RelocationPolicy<T>::use_memcpy, void>::type Relocate_AssumeDisjoint(T* SG_RESTRICT dst, T* SG_RESTRICT src, size_t count)
{
    relocate_internal::Relocate_Forward(dst, src, count);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
// Relocate methods are used to conceptually move objects from one buffer to
// another.
// Such method must be called with src containing count object fully
// initialized and dst an uninitialized buffer. At the end, src become an
// uninitialized buffer and dst contains count copies of the input objects.
//
// Note that these methods do not support exceptions.
template <typename T>
void Relocate_AssumeDisjoint(T* dst, T* src, size_t count)
{
    SG_ASSERT(0 <= src - (dst+count) || 0 <= dst - (src+count));
    relocate_internal::Relocate_AssumeDisjoint(dst, src, count);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
void Relocate_Forward(T* dst, T* src, size_t count)
{
    SG_ASSERT(0 < src - dst || 0 <= dst - (src+count));
    relocate_internal::Relocate_Forward(dst, src, count);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
void Relocate_Backward(T* dst, T* src, size_t count)
{
    SG_ASSERT(0 <= src - (dst+count) || 0 < dst - src);
    relocate_internal::Relocate_Backward(dst, src, count);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
void Relocate(T* dst, T* src, size_t count)
{
    if(dst = src)
        return;
    if(0 < src - (dst + count))
        return Relocate_AssumeDisjoint(dst, src, count);
    if(0 < dst - (src + count))
        return Relocate_AssumeDisjoint(dst, src, count);
    if(0 < src - dst)
        return Relocate_Forward(dst, src, count);
    if(0 < dst - src)
        return Relocate_Backward(dst, src, count);
}
//=============================================================================

}

#endif

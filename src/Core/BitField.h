#ifndef Core_BitField_H
#define Core_BitField_H

#include "Assert.h"
#include "For.h"
#include "IntTypes.h"

// This class is an experiment to evaluate the alternatives to C++ bitfields,
// as they are/were considered suboptimal by some developpers
// (cf. https://www.youtube.com/watch?v=MvFj8qo1iuA).
// After some measures, however, it seems that it is equivalent to their C++
// counterparts and that the cost in Debug is huge due to functions that are
// not inlined. However, it may still be of use for the special functions
// without shift or the one that assume values in range.

namespace sg {
//=============================================================================
template <size_t... Ns>
class BitField
{
private:
    template <size_t N, size_t... Ns> struct Sum;
    template <size_t N> struct Sum<N> { enum { value = N }; };
    template <size_t N, size_t... Ns> struct Sum { enum { value = (N + Sum<Ns...>::value) }; };
    template <size_t S> struct Store;
    template <> struct Store<8>  { typedef u8  type; };
    template <> struct Store<16> { typedef u16 type; };
    template <> struct Store<32> { typedef u32 type; };
    template <> struct Store<64> { typedef u64 type; };
public:
    typedef typename Store<Sum<Ns...>::value>::type store_t;
private:
    template <size_t I, size_t S, size_t N, size_t... Ns> struct ItemTraitsImpl;
    template <size_t S, size_t N, size_t... Ns> struct ItemTraitsImpl<0, S, N, Ns...>
    {
        enum {
            lo_bit = S,
            bit_count = N,
        };
        static store_t constexpr value_mask = store_t((store_t(1) << bit_count) - 1);
        static store_t constexpr item_mask = store_t(value_mask << lo_bit);
        static_assert(bit_count < sizeof(size_t) * 8, "Why so many bits?");
    };
    template <size_t I, size_t S, size_t N, size_t... Ns> struct ItemTraitsImpl : public ItemTraitsImpl<I-1, S+N, Ns...> {};
public:
    template <size_t I> struct ItemTraits : public ItemTraitsImpl<I, 0, Ns...> { static_assert(I < sizeof...(Ns), "Invalid bitfield access"); };
    template <size_t I, size_t N>
    struct UnionMember
    {
        static_assert(N == BitField::ItemTraits<I>::bit_count, "N should match item bit count in BitField declaration");
        SG_FORCE_INLINE operator size_t() const { return reinterpret_cast<BitField const*>(this)->Get<I>(); }
        SG_FORCE_INLINE UnionMember& operator= (size_t v) { reinterpret_cast<BitField*>(this)->Set<I>(v); return *this; }

        struct Unsafe {
            SG_FORCE_INLINE Unsafe& operator= (size_t v) { reinterpret_cast<BitField*>(this)->Set_AssumeInRange<I>(v); return *this; }
        };
        Unsafe unsafe;
    };
    template <size_t I>
    struct UnionMember<I, 1>
    {
        static_assert(1 == BitField::ItemTraits<I>::bit_count, "N should match item bit count in BitField declaration");
        SG_FORCE_INLINE operator size_t() const { return reinterpret_cast<BitField const*>(this)->Get<I>(); }
        SG_FORCE_INLINE operator bool() const { return reinterpret_cast<BitField const*>(this)->Get<I>() == 0 ? false : true; }
        SG_FORCE_INLINE UnionMember& operator= (size_t v) { reinterpret_cast<BitField*>(this)->Set<I>(v); return *this; }
        SG_FORCE_INLINE UnionMember& operator= (bool v) { reinterpret_cast<BitField*>(this)->SetFlag<I>(v); return *this; }

        struct Unsafe {
            SG_FORCE_INLINE Unsafe& operator= (size_t v) { reinterpret_cast<BitField*>(this)->Set_AssumeInRange<I>(v); return *this; }
            SG_FORCE_INLINE UnionMember& operator= (bool v) { reinterpret_cast<BitField*>(this)->SetFlag<I>(v); return *this; }
        };
        Unsafe unsafe;
    };
public:
    SG_FORCE_INLINE BitField() : m_store() {}
    SG_FORCE_INLINE BitField(uninitialized_t) {}
    SG_FORCE_INLINE void Clear() { m_store = 0; }
    template <size_t I> size_t Get() const;
    template <size_t I> void Set(size_t v);
    template <size_t I> void Set_AssumeInRange(size_t v);
    template <size_t I> typename std::enable_if<1 == ItemTraits<I>::bit_count, void>::type SetFlag(bool v);
    template <size_t I> typename std::enable_if<1 == ItemTraits<I>::bit_count, void>::type SetFlag();
    template <size_t I> typename std::enable_if<1 == ItemTraits<I>::bit_count, void>::type UnsetFlag();
    template <size_t I> store_t GetNoSifht() const;
    template <size_t I> void SetNoSifht(store_t v);
    template <size_t I> void SetNoSifht_AssumeInRange(store_t v);
    SG_FORCE_INLINE store_t GetStore() const { return m_store; }
    SG_FORCE_INLINE void SetStore(store_t store) { m_store = store; }
private:
    store_t m_store;
};
//=============================================================================
template <size_t... Ns>
template <size_t I> SG_FORCE_INLINE size_t BitField<Ns...>::Get() const
{
    size_t const bit_count = ItemTraits<I>::bit_count;
    size_t const lo_bit = ItemTraits<I>::lo_bit;
    store_t const item_mask = ItemTraits<I>::item_mask;
    if(SG_CONSTANT_CONDITION(bit_count == 1))
        return (m_store & item_mask) != 0 ? 1 : 0;
#if 0
    return size_t((m_store & item_mask) >> lo_bit);
#else
    // This solution seems faster on Visual 2015
    store_t const value_mask = ItemTraits<I>::value_mask;
    return size_t((m_store >> lo_bit) & value_mask);
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t... Ns>
template <size_t I> SG_FORCE_INLINE void BitField<Ns...>::Set(size_t v)
{
    size_t const lo_bit = ItemTraits<I>::lo_bit;
    store_t const item_mask = ItemTraits<I>::item_mask;
#if 0
    m_store = (m_store & ~item_mask) | ((store_t(v) << lo_bit) & item_mask);
#else
    // This solution seems faster on Visual 2015
    store_t const value_mask = ItemTraits<I>::value_mask;
    m_store = (m_store & ~item_mask) | ((store_t(v) & value_mask) << lo_bit);
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t... Ns>
template <size_t I> SG_FORCE_INLINE void BitField<Ns...>::Set_AssumeInRange(size_t v)
{
    size_t const lo_bit = ItemTraits<I>::lo_bit;
    store_t const value_mask = ItemTraits<I>::value_mask;
    SG_ASSERT_MSG((v & ~value_mask) == 0, "Value too big");
    store_t const item_mask = ItemTraits<I>::item_mask;
    m_store = (m_store & ~item_mask) | (store_t(v) << lo_bit);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t... Ns>
template <size_t I>
SG_FORCE_INLINE typename std::enable_if<1 == BitField<Ns...>::ItemTraits<I>::bit_count, void>::type BitField<Ns...>::SetFlag(bool v)
{
    if(v)
        SetFlag<I>();
    else
        UnsetFlag<I>();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t... Ns>
template <size_t I>
SG_FORCE_INLINE typename std::enable_if<1 == BitField<Ns...>::ItemTraits<I>::bit_count, void>::type BitField<Ns...>::SetFlag()
{
    store_t const item_mask = ItemTraits<I>::item_mask;
    m_store |= item_mask;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t... Ns>
template <size_t I>
SG_FORCE_INLINE typename std::enable_if<1 == BitField<Ns...>::ItemTraits<I>::bit_count, void>::type BitField<Ns...>::UnsetFlag()
{
    store_t const item_mask = ItemTraits<I>::item_mask;
    m_store &= ~item_mask;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t... Ns>
template <size_t I> SG_FORCE_INLINE typename BitField<Ns...>::store_t BitField<Ns...>::GetNoSifht() const
{
    store_t const item_mask = ItemTraits<I>::item_mask;
    return m_store & item_mask;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t... Ns>
template <size_t I> SG_FORCE_INLINE void BitField<Ns...>::SetNoSifht(store_t v)
{
    store_t const item_mask = ItemTraits<I>::item_mask;
    m_store |= (v & item_mask);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t... Ns>
template <size_t I> SG_FORCE_INLINE void BitField<Ns...>::SetNoSifht_AssumeInRange(store_t v)
{
    store_t const item_mask = ItemTraits<I>::item_mask;
    SG_ASSERT((v & ~item_mask) == 0);
    m_store |= v;
}
//=============================================================================
}

#endif

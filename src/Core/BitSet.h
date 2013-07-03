#ifndef Core_BitSet_H
#define Core_BitSet_H

#include "Assert.h"
#include "Config.h"
#include "For.h"
#include "IntTypes.h"
#include <type_traits>

#define SG_BIT_SET_GARANTEES_EXCEEDING_BITS_ARE_0 1

namespace sg {
//=============================================================================
template <size_t N>
class BitSet
{
public:
    typedef typename std::conditional<N <= 16, typename std::conditional<N <= 8, u8, u16>::type, typename std::conditional<N <= 32, u32, size_t>::type>::type slot_type;
    static size_t const SLOT_BIT_COUNT = sizeof(slot_type) * 8;
    static size_t const SLOT_COUNT = (N + SLOT_BIT_COUNT - 1) / SLOT_BIT_COUNT;
    static size_t const ADRESS_SHIFT = SLOT_BIT_COUNT == 8 ? 3 :
        SLOT_BIT_COUNT == 16 ? 4 :
        SLOT_BIT_COUNT == 32 ? 5 :
        SLOT_BIT_COUNT == 64 ? 6 : 0;
    static_assert((size_t(1) << ADRESS_SHIFT) == SLOT_BIT_COUNT, "");
    static size_t const ADRESS_MASK = (1 << ADRESS_SHIFT) - 1;
    static size_t const LAST_SLOT_BIT_COUNT = ((N-1) & ADRESS_MASK) + 1;
    static_assert(sizeof(size_t) >= sizeof(u32), "LAST_SLOT_MASK may not fit a size_t");
    static_assert(LAST_SLOT_BIT_COUNT <= 8*sizeof(size_t), "LAST_SLOT_BIT_COUNT cannot exceed a size_t");
    static size_t const LAST_SLOT_MASK = ~size_t(0) >> (8*sizeof(size_t) - LAST_SLOT_BIT_COUNT);
public:
    BitSet() { reset(); }
    BitSet(uninitialized_t) {}
    explicit BitSet(slot_type iData);
    ~BitSet() {}

    BitSet(BitSet const& other) = default;
    BitSet& operator=(BitSet const& other) = default;

    bool operator[] (size_t i) const { SG_ASSERT(i < N); return 0 != (slot_type(1) & (SlotForAdress(i) >> (i & ADRESS_MASK))); }
    void set(size_t i, bool v) { if(v) set(i); else reset(i); }
    void set(size_t i) { SG_ASSERT(i < N); SlotForAdress(i) |=  (slot_type(1) << (i & ADRESS_MASK)); }
    void reset(size_t i) { SG_ASSERT(i < N); SlotForAdress(i) &=  ~(slot_type(1) << (i & ADRESS_MASK)); }
    void reset() { for_range(size_t, i, 0, SLOT_COUNT) { m_data[i] = 0; } }
    void flip(size_t i) { SG_ASSERT(i < N); SlotForAdress(i) ^=  (slot_type(1) << (i & ADRESS_MASK)); }
    void flip();

    size_t count() const;
    bool any() const;
    bool none() const;
    bool all() const;

    size_t size() const { return N; }
private:
    slot_type const& SlotForAdress(size_t i) const { SG_ASSERT(i < N); return SLOT_COUNT == 1 ? m_data[0] : m_data[i >> ADRESS_SHIFT]; }
    slot_type& SlotForAdress(size_t i) { SG_ASSERT(i < N); return SLOT_COUNT == 1 ? m_data[0] : m_data[i >> ADRESS_SHIFT]; }
private:
    slot_type m_data[SLOT_COUNT];
};
//=============================================================================
template <size_t N>
BitSet<N>::BitSet(slot_type iData)
{
    static_assert(1 == SLOT_COUNT, "this cannot be used for multiple slots bitset");
    SG_ASSERT_MSG(0 == (iData & ~LAST_SLOT_MASK), "too many bits are set");
#if SG_BIT_SET_GARANTEES_EXCEEDING_BITS_ARE_0
    m_data[0] = iData & LAST_SLOT_MASK;
#else
    m_data[0] = iData;
#endif
}
//=============================================================================
template <size_t N>
size_t BitSet<N>::count() const
{
    size_t const bit_count[256] = {
        0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 2, 3, 4,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 3, 4, 5,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 3, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 4, 5, 6,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 3, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 4, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 4, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 5, 6, 7,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 3, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 4, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 4, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 5, 6, 7,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 4, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 4, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 5, 6, 7,
        4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 6, 7, 8,
    };
    size_t acc = 0;
    slot_type const* data = m_data;
    size_t const fullSlotCount = N >> ADRESS_SHIFT;
    for_range(size_t, i, 0, fullSlotCount)
    {
        static_assert(SLOT_BIT_COUNT == 8 || SLOT_BIT_COUNT == 16 || SLOT_BIT_COUNT == 32 || SLOT_BIT_COUNT == 64, "");
        slot_type slot = data[i];
        acc += bit_count[slot & 0xFF];
        if(SG_CONSTANT_CONDITION(SLOT_BIT_COUNT >= 16))
        {
            slot >>= 8; acc += bit_count[slot & 0xFF];
        }
        if(SG_CONSTANT_CONDITION(SLOT_BIT_COUNT >= 32))
        {
            slot >>= 8; acc += bit_count[slot & 0xFF];
            slot >>= 8; acc += bit_count[slot & 0xFF];
        }
        if(SG_CONSTANT_CONDITION(SLOT_BIT_COUNT >= 64))
        {
            slot >>= 8; acc += bit_count[slot & 0xFF];
            slot >>= 8; acc += bit_count[slot & 0xFF];
            slot >>= 8; acc += bit_count[slot & 0xFF];
            slot >>= 8; acc += bit_count[slot & 0xFF];
        }
    }
    if(SG_CONSTANT_CONDITION((N & ADRESS_MASK) != 0))
    {
        slot_type const lastSlot = data[N >> ADRESS_SHIFT];
        size_t const remainingFullBytes = (N & ADRESS_MASK) >> 3;
        for_range(size_t, i, 0, remainingFullBytes)
        {
            acc += bit_count[(lastSlot >> i * 8) & 0xFF];
        }
        if(SG_CONSTANT_CONDITION((N & 7) != 0))
        {
            u8 const lastByte = u8(lastSlot >> remainingFullBytes * 8);
            u8 const lastByteMask = (1 << (N & 7)) - 1;
#if SG_BIT_SET_GARANTEES_EXCEEDING_BITS_ARE_0
            SG_UNUSED(lastByteMask);
            SG_ASSERT(0 == (lastByte & ~lastByteMask));
            acc += bit_count[lastByte];
#else
            acc += bit_count[dataAsu8[N >> 3] & lastByteMask];
#endif
        }
    }
    return acc;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t N>
bool BitSet<N>::any() const
{
    slot_type acc = 0;
#if SG_BIT_SET_GARANTEES_EXCEEDING_BITS_ARE_0
    SG_ASSERT(0 == (m_data[SLOT_COUNT-1] & ~LAST_SLOT_MASK));
    for_range(size_t, i, 0, SLOT_COUNT) { acc |= m_data[i]; }
#else
    for_range(size_t, i, 0, SLOT_COUNT-1) { acc |= m_data[i]; }
    acc |= m_data[SLOT_COUNT-1] & LAST_SLOT_MASK;
#endif
    return 0 != acc;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t N>
bool BitSet<N>::none() const
{
    slot_type acc = 0;
#if SG_BIT_SET_GARANTEES_EXCEEDING_BITS_ARE_0
    SG_ASSERT(0 == (m_data[SLOT_COUNT-1] & ~LAST_SLOT_MASK));
    for_range(size_t, i, 0, SLOT_COUNT) { acc |= m_data[i]; }
#else
    for_range(size_t, i, 0, SLOT_COUNT-1) { acc |= m_data[i]; }
    acc |= m_data[SLOT_COUNT-1] & LAST_SLOT_MASK;
#endif
    return 0 == acc;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t N>
bool BitSet<N>::all() const
{
    slot_type acc = slot_type(~0);
    for_range(size_t, i, 0, SLOT_COUNT-1) { acc &= m_data[i]; }
    acc &= m_data[SLOT_COUNT-1] | ~LAST_SLOT_MASK;
    return 0 == ~acc;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t N>
void BitSet<N>::flip()
{
#if SG_BIT_SET_GARANTEES_EXCEEDING_BITS_ARE_0
    for_range(size_t, i, 0, SLOT_COUNT-1) { m_data[i] = ~m_data[i]; }
    m_data[SLOT_COUNT-1] ^= LAST_SLOT_MASK;
    SG_ASSERT(0 == (m_data[SLOT_COUNT-1] & ~LAST_SLOT_MASK));
#else
    for_range(size_t, i, 0, SLOT_COUNT) { m_data[i] = ~m_data[i]; }
#endif
}
//=============================================================================
}

#endif

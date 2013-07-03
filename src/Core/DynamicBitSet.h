#ifndef Core_DynamicBitSet_H
#define Core_DynamicBitSet_H

#include "Assert.h"
#include "BitSet.h"
#include "Config.h"
#include "For.h"
#include "IntTypes.h"
#include <type_traits>
#include <vector>

namespace sg {
//=============================================================================
class DynamicBitSet
{
public:
    typedef size_t slot_type;
    static size_t const SLOT_BIT_COUNT = sizeof(slot_type) * 8;
    static size_t const ADRESS_SHIFT = SLOT_BIT_COUNT == 8 ? 3 :
        SLOT_BIT_COUNT == 16 ? 4 :
        SLOT_BIT_COUNT == 32 ? 5 :
        SLOT_BIT_COUNT == 64 ? 6 : 0;
    static_assert(size_t(1 << ADRESS_SHIFT) == SLOT_BIT_COUNT, "");
    static size_t const ADRESS_MASK = (1 << ADRESS_SHIFT) - 1;
    static_assert(sizeof(size_t) >= sizeof(u32), "LAST_SLOT_MASK may not fit a size_t");

public:
    DynamicBitSet(size_t N) : m_data(nullptr) { SetSizeDiscard(N); reset(); }
    ~DynamicBitSet() { delete[] m_data; }

    DynamicBitSet(DynamicBitSet const& other) = default;
    DynamicBitSet& operator=(DynamicBitSet const& other) = default;

    SG_FORCE_INLINE bool operator[] (size_t i) const { SG_ASSERT(i < m_size); return 0 != (slot_type(1) & (SlotForAdress(i) >> (i & ADRESS_MASK))); }
    SG_FORCE_INLINE void set(size_t i, bool v) { if(v) set(i); else reset(i); }
    SG_FORCE_INLINE void set(size_t i) { SG_ASSERT(i < m_size); SlotForAdress(i) |=  (slot_type(1) << (i & ADRESS_MASK)); }
    SG_FORCE_INLINE void reset(size_t i) { SG_ASSERT(i < m_size); SlotForAdress(i) &=  ~(slot_type(1) << (i & ADRESS_MASK)); }
    SG_FORCE_INLINE void reset() { slot_type const* end = m_data+SlotCount(); for(slot_type* pslot = m_data; pslot != end; ++pslot) { *pslot = 0; } }
    void flip(size_t i) { SG_ASSERT(i < m_size); SlotForAdress(i) ^=  (slot_type(1) << (i & ADRESS_MASK)); }
    void flip();

    size_t count() const;
    bool any() const { return !none(); }
    bool none() const;
    bool all() const;

    size_t size() const { return m_size; }
    void SetSizeDiscard(size_t N);
private:
    size_t SlotCount() const { return (m_size + (SLOT_BIT_COUNT - 1)) >> ADRESS_SHIFT; }
    slot_type const& SlotForAdress(size_t i) const { SG_ASSERT(i < m_size); return m_data[i >> ADRESS_SHIFT]; }
    slot_type& SlotForAdress(size_t i) { SG_ASSERT(i < m_size); return m_data[i >> ADRESS_SHIFT]; }
private:
    size_t m_size;
    slot_type* m_data;
};
//=============================================================================
inline bool DynamicBitSet::none() const
{
    size_t const LAST_SLOT_BIT_COUNT = ((m_size-1) & ADRESS_MASK) + 1;
    size_t const LAST_SLOT_MASK = ~size_t(0) >> (8*sizeof(size_t) - LAST_SLOT_BIT_COUNT);
    size_t const SLOT_COUNT = SlotCount();

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
inline bool DynamicBitSet::all() const
{
    size_t const LAST_SLOT_BIT_COUNT = ((m_size-1) & ADRESS_MASK) + 1;
    size_t const LAST_SLOT_MASK = ~size_t(0) >> (8*sizeof(size_t) - LAST_SLOT_BIT_COUNT);
    size_t const SLOT_COUNT = SlotCount();

    slot_type acc = slot_type(~0);
    for_range(size_t, i, 0, SLOT_COUNT-1) { acc &= m_data[i]; }
    acc &= m_data[SLOT_COUNT-1] | ~LAST_SLOT_MASK;
    return 0 == ~acc;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline void DynamicBitSet::flip()
{
    size_t const LAST_SLOT_BIT_COUNT = ((m_size-1) & ADRESS_MASK) + 1;
    size_t const LAST_SLOT_MASK = ~size_t(0) >> (8*sizeof(size_t) - LAST_SLOT_BIT_COUNT);
    size_t const SLOT_COUNT = SlotCount();

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

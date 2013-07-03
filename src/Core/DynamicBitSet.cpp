#include "stdafx.h"

#include "DynamicBitSet.h"

#include "BitSet.h"
#include "For.h"
#include <type_traits>
#include <vector>

namespace sg {
//=============================================================================
void DynamicBitSet::SetSizeDiscard(size_t N)
{
    m_size = N;
    size_t const SLOT_COUNT = SlotCount();
    delete[] m_data;
    m_data = new slot_type[SLOT_COUNT];
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
size_t DynamicBitSet::count() const
{
    size_t bit_count[256] = {
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
    size_t const fullSlotCount = m_size >> ADRESS_SHIFT;
    for_range(size_t, i, 0, fullSlotCount)
    {
        static_assert(SLOT_BIT_COUNT == 32 || SLOT_BIT_COUNT == 64, "");
        slot_type slot = data[i];
        acc += bit_count[slot & 0xFF];
        slot >>= 8; acc += bit_count[slot & 0xFF];
        slot >>= 8; acc += bit_count[slot & 0xFF];
        slot >>= 8; acc += bit_count[slot & 0xFF];
        if(SG_CONSTANT_CONDITION(SLOT_BIT_COUNT >= 64))
        {
            slot >>= 8; acc += bit_count[slot & 0xFF];
            slot >>= 8; acc += bit_count[slot & 0xFF];
            slot >>= 8; acc += bit_count[slot & 0xFF];
            slot >>= 8; acc += bit_count[slot & 0xFF];
        }
    }
    if((m_size & ADRESS_MASK) != 0)
    {
        slot_type const lastSlot = data[m_size >> ADRESS_SHIFT];
        size_t const remainingFullBytes = (m_size & ADRESS_MASK) >> 3;
        for_range(size_t, i, 0, remainingFullBytes)
        {
            acc += bit_count[(lastSlot >> i) & 0xFF];
        }
        if((m_size & 7) != 0)
        {
            u8 const lastByte = u8(lastSlot >> remainingFullBytes);
            u8 const lastByteMask = (1 << (m_size & 7)) - 1;
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
//=============================================================================
}


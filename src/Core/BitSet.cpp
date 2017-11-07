#include "stdafx.h"

#include "BitSet.h"

#if SG_ENABLE_UNIT_TESTS
#include "DynamicBitSet.h"
#include "Log.h"
#include "PerfLog.h"
#include "StringFormat.h"
#include "TestFramework.h"
#include <bitset>

namespace sg {
//=============================================================================
namespace {
template<typename BS>
void TestSieveOfErathosthenes(BS& isPrime)
{
    size_t const N = isPrime.size();
    //SG_LOG_INFO("Test", Format("Sieve of Erathosthenes until %0", N));
    isPrime.reset();
    isPrime.flip();
    isPrime.reset(0);
    isPrime.reset(1);
    for(size_t i = 0; i < N; ++i)
    {
        if(isPrime[i])
        {
            //SG_LOG_INFO("Test", Format("%0", i));
            size_t n = 2 * i;
            while(n < N)
            {
                isPrime.reset(n);
                n += i;
            }
        }
    }
    SG_ASSERT(isPrime[2]);
    SG_ASSERT(!isPrime[255]);
    SG_ASSERT(isPrime[257]);
    SG_ASSERT(isPrime[997]);
    SG_ASSERT(!isPrime[999]);
}
}
//=============================================================================
SG_TEST((sg,core), BitSet, (quick))
{
    static_assert(sizeof(BitSet<1>) == 1, "");
    static_assert(sizeof(BitSet<8>) == 1, "");
    static_assert(sizeof(BitSet<9>) == 2, "");

    static_assert(BitSet<8>::SLOT_BIT_COUNT == 8, "");
    static_assert(BitSet<8>::SLOT_COUNT == 1, "");
    static_assert(BitSet<8>::ADRESS_SHIFT == 3, "");
    static_assert(BitSet<8>::ADRESS_MASK == 0x07, "");
    static_assert(BitSet<8>::LAST_SLOT_BIT_COUNT == 8, "");
    static_assert(BitSet<8>::LAST_SLOT_MASK == 0x00FF, "");

    static_assert(BitSet<16>::SLOT_BIT_COUNT == 16, "");
    static_assert(BitSet<16>::SLOT_COUNT == 1, "");
    static_assert(BitSet<16>::ADRESS_SHIFT == 4, "");
    static_assert(BitSet<16>::ADRESS_MASK == 0x0F, "");
    static_assert(BitSet<16>::LAST_SLOT_BIT_COUNT == 16, "");
    static_assert(BitSet<16>::LAST_SLOT_MASK == 0xFFFF, "");

    static_assert(BitSet<32>::SLOT_BIT_COUNT == 32, "");
    static_assert(BitSet<32>::SLOT_COUNT == 1, "");
    static_assert(BitSet<32>::ADRESS_SHIFT == 5, "");
    static_assert(BitSet<32>::ADRESS_MASK == 0x1F, "");
    static_assert(BitSet<32>::LAST_SLOT_BIT_COUNT == 32, "");
    static_assert(BitSet<32>::LAST_SLOT_MASK == 0xFFFFFFFF, "");

    static_assert(BitSet<96>::LAST_SLOT_BIT_COUNT == 32, "");
    static_assert(BitSet<96>::LAST_SLOT_MASK == 0xFFFFFFFF, "");

    static_assert(BitSet<10>::SLOT_BIT_COUNT == 16, "");
    static_assert(BitSet<10>::SLOT_COUNT == 1, "");
    static_assert(BitSet<10>::ADRESS_SHIFT == 4, "");
    static_assert(BitSet<10>::ADRESS_MASK == 0x0F, "");
    static_assert(BitSet<10>::LAST_SLOT_BIT_COUNT == 10, "");
    static_assert(BitSet<10>::LAST_SLOT_MASK == 0x03FF, "");
    {
        BitSet<10> v;
        v.set(0);
        v.reset(1);
        v.set(2, false);
        v.set(3, true);
        v.set(4);
        v.set(5);
        v.reset(6);
        v.set(7);
        v.set(9);
        bool b;
        b = v[0];
        SG_ASSERT(b);
        b = v[1];
        SG_ASSERT(!b);
        b = v[2];
        SG_ASSERT(!b);
        b = v[3];
        SG_ASSERT(b);
        b = v[8];
        SG_ASSERT(!b);
        b = v.none();
        SG_ASSERT(!b);
        b = v.any();
        SG_ASSERT(b);
        b = v.all();
        SG_ASSERT(!b);
        v.reset();
        b = v.none();
        SG_ASSERT(b);
        b = v.any();
        SG_ASSERT(!b);
        v.set(2, 1);
        b = v.none();
        SG_ASSERT(!b);
        b = v.any();
        SG_ASSERT(b);
        v.set(2, 0);
        b = v.none();
        SG_ASSERT(b);
        v.set(8);
        b = v.any();
        SG_ASSERT(b);
    }
    {
        BitSet<100> v;
        SG_ASSERT(!v.any());
        size_t count = v.count();
        SG_ASSERT(0 == count);
        v.flip();
        SG_ASSERT(v.all());
        count = v.count();
        SG_ASSERT(v.size() == count);
        v.reset(3);
        v.flip();
        count = v.count();
        SG_ASSERT(1 == count);
        SG_ASSERT(v.any());
        v.set(67);
        count = v.count();
        SG_ASSERT(2 == count);
        bool b;
        b = v[0];
        SG_ASSERT(!b);
        b = v[3];
        SG_ASSERT(b);
        b = v[63];
        SG_ASSERT(!b);
        b = v[64];
        SG_ASSERT(!b);
        b = v[67];
        SG_ASSERT(b);
    }

    {
        BitSet<1000> bs;
        TestSieveOfErathosthenes(bs);
    }
    {
        DynamicBitSet bs(1000);
        TestSieveOfErathosthenes(bs);
    }
    {
        BitSet<1351> bs;
        TestSieveOfErathosthenes(bs);
    }
    {
        DynamicBitSet bs(1351);
        TestSieveOfErathosthenes(bs);
    }
}
//=============================================================================
SG_TEST((sg,core), BitSetPerf, (perf))
{
    {
        SIMPLE_CPU_PERF_LOG_SCOPE("std::bitset");
        std::bitset<4653> bs;
        for_range(size_t, i, 0, 100)
            TestSieveOfErathosthenes(bs);
    }
    {
        SIMPLE_CPU_PERF_LOG_SCOPE("BitSet");
        BitSet<4653> bs;
        for_range(size_t, i, 0, 100)
            TestSieveOfErathosthenes(bs);
    }
    {
        SIMPLE_CPU_PERF_LOG_SCOPE("DynamicBitSet");
        DynamicBitSet bs(4653);
        for_range(size_t, i, 0, 100)
            TestSieveOfErathosthenes(bs);
    }
}
//=============================================================================
}
#endif

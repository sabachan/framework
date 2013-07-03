#include "stdafx.h"

#include "BitField.h"
#include "Config.h"
#if SG_ENABLE_UNIT_TESTS
#include "Log.h"
#include "PerfLog.h"
#include "StringFormat.h"
#include "TestFramework.h"
#include <algorithm>
#include <random>

namespace sg {
//=============================================================================
SG_TEST((sg,core), BitField, (quick))
{
    {
        typedef BitField<1,1,1,1,4> bf_t;
        bf_t bf;
        SG_ASSERT(bf.Get<2>() == 0);
        bf.Set<0>(1);
        bf.Set<1>(0);
        bf.Set<2>(1);
        bf.Set<3>(1);
        bf.Set<4>(9);
        u8 const store = bf.GetStore();
        SG_ASSERT(store == 0x9d);
        bf.SetStore(0x42);
        SG_ASSERT(bf.Get<0>() == 0);
        SG_ASSERT(bf.Get<1>() == 1);
        SG_ASSERT(bf.Get<2>() == 0);
        SG_ASSERT(bf.Get<3>() == 0);
        SG_ASSERT(bf.Get<4>() == 4);
        SG_ASSERT(bf.GetNoSifht<1>() == 0x2);
        SG_ASSERT(bf.GetNoSifht<4>() == 0x40);
        bf.SetNoSifht<4>(0xe0);
        SG_ASSERT(bf.Get<4>() == 14);
    }
    {
        typedef BitField<24,24,11,5> bf_t;
        bf_t bf;
        SG_ASSERT(bf.Get<2>() == 0);
        bf.Set<0>(0xffaa00);
        bf.Set<1>(0x66bbee);
        bf.Set<2>(1058);
        bf.Set<3>(28);
        SG_ASSERT(bf.Get<0>() == 0xffaa00);
        SG_ASSERT(bf.Get<1>() == 0x66bbee);
        SG_ASSERT(bf.Get<2>() == 1058);
        SG_ASSERT(bf.Get<3>() == 28);
    }
}
//=============================================================================
namespace {
struct MyBigStruct
{
    int depth;
    int c0;
    int c1;
    bool isLeaf;
    bool isRoot;
    bool isInCycle;
};
struct MyPackedStructRef
{
    u32 depth : 5;
    u32 c0 : 12;
    u32 c1 : 12;
    u32 isLeaf : 1;
    u32 isRoot : 1;
    u32 isInCycle : 1;
};
static_assert(sizeof(MyPackedStructRef) == 4, "");
struct MyPackedStruct
{
    typedef BitField<5, 12, 12, 1, 1, 1> bf_t;
    union
    {
        bf_t::store_t impl;
        bf_t::UnionMember<0,  5> depth;
        bf_t::UnionMember<1, 12> c0;
        bf_t::UnionMember<2, 12> c1;
        bf_t::UnionMember<3,  1> isLeaf;
        bf_t::UnionMember<4,  1> isRoot;
        bf_t::UnionMember<5,  1> isInCycle;
    };
};
static_assert(sizeof(MyPackedStruct) == 4, "");
template<typename T, size_t N>
void TestPerfs(MyBigStruct (&ref)[N])
{
    size_t dummy = 0;
    T tab[N];
#if COMPILATION_CONFIG_IS_DEBUG
    size_t const KK = 50;
#else
    size_t const KK = 1000;
#endif
    {
        //SIMPLE_CPU_PERF_LOG_SCOPE("copy");
        for_range(size_t, kk, 0, KK)
        {
        for_range(size_t, i, 0, N)
        {
            tab[i].depth     = ref[i].depth;
            tab[i].c0        = ref[i].c0;
            tab[i].c1        = ref[i].c1;
            tab[i].isLeaf    = ref[i].isLeaf;
            tab[i].isRoot    = ref[i].isRoot;
            tab[i].isInCycle = ref[i].isInCycle;
        }
        }
    }
    {
        //SIMPLE_CPU_PERF_LOG_SCOPE("tag leaves");
        for_range(size_t, kk, 0, KK)
        {
        for_range(size_t, i, 0, N)
            tab[i].isLeaf = false;
        for_range(size_t, i, 0, N)
        {
            size_t const c0 = tab[i].c0;
            size_t const c1 = tab[i].c1;
            if(c0 == i && c1 == i)
                tab[i].isLeaf = true;
        }
        }
    }
    {
        //SIMPLE_CPU_PERF_LOG_SCOPE("tag roots");
        for_range(size_t, kk, 0, KK)
        {
        for_range(size_t, i, 0, N)
            tab[i].isRoot = true;
        for_range(size_t, i, 0, N)
        {
            if(!tab[i].isLeaf)
            {
                size_t const c0 = tab[i].c0;
                if(c0 != i)
                    tab[c0].isRoot = false;
                size_t const c1 = tab[i].c1;
                if(c1 != i)
                    tab[c1].isRoot = false;
            }
        }
        }
    }
    {
        //SIMPLE_CPU_PERF_LOG_SCOPE("compute depth");
        for_range(size_t, kk, 0, KK)
        {
        for_range(size_t, i, 0, N)
            tab[i].depth = 0;
        size_t queue[2][N];
        size_t queuesize[2] = { 0, 0 };
        size_t queueindex = 0;
        for_range(size_t, i, 0, N)
        {
            if(tab[i].isLeaf)
            {
                SG_ASSERT(0 == tab[i].depth);
                continue;
            }
            queue[queueindex][queuesize[queueindex]] = i;
            queuesize[queueindex]++;
        }
        while(0 != queuesize[queueindex])
        {
            size_t const readqueueindex = queueindex;
            queueindex = 1 - queueindex;
            queuesize[queueindex] = 0;
            for_range(size_t, qi, 0, queuesize[readqueueindex])
            {
                size_t const i = queue[readqueueindex][qi];
                SG_ASSERT(!tab[i].isLeaf);
                size_t maxChildrenDepth = 0;
                size_t const c0 = tab[i].c0;
                if(c0 != i)
                {
                    size_t const depth0 = tab[c0].depth;
                    if(tab[c0].isLeaf || depth0 != 0)
                        maxChildrenDepth = depth0;
                    else
                        maxChildrenDepth = all_ones;
                }
                size_t const c1 = tab[i].c1;
                if(maxChildrenDepth != all_ones && c1 != i)
                {
                    size_t const depth1 = tab[c1].depth;
                    if(tab[c1].isLeaf || depth1 != 0)
                        maxChildrenDepth = std::max(maxChildrenDepth, depth1);
                    else
                        maxChildrenDepth = all_ones;
                }
                if(maxChildrenDepth != all_ones)
                {
                    tab[i].depth = u32(maxChildrenDepth + 1);
                    dummy = std::max(dummy, size_t(maxChildrenDepth + 1));
                }
                else
                {
                    queue[queueindex][queuesize[queueindex]] = i;
                    queuesize[queueindex]++;
                }
            }
            SG_ASSERT(queuesize[queueindex] != queuesize[readqueueindex]);
        }
        }
    }
    SG_LOG_INFO(Format("dummy = %0", dummy));
}
}
//=============================================================================
SG_TEST((sg,core), BitFieldPerf, (perf, slow))
{
    size_t const N = 1 << 12;

    std::random_device rnd;
    size_t const seed = 0xdeadbeef; // rnd();
    //SG_LOG_INFO(Format("seed = %0", seed));
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> dist(0, N-1);

    MyBigStruct ref[N];
    for_range(int, i, 0, SG_ARRAYSIZE(ref))
    {
        ref[i].depth = 0;
        ref[i].c0 = dist(gen);
        ref[i].c1 = dist(gen);
        if(ref[i].c0 < i) ref[i].c0 = i;
        if(ref[i].c1 < i) ref[i].c1 = i;
        ref[i].isInCycle = false;
        ref[i].isLeaf = false;
        ref[i].isRoot = false;
    }

    for_range(size_t, kk, 0, 5)
    {
        SIMPLE_CPU_PERF_LOG_SCOPE("MyBigStruct");
        TestPerfs<MyBigStruct>(ref);
    }
    for_range(size_t, kk, 0, 5)
    {
        SIMPLE_CPU_PERF_LOG_SCOPE("MyPackedStructRef");
        TestPerfs<MyPackedStructRef>(ref);
    }
    for_range(size_t, kk, 0, 5)
    {
        SIMPLE_CPU_PERF_LOG_SCOPE("MyPackedStruct");
        TestPerfs<MyPackedStruct>(ref);
    }
}
//=============================================================================
}

#endif

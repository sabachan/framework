#include "stdafx.h"

#include "HashMap.h"

#include "TestFramework.h"

#if SG_ENABLE_UNIT_TESTS
#include "Log.h"
#include "PerfLog.h"
#include "StringFormat.h"
#include <unordered_map>

namespace sg {
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct HMTestKeyA
{
    HMTestKeyA(int iValue) : value(iValue) { ++s_count; }
    HMTestKeyA(HMTestKeyA const& iOther) : value(iOther.value) { ++s_count; }
    HMTestKeyA& operator= (HMTestKeyA const& iOther)  { value = iOther.value; }
    ~HMTestKeyA() { --s_count; }
    int value;
    static int s_count;
};
int HMTestKeyA::s_count = 0;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct HMTestKeyANoHasherComparer
{
    size_t operator()(HMTestKeyA const& a) const { return size_t(a.value); }
    bool operator()(HMTestKeyA const& a, HMTestKeyA const& b) const { return a.value == b.value; }

    size_t operator()(int a) const { return size_t(a); }
    bool operator()(int a, HMTestKeyA const& b) const { return a == b.value; }
    bool operator()(HMTestKeyA const& a, int b) const { return a.value == b; }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct HMTestKeyAHasherComparer
{
    size_t operator()(HMTestKeyA const& a) const { return size_t(a.value) * 0xA793C529; }
    bool operator()(HMTestKeyA const& a, HMTestKeyA const& b) const { return a.value == b.value; }

    size_t operator()(int a) const { return size_t(a) * 0xA793C529; }
    bool operator()(int a, HMTestKeyA const& b) const { return a == b.value; }
    bool operator()(HMTestKeyA const& a, int b) const { return a.value == b; }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct HMTestClassA
{
    HMTestClassA(int iValue) : value(iValue), value2(iValue * 10001) { ++s_count; }
    HMTestClassA(HMTestClassA const& iOther) : value(iOther.value), value2(iOther.value2) { ++s_count; }
    HMTestClassA& operator= (HMTestClassA const& iOther)  { value = iOther.value; value2 = iOther.value2; }
    ~HMTestClassA() { --s_count; }
    int value;
    int value2;
    static int s_count;
};
int HMTestClassA::s_count = 0;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
SG_TEST((sg,core), HashMapPool, (quick))
{
    int const& countkA = HMTestKeyA::s_count;
    SG_UNUSED(countkA);
    int const& countvA = HMTestClassA::s_count;
    SG_UNUSED(countvA);
    SG_ASSERT(0 == HMTestClassA::s_count);
    {
        u8 buffer[32 * sizeof(HMTestClassA)] = {};
        hashmap_internal::HashMapPool<HMTestClassA> hmp;
        std::pair<HMTestClassA*, size_t> ptrAndIndices[32] = {};
        for(auto& it : AsArrayView(ptrAndIndices))
            it.second = all_ones;

        ptrAndIndices[0] = hmp.Emplace_GetPtrAndIndex(buffer, 0);
        ptrAndIndices[1] = hmp.Emplace_GetPtrAndIndex(buffer, 1);
        ptrAndIndices[2] = hmp.Emplace_GetPtrAndIndex(buffer, 2);
        SG_ASSERT(3 == HMTestClassA::s_count);
        hmp.Erase(buffer, ptrAndIndices[1].second); ptrAndIndices[1].second = all_ones;
        SG_ASSERT(2 == HMTestClassA::s_count);
        ptrAndIndices[3] = hmp.Emplace_GetPtrAndIndex(buffer, 3);
        ptrAndIndices[4] = hmp.Emplace_GetPtrAndIndex(buffer, 4);
        ptrAndIndices[5] = hmp.Emplace_GetPtrAndIndex(buffer, 5);
        ptrAndIndices[6] = hmp.Emplace_GetPtrAndIndex(buffer, 6);
        ptrAndIndices[7] = hmp.Emplace_GetPtrAndIndex(buffer, 7);
        ptrAndIndices[8] = hmp.Emplace_GetPtrAndIndex(buffer, 8);
        hmp.Erase(buffer, ptrAndIndices[7].second); ptrAndIndices[7].second = all_ones;
        hmp.Erase(buffer, ptrAndIndices[5].second); ptrAndIndices[5].second = all_ones;
        hmp.Erase(buffer, ptrAndIndices[8].second); ptrAndIndices[8].second = all_ones;
        hmp.Erase(buffer, ptrAndIndices[4].second); ptrAndIndices[4].second = all_ones;
        hmp.Erase(buffer, ptrAndIndices[3].second); ptrAndIndices[3].second = all_ones;
        ptrAndIndices[9] = hmp.Emplace_GetPtrAndIndex(buffer, 9);
        ptrAndIndices[10] = hmp.Emplace_GetPtrAndIndex(buffer, 10);
        ptrAndIndices[11] = hmp.Emplace_GetPtrAndIndex(buffer, 11);
        ptrAndIndices[12] = hmp.Emplace_GetPtrAndIndex(buffer, 12);

        SG_ASSERT(ptrAndIndices[6].first == hmp.AtIndex(buffer, ptrAndIndices[6].second));
        u8 buffer2[32 * sizeof(HMTestClassA)] = {};
        hmp.CopyFreeChain(buffer2, buffer);
        hmp.MoveItem(buffer2, buffer, ptrAndIndices[0].second);
        hmp.MoveItem(buffer2, buffer, ptrAndIndices[2].second);
        hmp.MoveItem(buffer2, buffer, ptrAndIndices[6].second);
        hmp.MoveItem(buffer2, buffer, ptrAndIndices[9].second);
        hmp.MoveItem(buffer2, buffer, ptrAndIndices[10].second);
        hmp.MoveItem(buffer2, buffer, ptrAndIndices[11].second);
        hmp.MoveItem(buffer2, buffer, ptrAndIndices[12].second);

        ptrAndIndices[13] = hmp.Emplace_GetPtrAndIndex(buffer2, 13);
        ptrAndIndices[14] = hmp.Emplace_GetPtrAndIndex(buffer2, 14);
        SG_ASSERT(9 == HMTestClassA::s_count);

        for(auto& it : AsArrayView(ptrAndIndices))
        {
            if(it.second != all_ones)
            {
                hmp.Erase(buffer, it.second); it.second = all_ones;
            }
        }
    }
    SG_ASSERT(0 == HMTestClassA::s_count);
}
//=============================================================================
SG_TEST((sg,core), HashMap, (quick))
{
    int const& countkA = HMTestKeyA::s_count;
    SG_UNUSED(countkA);
    int const& countvA = HMTestClassA::s_count;
    SG_UNUSED(countvA);
    {
        HashMap<HMTestKeyA, HMTestClassA, HMTestKeyANoHasherComparer, HMTestKeyANoHasherComparer> hm;
        HMTestClassA* f = hm.Find(3);
        SG_ASSERT_AND_UNUSED(nullptr == f);
    }
    {
        HashMap<HMTestKeyA, HMTestClassA, HMTestKeyANoHasherComparer, HMTestKeyANoHasherComparer> hm;
        hm.Emplace(1, 1);
        hm.Emplace(2, 2);
        hm.Emplace(3, 3);
        HMTestClassA* f = hm.Find(3);
        SG_ASSERT(nullptr != f);
        SG_ASSERT(3 == f->value);
        hm.Emplace(17, 17);
        f = hm.Find(3);
        SG_ASSERT(nullptr != f && 3 == f->value);
        bool e = hm.Erase(1);
        SG_ASSERT(e);
        e = hm.Erase(3);
        SG_ASSERT(e);
        hm.Emplace(33, 33);
        hm.Emplace(32, 32);
        hm.Emplace(0, 0);
        hm.Emplace(15, 15);
        hm.Emplace(47, 47);
        f = hm.Find(2);
        SG_ASSERT(nullptr != f && 2 == f->value);
        f = hm.Find(47);
        SG_ASSERT(nullptr != f && 47 == f->value);
        f = hm.Find(15);
        SG_ASSERT(nullptr != f && 15 == f->value);
        f = hm.Find(3);
        SG_ASSERT(nullptr == f);
        e = hm.Erase(15);
        SG_ASSERT(e);
        f = hm.Find(47);
        SG_ASSERT(nullptr != f && 47 == f->value);
        hm.Emplace(31, 31);
        e = hm.Erase(2);
        SG_ASSERT(e);
        e = hm.Erase(17);
        SG_ASSERT(e);
        hm.Emplace(30, 30);
        hm.Emplace(14, 14);
        f = hm.Find(14);
        SG_ASSERT(nullptr != f && 14 == f->value);
        e = hm.Erase(14);
        SG_ASSERT(e);
        SG_ASSERT(6 == hm.Size());
        f = hm.Find(0);
        SG_ASSERT(nullptr != f && 0 == f->value);
        f = hm.Find(33);
        SG_ASSERT(nullptr != f && 33 == f->value);
        f = hm.Find(32);
        SG_ASSERT(nullptr != f && 32 == f->value);
        f = hm.Find(31);
        SG_ASSERT(nullptr != f && 31 == f->value);
        f = hm.Find(30);
        SG_ASSERT(nullptr != f && 30 == f->value);
        f = hm.Find(47);
        SG_ASSERT(nullptr != f && 47 == f->value);
    }
    {
        HashMap<HMTestKeyA, HMTestClassA, HMTestKeyAHasherComparer, HMTestKeyAHasherComparer> hm;
        hm.Reserve(6);
        hm.Clear();
        hm.Emplace(1, 1);
        hm.Emplace(2, 2);
        hm.Emplace(3, 3);
        hm.Emplace(42, 42);
        hm.Emplace(43, 43);
        SG_ASSERT(5 == HMTestKeyA::s_count);
        SG_ASSERT(5 == HMTestClassA::s_count);
        hm.Emplace(44, 44);
        hm.Emplace(45, 45);
        hm.Emplace(71, 71);
        hm.Emplace(72, 72);
        hm.Emplace(73, 73);
        hm.Emplace(74, 74);
        hm.Emplace(75, 75);
        hm.Emplace(76, 76);
        hm.Emplace(77, 77);
        hm.Emplace(78, 78);
        hm.Emplace(79, 79);
        hm.Emplace(2017, 2017);
        SG_ASSERT(17 == HMTestKeyA::s_count);
        SG_ASSERT(17 == HMTestClassA::s_count);

        auto f = hm.Find(46);
        SG_ASSERT(nullptr == f);
        f = hm.Find(45);
        SG_ASSERT(nullptr != f && f->value == 45);

        {
            auto begin = hm.begin();
            auto end = hm.end();
            SG_ASSERT(begin != end);
            SG_ASSERT(begin->first.value == begin->second.value);
            auto it = begin;
            ++it;
            ++it;
            --it;
            SG_ASSERT(it->first.value == it->second.value);
            SG_ASSERT(it->first.value != begin->second.value);
        }

        for(auto it : hm)
        {
            SG_ASSERT(it.first.value == it.second.value);
        }

        hm.Clear();
        SG_ASSERT(0 == HMTestKeyA::s_count);
        SG_ASSERT(0 == HMTestClassA::s_count);

        {
            auto begin = hm.begin();
            auto end = hm.end();
            SG_ASSERT(begin == end);
        }
    }
    SG_ASSERT(0 == HMTestKeyA::s_count);
    SG_ASSERT(0 == HMTestClassA::s_count);
}

SG_TEST((sg,core), HashMapPerf, (perf, slow, start))
{
    {
#if SG_CODE_IS_OPTIMIZED
        int const N = 2000000;
#else
        int const N = 20000;
#endif
        auto F = [](int x) { return int((size_t(x) * 0x957DC3A6 + 0x33) & 0x7FFFFFFF); };

        {
            HashMap<HMTestKeyA, HMTestClassA, HMTestKeyAHasherComparer, HMTestKeyAHasherComparer> hm;
            hm.Reserve(N);
            {
                SIMPLE_CPU_PERF_LOG_SCOPE("HashMap::Emplace");
                for_range(int, i, 0, N)
                {
                    auto r = hm.Emplace(F(i), i);
                    SG_ASSERT_AND_UNUSED(r.first->value == i || !r.second);
                }
            }
            int dummy = 0;
            {
                SIMPLE_CPU_PERF_LOG_SCOPE("HashMap::Find");
                for_range(int, i, 0, N)
                {
                    auto f = hm.Find(F(i));
                    SG_ASSERT(nullptr != f && f->value == i);
                    if(nullptr != f)
                        dummy += f->value2 - f->value;
                }
            }
            {
                SIMPLE_CPU_PERF_LOG_SCOPE("HashMap::Find (not present)");
                for_range(int, i, N, 2*N)
                {
                    auto f = hm.Find(F(i));
                    SG_ASSERT(nullptr == f);
                    if(nullptr != f)
                        dummy += f->value2 - f->value;
                }
            }
            {
                SIMPLE_CPU_PERF_LOG_SCOPE("HashMap::Erase");
                for_range(int, i, 0, N)
                {
                    bool f = hm.Erase(F(i));
                    SG_ASSERT_AND_UNUSED(f);
                }
            }
            SG_LOG_INFO(Format("dummy = %0", dummy));
            dummy = 0;
            {
                SIMPLE_CPU_PERF_LOG_SCOPE("HashMap - Misc");
                for_range(int, j, 0, N / 2)
                {
                    int i = unsigned(j * 0x256489) % N;
                    auto r = hm.Emplace(F(i), i);
                    SG_ASSERT_AND_UNUSED(r.first->value == i || !r.second);
                    dummy += r.second ? j : 0;
                }
                for_range(int, j, 0, N / 3)
                {
                    int i = unsigned(j * 0x36A3) % N;
                    auto f = hm.Erase(F(i));
                    dummy += f ? j : 0;
                }
                for_range(int, kk, 0, 10)
                for_range(int, i, 0, N)
                {
                    auto f = hm.Find(F(i));
                    if(nullptr != f)
                        dummy += f->value2 - f->value;
                }
                for_range(int, i, 0, N)
                {
                    auto r = hm.Emplace(F(i), i);
                    SG_ASSERT_AND_UNUSED(r.first->value == i || !r.second);
                }
                for_range(int, kk, 0, 10)
                for_range(int, i, 0, N)
                {
                    auto f = hm.Find(F(i));
                    SG_ASSERT_AND_UNUSED(nullptr != f && f->value == i);
                    if(nullptr != f)
                        dummy += f->value2 - f->value;
                }
            }
            SG_LOG_INFO(Format("dummy = %0", dummy));
        }
        {
            std::unordered_map<HMTestKeyA, HMTestClassA, HMTestKeyAHasherComparer, HMTestKeyAHasherComparer> hm;
            hm.reserve(N);
            {
                SIMPLE_CPU_PERF_LOG_SCOPE("std::unordered_map::emplace");
                for_range(int, i, 0, N)
                {
                    auto r = hm.emplace(F(i), i);
                    SG_ASSERT_AND_UNUSED(r.first->second.value == i || !r.second);
                }
            }
            int dummy = 0;
            {
                SIMPLE_CPU_PERF_LOG_SCOPE("std::unordered_map::find");
                for_range(int, i, 0, N)
                {
                    auto f = hm.find(F(i));
                    SG_ASSERT(hm.end() != f && f->second.value == i);
                    if(hm.end() != f)
                        dummy += f->second.value2 - f->second.value;
                }
            }
            {
                SIMPLE_CPU_PERF_LOG_SCOPE("std::unordered_map::find (not present)");
                for_range(int, i, N, 2*N)
                {
                    auto f = hm.find(F(i));
                    SG_ASSERT(hm.end() == f);
                    if(hm.end() != f)
                        dummy += f->second.value2 - f->second.value;
                }
            }
            {
                SIMPLE_CPU_PERF_LOG_SCOPE("std::unordered_map::erase");
                for_range(int, i, 0, N)
                {
                    size_t f = hm.erase(F(i));
                    SG_ASSERT_AND_UNUSED(1 == f);
                }
            }
            SG_LOG_INFO(Format("dummy = %0", dummy));
            dummy = 0;
            {
                SIMPLE_CPU_PERF_LOG_SCOPE("HashMap - Misc");
                for_range(int, j, 0, N / 2)
                {
                    int i = unsigned(j * 0x256489) % N;
                    auto r = hm.emplace(F(i), i);
                    SG_ASSERT_AND_UNUSED(r.first->second.value == i || !r.second);
                    dummy += r.second ? j : 0;
                }
                for_range(int, j, 0, N / 3)
                {
                    int i = unsigned(j * 0x36A3) % N;
                    size_t f = hm.erase(F(i));
                    dummy += int(f * j);
                }
                for_range(int, kk, 0, 10)
                for_range(int, i, 0, N)
                {
                    auto f = hm.find(F(i));
                    if(hm.end() != f)
                        dummy += f->second.value2 - f->second.value;
                }
                for_range(int, i, 0, N)
                {
                    auto r = hm.emplace(F(i), i);
                    SG_ASSERT_AND_UNUSED(r.first->second.value == i || !r.second);
                }
                for_range(int, kk, 0, 10)
                for_range(int, i, 0, N)
                {
                    auto f = hm.find(F(i));
                    SG_ASSERT_AND_UNUSED(hm.end() != f && f->second.value == i);
                    if(hm.end() != f)
                        dummy += f->second.value2 - f->second.value;
                }
            }
            SG_LOG_INFO(Format("dummy = %0", dummy));
        }
    }
    SG_ASSERT(0 == HMTestKeyA::s_count);
    SG_ASSERT(0 == HMTestClassA::s_count);
}
//=============================================================================
}
#endif


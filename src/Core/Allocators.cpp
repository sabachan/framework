#include "stdafx.h"

#include "Allocators.h"

#include "TestFramework.h"
#if SG_ENABLE_UNIT_TESTS
#include "SmartPtr.h"
#endif

namespace sg {
//=============================================================================
//=============================================================================
}


#if SG_ENABLE_UNIT_TESTS
namespace sg {
//=============================================================================
namespace {
template <typename A>
void CheckTypedAllocatorTraits()
{
    typedef A::value_type value_type;
}
template <typename A>
void CheckVoidAllocatorTraits()
{
    typedef A::value_type value_type;

    CheckTypedAllocatorTraits<typename A::template Rebind<u8>::type>();
    CheckTypedAllocatorTraits<typename A::template Rebind<int>::type>();
    CheckTypedAllocatorTraits<typename A::template Rebind<float>::type>();
}
}
//=============================================================================
SG_TEST((sg,core), Allocators, (quick))
{
    CheckVoidAllocatorTraits<StandardAllocator<>>();
    CheckVoidAllocatorTraits<InPlaceAllocator<16>>();
    CheckVoidAllocatorTraits<InPlaceAllocator<16, NullAllocator<>>>();
    CheckVoidAllocatorTraits<NullAllocator<>>();
    static_assert(sizeof(StandardAllocator<>) == 1, "");
    static_assert(sizeof(StandardAllocator<int>) == 1, "");
    static_assert(sizeof(StandardAllocator<std::string>) == 1, "");
    {
        StandardAllocator<> a;
        void* aa = &a;
        SG_ASSERT_AND_UNUSED(nullptr != aa);
    }
    {
        StandardAllocator<>::Rebind<int>::type a;
        size_t allocatedSize;
        void* buffer = a.AllocateAtLeast(10, allocatedSize);
        SG_ASSERT(10 <= allocatedSize);
        a.Deallocate(buffer, allocatedSize);
    }
    {
        InPlaceAllocator<16>::Rebind<int>::type a;
        size_t allocatedSize;
        void* buffer = a.AllocateAtLeast(0, allocatedSize);
        SG_ASSERT(16 == allocatedSize);
        SG_ASSERT(&a == buffer);
        size_t allocatedSize2;
        void* buffer2 = a.AllocateAtLeast(8, allocatedSize2);
        SG_ASSERT(8 <= allocatedSize2);
        SG_ASSERT(&a != buffer2);
        a.Deallocate(buffer2, allocatedSize2);
        a.Deallocate(buffer, allocatedSize);
        buffer = a.AllocateAtLeast(32, allocatedSize);
        SG_ASSERT(32 <= allocatedSize);
        buffer2 = a.AllocateAtLeast(8, allocatedSize2);
        SG_ASSERT(16 == allocatedSize2);
        SG_ASSERT(&a == buffer2);
        a.Deallocate(buffer2, allocatedSize2);
        a.Deallocate(buffer, allocatedSize);
    }
    {
        NullAllocator<int> a;
        size_t allocatedSize;
        void* buffer = a.AllocateAtLeast(0, allocatedSize);
        SG_ASSERT_AND_UNUSED(0 == allocatedSize);
        SG_ASSERT_AND_UNUSED(nullptr == buffer);
    }
}
//=============================================================================
namespace {
template <typename T> int ToInt(T const& t) { return int(t); }
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct MyType1
{
    int a;
    float b;
    bool c;

    MyType1() : a(), b(), c() {}
    MyType1(int i) : a(i), b(0.1f * i), c(i%3 == 0) {}
    operator int() const { return a; }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct MyType2 : public SafeCountable
{
    int a;

    MyType2() : a() {}
    MyType2(int i) : a(i) {}
    operator int() const { return a; }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct MyType3
{
    safeptr<MyType2> a;

    MyType3() : a() {}
    MyType3(int i) : a(new MyType2(i)) {}
    MyType3(MyType3&& other) : a(other.a) { other.a = nullptr; }
    ~MyType3() { MyType2* t = a.get(); a = nullptr; delete t; }
    operator int() const { return nullptr == a ? -1 : *a; }

    SG_NON_COPYABLE(MyType3)
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct MyType4
{
    safeptr<MyType2> a;

    MyType4() : a() {}
    MyType4(int i) : a(new MyType2(i)) {}
    ~MyType4() { MyType2* t = a.get(); a = nullptr; delete t; }
    operator int() const { return nullptr == a ? -1 : *a; }

    MyType4(MyType4 const& other) : a(new MyType2(int(other))) {}
    MyType4& operator=(MyType4 const& other) { MyType2* t = a.get(); a = nullptr; delete t; if(nullptr != other.a) { a = new MyType2(int(other)); } }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct MyType5
{
    safeptr<MyType2> a;

    MyType5() : a() {}
    MyType5(int i) : a(new MyType2(i)) {}
    ~MyType5() { MyType2* t = a.get(); a = nullptr; delete t; }
    operator int() const { return nullptr == a ? -1 : *a; }

    MyType5(MyType5 const& other) : a(new MyType2(int(other))) {}
    MyType5& operator=(MyType5 const& other) { MyType2* t = a.get(); a = nullptr; delete t; if(nullptr != other.a) { a = new MyType2(int(other)); } }
};
void swap(MyType5& a, MyType5& b) { swap(a.a, b.a); }
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
void TestRelocateIntLike()
{
    int const N = 128;
    u8 mem[N * sizeof(T)];
    T* buffer = reinterpret_cast<T*>(mem);
    for_range(int, i, 0, N)
        new(buffer + i) T(i);

    Relocate_AssumeDisjoint(buffer + 20, buffer + 60, 10);
    SG_ASSERT(19 == ToInt(buffer[19]));
    SG_ASSERT(60 == ToInt(buffer[20]));
    SG_ASSERT(69 == ToInt(buffer[29]));
    SG_ASSERT(30 == ToInt(buffer[30]));
    Relocate_Forward(buffer + 1, buffer + 2, 10);
    SG_ASSERT(0 == ToInt(buffer[0]));
    SG_ASSERT(2 == ToInt(buffer[1]));
    SG_ASSERT(11 == ToInt(buffer[10]));
    //SG_ASSERT(11 == ToInt(buffer[11]));
    Relocate_Backward(buffer + 50, buffer + 49, 11);
    //SG_ASSERT(49 == ToInt(buffer[49]));
    SG_ASSERT(49 == ToInt(buffer[50]));
    SG_ASSERT(58 == ToInt(buffer[59]));
    SG_ASSERT(59 == ToInt(buffer[60]));

    for_range(int, i, 0, N)
        buffer[i].~T();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
SG_TEST((sg,core), Relocate, (quick))
{
    static_assert(!relocate_internal::IsDedicatedlySwappable<MyType4>::value, "");
    static_assert(relocate_internal::IsDedicatedlySwappable<MyType5>::value, "");
    TestRelocateIntLike<int>();
    TestRelocateIntLike<MyType1>();
    TestRelocateIntLike<MyType2>();
    TestRelocateIntLike<MyType3>();
    TestRelocateIntLike<MyType4>();
    TestRelocateIntLike<MyType5>();
}
//=============================================================================
}
#endif


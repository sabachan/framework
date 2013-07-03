#include "stdafx.h"

#include "ArrayList.h"

#include "TestFramework.h"

#if SG_ENABLE_UNIT_TESTS
namespace sg {
//=============================================================================
namespace {
struct ALTest_ClassA
{
    ALTest_ClassA(int iValue) : value(iValue) { ++s_count; } 
    ALTest_ClassA(ALTest_ClassA const& iOther) : value(iOther.value) { ++s_count; }
    ALTest_ClassA& operator= (ALTest_ClassA const& iOther)  { value = iOther.value; }
    ~ALTest_ClassA() { --s_count; }
    int value;
    static int s_count;
};
int ALTest_ClassA::s_count = 0;
struct ALTest_ClassB
{
    ALTest_ClassB() : value(0) { ++s_count; }
    ALTest_ClassB(int iValue) : value(iValue) { ++s_count; }
    ALTest_ClassB(ALTest_ClassB const& ) = delete;
    ALTest_ClassB(ALTest_ClassB&& iOther) : value(iOther.value) { ++s_count; }
    ALTest_ClassB& operator= (ALTest_ClassB const&) = delete;
    ALTest_ClassB& operator= (ALTest_ClassB&& iOther)  { value = iOther.value; }
    ~ALTest_ClassB() { --s_count; }
    int value;
    static int s_count;
};
int ALTest_ClassB::s_count = 0;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
void TestArrayList()
{
    typedef T array_list_type;
    typedef array_list_type::value_type value_type;
    {
        array_list_type al1;
        array_list_type al2 = al1;
        array_list_type al3;
        SG_ASSERT(al1.empty());
        SG_ASSERT(al2.empty());
        al2.EmplaceBack(10);
        al2.Resize(3, 20);
        al2.EmplaceBack(30);
        SG_ASSERT(4 == value_type::s_count);
        al1 = al2;
        SG_ASSERT(8 == value_type::s_count);
        al2.Clear();
        SG_ASSERT(4 == value_type::s_count);
        al2 = std::move(al1);
        SG_ASSERT(4 <= value_type::s_count);
        al1.Clear();
        SG_ASSERT(4 == value_type::s_count);
    }
    SG_ASSERT(0 == value_type::s_count);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template class ArrayList<ALTest_ClassA>;
template class ArrayList<ALTest_ClassB>;
template class ArrayList<ALTest_ClassA, InPlaceAllocator<8>>;
template class ArrayList<ALTest_ClassB, InPlaceAllocator<8>>;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
SG_TEST((sg,core), ArrayList, (quick))
{
    {
        size_t const N = 64;
        u8 buffer[N * sizeof(ALTest_ClassA)];
        size_t size = 0;
        MaxSizeArrayListView<ALTest_ClassA> list(reinterpret_cast<ALTest_ClassA*>(buffer), size, N);
        SG_ASSERT(0 == ALTest_ClassA::s_count);
        list.EmplaceBack_AssumeCapacity(1);
        SG_ASSERT(1 == ALTest_ClassA::s_count);
        {
            ALTest_ClassA a(2);
            SG_ASSERT(2 == ALTest_ClassA::s_count);
            for_range(size_t, i, 0, 2)
                list.PushBack_AssumeCapacity(a);
            SG_ASSERT(4 == ALTest_ClassA::s_count);
        }
        SG_ASSERT(3 == ALTest_ClassA::s_count);
        list.EmplaceBack_AssumeCapacity(3);
        SG_ASSERT(4 == ALTest_ClassA::s_count);
        list.PopBack();
        SG_ASSERT(3 == ALTest_ClassA::s_count);
        {
            ALTest_ClassA a(4);
            for_range(size_t, i, 0, 7)
                list.PushBack_AssumeCapacity(a);
        }
        SG_ASSERT(10 == ALTest_ClassA::s_count);
        list.SizeDown(4);
        SG_ASSERT(4 == ALTest_ClassA::s_count);
        list.Resize_AssumeCapacity(7, 5);
        SG_ASSERT(7 == ALTest_ClassA::s_count);
        list.Clear();
        SG_ASSERT(0 == ALTest_ClassA::s_count);
    }
    TestArrayList<ArrayList<ALTest_ClassA>>();
    TestArrayList<ArrayList<ALTest_ClassA, InPlaceAllocator<8>>>();
    TestArrayList<ArrayList<ALTest_ClassA, InPlaceAllocator<8, NullAllocator<>>>>();
    {
        int const& count = ALTest_ClassA::s_count;
        SG_UNUSED(count);
        ArrayList<ALTest_ClassA, InPlaceAllocator<8>> al1;
        ArrayList<ALTest_ClassA, InPlaceAllocator<8>> al2;
        ArrayList<ALTest_ClassA, InPlaceAllocator<8>> al3;
        ArrayList<ALTest_ClassA, InPlaceAllocator<8>> al4;
        al1.Reserve(32);
        al1.Resize(4, 10);
        SG_ASSERT(4 == ALTest_ClassA::s_count);
        al2.Reserve(16);
        al2 = std::move(al1); // this should swap buffers
        SG_ASSERT(4 == ALTest_ClassA::s_count);
        al3 = std::move(al2); // this should swap buffers
        SG_ASSERT(4 == ALTest_ClassA::s_count);
        al4.EmplaceBack(20);
        al4 = std::move(al3); // this should move copy objects (here, copy)
        SG_ASSERT(8 == ALTest_ClassA::s_count);
    }
    SG_ASSERT(0 == ALTest_ClassA::s_count);
    {
        typedef ArrayList<ALTest_ClassB> array_list_type;
        typedef array_list_type::value_type value_type;
        array_list_type al1;
        array_list_type al2;
        SG_ASSERT(al1.empty());
        SG_ASSERT(al2.empty());
        al2.EmplaceBack(10);
        al2.Resize(3, 20);
        al2.EmplaceBack();
        al2.Resize(6);
        SG_ASSERT(6 == value_type::s_count);
        al1 = std::move(al2);
        SG_ASSERT(6 <= value_type::s_count);
        al2.Clear();
        SG_ASSERT(6 == value_type::s_count);
        void* prevptr = al1.data();
        al1.Resize(64);
        SG_ASSERT(64 == value_type::s_count);
        SG_ASSERT_AND_UNUSED(prevptr != al1.data());
    }
    SG_ASSERT(0 == ALTest_ClassB::s_count);
}
//=============================================================================
}
#endif


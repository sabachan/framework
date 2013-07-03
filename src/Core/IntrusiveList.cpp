#include "stdafx.h"

#include "IntrusiveList.h"

#if SG_ENABLE_UNIT_TESTS
#include "Log.h"
#include "SmartPtr.h"
#include "TestFramework.h"

namespace sg {
//=============================================================================
namespace testintrusivelist {
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class A : public IntrusiveListElem<A>
{
public:
    A(size_t iVal) : _(iVal) {}
    size_t _;
};

class B : public RefAndSafeCountable
        , public IntrusiveListRefCountableElem<B>
{
public:
    B(size_t iVal) : _(iVal) { ++s_count; }
    ~B() { --s_count; }
    size_t _;
    static size_t s_count;
};
size_t B::s_count = 0;

class C : public IntrusiveListElem<C, 2>
{
public:
    C(size_t iVal) : _(iVal) {}
    size_t _;
};

class D : public RefCountable
        , public IntrusiveListRefCountableElem<D, 2>
{
public:
    D(size_t iVal) : _(iVal) { ++s_count; }
    ~D() { --s_count; }
    size_t _;
    static size_t s_count;
};
size_t D::s_count = 0;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
}
//=============================================================================
SG_TEST((sg), IntrusiveList, (Core, quick))
{
    using namespace testintrusivelist;
    {
        A a(0);
        A b(1);
        A c(2);
        A d(3);
        A e(4);
        {
            IntrusiveList<A> list;
            SG_ASSERT(list.Empty());
            list.PushBack(&a);
            SG_ASSERT(!list.Empty());
            list.PushBack(&b);
            list.PushFront(&c);
            list.PushBack(&d);
            list.PushBack(&e);
            {
                size_t const ref[] = {2,0,1,3,4};
                size_t i = 0;
                for(auto it = list.begin(); it != list.end(); ++it, ++i)
                {
                    size_t const val = it->_;
                    SG_ASSERT_AND_UNUSED(ref[i] == val);
                }
            }
            SG_ASSERT(!list.Empty());
            list.Clear();
            SG_ASSERT(list.Empty());
            list.PushFront(&a);
            SG_ASSERT(0 == list.Front()->_);
            SG_ASSERT(0 == list.Back()->_);
            list.PushFront(&b);
            SG_ASSERT(1 == list.Front()->_);
            SG_ASSERT(0 == list.Back()->_);
            list.PushBack(&c);
            SG_ASSERT(1 == list.Front()->_);
            SG_ASSERT(2 == list.Back()->_);
            list.PopFront();
            SG_ASSERT(0 == list.Front()->_);
            SG_ASSERT(2 == list.Back()->_);
            list.PopBack();
            list.PopBack();
            SG_ASSERT(list.Empty());
        }
        {
            IntrusiveList<A> list;
            SG_ASSERT(list.Empty());
            list.PushBack(&a);
            list.InsertAfter(&a, &b);
            list.InsertBefore(&b, &c);
            list.InsertBefore(&a, &d);
            list.InsertAfter(&b, &e);
            SG_ASSERT(3 == list.Front()->_);
            list.PopFront();
            SG_ASSERT(0 == list.Front()->_);
            list.PopFront();
            SG_ASSERT(2 == list.Front()->_);
            list.PopFront();
            SG_ASSERT(1 == list.Front()->_);
            list.PopFront();
            SG_ASSERT(4 == list.Front()->_);
            list.PopFront();
            SG_ASSERT(list.Empty());
        }
        {
            IntrusiveList<A> list;
            SG_ASSERT(list.Empty());
            list.PushBack(&a);
            list.PushBack(&b);
            list.PushBack(&c);
            list.PushBack(&d);
            SG_ASSERT(list.Contains(&a));
            SG_ASSERT(!list.Contains(&e));
            A* v = list.After(&a);
            SG_ASSERT(1 == v->_);
            v = list.After(v);
            SG_ASSERT(2 == v->_);
            v = list.Before(v);
            SG_ASSERT(1 == v->_);
            v = list.Before(&d);
            SG_ASSERT(2 == v->_);
        }
        {
            IntrusiveList<A> list;
            SG_ASSERT(list.Empty());
            list.PushBack(&a);
            list.PushBack(&b);
            list.PushBack(&c);
            list.PushBack(&d);
            list.PushBack(&e);
            list.MoveToAfter(&a, &d);
            list.MoveToAfter(&e, &c);
            list.MoveToAfter(&b, &a);
            {
                size_t const ref[] = {3,1,0,4,2};
                size_t i = 0;
                for(auto it = list.begin(); it != list.end(); ++it, ++i)
                {
                    size_t const val = it->_;
                    SG_ASSERT_AND_UNUSED(ref[i] == val);
                }
            }
            list.MoveToBefore(&d, &a);
            list.MoveToBefore(&e, &d);
            list.MoveToBefore(&d, &c);
            {
                size_t const ref[] = {0,1,2,3,4};
                size_t i = 0;
                for(auto it = list.begin(); it != list.end(); ++it, ++i)
                {
                    size_t const val = it->_;
                    SG_ASSERT_AND_UNUSED(ref[i] == val);
                }
            }
        }
    }

    {
        IntrusiveList<B> list;
        SG_ASSERT(0 == B::s_count);
        B* b = new B(0);
        SG_ASSERT(1 == B::s_count);
        list.PushBack(b);
        refptr<B> rb = new B(1);
        list.PushBack(rb.get());
        {
            refptr<B> rb2 = new B(2);
            list.PushBack(rb2.get());
        }
        list.PushBack(new B(3));
        list.PushBack(new B(4));
        SG_ASSERT(5 == B::s_count);
        SG_ASSERT(!list.Empty());
        {
            size_t const ref[] = {0,1,2,3,4};
            size_t i = 0;
            for(auto it = list.begin(); it != list.end(); ++it, ++i)
            {
                size_t const val = it->_;
                SG_ASSERT_AND_UNUSED(ref[i] == val);
            }
        }
        SG_ASSERT(0 == list.Front()->_);
        refptr<B> tmp = list.Back();
        SG_ASSERT(4 == tmp->_);
        list.PopBack();
        list.PushFront(tmp.get());
        tmp = nullptr;
        SG_ASSERT(4 == list.Front()->_);
        SG_ASSERT(3 == list.Back()->_);
        SG_ASSERT(5 == B::s_count);
        list.PopBack();
        SG_ASSERT(4 == B::s_count);
        list.PopFront();
        SG_ASSERT(3 == B::s_count);
        SG_ASSERT(0 == list.Front()->_);
        SG_ASSERT(2 == list.Back()->_);
        SG_ASSERT(!list.Empty());
        list.Clear();
        SG_ASSERT(list.Empty());
        SG_ASSERT(1 == rb->_);
        SG_ASSERT(1 == B::s_count);
        rb = nullptr;
        SG_ASSERT(0 == B::s_count);
    }

    {
        C a(0);
        C b(1);
        C c(2);
        C d(3);
        C e(4);
        {
            IntrusiveList<C, 0, 2> list0a;
            IntrusiveList<C, 0, 2> list0b;
            IntrusiveList<C, 1, 2> list1;
            list0a.PushBack(&a);
            list0a.PushBack(&b);
            list0a.PushBack(&c);
            list0a.PushBack(&d);
            list0a.PushBack(&e);
            list1.PushFront(&b);
            list1.PushFront(&d);
            SG_ASSERT(3 == list1.Front()->_);
            SG_ASSERT(1 == list1.Back()->_);
            C* tmp = list0a.Front();
            list0a.PopFront();
            list0b.PushBack(tmp);
            list0a.Remove(&c);
            list0b.PushBack(&c);
            SG_ASSERT(0 == list0b.Front()->_);
            SG_ASSERT(2 == list0b.Back()->_);
        }
    }

    {
        IntrusiveList<D, 0, 2> list0;
        IntrusiveList<D, 1, 2> list1a;
        IntrusiveList<D, 1, 2> list1b;
        SG_ASSERT(0 == D::s_count);
        list0.PushBack(new D(0));
        list1a.PushBack(list0.Back());
        list0.PushBack(new D(1));
        list1b.PushBack(list0.Back());
        list0.PushBack(new D(2));
        list1a.PushBack(list0.Back());
        list0.PushBack(new D(3));
        list1b.PushBack(list0.Back());
        list0.PushBack(new D(4));
        list1a.PushBack(list0.Back());
        SG_ASSERT(0 == list1a.Front()->_);
        SG_ASSERT(4 == list1a.Back()->_);
        SG_ASSERT(1 == list1b.Front()->_);
        SG_ASSERT(3 == list1b.Back()->_);
        SG_ASSERT(5 == D::s_count);
        D* tmp = list1a.Back();
        list1a.PopBack();
        list1b.PushFront(tmp);
        SG_ASSERT(5 == D::s_count);
        SG_ASSERT(4 == list1b.Front()->_);
        SG_ASSERT(3 == list1b.Back()->_);
        list0.Clear();
        SG_ASSERT(5 == D::s_count);
        list1b.PopFront();
        SG_ASSERT(1 == list1b.Front()->_);
        SG_ASSERT(3 == list1b.Back()->_);
        SG_ASSERT(4 == D::s_count);
        for(auto& it : list1a)
        {
            list0.PushBack(&it);
        }
        for(auto& it : list1b)
        {
            list0.PushBack(&it);
        }
        {
            size_t const ref[] = {0,2,1,3};
            size_t i = 0;
            for(auto it = list0.begin(); it != list0.end(); ++it, ++i)
            {
                size_t const val = it->_;
                SG_ASSERT_AND_UNUSED(ref[i] == val);
            }
        }
        list1a.Clear();
        list1b.Clear();
        SG_ASSERT(4 == D::s_count);
        list0.Clear();
        SG_ASSERT(0 == D::s_count);
    }
}
//=============================================================================
}
#endif

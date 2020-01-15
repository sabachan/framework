#include "stdafx.h"

#include "SmartPtr.h"
#include "TestFramework.h"

namespace sg {
//=============================================================================
#if SG_ENABLE_ASSERT
RefCountable::~RefCountable()
{
    SG_ASSERT(0 == m_refcount);
    SG_ASSERT(std::this_thread::get_id() == m_threadId);
}
#endif
//=============================================================================
#if SG_ENABLE_ASSERT
SafeCountable::~SafeCountable()
{
    SG_ASSERT(0 == m_safecount);
    m_safecount = 0;
}
#endif
//=============================================================================
}

#if SG_ENABLE_UNIT_TESTS
namespace sg {
//=============================================================================
namespace testsmartptr {
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class A : public RefWeakAndSafeCountable
{
public:
    A() { ++s_nbInstance; }
    virtual ~A() { --s_nbInstance; }
    static int NbInstance() { return s_nbInstance; }

public:
    refptr<A> m_ra;
private:
    static int s_nbInstance;
};
int A::s_nbInstance = 0;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class I1 : public VirtualRefAndSafeCountable
{
public:
    I1() { ++s_nbInstance; }
    ~I1() { --s_nbInstance; }
    virtual void  VirtualFunction1() = 0;
    static int NbInstance() { return s_nbInstance; }
private:
    static int s_nbInstance;
};
int I1::s_nbInstance = 0;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class I2 : public VirtualRefCountable
{
public:
    I2() { ++s_nbInstance; }
    ~I2() { --s_nbInstance; }
    virtual void  VirtualFunction2() = 0;
    static int NbInstance() { return s_nbInstance; }
private:
    static int s_nbInstance;
};
int I2::s_nbInstance = 0;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class I3 : public SafeCountable
{
public:
    I3() { ++s_nbInstance; }
    ~I3() { --s_nbInstance; }
    virtual void  VirtualFunction3() = 0;
    static int NbInstance() { return s_nbInstance; }
private:
    static int s_nbInstance;
};
int I3::s_nbInstance = 0;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class I4 : public VirtualRefAndWeakCountable
{
public:
    I4() { ++s_nbInstance; }
    ~I4() { --s_nbInstance; }
    virtual void  VirtualFunction2() = 0;
    static int NbInstance() { return s_nbInstance; }
private:
    static int s_nbInstance;
};
int I4::s_nbInstance = 0;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class B : public A
        , public I1
        , public I2
        , public I3
        , public I4
{
    PARENT_REF_COUNTABLE(A)
    PARENT_WEAK_COUNTABLE(A)
    PARENT_SAFE_COUNTABLE(A)
public:
    virtual void  VirtualFunction1() override {}
    virtual void  VirtualFunction2() override {}
    virtual void  VirtualFunction3() override {}
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class C : public I1
        , private RefCountable
{
    PARENT_REF_COUNTABLE(RefCountable)
public:
    C() { ++s_nbInstance; }
    ~C() { --s_nbInstance; }
    virtual void  VirtualFunction1() override {}
    static int NbInstance() { return s_nbInstance; }
private:
    static int s_nbInstance;
};
int C::s_nbInstance = 0;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
}
//=============================================================================
SG_TEST((sg), SmartPtr, (Core, quick))
{
    using namespace testsmartptr;
    {
        SG_ASSERT(A::NbInstance() == 0);
        A a;
        SG_ASSERT(A::NbInstance() == 1);
    }
    SG_ASSERT(A::NbInstance() == 0);
    {
        refptr<A> ra0 = new A();
        safeptr<A> sa0 = ra0.get();
        weakptr<A> wa0 = ra0.get();
        SG_ASSERT(A::NbInstance() == 1);
        refptr<A> ra1 = ra0;
        SG_ASSERT(A::NbInstance() == 1);
        {
            refptr<A> p = wa0.Lock();
            SG_ASSERT(p == ra1);
        }
        ra0 = new A();
        sa0 = ra0.get();
        SG_ASSERT(A::NbInstance() == 2);
        ra1 = ra0;
        SG_ASSERT(A::NbInstance() == 1);
        {
            refptr<A> p;
            wa0.Lock(p);
            SG_ASSERT(nullptr == p);
        }
        ra0 = ra0;
        SG_ASSERT(A::NbInstance() == 1);
        ra0 = nullptr;
        SG_ASSERT(A::NbInstance() == 1);
        sa0 = nullptr;
        ra1 = ra0;
        SG_ASSERT(A::NbInstance() == 0);
    }
    {
        A* pa0 = new A();
        refptr<A> ra0 = pa0;
        SG_ASSERT(A::NbInstance() == 1);
        refptr<A> ra1 = new A();
        SG_ASSERT(A::NbInstance() == 2);
        ra0->m_ra = ra1;
        ra1->m_ra = ra0;
        SG_ASSERT(A::NbInstance() == 2);
        ra0 = nullptr;
        SG_ASSERT(A::NbInstance() == 2);
        ra1 = nullptr;
        SG_ASSERT(A::NbInstance() == 2);
        _CrtCheckMemory();
        SG_ASSERT(A::NbInstance() == 2);
        pa0->m_ra = nullptr;
        _CrtCheckMemory();
        SG_ASSERT(A::NbInstance() == 0);
        _CrtCheckMemory();
    }
    {
        safeptr<A> sa0;
        scopedptr<A> a0(new A());
        sa0 = a0.get();
        SG_ASSERT(A::NbInstance() == 1);
        {
            scopedptr<A> a1(new A());
            SG_ASSERT(A::NbInstance() == 2);
            safeptr<A> sa1 = a0.get();
        }
        SG_ASSERT(A::NbInstance() == 1);
        sa0 = nullptr;
    }
    SG_ASSERT(A::NbInstance() == 0);
    {
        B* pb = new B();
        SG_ASSERT(A::NbInstance() == 1);
        SG_ASSERT(I1::NbInstance() == 1);
        SG_ASSERT(I2::NbInstance() == 1);
        SG_ASSERT(I3::NbInstance() == 1);
        refptr<A> ra = pb;
        refptr<B> rb = pb;
        // TODO: remove following ".get()"s
        safeptr<I1> si1 = rb.get();
        safeptr<I3> si3 = rb.get();
        refptr<I1> ri1 = si1.get();
        refptr<I2> ri2 = rb.get();
        weakptr<I4> wi4 = rb.get();
        SG_ASSERT(A::NbInstance() == 1);
        ra = new A();
        SG_ASSERT(A::NbInstance() == 2);
        rb = nullptr;
        SG_ASSERT(A::NbInstance() == 2);
        ri1 = nullptr;
        si1 = nullptr;
        si3 = nullptr;
        SG_ASSERT(A::NbInstance() == 2);
        SG_ASSERT(I1::NbInstance() == 1);
        SG_ASSERT(I2::NbInstance() == 1);
        SG_ASSERT(I3::NbInstance() == 1);
        {
            refptr<I4> p = wi4.Lock();
            SG_ASSERT(nullptr != p);
        }
        ri2 = nullptr;
        {
            refptr<I4> p = wi4.Lock();
            SG_ASSERT(nullptr == p);
        }
        SG_ASSERT(A::NbInstance() == 1);
        SG_ASSERT(I1::NbInstance() == 0);
        SG_ASSERT(I2::NbInstance() == 0);
        SG_ASSERT(I3::NbInstance() == 0);
    }
    SG_ASSERT(A::NbInstance() == 0);
    {
        SG_ASSERT(C::NbInstance() == 0);
        C* pc = new C();
        SG_ASSERT(C::NbInstance() == 1);
        refptr<I1> ri1 = pc;
        pc = new C();
        SG_ASSERT(C::NbInstance() == 2);
        delete pc;
        SG_ASSERT(C::NbInstance() == 1);
        ri1 = nullptr;
        SG_ASSERT(C::NbInstance() == 0);
    }
    SG_ASSERT(A::NbInstance() == 0);
    {
        refptrOrInt<A> ra0 = new A;
        refptrOrInt<A> ra1 = new A;
        SG_ASSERT(A::NbInstance() == 2);
        ra0 = ra1;
        SG_ASSERT(A::NbInstance() == 1);
        ra1.SetInt(3);
        ra0 = ra1;
        SG_ASSERT(A::NbInstance() == 0);
        SG_ASSERT(ra0.IsInt());
        SG_ASSERT(ra0.GetInt() == 3);
        ra1 = new A;
        ra0 = ra1;
        SG_ASSERT(ra0.IsPtr());
        ra1.SetInt(-1);
        SG_ASSERT(ra1.GetInt() == -1);
        ra1.SetInt(0);
        SG_ASSERT(ra1.GetInt() == 0);
        ra1.SetInt(-64000);
        SG_ASSERT(ra1.GetInt() == -64000);
        ra1.SetInt(64000);
        SG_ASSERT(ra1.GetInt() == 64000);
        SG_ASSERT(A::NbInstance() == 1);
    }
    SG_ASSERT(A::NbInstance() == 0);
}
//=============================================================================
}
#endif

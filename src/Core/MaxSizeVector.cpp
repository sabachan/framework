#include "stdafx.h"

#include "MaxSizeVector.h"

#if SG_ENABLE_UNIT_TESTS
#include "TestFramework.h"

namespace sg {
//=============================================================================
namespace testmaxsizevector {
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class A
{
public:
    A(int iA = 1, int iB = 10) : a(iA), b(iB) { ++s_nbInstance; }
    A(A const& iOther) : a(iOther.a), b(iOther.b) { ++s_nbInstance; }
    A& operator=(A const& iOther) = default;
    ~A() { --s_nbInstance; }
    static int NbInstance() { return s_nbInstance; }

public:
    int a;
    int b;
private:
    static int s_nbInstance;
};
int A::s_nbInstance = 0;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
}
//=============================================================================
SG_TEST((sg), MaxSizeVector, (Core, quick))
{
    using namespace testmaxsizevector;
    {
        MaxSizeVector<int, 10> v;
        SG_ASSERT(v.empty());
        SG_ASSERT(0 == v.size());
        v.emplace_back();
        SG_ASSERT(!v.empty());
        SG_ASSERT(1 == v.size());
        SG_ASSERT(0 == v[0]);
        v.emplace_back(6);
        SG_ASSERT(2 == v.size());
        SG_ASSERT(6 == v[1]);
        v.push_back(9);
        SG_ASSERT(3 == v.size());
        SG_ASSERT(9 == v[2]);
        v.clear();
        SG_ASSERT(v.empty());
        for(size_t i = 0; i < 10; ++i)
        {
            v.push_back(int(i*i));
        }
        SG_ASSERT(10 == v.size());
        SG_ASSERT(64 == v[8]);
    }

    {
        SG_ASSERT(0 == A::NbInstance());
        MaxSizeVector<A, 10> v;
        SG_ASSERT(v.empty());
        SG_ASSERT(0 == v.size());
        v.emplace_back();
        SG_ASSERT(1 == A::NbInstance());
        SG_ASSERT(!v.empty());
        SG_ASSERT(1 == v.size());
        SG_ASSERT(1 == v[0].a);
        SG_ASSERT(10 == v[0].b);
        v.emplace_back(2);
        SG_ASSERT(2 == v.size());
        SG_ASSERT(2 == v[1].a);
        SG_ASSERT(10 == v[1].b);
        v.emplace_back(3, 9);
        SG_ASSERT(3 == v.size());
        SG_ASSERT(3 == v[2].a);
        SG_ASSERT(9 == v[2].b);
        SG_ASSERT(3 == A::NbInstance());
        v.push_back(A(4, 16));
        SG_ASSERT(4 == v.size());
        SG_ASSERT(4 == v[3].a);
        SG_ASSERT(16 == v[3].b);
        SG_ASSERT(4 == A::NbInstance());
        v.clear();
        SG_ASSERT(0 == A::NbInstance());
        SG_ASSERT(v.empty());
        for(size_t i = 0; i < 10; ++i)
        {
            v.push_back(A(int(i), int(i*i)));
        }
        SG_ASSERT(10 == A::NbInstance());
        SG_ASSERT(10 == v.size());
        SG_ASSERT(8 == v[8].a);
        SG_ASSERT(64 == v[8].b);
        v.clear();
        SG_ASSERT(0 == A::NbInstance());
    }
}
//=============================================================================
}

#endif

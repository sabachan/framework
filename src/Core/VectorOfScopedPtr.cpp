#include "stdafx.h"

#include "VectorOfScopedPtr.h"

#if SG_ENABLE_UNIT_TESTS
#include "TestFramework.h"

namespace sg {
//=============================================================================
namespace testvectorofscopedptr {
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
SG_TEST((sg), VectorOfScopedPtr, (Core, quick))
{
    {
        using testvectorofscopedptr::A;
        VectorOfScopedPtr<A> v;
        SG_ASSERT(v.empty());
        SG_ASSERT(0 == v.size());
        v.push_back(new A);
        v.push_back(new A(2));
        v.push_back(new A(3, 20));
        SG_ASSERT(3 == A::NbInstance());

        {
            VectorOfScopedPtr<A> v2 = std::move(v);
            v2.push_back(new A(4));
            SG_ASSERT(4 == A::NbInstance());
            v.resize(10);
            for_range(size_t, i, 0, 10)
                v.reset(i, new A(int((i))));
            SG_ASSERT(14 == A::NbInstance());
            for_range(size_t, i, 0, 10)
                v.reset(i, new A(int(i+100)));
            SG_ASSERT(14 == A::NbInstance());
            int sum = 0;
            for(auto it : v)
                sum += it->a;
            SG_ASSERT(1045 == sum);
#if 0
            // All these should fail
            v[0] = new A;
            v.data()[0] = new A;
            v.front() = new A;
            v.back() = new A;
            for(auto& it : v)
                it = new A;
#endif
            v = std::move(v2);
            SG_ASSERT(4 == A::NbInstance());
        }
        v.clear();
        SG_ASSERT(0 == A::NbInstance());
    }
//=============================================================================
}
}
#endif

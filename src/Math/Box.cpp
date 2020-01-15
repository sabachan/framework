#include "stdafx.h"

#include "Box.h"

#include <Core/Config.h>

#if SG_ENABLE_UNIT_TESTS
#include <Core/Assert.h>
#include <Core/TestFramework.h>

namespace sg {
namespace math {
//=============================================================================
SG_TEST((sg, math), Box, (Math, quick))
{
    {
        box1i b;
        b.Grow(3);
        b.Grow(7);
        SG_ASSERT(4 == b.Delta());
        SG_ASSERT(5 == b.Center());
        b.Grow(8);
        SG_ASSERT(5 == b.Center());
        b.Grow(2);
        SG_ASSERT(5 == b.Center());
        b.Grow(1);
        SG_ASSERT(4 == b.Center());
        box1i b2(uninitialized);
        b2 = box1i::FromMinDelta(-1, 5);
        b = Intersection(b, b2);
        SG_ASSERT(1 == b.Min());
        SG_ASSERT(4 == b.Max());
        b = Intersection(box1i::FromMinDelta(2, 7), box1i::FromMaxDelta(12, 5));
        SG_ASSERT(box1i::FromMinMax(7, 9) == b);
        box1u bu = box1u(b);
        SG_ASSERT(box1u::FromMinMax(7, 9) == bu);
        b = box1i::FromMaxDelta(5, 13);
        bu = box1u(b);
        SG_ASSERT(5 == b.Max());
        SG_ASSERT(u32(-8) == b.Min());
    }
    {
        box2f b;
        b.Grow(float2(1, 5));
        b.Grow(float2(3, 1));
        SG_ASSERT(float2(2, 4) == b.Delta());
        SG_ASSERT(float2(2, 3) == b.Center());
        box2f b2 = box2f::FromMinDelta(float2(2, -1), float2(5, 5));
        b = Intersection(b, b2);
        SG_ASSERT(float2(2, 1) == b.Min());
        SG_ASSERT(float2(3, 4) == b.Max());
        SG_ASSERT(b.Contains(float2(2.5f, 3.f)));
        SG_ASSERT(b.ContainsStrict(float2(2.5f, 3.f)));
        SG_ASSERT(b.Contains(float2(2.f, 3.f)));
        SG_ASSERT(!b.ContainsStrict(float2(2.f, 3.f)));
        SG_ASSERT(b.Contains(float2(2.5f, 4.f)));
        SG_ASSERT(!b.ContainsStrict(float2(2.5f, 4.f)));
        SG_ASSERT(!b.Contains(float2(1.f, 3.f)));
        SG_ASSERT(!b.ContainsStrict(float2(1.f, 3.f)));
        SG_ASSERT(!b.Contains(float2(4.f, 3.f)));
        SG_ASSERT(!b.ContainsStrict(float2(4.f, 3.f)));
        SG_ASSERT(!b.Contains(float2(2.5f, 0.f)));
        SG_ASSERT(!b.ContainsStrict(float2(2.5f, 0.f)));
        SG_ASSERT(!b.Contains(float2(2.5f, 5.f)));
        SG_ASSERT(!b.ContainsStrict(float2(2.5f, 5.f)));
    }
    {
        box2f b1 = box2f::FromMinMax(float2(2, -1), float2(5, 5));
        box2f b2 = box2f::FromMinMax(float2(3, 6), float2(4, 7));
        SG_ASSERT(!IntersectOrTouch(b1, b2));
        SG_ASSERT(!IntersectStrict(b1, b2));
        b2 = box2f::FromMinMax(float2(5, 3), float2(6, 6));
        SG_ASSERT(IntersectOrTouch(b1, b2));
        SG_ASSERT(!IntersectStrict(b1, b2));
        b2 = box2f::FromMinMax(float2(-1, 0), float2(1, 7));
        SG_ASSERT(!IntersectOrTouch(b1, b2));
        SG_ASSERT(!IntersectStrict(b1, b2));
        b2 = box2f::FromMinMax(float2(3, 4), float2(4, 7));
        SG_ASSERT(IntersectOrTouch(b1, b2));
        SG_ASSERT(IntersectStrict(b1, b2));
    }
    {
        box2i const box = box2i::FromMinMax(int2(10,20),int2(30,40));
        SG_ASSERT(box.Corner(BitSet<2>(0x0)) == int2(10,20));
        SG_ASSERT(box.Corner(BitSet<2>(0x1)) == int2(30,20));
        SG_ASSERT(box.Corner(BitSet<2>(0x2)) == int2(10,40));
        SG_ASSERT(box.Corner(BitSet<2>(0x3)) == int2(30,40));
    }
    {
        typedef Vector<int, 100> int100;
        typedef Box<int, 100> box100i;
        box100i const box = box100i::FromMinMax(int100(10),int100(20));
        BitSet<100> cornerDesc;
        SG_ASSERT(box.Corner(cornerDesc) == int100(10));
        cornerDesc.set(75);
        SG_ASSERT(box.Corner(cornerDesc)[74] == 10);
        SG_ASSERT(box.Corner(cornerDesc)[75] == 20);
        SG_ASSERT(box.Corner(cornerDesc)[76] == 10);
    }
    {
        box1i const a = box1i::FromMinMax(0, 10);
        box1i const b = box1i::FromMinMax(5, 6);
        box2i const c = box2i::FromMinMax(int2(1,2), int2(7,3));
        box4i const d = box4i::FromMinMax(int4(0,5,1,2), int4(10,6,7,3));
        box4i e = CartesianProduct(a, b, c);
        SG_ASSERT(d == e);
        box1i const f = d.SubBox<1>(1);
        SG_ASSERT(b == f);
        box2i const g = e.SubBox<2>(2);
        SG_ASSERT(c == g);
        e.SetSubBox(2,a);
        SG_ASSERT(int4(0,5,0,2) == e.Min());
        SG_ASSERT(int4(10,6,10,3) == e.Max());
        e.SetSubBox(2,c);
        SG_ASSERT(d == e);
    }
    {
        box2f b1 = box2f::FromMinMax(float2(2, -1), float2(5, 5));
        box2f b2 = b1 + float2(10, -20);
        //SG_ASSERT(EqualsWithToleranceb2)
    }
}
//=============================================================================
}
}
#endif


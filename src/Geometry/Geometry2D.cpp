#include "stdafx.h"

#include "Geometry2D.h"

#include <Core/Assert.h>
#include <Core/Config.h>
#include <Core/TestFramework.h>
#include <Image/DebugImage.h>

#define SG_ENABLE_GEOMETRY_TEST_DEBUG_IMAGE 1

#if SG_ENABLE_UNIT_TESTS
namespace sg {
namespace geometry {
//=============================================================================
namespace {
void TestSegmentToSegment(Point2D const& A, Point2D const& B, Point2D const& C, Point2D const& D, bool ref, float2 const& refP)
{
    bool const t = SegmentToSegment<Intersect2DMode::TestOnly>(A, B, C, D);
    SG_ASSERT_AND_UNUSED(t == ref);
    auto const r0 = SegmentToSegment<Intersect2DMode::LP_Position>(A, B, C, D);
    SG_ASSERT_AND_UNUSED(r0.first == ref);
    auto const r1 = SegmentToSegment<Intersect2DMode::HP_Position>(A, B, C, D);
    SG_ASSERT_AND_UNUSED(r1.first == ref);
    if(ref)
    {
        SG_ASSERT_AND_UNUSED(EqualsWithTolerance(refP, r0.second, 0.001f));
        SG_ASSERT_AND_UNUSED(EqualsWithTolerance(refP, r1.second, 0.001f));
    }
#if SG_ENABLE_GEOMETRY_TEST_DEBUG_IMAGE
    auto brush = image::brush::Blend<image::blend::PremultipliedRGBA>().Monospace();
    size_t const dbgSize = 1 << 9;
    image::RGBImage dbgImg((uint2(dbgSize)));
    dbgImg.Fill(ubyte3(0));
    box2f bbox;
    bbox.Grow(A, B, C, D);
    image::WindowToViewport toDbgFrame(bbox, box2f::FromCenterDelta(0.5f * dbgImg.WidthHeight(), 0.96f * dbgImg.WidthHeight()));
    image::DrawLine1pxNoAA(dbgImg, roundi(toDbgFrame(A)), roundi(toDbgFrame(B)), brush.Stroke(ubyte4(128, 128, 128, 0), 1));
    image::DrawLine1pxNoAA(dbgImg, roundi(toDbgFrame(C)), roundi(toDbgFrame(D)), brush.Stroke(ubyte4(128, 128, 128, 0), 1));

    image::DrawRect(dbgImg, box2f::FromCenterDelta(toDbgFrame(A), float2(3.f)), brush.Stroke(ubyte4(0,0,255, 0), 1));
    image::DrawRect(dbgImg, box2f::FromCenterDelta(toDbgFrame(C), float2(3.f)), brush.Stroke(ubyte4(0,0,255, 0), 1));

    if(r0.first)
        image::DrawRect(dbgImg, box2f::FromCenterDelta(toDbgFrame(r0.second), float2(3.f)), brush.Stroke(ubyte4(255,0,0, 0), 1));
    if(r1.first)
        image::DrawRect(dbgImg, box2f::FromCenterDelta(toDbgFrame(r1.second), float2(3.f)), brush.Stroke(ubyte4(0,255,0, 0), 1));
#endif
}
//=============================================================================
void TestLineToCircle(Point2D const& A, Point2D const& B, Point2D const& C, float R, bool ref, float2 const& refP0, float2 const& refP1)
{
    SG_UNUSED((refP0, refP1));
    bool const t = LineToCircle<Intersect2DMode::TestOnly>(A, B, C, R);
    SG_ASSERT(t == ref);
    auto const r0 = LineToCircle<Intersect2DMode::LP_Position>(A, B, C, R);
    SG_ASSERT(r0.first == ref);
    //auto const r1 = LineToCircle<Intersect2DMode::HP_Position>(A, B, C, R);
    //SG_ASSERT(r1.first == ref);
    if(ref)
    {
        SG_ASSERT(EqualsWithTolerance(refP0, r0.second.first, 0.001f) || EqualsWithTolerance(refP0, r0.second.second, 0.001f));
        SG_ASSERT(EqualsWithTolerance(refP1, r0.second.first, 0.001f) || EqualsWithTolerance(refP1, r0.second.second, 0.001f));
        //SG_ASSERT(EqualsWithTolerance(refP0, r1.second.first, 0.001f) || EqualsWithTolerance(refP0, r1.second.second, 0.001f));
        //SG_ASSERT(EqualsWithTolerance(refP1, r1.second.first, 0.001f) || EqualsWithTolerance(refP1, r1.second.second, 0.001f));
    }
#if SG_ENABLE_GEOMETRY_TEST_DEBUG_IMAGE
    auto brush = image::brush::Blend<image::blend::PremultipliedRGBA>().Monospace();
    size_t const dbgSize = 1 << 9;
    image::RGBImage dbgImg((uint2(dbgSize)));
    dbgImg.Fill(ubyte3(0));
    box2f bbox;
    bbox.Grow(A, B, box2f::FromCenterDelta(C, float2(2*R)));
    image::WindowToViewport toDbgFrame(bbox, box2f::FromCenterDelta(0.5f * dbgImg.WidthHeight(), 0.96f * dbgImg.WidthHeight()));
    box2f viewport = box2f::FromMinMax(float2(0), float2(dbgImg.WidthHeight()));
    float2 const dbgA = toDbgFrame(A);
    float2 const dbgB = toDbgFrame(B);
    float2 const AB = dbgB-dbgA;
    float2 const signAB = float2(AB.x() > 0 ? 1.f : -1.f, AB.y() > 0 ? 1.f : -1.f);
    float2 const tA2f = (viewport.Center() - 0.5f * signAB * viewport.Delta() - dbgA) / AB;
    float const tA = std::max(tA2f.x(), tA2f.y());
    float2 Ab = lerp(dbgA, dbgB, tA);
    float2 const tB2f = (viewport.Center() + 0.5f * signAB * viewport.Delta() - dbgA) / AB;
    float const tB = std::min(tB2f.x(), tB2f.y());
    float2 Bb = lerp(dbgA, dbgB, tB);
    image::DrawLine1pxNoAA(dbgImg, roundi(Ab), roundi(Bb), brush.Stroke(ubyte4(128, 128, 128, 0), 1));
    image::DrawRect(dbgImg, box2f::FromCenterDelta(dbgA, float2(3.f)), brush.Stroke(ubyte4(0,0,255, 0), 1));
    image::DrawRect(dbgImg, box2f::FromCenterDelta(dbgB, float2(3.f)), brush.Stroke(ubyte4(0,0,255, 0), 1));

    float const dbgR = toDbgFrame.Rescale(float2(R)).x();
    image::DrawCircle1pxNoAA(dbgImg, roundi(toDbgFrame(C)), roundi(dbgR), brush.Stroke(ubyte4(128, 128, 128, 0), 1.f));

    if(r0.first)
    {
        image::DrawRect(dbgImg, box2f::FromCenterDelta(toDbgFrame(r0.second.first), float2(3.f)), brush.Stroke(ubyte4(255,0,0, 0), 1));
        image::DrawRect(dbgImg, box2f::FromCenterDelta(toDbgFrame(r0.second.second), float2(3.f)), brush.Stroke(ubyte4(255,0,0, 0), 1));
    }
    //if(r1.first)
    //{
    //    image::DrawRect(dbgImg, box2f::FromCenterDelta(toDbgFrame(r1.second.first), float2(3.f)), brush.Stroke(ubyte4(255,0,0, 0), 1));
    //    image::DrawRect(dbgImg, box2f::FromCenterDelta(toDbgFrame(r1.second.second), float2(3.f)), brush.Stroke(ubyte4(255,0,0, 0), 1));
    //}
#endif
}
//=============================================================================
void TestCircleToCircle(Point2D const& C0, float R0, Point2D const& C1, float R1, bool ref, float2 const& refP0, float2 const& refP1)
{
    bool const t = CircleToCircle<Intersect2DMode::TestOnly>(C0, R0, C1, R1);
    SG_ASSERT_AND_UNUSED(t == ref);
    auto const r0 = CircleToCircle<Intersect2DMode::LP_Position>(C0, R0, C1, R1);
    SG_ASSERT_AND_UNUSED(r0.first == ref);
    //auto const r1 = CircleToCircle<Intersect2DMode::HP_Position>(C0, R0, C1, R1);
    //SG_ASSERT(r1.first == ref);
    if(ref)
    {
        SG_ASSERT_AND_UNUSED(EqualsWithTolerance(refP0, r0.second.first, 0.001f) || EqualsWithTolerance(refP0, r0.second.second, 0.001f));
        SG_ASSERT_AND_UNUSED(EqualsWithTolerance(refP1, r0.second.first, 0.001f) || EqualsWithTolerance(refP1, r0.second.second, 0.001f));
        //SG_ASSERT(EqualsWithTolerance(refP0, r1.second.first, 0.001f) || EqualsWithTolerance(refP0, r1.second.second, 0.001f));
        //SG_ASSERT(EqualsWithTolerance(refP1, r1.second.first, 0.001f) || EqualsWithTolerance(refP1, r1.second.second, 0.001f));
    }
#if SG_ENABLE_GEOMETRY_TEST_DEBUG_IMAGE
    auto brush = image::brush::Blend<image::blend::PremultipliedRGBA>().Monospace();
    size_t const dbgSize = 1 << 9;
    image::RGBImage dbgImg((uint2(dbgSize)));
    dbgImg.Fill(ubyte3(0));
    box2f bbox;
    bbox.Grow(box2f::FromCenterDelta(C0, float2(2*R0)), box2f::FromCenterDelta(C1, float2(2*R1)));
    image::WindowToViewport toDbgFrame(bbox, box2f::FromCenterDelta(0.5f * dbgImg.WidthHeight(), 0.96f * dbgImg.WidthHeight()));

    float const dbgR0 = toDbgFrame.Rescale(float2(R0)).x();
    image::DrawCircle1pxNoAA(dbgImg, roundi(toDbgFrame(C0)), roundi(dbgR0), brush.Stroke(ubyte4(128, 128, 128, 0), 1.f));
    float const dbgR1 = toDbgFrame.Rescale(float2(R1)).x();
    image::DrawCircle1pxNoAA(dbgImg, roundi(toDbgFrame(C1)), roundi(dbgR1), brush.Stroke(ubyte4(128, 128, 128, 0), 1.f));

    if(r0.first)
    {
        image::DrawRect(dbgImg, box2f::FromCenterDelta(toDbgFrame(r0.second.first), float2(3.f)), brush.Stroke(ubyte4(255,0,0, 0), 1));
        image::DrawRect(dbgImg, box2f::FromCenterDelta(toDbgFrame(r0.second.second), float2(3.f)), brush.Stroke(ubyte4(255,0,0, 0), 1));
    }
    //if(r1.first)
    //{
    //    image::DrawRect(dbgImg, box2f::FromCenterDelta(toDbgFrame(r1.second.first), float2(3.f)), brush.Stroke(ubyte4(255,0,0, 0), 1));
    //    image::DrawRect(dbgImg, box2f::FromCenterDelta(toDbgFrame(r1.second.second), float2(3.f)), brush.Stroke(ubyte4(255,0,0, 0), 1));
    //}
#endif
}
}
//=============================================================================
SG_TEST((sg, geometry), Geometry2D, (Geometry, quick))
{
    {
        float2 const A(0,0), B(3,18), C(15,6), D(9,18), P(7,14);
        TestSegmentToSegment(Point(A), Point(B), Point(C), Point(D), false, P);
        TestSegmentToSegment(Point(B), Point(A), Point(C), Point(D), false, P);
        TestSegmentToSegment(Point(A), Point(C), Point(D), Point(B), false, P);
        TestSegmentToSegment(Point(A), Point(D), Point(C), Point(B), true, P);
        TestSegmentToSegment(Point(A), Point(D), Point(B), Point(C), true, P);
    }

    {
        TestLineToCircle(Point(float2(0, 10)),  Point(float2(9, 2)),   Point(float2(1, 3)),   4.f,  false, float2(0, 0), float2(0, 0));
        TestLineToCircle(Point(float2(0, 10)),  Point(float2(9, 2)),   Point(float2(1, 3)),   5.f,  true,  float2(5.55485535f, 5.06235075f), float2(2.51410985f, 7.76523590f));
        TestLineToCircle(Point(float2(0, 10)),  Point(float2(9, 2)),   Point(float2(1, 3)),   20.f, true,  float2(18.5876350f, -6.52234268f), float2(-10.5186710f, 19.3499298f));
        TestLineToCircle(Point(float2(9, 2)),   Point(float2(0, 10)),  Point(float2(1, 3)),   4.f,  false, float2(0, 0), float2(0, 0));
        TestLineToCircle(Point(float2(9, 2)),   Point(float2(0, 10)),  Point(float2(1, 3)),   5.f,  true,  float2(5.55485535f, 5.06235075f), float2(2.51410985f, 7.76523590f));
        TestLineToCircle(Point(float2(9, 2)),   Point(float2(0, 10)),  Point(float2(1, 3)),   20.f, true,  float2(18.5876350f, -6.52234268f), float2(-10.5186710f, 19.3499298f));
        TestLineToCircle(Point(float2(-5, 10)), Point(float2(-2, 9)),  Point(float2(10, 3)),  1.f,  false, float2(0, 0), float2(0, 0));
        TestLineToCircle(Point(float2(-5, 10)), Point(float2(-2, 9)),  Point(float2(10, 3)),  5.f,  true,  float2(14.9886208f, 3.33712626f), float2(6.21137810f, 6.26287365f));
        TestLineToCircle(Point(float2(-5, 10)), Point(float2(-2, 9)),  Point(float2(10, 3)),  20.f, true,  float2(29.4880905f, -1.49603081f), float2(-8.28809166f, 11.0960302f));
        TestLineToCircle(Point(float2(-2, 9)),  Point(float2(-5, 10)),  Point(float2(10, 3)), 1.f,  false, float2(0, 0), float2(0, 0));
        TestLineToCircle(Point(float2(-2, 9)),  Point(float2(-5, 10)), Point(float2(10, 3)),  5.f,  true,  float2(14.9886208f, 3.33712626f), float2(6.21137810f, 6.26287365f));
        TestLineToCircle(Point(float2(-2, 9)),  Point(float2(-5, 10)), Point(float2(10, 3)),  20.f, true,  float2(29.4880905f, -1.49603081f), float2(-8.28809166f, 11.0960302f));
    }
    {
        TestCircleToCircle(Point(float2(3, 10)), 2.f,  Point(float2(9, 3)), 4.f,  false, float2(0, 0), float2(0, 0));
        TestCircleToCircle(Point(float2(3, 10)), 4.f,  Point(float2(9, 3)), 4.f,  false, float2(0, 0), float2(0, 0));
        TestCircleToCircle(Point(float2(3, 10)), 6.f,  Point(float2(9, 3)), 4.f,  true,  float2(8.14116478f, 6.90671253f), float2(5.27059984f, 4.44622898f));
        TestCircleToCircle(Point(float2(3, 10)), 10.f, Point(float2(9, 3)), 4.f,  true,  float2(12.0014534f, 5.64410353f), float2(5.92795753f, 0.438250065f));
        TestCircleToCircle(Point(float2(3, 10)), 13.f, Point(float2(9, 3)), 4.f,  true,  float2(12.5762348f, 1.20820189f), float2(10.2237644f, -0.808202267f));
        TestCircleToCircle(Point(float2(3, 10)), 14.f, Point(float2(9, 3)), 4.f,  false, float2(0, 0), float2(0, 0));
        TestCircleToCircle(Point(float2(3, 10)), 5.f,  Point(float2(9, 3)), 5.f,  true,  float2(7.47029448f, 7.76025200f), float2(4.52970552f, 5.23974800f));
        TestCircleToCircle(Point(float2(3, 10)), 50.f, Point(float2(9, 3)), 51.f, true,  float2(40.3924103f, 43.1934891f), float2(-35.5218201f, -21.8758450f));
        TestCircleToCircle(Point(float2(3, 10)),250.f, Point(float2(9, 3)),251.f, true,  float2(177.357315f, 189.163406f), float2(-200.722031f, -134.904587f));
    }
}
//=============================================================================
}
}
#endif


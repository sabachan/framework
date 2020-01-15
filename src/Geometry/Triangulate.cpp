#include "stdafx.h"

#include "Triangulate.h"

#include "Geometry2D.h"

#if SG_ENABLE_UNIT_TESTS
#include "PolygonUtils.h"
#include <Image/DebugImage.h>
#include <Core/DynamicBitSet.h>
#include <Core/FileSystem.h>
#include <Core/PerfLog.h>
#include <Core/TestFramework.h>
#endif

#if SG_ENABLE_ASSERT
#define SG_TRIANGULATE_ENABLE_DEBUG_IMAGE 0
#else
#define SG_TRIANGULATE_ENABLE_DEBUG_IMAGE 0
#endif

#if SG_TRIANGULATE_ENABLE_DEBUG_IMAGE || SG_ENABLE_UNIT_TESTS
#include <Core/StringFormat.h>
#endif

namespace sg {
namespace geometry {
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TriangulateReturnCode TriangulateBySimpleEarClipping_NoHole(
    ArrayView<float2 const> iVertices,
    ArrayView<size_t const> iOutlines,
    ArrayView<size_t> ioTriangles,
    TriangulateParameters const& iParameters)
{
    SG_UNUSED(iOutlines);
    SG_UNUSED(iParameters);

    SG_ASSERT(1 == iOutlines.size());
    SG_ASSERT(iVertices.size() == iOutlines[0]);
    size_t const N = iVertices.size();
    SG_ASSERT(2 < N);
    SG_ASSERT(3*(N-2)  == ioTriangles.size());
    DynamicBitSet used(N);

#if SG_TRIANGULATE_ENABLE_DEBUG_IMAGE
    size_t const dbgSize = 1 << 10;
    image::RGBImage dbgImg((uint2(dbgSize)));
    auto const brush = image::CreateBrush();
    dbgImg.Fill(ubyte3(0));

    box2f bbox;
    bbox.Grow(iVertices);
    image::WindowToViewport toDbgFrame(bbox, box2f::FromCenterDelta(0.5f * dbgImg.WidthHeight(), 0.96f * dbgImg.WidthHeight()));
    {
        float2 A = toDbgFrame(iVertices.back());
        for(auto const& p : iVertices)
        {
            float2 const B = toDbgFrame(p);
            image::DrawLine1pxNoAA(dbgImg, roundi(A), roundi(B), image::brush::Stroke(ubyte3(96)));
            //image::DrawText(dbgImg, roundi(B) + float2(1,0), Format("%0", size_t(&p - iVertices.data())), brush.Fill(ubyte3(32)));
            A = B;
        }
    }
#endif

    struct PriorityEar
    {
        size_t iA, iB, iC;
        float priority;
    };

    auto IsValidEarAndUpdate = [&iVertices, &used, N](PriorityEar& ear)
    {
        float2 const A = iVertices[ear.iA];
        float2 const B = iVertices[ear.iB];
        float2 const C = iVertices[ear.iC];
        SG_ASSERT(!used[ear.iA] && !used[ear.iB] && !used[ear.iC]);
        float2 const AB = B-A;
        float2 const BC = C-B;
        float2 const CA = A-C;
        float detABC = det(AB, BC);
        if(detABC <= 0)
            return false;

        // Note that it is useless to test directly previous and next segments.
        if(ear.iA < ear.iC)
        {
            float2 D = iVertices[0];
            for_range(size_t, iE, 1, ear.iA)
            {
                float2 const E = iVertices[iE];
                bool const intersect = SegmentToSegment<Intersect2DMode::TestOnly>(Point(A), Point(C), Point(D), Point(E));
                if(intersect)
                    return false;
                D = E;
            }
            if(ear.iC + 1 < N)
            {
                D = iVertices[ear.iC+1];
                for_range(size_t, iE, ear.iC+2, N)
                {
                    float2 const E = iVertices[iE];
                    bool const intersect = SegmentToSegment<Intersect2DMode::TestOnly>(Point(A), Point(C), Point(D), Point(E));
                    if(intersect)
                        return false;
                    D = E;
                }
            }
            if(0 < ear.iA && ear.iC+1 < N)
            {
                D = iVertices.back();
                float2 const E = iVertices[0];
                bool const intersect = SegmentToSegment<Intersect2DMode::TestOnly>(Point(A), Point(C), Point(D), Point(E));
                if(intersect)
                    return false;
            }
        }
        else
        {
            float2 D = iVertices[ear.iC+1];
            for_range(size_t, iE, ear.iC+2, ear.iA)
            {
                float2 const E = iVertices[iE];
                bool const intersect = SegmentToSegment<Intersect2DMode::TestOnly>(Point(A), Point(C), Point(D), Point(E));
                if(intersect)
                    return false;
                D = E;
            }
        }

        float2 const vs[] = { A, B, C };
        float const dsq[] = { BC.LengthSq(), CA.LengthSq(), AB.LengthSq() };
        size_t const j = dsq[0] >= dsq[1] ? dsq[0] >= dsq[2] ? 0 : 2 : dsq[1] >= dsq[2] ? 1 : 2;
        float const hd = det(vs[(j+2)%3]-vs[(j+1)%3], vs[j]-vs[(j+1)%3]);
#if 0
        // more regular triangle first
        ear.priority = dsq[j] / hd;
#elif 0
        // smallest longest edge triangle first
        ear.priority = dsq[j];
#else
        // smallest longest edge triangle first, with penalty if too much flattened
        float flattenedFactor = dsq[j] / hd;
        ear.priority = dsq[j] * std::max(flattenedFactor - 100.f, 1.f);
#endif
        return true;
    };

    auto Comp = [](PriorityEar const& a, PriorityEar const& b) { return a.priority > b.priority; };
    std::vector<PriorityEar> heap;
    heap.reserve(3*N);

    auto PushEarIfValid = [&](PriorityEar& ear)
    {
        ear.priority = 0.f;
        bool isValid = IsValidEarAndUpdate(ear);

        if(isValid)
        {
#if 0 //SG_TRIANGULATE_ENABLE_DEBUG_IMAGE
            float2 const A = toDbgFrame(iVertices[ear.iA]);
            float2 const C = toDbgFrame(iVertices[ear.iC]);
            image::DrawLine1pxNoAA(dbgImg, roundi(A), roundi(C), image::brush::Stroke(ubyte3(16)));
            image::DrawText(dbgImg, 0.5f*(A+C) + float2(0,0), Format("%0", ear.priority), brush.Fill(ubyte3(32)));
#endif
            heap.push_back(ear);
            std::push_heap(heap.begin(), heap.end(), Comp);
        }
    };

    {
        PriorityEar ear;
        ear.iA = N-2;
        ear.iB = N-1;
        ear.iC = 0;
        for(; ear.iC < N; ++ear.iC)
        {
            PushEarIfValid(ear);

            ear.iA = ear.iB;
            ear.iB = ear.iC;
        }
    }

    size_t triangleCursor = 0;
    auto PushTriangle = [&triangleCursor, &ioTriangles](PriorityEar const& ear)
    {
        SG_ASSERT(triangleCursor+3 <= ioTriangles.size());
        ioTriangles[triangleCursor++] = ear.iA;
        ioTriangles[triangleCursor++] = ear.iB;
        ioTriangles[triangleCursor++] = ear.iC;
    };

    if(3 == N)
    {
        SG_ASSERT(!heap.empty());
        PriorityEar const& toTreat = heap.front();
        PushTriangle(toTreat);
        SG_ASSERT(triangleCursor == ioTriangles.size());
        return TriangulateReturnCode::Ok;
    }

    while(!heap.empty())
    {
        std::pop_heap(heap.begin(), heap.end(), Comp);
        PriorityEar const toTreat = heap.back();
        heap.pop_back();

        if(used[toTreat.iA] || used[toTreat.iB] || used[toTreat.iC])
            continue;
        PushTriangle(toTreat);
        used.set(toTreat.iB);

#if SG_TRIANGULATE_ENABLE_DEBUG_IMAGE
        {
            float2 const A = toDbgFrame(iVertices[toTreat.iA]);
            float2 const C = toDbgFrame(iVertices[toTreat.iC]);
            image::DrawLine1pxNoAA(dbgImg, roundi(A), roundi(C), image::brush::Stroke(ubyte3(16,42,16)));
        }
#endif

        size_t next = toTreat.iC + 1;
        for(;;)
        {
            if(N == next)
                next = 0;
            if(!used[next])
                break;
            ++next;
            SG_ASSERT(next != toTreat.iA);
        }
        size_t prev = toTreat.iA - 1;
        for(;;)
        {
            if(0 == prev+1)
                prev = N-1;
            if(!used[prev])
                break;
            --prev;
            SG_ASSERT(prev != toTreat.iC);
        }
        if(prev == next)
        {
            PriorityEar ear;
            ear.iA = toTreat.iA;
            ear.iB = toTreat.iC;
            ear.iC = next;
            PushTriangle(ear);
            break;
        }
        else
        {
            PriorityEar ear;
            ear.iA = toTreat.iA;
            ear.iB = toTreat.iC;
            ear.iC = next;
            PushEarIfValid(ear);
            ear.iA = prev;
            ear.iB = toTreat.iA;
            ear.iC = toTreat.iC;
            PushEarIfValid(ear);
        }
    }

    if(heap.empty())
        return TriangulateReturnCode::UnknownError;

    SG_ASSERT(triangleCursor == ioTriangles.size());
    return TriangulateReturnCode::Ok;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TriangulateReturnCode TriangulateBySimpleEarClipping(
    ArrayView<float2 const> iVertices,
    ArrayView<size_t const> iOutlines,
    ArrayView<size_t> ioTriangles,
    TriangulateParameters const& iParameters)
{
    SG_UNUSED((iVertices, iOutlines, ioTriangles, iParameters));
    SG_ASSERT_NOT_IMPLEMENTED();
    return TriangulateReturnCode::UnknownError;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TriangulateReturnCode TriangulateUsingMonotonPolygons(
    ArrayView<float2 const> iVertices,
    ArrayView<size_t const> iOutlines,
    ArrayView<size_t> ioTriangles,
    TriangulateParameters const& iParameters)
{
    SG_UNUSED((iVertices, iOutlines, ioTriangles, iParameters));
    SG_ASSERT_NOT_IMPLEMENTED();
    return TriangulateReturnCode::UnknownError;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TriangulateReturnCode TriangulateUsingTrapezoids(
    ArrayView<float2 const> iVertices,
    ArrayView<size_t const> iOutlines,
    ArrayView<size_t> ioTriangles,
    TriangulateParameters const& iParameters)
{
    SG_UNUSED((iVertices, iOutlines, ioTriangles, iParameters));
    SG_ASSERT_NOT_IMPLEMENTED();
    return TriangulateReturnCode::UnknownError;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TriangulateReturnCode TriangulateImpl(
    ArrayView<float2 const> iVertices,
    ArrayView<size_t const> iOutlines,
    ArrayView<size_t> ioTriangles,
    TriangulateParameters const& iParameters)
{
    if(iOutlines.size() == 1)
        return TriangulateBySimpleEarClipping_NoHole(iVertices, iOutlines, ioTriangles, iParameters);

    SG_ASSERT_NOT_IMPLEMENTED();
    return TriangulateReturnCode::UnknownError;
}
}
//=============================================================================
TriangulateReturnCode Triangulate(
    ArrayView<float2 const> iVertices,
    ArrayView<size_t const> iOutlines,
    ArrayView<size_t> ioTriangles,
    TriangulateParameters const* iOptionalParameters)
{
    if(nullptr != iOptionalParameters)
        return TriangulateImpl(iVertices, iOutlines, ioTriangles, *iOptionalParameters);
    TriangulateParameters param;
    param.epsilon = 0.f;
    return TriangulateImpl(iVertices, iOutlines, ioTriangles, param);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TriangulateReturnCode Triangulate(
    ArrayView<float2 const> iVertices,
    ArrayView<size_t> ioTriangles,
    TriangulateParameters const* iOptionalParameters)
{
    size_t const vertexCount = iVertices.size();
    return Triangulate(iVertices, AsArrayView(&vertexCount, 1), ioTriangles, iOptionalParameters);
}
//=============================================================================
}
}

#if SG_ENABLE_UNIT_TESTS
#if SG_ENABLE_ASSERT
#define SG_TEST_TRIANGULATE_ENABLE_DEBUG_IMAGE 1
#else
#define SG_TEST_TRIANGULATE_ENABLE_DEBUG_IMAGE 0
#endif
namespace sg {
namespace geometry {
namespace {
//=============================================================================
void GenerateTestPolygon(size_t i, std::vector<float2>& oPoints)
{
    std::vector<float2> pointbuffer;
    switch(i)
    {
    case 0: // regular star
    case 1: // regular Koch flake
    {
#if SG_CODE_IS_OPTIMIZED
        size_t const pointCount = 0==i ? 12 : 3*1 << 10;
#else
        size_t const pointCount = 0==i ? 12 : 3*1 << 6;
#endif
        pointbuffer.resize(pointCount);
        ArrayView<float2> points = AsArrayView(pointbuffer);

        {
            float const pi = float(math::constant::PI);
            float const pio3 = pi/3.f;
            float2 initPoints[] = {
                float2(0,1),
                float2(-sin(pio3), -cos(pio3)),
                float2(sin(pio3), -cos(pio3)),
            };
            float2 recursionPoints[] = { float2(1.f/3.f,0.f), float2(1.f/2.f,-1.f/3.f*sqrt(1.f-1.f/4.f)), float2(2.f/3.f,0.f) };
            FractalPolygonParameters::Recursion recursions[] = { { 1.f, 0, 3 } };
            FractalPolygonParameters p;
            p.initPoints = AsArrayView(initPoints);
            p.recursionPoints = AsArrayView(recursionPoints);
            p.recursions = AsArrayView(recursions);
            p.regularityFactor = 1.f;
            p.seed = 0xDEAFBABE;
            GenerateFractalPolygon(points, &p);
        }
        break;
    }
    case 2: // small random fractal
    {
        size_t const pointCount = 42;
        pointbuffer.resize(pointCount);
        ArrayView<float2> points = AsArrayView(pointbuffer);

        {
            float const pi = float(math::constant::PI);
            float const pio3 = pi/3.f;
            float2 initPoints[] = {
                float2(0,1),
                float2(-0.5f, -0.7f),
                float2(0.7f, -0.8f),
            };
            float2 recursionPoints[] = {
                float2(1.f/3.f,0.f), float2(1.f/2.f,-1.f/3.f*sqrt(1.f-1.f/4.f)), float2(2.f/3.f,0.f),
                float2(1.f/3.f,0.f), float2(1.f/2.f, 1.f/6.f*sqrt(1.f-1.f/4.f)), float2(2.f/3.f,0.f),
            };
            FractalPolygonParameters::Recursion recursions[] = { { 1.f, 0, 3 }, { 1.f, 3, 6 } };
            FractalPolygonParameters p;
            p.initPoints = AsArrayView(initPoints);
            p.recursionPoints = AsArrayView(recursionPoints);
            p.recursions = AsArrayView(recursions);
            p.regularityFactor = 0.f;
            p.seed = 0xDEAFBABE;
            GenerateFractalPolygon(points, &p);
        }
        break;
    }
    case 3: // large random fractal
    {
#if SG_CODE_IS_OPTIMIZED
        size_t const pointCount = 3*1 << 10;
#else
        size_t const pointCount = 3*1 << 6;
#endif
        pointbuffer.resize(pointCount);
        ArrayView<float2> points = AsArrayView(pointbuffer);

        {
            float const pi = float(math::constant::PI);
            float const pio3 = pi/3.f;
            float2 initPoints[] = {
                float2(0,1),
                float2(-sin(pio3), -cos(pio3)),
                float2(sin(pio3), -cos(pio3)),
            };
            float2 recursionPoints[] = {
                float2(1.f/3.f,0.f), float2(1.f/2.f,-1.f/3.f*sqrt(1.f-1.f/4.f)), float2(2.f/3.f,0.f),
                float2(1.f/3.f,0.f), float2(1.f/2.f, 1.f/6.f*sqrt(1.f-1.f/4.f)), float2(2.f/3.f,0.f),
            };
            FractalPolygonParameters::Recursion recursions[] = { { 1.f, 0, 3 }, { 1.f, 3, 6 } };
            FractalPolygonParameters p;
            p.initPoints = AsArrayView(initPoints);
            p.recursionPoints = AsArrayView(recursionPoints);
            p.recursions = AsArrayView(recursions);
            p.regularityFactor = 0.f;
            p.seed = 0xDEAFBABE;
            GenerateFractalPolygon(points, &p);
        }
        break;
    }
    case 4: // circle
    {
        size_t const pointCount = 1 << 8;
        float const pi = float(math::constant::PI);
        float const pi2oN = 2*pi/pointCount;
        pointbuffer.reserve(pointCount);
        for_range(size_t, j, 0, pointCount)
            pointbuffer.push_back(float2(cos(j * pi2oN), sin(j * pi2oN)));
        break;
    }
    default:
        SG_ASSERT_NOT_REACHED();
    }
    std::swap(pointbuffer, oPoints);
}
}
//=============================================================================
SG_TEST((sg, geometry), Triangulate, (Geometry, slow))
{
    filesystem::Init();
    filesystem::MountDeclaredMountingPoints();

#if SG_TEST_TRIANGULATE_ENABLE_DEBUG_IMAGE
    size_t const dbgSize = 1 << 10;
    image::RGBImage dbgImg((uint2(dbgSize)));
#endif
    for_range(size_t, i, 0, 5)
    {
        std::vector<float2> points;
        GenerateTestPolygon(i, points);

        std::vector<size_t> trianglebuffer(3*(points.size()-2));
        ArrayView<size_t> triangles = AsArrayView(trianglebuffer);
        {
            SG_SIMPLE_CPU_PERF_LOG_SCOPE("Triangulate");
            TriangulateParameters param;
            param.epsilon = 0.0001f;
            TriangulateReturnCode const rc = Triangulate(AsArrayView(points), triangles, &param);
            SG_ASSERT(TriangulateReturnCode::Ok == rc);
        }

#if SG_TEST_TRIANGULATE_ENABLE_DEBUG_IMAGE
        auto const brush = image::CreateBrush();
        dbgImg.Fill(ubyte3(0));
        box2f bbox;
        bbox.Grow(AsArrayView(points));
        image::WindowToViewport toDbgFrame(bbox, box2f::FromCenterDelta(0.5f * dbgImg.WidthHeight(), 0.96f * dbgImg.WidthHeight()));
        //{
        //    float2 A = toDbgFrame(points.back());
        //    for(auto const& p : points)
        //    {
        //        float2 const B = toDbgFrame(p);
        //        image::DrawLine1pxNoAA(dbgImg, roundi(A), roundi(B), image::brush::Stroke(ubyte3(64)));
        //        A = B;
        //    }
        //}
        size_t const N = triangles.size();
        SG_ASSERT(N % 3 == 0);
        for(size_t t = 0; t < N; t+=3)
        {
            float2 const A = toDbgFrame(points[triangles[t]]);
            float2 const B = toDbgFrame(points[triangles[t+1]]);
            float2 const C = toDbgFrame(points[triangles[t+2]]);
            image::DrawLine1pxNoAA(dbgImg, roundi(A), roundi(B), image::brush::Stroke(ubyte3(255)));
            image::DrawLine1pxNoAA(dbgImg, roundi(B), roundi(C), image::brush::Stroke(ubyte3(255)));
            image::DrawLine1pxNoAA(dbgImg, roundi(C), roundi(A), image::brush::Stroke(ubyte3(255)));
        }
#endif
    }

    filesystem::Shutdown();
}
//=============================================================================
}
}
#undef SG_TEST_TRIANGULATE_ENABLE_DEBUG_IMAGE
#endif

#undef SG_TRIANGULATE_ENABLE_DEBUG_IMAGE

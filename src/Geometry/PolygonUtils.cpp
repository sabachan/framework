#include "stdafx.h"

#include "PolygonUtils.h"
#include <Math/Matrix.h>
#include <Math/NumericalUtils.h>
#include <Core/For.h>
#include <random>

#if SG_ENABLE_UNIT_TESTS
#include <Image/DebugImage.h>
#include <Core/FileSystem.h>
#include <Core/PerfLog.h>
#include <Core/TestFramework.h>
#endif

namespace sg {
namespace geometry {
//=============================================================================
void GeneratePointCloud(ArrayView<float2>& oPoints, PointCloudParameters const* iOptionalParameters)
{
    PointCloudParameters params;
    if(nullptr != iOptionalParameters)
        params = *iOptionalParameters;
    else
    {
        std::random_device rd;
        params.distribution = PointCloudParameters::Uniform;
        params.radius = 1000.f;
        params.seed = rd();
    }

    size_t const N = oPoints.size();
    float radius = params.radius;
    std::mt19937 gen(unsigned int(params.seed));
    switch(params.distribution)
    {
    case PointCloudParameters::Uniform:
    {
        std::uniform_real_distribution<float> dis(-radius, radius);
        for(size_t i = 0; i < N; ++i)
        {
            float2 const pos(dis(gen), dis(gen));
            oPoints[i] = pos;
        }
        break;
    }
    case PointCloudParameters::Ring:
    {
        float const sqRadius = sq(radius);
        float const relativeInnerRadius = 0.7f;
        std::uniform_real_distribution<float> disA(0, float(2*math::constant::PI));
        std::uniform_real_distribution<float> disSqR(sq(relativeInnerRadius), 1);
        for(size_t i = 0; i < N; ++i)
        {
            float const a = disA(gen) + i * 0.1f;
            float const sqR = disSqR(gen)*sqRadius;
            float const r = sqrt(sqR);
            float2 const pos = r * float2(cos(a), sin(a));
            oPoints[i] = pos;
        }
        break;
    }
    case PointCloudParameters::Disk:
    {
        float const sqRadius = sq(radius);
        std::uniform_real_distribution<float> disA(0, float(2*math::constant::PI));
        std::uniform_real_distribution<float> disSqR(0, 1);
        for(size_t i = 0; i < N; ++i)
        {
            float const a = disA(gen) + i * 0.1f;
            float const sqR = disSqR(gen) * sqRadius;
            float const r = sqrt(sqR);
            float2 const pos = r * float2(cos(a), sin(a));
            oPoints[i] = pos;
        }
        break;
    }
    case PointCloudParameters::GalacticCenter:
    {
        std::uniform_real_distribution<float> disA(0, float(2*math::constant::PI));
        std::uniform_real_distribution<float> disR(0, 1);
        for(size_t i = 0; i < N; ++i)
        {
            float const a = disA(gen) + i * 0.1f;
            float const r = sq(disR(gen)) * radius;
            float2 const pos = r * float2(cos(a), sin(a));
            oPoints[i] = pos;
        }
        break;
    }
    case PointCloudParameters::UniformBinaries:
    {
        SG_ASSERT(N%2 == 0);
        float const avgDist = radius / sqrt(float(N));
        float const maxBinaryRadius = 0.1f * avgDist;
        float const sqMaxBinaryRadius = sq(maxBinaryRadius);
        std::uniform_real_distribution<float> dis(-radius, radius);
        std::uniform_real_distribution<float> disA(0, float(2*math::constant::PI));
        std::uniform_real_distribution<float> disSqR(sq(0.1f), 1);
        for(size_t i = 0; i < N/2; ++i)
        {
            float2 const pos(dis(gen), dis(gen));
            oPoints[2*i] = pos;

            float const a = disA(gen) + i * 0.1f;
            float const sqR = disSqR(gen)*sqMaxBinaryRadius;
            float const r = sqrt(sqR);
            float2 const pos2 = pos + r * float2(cos(a), sin(a));
            oPoints[2*i+1] = pos2;
        }
        break;
    }
    case PointCloudParameters::Clusters:
    {
        size_t const NClusters = roundi(sqrt(float(N)));
        float const avgClusterDist = radius / sqrt(float(NClusters));
        std::uniform_real_distribution<float> disA(0, float(2*math::constant::PI));
        std::uniform_real_distribution<float> disR(0, 1);
        std::uniform_real_distribution<float> disCR(0.2f, 3.f);
        size_t starCount = 0;
        for(size_t i = 0; i < NClusters; ++i)
        {
            float const ca = disA(gen) + i * 0.1f;
            float const cr = disR(gen) * radius;
            float2 const clusterPos = cr * float2(cos(ca), sin(ca));

            float const clusterRadius = disCR(gen) * avgClusterDist;
            size_t const clusterStarCount = (N-starCount) / (NClusters-i);
            for(size_t j = 0; j < clusterStarCount; ++j)
            {
                float const a = disA(gen) + i * 0.1f;
                float const r = disR(gen) * clusterRadius;
                float2 const pos = clusterPos + r * float2(cos(a), sin(a));
                oPoints[starCount++] = pos;
            }
        }
        break;
    }
    case PointCloudParameters::Spiral:
    {
        float const armThickness = 0.4f;
        float const rotation = 1.6f;
        float const contractionFactor = -0.5f;

        size_t const NClusters = roundi(sqrt(float(N)));
        float const avgClusterDist = radius / sqrt(float(NClusters));
        std::uniform_real_distribution<float> disA(0, float(2*math::constant::PI));
        std::uniform_real_distribution<float> disR(0, 1);
        std::uniform_real_distribution<float> disCR(0.2f, 3.f);
        size_t starCount = 0;
        for(size_t i = 0; i < NClusters; ++i)
        {
            float const ca = disA(gen) + i * 0.1f;
            float const cr = pow(disR(gen), 0.7f);
            float2 dir = float2(cos(ca), sin(ca));
            if(abs(dir.x()) > abs(dir.y())) dir.y() *= (abs(dir.x()) + armThickness) / (abs(dir.y()) + armThickness);
            else dir.x() *= (abs(dir.y()) + armThickness) / (abs(dir.x()) + armThickness);
            float const minDir = std::min(abs(dir.x()), abs(dir.y()));
            float const maxDir = std::max(abs(dir.x()), abs(dir.y()));
            float const b = minDir / (maxDir + 0.00001f);
            float const contraction = lerp(1.f - contractionFactor, 1.f, (1.f - maxDir) * b);
            float2 const preRotation = dir;
            float2x2 const rot = matrix::Rotation(1/(cr+0.1f) * rotation);
            float2 const clusterPos = rot * preRotation * radius * cr * contraction;

            float const clusterRadius = disCR(gen) * avgClusterDist;
            size_t const clusterStarCount = (N-starCount) / (NClusters-i);
            for(size_t j = 0; j < clusterStarCount; ++j)
            {
                float const a = disA(gen) + i * 0.1f;
                float const r = disR(gen) * clusterRadius;
                float2 const pos = clusterPos + r * float2(cos(a), sin(a));
                oPoints[starCount++] = pos;
            }
        }
        break;
    }
    default:
        SG_ASSERT_NOT_REACHED();
    }
}
//=============================================================================
namespace {
void GenerateFractalPolygonImpl(ArrayView<float2> const& oPoints, FractalPolygonParameters const& iParameters)
{
    std::mt19937 gen(unsigned int(iParameters.seed));
    auto ComputeDivisionIndices = [&gen, &iParameters](std::vector<size_t>& indexStack, size_t n, size_t leftIndex, size_t rightIndex)
    {
        size_t const intervalCount = n+1;
        float const ooIntervalCount = 1.f / intervalCount;
        size_t const N = rightIndex - leftIndex;
        float const regularIntervalLength = float(N) * ooIntervalCount;
        std::uniform_real_distribution<float> uniformDist(0.f, 1.f);

        size_t binomialMin = intervalCount <= N ? 1 : 0;
        size_t binomialIndex = rightIndex;
        size_t binomialN = N - binomialMin * intervalCount;
        for_range(size_t, i, 0, n)
        {
            float const regularIndexF = leftIndex + regularIntervalLength * (n-i);
            float const bernouilliP = 1.f/(intervalCount-i);
            std::binomial_distribution<size_t> binomialDist(binomialN, bernouilliP);
            size_t const binomialK = binomialDist(gen);
            SG_ASSERT(binomialK <= binomialN);
            binomialIndex = binomialIndex - binomialMin - binomialK;
            binomialN -= binomialK;
            SG_ASSERT(leftIndex <= binomialIndex && binomialIndex <= rightIndex);
            float const indexF = lerp(float(binomialIndex), regularIndexF, iParameters.regularityFactor);
            size_t const floorIndex = size_t(floor(indexF));
            float const fracIndex = indexF - floorIndex;
            float const uniform = uniformDist(gen);
            size_t index = floorIndex + (uniform < fracIndex ? 1 : 0);
            if(index == indexStack.back() && intervalCount <= N)
                index -= 1;
            SG_ASSERT(leftIndex <= index && index <= rightIndex);
            SG_ASSERT((leftIndex < index && index < rightIndex) || n >= N);
            SG_ASSERT(index <= indexStack.back());
            indexStack.push_back(index);
        }
    };

    float sumWeigth = 0;
    for(auto const& r : iParameters.recursions) { sumWeigth += r.weight; }
    std::uniform_real_distribution<float> recursionDist(0.f, sumWeigth);

    size_t const N = oPoints.size();
    std::vector<size_t> indexStack;
    indexStack.push_back(N);
    size_t currentIndex = 0;
    float2 currentPos = iParameters.initPoints[0];

    {
        size_t const lineCount = iParameters.initPoints.size();
        SG_ASSERT(3 <= lineCount);
        oPoints[0] = iParameters.initPoints[0];
        ComputeDivisionIndices(indexStack, lineCount-1, 0, N);
        size_t const stackSize = indexStack.size();
        for_range(size_t, i, 1, lineCount)
            oPoints[indexStack[stackSize - i]] = iParameters.initPoints[i];
    }

    while(!indexStack.empty())
    {
        size_t const top = indexStack.back();
        if(top <= currentIndex + 1)
        {
            currentIndex = top;
            indexStack.pop_back();
            continue;
        }
        float v = recursionDist(gen);
        for(auto const& r : iParameters.recursions)
        {
            if(v > r.weight)
            {
                v -= r.weight;
                continue;
            }
            size_t const recursionSize = r.end - r.begin;
            if((top - currentIndex) <= recursionSize && top != N)
            {
                currentIndex = currentIndex+1;
                oPoints[currentIndex] = oPoints[top];
                indexStack.pop_back();
                break;
            }
            float2 const A = oPoints[currentIndex];
            float2 const B = oPoints[top == N ? 0 : top];
            float2 const t = B-A;
            float2 const n = Orthogonal(t);
            ComputeDivisionIndices(indexStack, recursionSize, currentIndex, top);
            size_t const stackSize = indexStack.size();
            for_range(size_t, i, 0, recursionSize)
            {
                size_t const index = indexStack[stackSize - i - 1];
                if(index == currentIndex)
                    continue;
                if(index == top)
                    continue;
                float2 const p = iParameters.recursionPoints[r.begin + i];
                float2 const P = A + p.x() * t + p.y() * n;
                oPoints[index] = P;
            }
            break;
        }
    }
}
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void GenerateFractalPolygon(ArrayView<float2>& oPoints, FractalPolygonParameters const* iOptionalParameters)
{
    if(nullptr != iOptionalParameters)
    {
        GenerateFractalPolygonImpl(oPoints, *iOptionalParameters);
    }
    else
    {
        std::random_device rd;
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
        p.seed = rd();
        GenerateFractalPolygonImpl(oPoints, p);
    }
}
//=============================================================================
}
}

#if SG_ENABLE_UNIT_TESTS
#if SG_ENABLE_ASSERT
#define SG_TEST_POLYGON_UTILS_ENABLE_DEBUG_IMAGE 0
#else
#define SG_TEST_POLYGON_UTILS_ENABLE_DEBUG_IMAGE 0
#endif
namespace sg {
namespace geometry {
//=============================================================================
namespace {
void GenerateTestPointCloud(size_t i, std::vector<float2>& oPoints)
{
    std::vector<float2> pointbuffer;
    switch(i)
    {
    case 0:
    {
        size_t const pointCount = 1 << 12;
        pointbuffer.resize(pointCount);
        ArrayView<float2> points = AsArrayView(pointbuffer);
        PointCloudParameters param;
        param.radius = 10000.f;
        param.distribution = PointCloudParameters::Uniform;
        param.seed = 0;
        GeneratePointCloud(points, &param);
        break;
    }
    case 1:
    {
        size_t const pointCount = 1 << 12;
        pointbuffer.resize(pointCount);
        ArrayView<float2> points = AsArrayView(pointbuffer);
        PointCloudParameters param;
        param.radius = 10000.f;
        param.distribution = PointCloudParameters::Ring;
        param.seed = 0;
        GeneratePointCloud(points, &param);
        break;
    }
    case 2:
    {
        size_t const pointCount = 1 << 12;
        pointbuffer.resize(pointCount);
        ArrayView<float2> points = AsArrayView(pointbuffer);
        PointCloudParameters param;
        param.radius = 10000.f;
        param.distribution = PointCloudParameters::Disk;
        param.seed = 0;
        GeneratePointCloud(points, &param);
        break;
    }
    case 3:
    {
        size_t const pointCount = 1 << 12;
        pointbuffer.resize(pointCount);
        ArrayView<float2> points = AsArrayView(pointbuffer);
        PointCloudParameters param;
        param.radius = 10000.f;
        param.distribution = PointCloudParameters::GalacticCenter;
        param.seed = 0;
        GeneratePointCloud(points, &param);
        break;
    }
    case 4:
    {
        size_t const pointCount = 1 << 12;
        pointbuffer.resize(pointCount);
        ArrayView<float2> points = AsArrayView(pointbuffer);
        PointCloudParameters param;
        param.radius = 10000.f;
        param.distribution = PointCloudParameters::Clusters;
        param.seed = 0;
        GeneratePointCloud(points, &param);
        break;
    }
    case 5:
    {
        size_t const pointCount = 1 << 12;
        pointbuffer.resize(pointCount);
        ArrayView<float2> points = AsArrayView(pointbuffer);
        PointCloudParameters param;
        param.radius = 10000.f;
        param.distribution = PointCloudParameters::Spiral;
        param.seed = 0;
        GeneratePointCloud(points, &param);
        break;
    }
    default:
        SG_ASSERT_NOT_REACHED();
    }
    std::swap(pointbuffer, oPoints);
}
void GenerateTestPolygon(size_t i, std::vector<float2>& oPoints)
{
    std::vector<float2> pointbuffer;
    switch(i)
    {
    case 0:
    case 1:
    {
        size_t const pointCount = 0==i ? 12 : 3*1 << 14;
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
    case 2:
    {
        size_t const pointCount = 3*1 << 14;
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
    default:
        SG_ASSERT_NOT_REACHED();
    }
    std::swap(pointbuffer, oPoints);
}
void TestConvexHull(ArrayView<float2 const> points)
{
#if SG_TEST_POLYGON_UTILS_ENABLE_DEBUG_IMAGE
    size_t const dbgSize = 2048;
    typedef image::blend::BlendState<image::blend::One, image::blend::One, image::blend::Add> blendAdd;
#if SG_CODE_IS_OPTIMIZED
    auto const brush = image::brush::Blend<blendAdd>();
    ubyte3 const pointCol = ubyte3(5);
#else
    auto const brush = image::CreateBrush();
    ubyte3 const pointCol = ubyte3(255);
#endif
    image::RGBImage dbgImg((uint2(dbgSize)));
    dbgImg.Fill(ubyte3(0));
    box2f bbox;
    bbox.Grow(points);
    image::WindowToViewport toDbgFrame(bbox, box2f::FromCenterDelta(0.5f * dbgImg.WidthHeight(), 0.96f * dbgImg.WidthHeight()));
    for(auto const& p : points)
    {
        float2 const pf = toDbgFrame(p);
        image::DrawRect(dbgImg, box2f::FromCenterDelta(pf, float2(1)), brush.Fill(pointCol));
        //image::DrawText(dbgImg, pf + int2(3,0), Format("%0", &p - points.data()), image::brush::Fill(ubyte3(255,255,255)));
    }
#endif

    std::vector<size_t> convexHullIndices;
    ComputeConvexHull(convexHullIndices, points);

#if SG_TEST_POLYGON_UTILS_ENABLE_DEBUG_IMAGE
    {
        float2 A = toDbgFrame(points[convexHullIndices.back()]);
        for(size_t const i : convexHullIndices)
        {
            float2 const B = toDbgFrame(points[i]);
            image::DrawLine1pxNoAA(dbgImg, roundi(A), roundi(B), image::brush::Stroke(ubyte3(200,160,50)));
            A = B;
        }
    }
#endif

    std::vector<float2> pointbuffer2;
    pointbuffer2.reserve(convexHullIndices.size());
    for(size_t i : convexHullIndices)
        pointbuffer2.push_back(points[i]);
    ArrayView<float2> points2 = AsArrayView(pointbuffer2);

    std::vector<size_t> convexHullIndices2;
    ComputeConvexHull(convexHullIndices2, points2);

    if(convexHullIndices2.size() != points2.size())
    {
        size_t const size = points2.size();
        size_t i = convexHullIndices2.back();
        for(size_t const j : convexHullIndices2)
        {
            size_t im1 = i;
            i = (size-1 == i) ? 0 : i+1;
            while(i != j)
            {
                size_t const ip1 = (size-1 == i) ? 0 : i+1;
                float2 const A = points2[im1];
                float2 const B = points2[i];
                float2 const C = points2[ip1];
                float2 const AB = B-A;
                float2 const BC = C-B;
                float const detABC = det(AB.Normalised(), BC.Normalised());
                SG_ASSERT(-0.001f < detABC);
                im1 = i;
                i = ip1;
            }
        }
    }

#if SG_TEST_POLYGON_UTILS_ENABLE_DEBUG_IMAGE
    {
        float2 A = toDbgFrame(points2[convexHullIndices2.back()]);
        for(size_t const i : convexHullIndices2)
        {
            float2 const B = toDbgFrame(points2[i]);
            image::DrawLine1pxNoAA(dbgImg, roundi(A), roundi(B), image::brush::Stroke(ubyte3(160,200,50)));
            A = B;
        }
    }
#endif
}
void TestPolgon(ArrayView<float2 const> points)
{
#if SG_TEST_POLYGON_UTILS_ENABLE_DEBUG_IMAGE
    size_t const dbgSize = 1 << 10;

    auto const brush = image::CreateBrush();
    ubyte3 const pointCol = ubyte3(255);

    image::RGBImage dbgImg((uint2(dbgSize)));
    dbgImg.Fill(ubyte3(0));

    box2f bbox;
    bbox.Grow(points);
    image::WindowToViewport toDbgFrame(bbox, box2f::FromCenterDelta(0.5f * dbgImg.WidthHeight(), 0.96f * dbgImg.WidthHeight()));
    {
        float2 A = toDbgFrame(points.back());
        for(auto const& p : points)
        {
            float2 const B = toDbgFrame(p);
            image::DrawLine1pxNoAA(dbgImg, roundi(A), roundi(B), image::brush::Stroke(ubyte3(64)));
            A = B;
        }
    }
#endif

    TestConvexHull(points);

    float const refArea = ComputeArea(points);
    float area = 0;
    float2 areaCentroid;
    ComputeAreaAndCentroid(area, areaCentroid, points);
    SG_ASSERT(EqualsWithTolerance(area, refArea, 0.000001f));

    std::vector<float2> transformedPoints;
    transformedPoints.reserve(points.size());
    {
        float2 const t = float2(1.f, 2.f);
        for(float2 const& p : points)
            transformedPoints.push_back(p + t);
        float transformedArea = 0;
        float2 transformedAreaCentroid;
        ComputeAreaAndCentroid(transformedArea, transformedAreaCentroid, AsArrayView(transformedPoints));
        SG_ASSERT(EqualsWithTolerance(area, transformedArea, 0.0001f));
        SG_ASSERT(EqualsWithTolerance(areaCentroid + t, transformedAreaCentroid, 0.0001f));
    }
    transformedPoints.clear();
    {
        float const s = 2;
        for(float2 const& p : points)
            transformedPoints.push_back(p * s);
        float transformedArea = 0;
        float2 transformedAreaCentroid;
        ComputeAreaAndCentroid(transformedArea, transformedAreaCentroid, AsArrayView(transformedPoints));
        SG_ASSERT(EqualsWithTolerance(area * sq(s), transformedArea, 0.0001f));
        SG_ASSERT(EqualsWithTolerance(areaCentroid * s, transformedAreaCentroid, 0.0001f));
    }
}
}
//=============================================================================
SG_TEST((sg, geometry), PolygonUtils, (Geometry, quick))
{
    filesystem::Init();
    filesystem::MountDeclaredMountingPoints();

    {
        for_range(size_t, i, 0, 6)
        {
            SIMPLE_CPU_PERF_LOG_SCOPE("Test Convex Hull");
            std::vector<float2> points;
            GenerateTestPointCloud(i, points);
            TestConvexHull(AsArrayView(points));
        }

        for_range(size_t, i, 0, 3)
        {
            SIMPLE_CPU_PERF_LOG_SCOPE("Test Polygon");
            std::vector<float2> points;
            GenerateTestPolygon(i, points);
            TestPolgon(AsArrayView(points));
        }
    }

    filesystem::Shutdown();
}
//=============================================================================
}
}
#undef SG_TEST_POLYGON_UTILS_ENABLE_DEBUG_IMAGE
#endif


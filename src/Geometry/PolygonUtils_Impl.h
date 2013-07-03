#ifndef Geometry_PolygonUtils_Include_Impl
#error "this file must be used only by PolygonUtils.h"
#endif

namespace sg {
namespace geometry {
//=============================================================================
template <typename T>
float ComputeArea(ArrayView<math::Vector<T,2> const> iPoints)
{
    typedef math::Vector<T,2> vector_type;
    T preArea = T(0);
    vector_type A = iPoints.back();
    for(vector_type const B : iPoints)
    {
        preArea += det(A,B);
        A = B;
    }
    return 0.5f * preArea;
}
//=============================================================================
template <typename T>
void ComputeAreaAndCentroid(float& oArea, float2& oAreaCentroid, ArrayView<math::Vector<T,2> const> iPoints)
{
    typedef decltype(std::declval<T>() + std::declval<T>()) compute_type;
    typedef math::Vector<compute_type,2> vector_type;
    T preArea = T(0);
    float2 preCentroid = float2(0);
    vector_type A = iPoints.back();
    for(vector_type const B : iPoints)
    {
        compute_type const detAB = det(A,B);
        preArea += detAB;
        // Centroid of triangle is at 2/3 of a mediane (here, from 0 to middle
        // of AB). Constants will be applied at the end.
        preCentroid += detAB * (A + B);
        A = B;
    }
    oArea = 0.5f * preArea;
    oAreaCentroid = (1.f/6.f) * (1.f/oArea) * preCentroid;
}
//=============================================================================
template <typename T>
void ComputeConvexHull(std::vector<size_t>& oIndices, ArrayView<math::Vector<T,2> const> iPoints)
{
    typedef math::Vector<T,2> point_t;
    oIndices.clear();
    size_t const pointCount = iPoints.size();
    if(0 == pointCount)
        return;
    std::vector<point_t const*> hull;
    hull.reserve(pointCount);
    point_t const* const ppointsBegin = iPoints.data();
    point_t const* const ppointsEnd = ppointsBegin + pointCount;
    point_t const* ppoints = ppointsBegin;
    point_t const* left = ppoints;
    hull.push_back(ppoints++);
    while(ppoints != ppointsEnd)
    {
        if(ppoints->_[0] < left->_[0])
            left = ppoints;
        hull.push_back(ppoints++);
    }

    point_t const O = *left;
    size_t const leftIndex = left - ppointsBegin;
    oIndices.push_back(leftIndex);
    hull[leftIndex] = hull.back();
    hull.pop_back();
    sort(hull.begin(), hull.end(), [O](point_t const* A, point_t const* B)
    {
        point_t const OA = *A - O;
        point_t const OB = *B - O;
        return  OA.x() * OB.y() < OA.y() * OB.x();
    });

    oIndices.push_back(hull[0] - ppointsBegin);
    for_range(size_t, i, 1, hull.size())
    {
        float2 const* const pC = hull[i];
        size_t const indexC = pC - ppointsBegin;
        float2 const& C = *pC;
        float2 const OC = C-O;
        size_t size = oIndices.size();
        SG_ASSERT(size >= 2);
        do
        {
            size_t const indexA = oIndices[size - 2];
            size_t const indexB = oIndices[size - 1];
            float2 const& A = ppointsBegin[indexA];
            float2 const& B = ppointsBegin[indexB];
            float2 const BA = A-B;
            float2 const BC = C-B;
            float2 const OB = B-O;
            // This test is useful to reject most of the points, so that the
            // next test (the "det test") has less risk of errors (and, even in
            // this case, the errors would stay negligible).
            if(0 < dot(OB,BA) && 0 < dot(OB,BC))
               --size;
            else
            {
                // This is the important test, but which can fail due to
                // floating point errors for some points.
                float const detABBC = det(B-A, C-B);
                if(detABBC >= 0)
                    --size;
                else
                    break;
            }
        } while(size >= 2);
        SG_ASSERT(size >= 1);
        oIndices.resize(size);
        oIndices.push_back(indexC);
    }
}
//=============================================================================
}
}


#ifndef Geometry_PolygonUtils_H
#define Geometry_PolygonUtils_H

#include <Math/Box.h>
#include <Math/Vector.h>
#include <Core/Assert.h>
#include <Core/ArrayView.h>
#include <Core/Config.h>
#include <Core/Utils.h>
#include <type_traits>

namespace sg {
namespace geometry {
//=============================================================================
template <typename T> float ComputeArea(ArrayView<math::Vector<T,2> const> iPoints);
template <typename T> float ComputeArea(ArrayView<math::Vector<T,2>> iPoints) { return ComputeArea(iPoints.ConstView()); }
template <typename T> void ComputeAreaAndCentroid(float& oArea, float2& oAreaCentroid, ArrayView<math::Vector<T,2> const> iPoints);
template <typename T> void ComputeAreaAndCentroid(float& oArea, float2& oAreaCentroid, ArrayView<math::Vector<T,2>> iPoints) { return ComputeAreaAndCentroid(oArea, oAreaCentroid, iPoints.ConstView()); }
template <typename T> void ComputeConvexHull(std::vector<size_t>& oIndices, ArrayView<math::Vector<T,2> const> iPoints);
template <typename T> void ComputeConvexHull(std::vector<size_t>& oIndices, ArrayView<math::Vector<T,2>> iPoints) { return ComputeConvexHull(oIndices, ArrayView<math::Vector<T,2> const>(iPoints)); }
//=============================================================================
struct PointCloudParameters
{
    float radius;
    enum { Uniform, Ring, Disk, GalacticCenter, UniformBinaries, Clusters, Spiral } distribution;
    size_t seed;
};
void GeneratePointCloud(ArrayView<float2>& oPoints, PointCloudParameters const* iOptionalParameters = nullptr);
struct FractalPolygonParameters
{
    struct Recursion
    {
        float weight;
        size_t begin;
        size_t end;
    };
    ArrayView<float2> initPoints;
    ArrayView<float2> recursionPoints;
    ArrayView<Recursion> recursions;
    float regularityFactor;
    size_t seed;
};
void GenerateFractalPolygon(ArrayView<float2>& oPoints, FractalPolygonParameters const* iOptionalParameters = nullptr);
//=============================================================================
}
}

#define Geometry_PolygonUtils_Include_Impl
#include "PolygonUtils_Impl.h"
#undef Geometry_PolygonUtils_Include_Impl

#endif

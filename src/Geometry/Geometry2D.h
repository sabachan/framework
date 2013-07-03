#ifndef Math_Geometry2D_H
#define Math_Geometry2D_H

#include <Math/NumericalUtils.h>
#include <Math/Vector.h>
#include <Core/Config.h>
#include <Core/IntTypes.h>
#include <Core/Utils.h>
#include <limits>
#include <type_traits>

namespace sg {
namespace geometry {
//=============================================================================
// These classes are used for method overloading
template <size_t N> class PointTpl : public math::Vector<float, N>
{
public:
    explicit PointTpl(math::Vector<float, N> const& v) : math::Vector<float, N>(v) {}
};
template <size_t N>
class VectorTpl : public math::Vector<float, N>
{
public:
    explicit VectorTpl(math::Vector<float, N> const& v) : math::Vector<float, N>(v) {}
};
typedef PointTpl<2> Point2D;
typedef VectorTpl<2> Vector2D;
typedef PointTpl<3> Point3D;
typedef VectorTpl<3> Vector3D;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
// These methods are for easy tagging of parameter by client code.
SG_FORCE_INLINE Point2D Point(float2 const& v) { return Point2D(v); }
SG_FORCE_INLINE Vector2D Vector(float2 const& v) { return Vector2D(v); }
SG_FORCE_INLINE Point3D Point(float3 const& v) { return Point3D(v); }
SG_FORCE_INLINE Vector3D Vector(float3 const& v) { return Vector3D(v); }
//=============================================================================
enum class Intersect2DMode { TestOnly, LP_Position, HP_Position, FullData };
//=============================================================================
template <Intersect2DMode mode, typename PositionType = float2, typename FullDataType = void>
struct Intersect2DTraits
{
    typedef bool type;
    static type Make(bool b, PositionType const&, FullDataType const&) { return b; }
};
template <typename PositionType, typename FullDataType>
struct Intersect2DTraits<Intersect2DMode::LP_Position, PositionType, FullDataType>
{
    typedef std::pair<bool, PositionType> type;
    static type Make(bool b, PositionType const& v, FullDataType const&) { return std::make_pair(b, v); }
};
template <typename PositionType, typename FullDataType>
struct Intersect2DTraits<Intersect2DMode::HP_Position, PositionType, FullDataType>
{
    typedef std::pair<bool, PositionType> type;
    static type Make(bool b, PositionType const& v, FullDataType const&) { return std::make_pair(b, v); }
};
template <typename PositionType, typename FullDataType>
struct Intersect2DTraits<Intersect2DMode::FullData, PositionType, FullDataType>
{
    typedef std::tuple<bool, PositionType, FullDataType> type;
    static type Make(bool b, PositionType const& v, FullDataType const& d) { return std::make_tuple(b, v, d); }
};
template <typename PositionType>
struct Intersect2DTraits<Intersect2DMode::TestOnly, PositionType, void>
{
    typedef bool type;
    static type Make(bool b, PositionType const&) { return b; }
};
template <typename PositionType>
struct Intersect2DTraits<Intersect2DMode::LP_Position, PositionType, void>
{
    typedef std::pair<bool, PositionType> type;
    static type Make(bool b, PositionType const& v) { return std::make_pair(b, v); }
};
template <typename PositionType>
struct Intersect2DTraits<Intersect2DMode::HP_Position, PositionType, void>
{
    typedef std::pair<bool, PositionType> type;
    static type Make(bool b, PositionType const& v) { return std::make_pair(b, v); }
};
//=============================================================================
template <Intersect2DMode mode = Intersect2DMode::LP_Position>
SG_FORCE_INLINE typename Intersect2DTraits<mode>::type SegmentToSegment(Point2D const& A, Point2D const& B, Point2D const& C, Point2D const& D)
{
    // TODO: handle limit cases
    typedef Intersect2DTraits<mode> traits;
    float2 const AB = B-A;
    float2 const AC = C-A;
    float2 const AD = D-A;
    float const detABC = det(AB,AC);
    float const detABD = det(AB,AD);
    float const testAB = detABC * detABD;
    if(0 <= testAB)
        return traits::Make(false, A);
    float2 const CD = D-C;
    float2 const CA = -AC;
    float2 const CB = B-C;
    float const detCDA = det(CD,CA);
    float const detCDB = det(CD,CB);
    float const testCD = detCDA * detCDB;
    if(0 <= testCD)
        return traits::Make(false, A);
    if(SG_CONSTANT_CONDITION(mode != Intersect2DMode::TestOnly))
    {
        float2 const PAB = lerp(A, B, detCDA / (detCDA-detCDB));
        if(SG_CONSTANT_CONDITION(mode == Intersect2DMode::LP_Position))
        {
            return traits::Make(true, PAB);
        }
        float2 const PCD = lerp(C, D, detABC / (detABC-detABD));
        float2 const P = 0.5f * (PAB+PCD);
        return traits::Make(true, P);
    }
    else
        return traits::Make(true, A);
}
//=============================================================================
template <Intersect2DMode mode = Intersect2DMode::LP_Position>
SG_FORCE_INLINE typename Intersect2DTraits<mode, std::pair<float2, float2>>::type LineToCircle(Point2D const& A, Vector2D const& D, Point2D const& C, float const& R)
{
    // TODO: handle limit cases
    typedef Intersect2DTraits<mode, std::pair<float2, float2>> traits;
    // circle:      ||X-C||^2 = R^2
    //      <=>     X^2 - 2(X.C) + C^2 - R^2 = 0
    // line:        X = A + t * AB
    // We have to solve: a.t^2 + b.t + c = 0
    float2 const AB = D;
    float2 const AC = C-A;
    float const a = AB.LengthSq();
    float const b = -2 * dot(AB,A-C);
    float const c = AC.LengthSq() - sq(R);
    float const delta = sq(b)-4*a*c;
    if(delta < 0.f)
        return traits::Make(false, std::make_pair(A,A));
    if(SG_CONSTANT_CONDITION(mode == Intersect2DMode::TestOnly))
        return traits::Make(true, std::make_pair(A,A));
    float const oo2a = 0.5f*(1/a);
    if(delta == 0.f)
    {
        float const t = - b * oo2a;
        return traits::Make(true, std::make_pair(A,A));
    }
    float const sqrtDelta = sqrt(delta);
    float const t0 = (b + sqrtDelta) * oo2a;
    float const t1 = (b - sqrtDelta) * oo2a;
    float2 const P0 = A + t0 * AB;
    float2 const P1 = A + t1 * AB;
    if(SG_CONSTANT_CONDITION(mode == Intersect2DMode::LP_Position))
        return traits::Make(true, std::make_pair(P0,P1));
    else
    {
        SG_ASSERT_NOT_IMPLEMENTED();
        return traits::Make(true, std::make_pair(P0,P1));
    }
}
template <Intersect2DMode mode = Intersect2DMode::LP_Position>
SG_FORCE_INLINE typename Intersect2DTraits<mode, std::pair<float2, float2>>::type LineToCircle(Point2D const& A, Point2D const& B, Point2D const& C, float const& R)
{
    float2 const AB = B-A;
    return LineToCircle<mode>(A, Vector(AB), C, R);
}
//=============================================================================
template <Intersect2DMode mode = Intersect2DMode::LP_Position>
SG_FORCE_INLINE typename Intersect2DTraits<mode, std::pair<float2, float2>>::type CircleToCircle(Point2D const& C0, float R0, Point2D const& C1, float const& R1)
{
    // TODO: handle limit cases
    typedef Intersect2DTraits<mode, std::pair<float2, float2>> traits;

    float2 const C0C1 = C1-C0;
    float const d2 = C0C1.LengthSq();
    float const sumR = R0+R1;
    float const sqsumR = sq(sumR);
    if(d2 > sqsumR)
        return traits::Make(false, std::make_pair(C0, C0));
    if(d2 == sqsumR)
    {
        float2 const P = lerp(C0, C1, R0 / sumR);
        return traits::Make(true, std::make_pair(P, P));
    }
    float const diffR = R1-R0;
    float const sqdiffR = sq(diffR);
    if(0 == d2 && 0 == sqdiffR)
    {
        return traits::Make(true, std::make_pair(C0 + float2(R0, 0), C0 - float2(R0, 0)));
    }
    if(d2 < sqdiffR)
        return traits::Make(false, std::make_pair(C0, C0));
    if(SG_CONSTANT_CONDITION(mode == Intersect2DMode::TestOnly))
        return traits::Make(true, std::make_pair(C0, C0));
    if(d2 == sqdiffR)
    {
        float2 const P = lerp(C0, C1, R0 / diffR);
        return traits::Make(true, std::make_pair(P, P));
    }

    // The intersection of 2 circles is two points, symmetric around C0C1.
    // Their distance to C0 and C1 are respectively R0 and R1.
    // We will try to find their coordinates, uv, in the frame (C0C1, C0C1^t).
    // u,v verify:
    // u^2 + v^2 = R0^2 / d2
    // (1-u)^2 + v^2 = R1^2 / d2
    // Let's replace v^2 from (1) in (2)
    // (1-u)^2 - u^2 + R0^2 - R1^2 = 0
    // We have to solve: -2*u + 1 + R0^2 - R1^2
    float const u = 0.5f * (1 + (sq(R0) - sq(R1)) / d2);
    float const v2 = sq(R0)/d2 - sq(u);
    SG_ASSERT(v2 > 0.f);
    float const v = sqrt(v2);
    float2 const C0C1t = Orthogonal(C0C1);
    float2 const P0 = C0 + u * C0C1 + v * C0C1t;
    float2 const P1 = C0 + u * C0C1 - v * C0C1t;
    if(SG_CONSTANT_CONDITION(mode == Intersect2DMode::LP_Position))
        return traits::Make(true, std::make_pair(P0,P1));
    else
    {
        SG_ASSERT_NOT_IMPLEMENTED();
        return traits::Make(true, std::make_pair(P0,P1));
    }
}
//=============================================================================
}
}

#endif

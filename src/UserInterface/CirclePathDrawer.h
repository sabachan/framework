#ifndef UserInterface_CirclePathDrawer_H
#define UserInterface_CirclePathDrawer_H

#include "Context.h"
#include <Core/ArrayView.h>
#include <Core/IndexedBuffer.h>
#include <Geometry/Triangulator.h>
#include <Math/Vector.h>
#include <Rendering/Color.h>
#include <RenderEngine/ShaderDescriptors.h>
#include <Reflection/BaseClass.h>

namespace sg {
class toto;
//=============================================================================
// A circle path is a sequence of circular arcs and/or straight lines in 2D.
// The initial default position is (0, 0). However, it can be changed using
// a Begin node. The inital default direction of the path is the positive x
// direction.
// Circular arcs are constrained to have the same tangent as previous segment
// at the common point.
// Lines can have any angle with previous angle. To have angles between arcs,
// it is possible to use a Line of length 0.
// All angles and directions are expressed in [0, 1].
//=============================================================================
namespace circlepath {
enum class NodeType
{
    Begin,
    LineToAngle,
    LineToDirection,
    LineToPoint,
    ArcToAngle,
    ArcToDirection,
    ArcToPoint,
    Close,
};
struct vec2 { float x, y; };
struct NodeCommon { NodeType type; };
struct NodeBegin : public NodeCommon { vec2 position; };
struct NodeLineToAngle : public NodeCommon { float unitAngle; float length; };
struct NodeLineToDirection : public NodeCommon { float unitDirection; float length; };
struct NodeLineToPoint : public NodeCommon { vec2 destination; };
struct NodeArcToAngle : public NodeCommon { float unitAngle; float radius; };
struct NodeArcToDirection : public NodeCommon { float unitDirection; float radius; };
struct NodeArcToPoint : public NodeCommon { vec2 destination; };
struct NodeClose : public NodeCommon { };
struct Node
{
    union
    {
        NodeType type;
        NodeBegin begin;
        NodeLineToAngle lineToAngle;
        NodeLineToDirection lineToDirection;
        NodeLineToPoint lineToPoint;
        NodeArcToAngle arcToAngle;
        NodeArcToDirection arcToDirection;
        NodeArcToPoint arcToPoint;
        NodeClose close;
    };
};
inline float2 ToFloat2(vec2 const& v) { return float2(v.x, v.y); }
inline vec2 ToVec2(float2 const& v) { return {v.x(), v.y()}; }
inline Node Begin(float2 position)                             { Node n; auto& r = n.begin;           r.type = NodeType::Begin;           r.position = ToVec2(position); return n; }
inline Node LineToAngle(float unitAngle, float length)         { Node n; auto& r = n.lineToAngle;     r.type = NodeType::LineToAngle;     r.unitAngle = unitAngle; r.length = length; return n; };
inline Node LineToDirection(float unitDirection, float length) { Node n; auto& r = n.lineToDirection; r.type = NodeType::LineToDirection; r.unitDirection = unitDirection; r.length = length; return n; };
inline Node LineToPoint(float2 const& destination)             { Node n; auto& r = n.lineToPoint;     r.type = NodeType::LineToPoint;     r.destination = ToVec2(destination); return n; };
inline Node ArcToAngle(float unitAngle, float radius)          { Node n; auto& r = n.arcToAngle;      r.type = NodeType::ArcToAngle;      r.unitAngle = unitAngle; r.radius = radius; return n; };
inline Node ArcToDirection(float unitDirection, float radius)  { Node n; auto& r = n.arcToDirection;  r.type = NodeType::ArcToDirection;  r.unitDirection = unitDirection; r.radius = radius; return n; };
inline Node ArcToPoint(float2 const& destination)              { Node n; auto& r = n.arcToPoint;      r.type = NodeType::ArcToPoint;      r.destination = ToVec2(destination); return n; };
inline Node Close()                                            { Node n; auto& r = n.close;           r.type = NodeType::Close;           return n; }
//=============================================================================
bool IsValid(ArrayView<Node> const& iPath);
//=============================================================================
}
}

namespace sg {
namespace rendering {
    class Material;
}
}

namespace sg {
namespace ui {
//=============================================================================
//
// LineCap describes how to close the stroke at the end points of a path.
//   Butt: Ends the stroke flush with the start or end of the line.
//   Round: Ends the line with a circle the diameter of the stroke.
//   Square: Extends the line to match the stroke's thickness.
// LineJoin describes how to draw the stroke at an angle in a path.
//   Mitter: Extends the stroke to where the edges on each side bisect.
//   Round: Rounds the outside edge with a circle the diameter of the stroke.
//   Bevel: Cuts the outside edge off where a circle the diameter of the stroke
//          intersects the stroke.
// For too sharp angles, using Mitter mode can exend the stroke far from the
// line (up to infinity). The mitter limit is used to constrain the maximum
// distance of the stroke to the line. It is expressed as a distance relative
// to stroke width. A LineJoin type can be given for when this limit is
// exceeded. If specifying Mitter again, the stroke will be cut at the limit
// distance.
//
// aaMaxradius is used to expand geometry sufficiently for anti-aliasing. In
// 2D without scale, 1 is a good value.
//=============================================================================
class DrawContext;
//=============================================================================
class CirclePathDrawer : public RefAndSafeCountable
{
public:
    static size_t VertexSize();
    static ArrayView<D3D11_INPUT_ELEMENT_DESC const> VertexDescriptor();
public:
    CirclePathDrawer(rendering::Material const* iMaterial);
    ~CirclePathDrawer();

    enum class LineCap { Butt, Round, Square };
    enum class LineJoin { Mitter, Round, Bevel };

    struct DrawParameters
    {
        Color4f fillColor;
        Color4f strokeColor;
        float strokeWidth;

        DrawParameters(uninitialized_t) {}
        DrawParameters() : fillColor(0,0,0,0), strokeColor(0,0,0,1), strokeWidth(1) {}
    };
    struct GeometryParameters
    {
        float maxStrokeWidth;
        LineCap strokeLineCap;
        LineJoin strokeLineJoin;
        LineJoin strokeLineJoinForMitterLimit;
        // The mitter length is the distance between the central line and the
        // farthest point of the stroke.
        float strokeMitterLimit;
        float aaMaxRadius;

        GeometryParameters(uninitialized_t) {}
        GeometryParameters()
            : maxStrokeWidth(1)
            , strokeLineCap(LineCap::Butt)
            , strokeLineJoin(LineJoin::Mitter)
            , strokeLineJoinForMitterLimit(LineJoin::Mitter)
            , strokeMitterLimit(1)
            , aaMaxRadius(1) {}
    };
private:
    struct Segment
    {
        enum class Type { Line, Arc };
        Type type;
        float2 position[2];
        float2 direction[2];
        // IFA: if applicable
        float2 centerIFA;
        float arcAngleIFA[2];
        float radiusIFA;
        Segment();
    };
public:
    struct OutlineVertex
    {
        // TODO: separate positions and texcoord
        //u16 posIndex;
        float2 position;
        float4 tex;
    };
    struct Tesselation
    {
        // TODO: separate positions and texcoord
        //std::vector<float2> positions;
        //std::vector<float3> texs;
        //IndexedBuffer<OutlineVertex, u16> outline;
        IndexedBuffer<OutlineVertex, u16> outline;
        std::vector<float2> convexHull;
        bool isClosed;
#if SG_ENABLE_ASSERT
        float maxStrokeWidth;
        float aaMaxRadius;
#endif
    };

    struct SegmentEndPoint // TO REMOVE
    {
        static size_t const MAX_POINT_COUNT = 3;
        size_t pointCount; // either {left, right} or {left, right, middle}
        float2 positions[MAX_POINT_COUNT];
        float4 texs[MAX_POINT_COUNT]; // uv + relative width of unit stroke
        SegmentEndPoint() : pointCount(0) {}
    };
    struct SegmentEndPoints // TO REMOVE
    {
        SegmentEndPoint _[2];
    };
    void DrawSegment(DrawContext const& iContext,
                     float2 const& iPos,
                     Segment const& iSegment,
                     SegmentEndPoints const& iEndPoints,
                     DrawParameters const& iDrawParameters,
                     size_t layer = -1) const; // TO REMOVE

    void DrawDiskProto(DrawContext const& iContext,
                  float2 const& iCenter,
                  float iRadius,
                  DrawParameters const& iDrawParameters,
                  size_t layer = -1) const; // TO REMOVE
    void DrawArcProto(DrawContext const& iContext,
                 float2 const& iCenter,
                 float iRadius,
                 float iUnitAngleBegin,
                 float iUnitAngleEnd,
                 DrawParameters const& iDrawParameters,
                 size_t layer = -1) const; // TO REMOVE
    void DrawLineProto(DrawContext const& iContext,
                  float2 const& iA,
                  float2 const& iB,
                  DrawParameters const& iDrawParameters,
                  size_t layer = -1) const; // TO REMOVE


    void DrawPath(DrawContext const& iContext,
                  float2 const& iPos,
                  ArrayView<circlepath::Node> iPath,
                  GeometryParameters const& iGeometryParameters,
                  DrawParameters const& iDrawParameters,
                  size_t layer = -1) const;
    void ComputeTesselation(ArrayView<circlepath::Node> iPath,
                            Tesselation& oTesselation,
                            GeometryParameters const& iGeometryParameters) const;
    void Draw(DrawContext const& iContext,
              float2 const& iPos,
              Tesselation const& iTesselation,
              DrawParameters const& iDrawParameters,
              size_t layer = -1) const;
private:
    template<Context::TransformType transformType>
    void DrawImpl(DrawContext const& iContext,
                  float2 const& iPos,
                  Tesselation const& iTesselation,
                  DrawParameters const& iDrawParameters,
                  size_t layer = -1) const;
    void DrawLine(DrawContext const& iContext,
                  float2 const& iPos,
                  Segment const& iSegment,
                  SegmentEndPoints const& iEndPoints,
                  DrawParameters const& iDrawParameters,
                  size_t layer = -1) const; // TO REMOVE
    void DrawArc(DrawContext const& iContext,
                 float2 const& iPos,
                 Segment const& iSegment,
                 SegmentEndPoints const& iEndPoints,
                 DrawParameters const& iDrawParameters,
                 size_t layer = -1) const; // TO REMOVE

    void ExplicitPath(std::vector<Segment>& oExplicitPath,
                      ArrayView<circlepath::Node> iPath) const;
    void ComputeCapsAndEndPoints(std::vector<Segment>& ioExplicitPath,
                                 std::vector<SegmentEndPoints>& oEndPoints,
                                 bool iIsClosed,
                                 GeometryParameters const& iGeometryParameters) const; // TO REMOVE
    void ComputeOutlineTesselation(std::vector<Segment>& ioExplicitPath,
                                   IndexedBuffer<OutlineVertex, u16>& oTesselation,
                                   std::vector<u16>& oInteriorIndices,
                                   std::vector<u16>& oExteriorIndices,
                                   bool iIsClosed,
                                   GeometryParameters const& iGeometryParameters) const; // TO REMOVE
private:
    refptr<rendering::Material const> m_material;
};
//=============================================================================
class CirclePathDrawerDescriptor : public reflection::BaseClass
{
    REFLECTION_CLASS_HEADER(CirclePathDrawerDescriptor, reflection::BaseClass)
public:
    CirclePathDrawerDescriptor();
    virtual ~CirclePathDrawerDescriptor() override;
    CirclePathDrawer const* GetDrawer_AssumeAvailable() const { SG_ASSERT(nullptr != m_drawer); return m_drawer.get(); }
    CirclePathDrawer const* GetDrawer() { if(nullptr == m_drawer) { CreateDrawer(); } return m_drawer.get(); }
private:
    void CreateDrawer();
private:
    refptr<CirclePathDrawer> m_drawer;
    refptr<renderengine::PixelShaderDescriptor const> m_pixelShader;
};
//=============================================================================
}
}

#endif

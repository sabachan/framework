#include "stdafx.h"

#include "CirclePathDrawer.h"

#include "Context.h"
#include <Core/Log.h>
#include <Geometry/Geometry2D.h>
#include <Geometry/PolygonUtils.h>
#include <Geometry/Triangulator.h>
#include <Rendering/Material.h>
#include <Rendering/RenderDevice.h>
#include <Rendering/TransientRenderBatch.h>
#include <Rendering/VertexTypes.h>
#include <RenderEngine/CompositingLayer.h>

#define CIRCLE_PATH_ENABLE_DEBUG_IMAGE SG_ENABLE_ASSERT

#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
#include <Image/DebugImage.h>
#include <Core/StringFormat.h>
#define ONLY_WITH_CIRCLE_PATH_DEBUG_IMAGE(code) code
#else
#define ONLY_WITH_CIRCLE_PATH_DEBUG_IMAGE(code)
#endif

namespace sg {
namespace circlepath {
//=============================================================================
bool IsValid(ArrayView<Node> const& iPath)
{
    size_t const size = iPath.size();
    bool isValid = true;
    for(size_t i = 0; i < size; ++i)
    {
        if(iPath[i].type == NodeType::Close && i != size - 1)
            SG_LOG_ERROR("A Close node can only be the last one."), isValid = false;
        if(iPath[i].type == NodeType::Begin && i != 0)
            SG_LOG_ERROR("A Begin node can only be the first one."), isValid = false;
    }
    if(iPath.back().type == NodeType::Close)
    {
        // TODO: Walk path to check that the path is closed.
    }
    return isValid;
}
//=============================================================================
}
}

namespace sg {
namespace ui {
//=============================================================================
namespace {
    typedef rendering::Vertex_Pos2f_Tex4f_2Col4f CirclePathDrawerVertex;
}
//=============================================================================
size_t CirclePathDrawer::VertexSize() { return sizeof(CirclePathDrawerVertex); }
ArrayView<D3D11_INPUT_ELEMENT_DESC const> CirclePathDrawer::VertexDescriptor() { return CirclePathDrawerVertex::InputEltDesc(); }
//=============================================================================
CirclePathDrawer::CirclePathDrawer(rendering::Material const* iMaterial)
: m_material(iMaterial)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
CirclePathDrawer::~CirclePathDrawer()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CirclePathDrawer::DrawDiskProto(DrawContext const& iContext,
                                     float2 const& iCenter,
                                     float iRadius,
                                     DrawParameters const& iDrawParameters,
                                     size_t layer) const
{
    float const aaMaxRadius = 1;
    float const extDiameter = 2 * iRadius + iDrawParameters.strokeWidth + 2 * aaMaxRadius;
    box2f const box = box2f::FromCenterDelta(iCenter, float2(extDiameter));
    // TODO: Use 6 or 8 corners, and use hollow geometry if not filled.
    float2 corners[4] = { float2(uninitialized), float2(uninitialized), float2(uninitialized), float2(uninitialized)};
    Context::TransformType const transformType = iContext.GetTransformType();
    switch(transformType)
    {
    case Context::TransformType::None:
        corners[0] = float2(box.min.x(), box.min.y());
        corners[1] = float2(box.max.x(), box.min.y());
        corners[2] = float2(box.min.x(), box.max.y());
        corners[3] = float2(box.max.x(), box.max.y());
        break;
    case Context::TransformType::Translate2D:
        {
            float4x4 const& transform = iContext.GetTransform();
            float2 const T = transform.Col(3).xy();
            corners[0] = float2(box.min.x(), box.min.y()) + T;
            corners[1] = float2(box.max.x(), box.min.y()) + T;
            corners[2] = float2(box.min.x(), box.max.y()) + T;
            corners[3] = float2(box.max.x(), box.max.y()) + T;
        }
        break;
    case Context::TransformType::Transform2D:
        {
            float4x4 const& transform = iContext.GetTransform();
            float2x2 const R = transform.SubMatrix<2, 2>(0, 0);
            float2 const T = transform.Col(3).xy();
            corners[0] = R * float2(box.min.x(), box.min.y()) + T;
            corners[1] = R * float2(box.max.x(), box.min.y()) + T;
            corners[2] = R * float2(box.min.x(), box.max.y()) + T;
            corners[3] = R * float2(box.max.x(), box.max.y()) + T;
        }
        break;
    case Context::TransformType::Transform3D:
        SG_ASSERT_NOT_IMPLEMENTED();
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }

    if(-1 == layer)
    {
        box2f boxToLock(uninitialized);
        switch(transformType)
        {
        case Context::TransformType::None: boxToLock = box; break;
        case Context::TransformType::Translate2D:
            {
                float4x4 const& transform = iContext.GetTransform();
                float2 const T = transform.Col(3).xy();
                boxToLock = box + T;
            }
            break;
        case Context::TransformType::Transform2D:
            {
                boxToLock = box2f::FromMinDelta(corners[0], float2(0));
                for_range(size_t, i, 1, 4)
                    boxToLock.Grow(corners[i]);
            }
            break;
        case Context::TransformType::Transform3D:
            SG_ASSERT_NOT_IMPLEMENTED();
            break;
        default:
            SG_ASSUME_NOT_REACHED();
        }
        LayerManager* layerManager = iContext.GetLayerManager();
        layer = layerManager->Lock(boxToLock)[0];
    }

    rendering::TransientRenderBatch::Properties properties;
    properties.indexSize = sizeof(u32);
    properties.vertexSize = sizeof(CirclePathDrawerVertex);
    rendering::TransientRenderBatch* renderBatch = iContext.GetCompositingLayer()->GetOrCreateTransientRenderBatch(
        *m_material,
        properties);

    float const strokeWidthRatio = iDrawParameters.strokeWidth / iRadius;
    float const maxTex = 0.5f*extDiameter / iRadius;
    rendering::TransientRenderBatch::WriteAccess<CirclePathDrawerVertex> writeAccess(*renderBatch, 4, 6, layer);
    CirclePathDrawerVertex* vertices = writeAccess.GetVertexPointerForWriting();
    vertices[0].pos = corners[0];
    vertices[1].pos = corners[1];
    vertices[2].pos = corners[2];
    vertices[3].pos = corners[3];
    vertices[0].tex = (maxTex * float2(-1,-1)).Append(strokeWidthRatio).Append(1);
    vertices[1].tex = (maxTex * float2(-1, 1)).Append(strokeWidthRatio).Append(1);
    vertices[2].tex = (maxTex * float2( 1,-1)).Append(strokeWidthRatio).Append(1);
    vertices[3].tex = (maxTex * float2( 1, 1)).Append(strokeWidthRatio).Append(1);
    vertices[0].col[0] = float4(iDrawParameters.strokeColor);
    vertices[1].col[0] = float4(iDrawParameters.strokeColor);
    vertices[2].col[0] = float4(iDrawParameters.strokeColor);
    vertices[3].col[0] = float4(iDrawParameters.strokeColor);
    vertices[0].col[1] = float4(iDrawParameters.fillColor);
    vertices[1].col[1] = float4(iDrawParameters.fillColor);
    vertices[2].col[1] = float4(iDrawParameters.fillColor);
    vertices[3].col[1] = float4(iDrawParameters.fillColor);
    writeAccess.PushIndices(0, 2, 1);
    writeAccess.PushIndices(1, 2, 3);
    writeAccess.FinishWritingVertex(4);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CirclePathDrawer::DrawArcProto(DrawContext const& iContext,
                                    float2 const& iCenter,
                                    float iRadius,
                                    float iUnitAngleBegin,
                                    float iUnitAngleEnd,
                                    DrawParameters const& iDrawParameters,
                                    size_t layer) const
{
    SG_ASSERT(iUnitAngleEnd - iUnitAngleBegin >= 0.f);
    SG_ASSERT(iUnitAngleEnd - iUnitAngleBegin <= 1.f);
    float const aaMaxRadius = 1;
    float const extRadius = iRadius + 0.5f * iDrawParameters.strokeWidth + aaMaxRadius;
    float const intRadius = iRadius - 0.5f * iDrawParameters.strokeWidth - aaMaxRadius;
    float const ooRadius = 1.f / iRadius;
    float const strokeWidthRatio = iDrawParameters.strokeWidth * ooRadius;

    //              1               3               5
    // interior     +               +               +
    //                                                  ...
    // exterior     +       +               +
    //              0       2 <-----------> 4
    //                           Sector
    //
    // Having exterior end-points in the middle of a sector is interesting for
    // joining with previous or next arc or line as such point can be placed
    // nearly as possible to the curve.
    size_t const MAX_SECTOR_COUNT = 8;
    size_t const MAX_HALF_SECTOR_COUNT = 2 * MAX_SECTOR_COUNT;
    float const extVertexDistanceRatio = 1.f / cos(float(2 * math::constant::PI / (2 * MAX_SECTOR_COUNT)));
    float const fullDeltaAngle = (iUnitAngleEnd - iUnitAngleBegin);
    bool const closed = 1.f == fullDeltaAngle;
    size_t const sectorCount = ceili(fullDeltaAngle * MAX_SECTOR_COUNT);
    size_t const halfSectorCount = sectorCount * 2;
    SG_ASSERT(!closed || halfSectorCount == MAX_HALF_SECTOR_COUNT);
    float const halfSectorAngle = fullDeltaAngle / halfSectorCount;
    float const cos0 = cos(iUnitAngleBegin * float(2 * math::constant::PI));
    float const sin0 = sin(iUnitAngleBegin * float(2 * math::constant::PI));
    float2 const dir0 = float2(cos0, sin0);
    float2x2 const RhalfSector = matrix::Rotation(halfSectorAngle * float(2 * math::constant::PI));
    struct PreVertex
    {
        float2 pos;
        float2 tex;
    };
    size_t const MAX_PREVERTEX_COUNT = MAX_HALF_SECTOR_COUNT + 3;
    PreVertex preVertices[MAX_PREVERTEX_COUNT];
    size_t const triangleCount = closed ? MAX_HALF_SECTOR_COUNT : halfSectorCount + 1;
    size_t const vertexCount = closed ? MAX_HALF_SECTOR_COUNT + 1 : halfSectorCount + 3;
    float2 dir = dir0;
    float radius[2] = { extVertexDistanceRatio * extRadius, intRadius };
    {
        float2 const u = dir * extRadius;
        preVertices[0].pos = iCenter + u;
        preVertices[0].tex = u * ooRadius;
    }
    for_range(size_t, i, 1, vertexCount-1)
    {
        float2 const u = dir * radius[i%2];
        preVertices[i].pos = iCenter + u;
        preVertices[i].tex = u * ooRadius;
        if(closed || i < (vertexCount-2))
            dir = RhalfSector * dir;
    }
    {
        float2 const u = dir * extRadius;
        preVertices[vertexCount-1].pos = iCenter + u;
        preVertices[vertexCount-1].tex = u * ooRadius;
    }

    box2f bbox;
    Context::TransformType const transformType = iContext.GetTransformType();
    switch(transformType)
    {
    case Context::TransformType::None:
        {
            for_range(size_t, i, 0, (vertexCount+1)/2)
                bbox.Grow(preVertices[2*i].pos);
        }
        break;
    case Context::TransformType::Translate2D:
        {
            float4x4 const& transform = iContext.GetTransform();
            float2 const T = transform.Col(3).xy();
            for_range(size_t, i, 0, vertexCount)
            {
                preVertices[i].pos += T;
                bbox.Grow(preVertices[i].pos);
            }
        }
        break;
    case Context::TransformType::Transform2D:
        {
            float4x4 const& transform = iContext.GetTransform();
            float2x2 const R = transform.SubMatrix<2, 2>(0, 0);
            float2 const T = transform.Col(3).xy();
            for_range(size_t, i, 0, vertexCount)
            {
                preVertices[i].pos = R * preVertices[i].pos + T;
                bbox.Grow(preVertices[i].pos);
            }
        }
        break;
    case Context::TransformType::Transform3D:
        SG_ASSERT_NOT_IMPLEMENTED();
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }

    if(-1 == layer)
    {
        LayerManager* layerManager = iContext.GetLayerManager();
        layer = layerManager->Lock(bbox)[0];
    }

    rendering::TransientRenderBatch::Properties properties;
    properties.indexSize = sizeof(u32);
    properties.vertexSize = sizeof(CirclePathDrawerVertex);
    rendering::TransientRenderBatch* renderBatch = iContext.GetCompositingLayer()->GetOrCreateTransientRenderBatch(
        *m_material,
        properties);

    rendering::TransientRenderBatch::WriteAccess<CirclePathDrawerVertex> writeAccess(*renderBatch, vertexCount, triangleCount*3, layer);
    CirclePathDrawerVertex* vertices = writeAccess.GetVertexPointerForWriting();
    for_range(size_t, i, 0, vertexCount)
    {
        vertices[i].pos = preVertices[i].pos;
        vertices[i].tex.xy() = preVertices[i].tex;
        vertices[i].tex.z() = strokeWidthRatio;
        vertices[i].col[0] = float4(iDrawParameters.strokeColor);
        vertices[i].col[1] = float4(0,0,0,0);
    }
    for_range(size_t, i, 0, closed ? triangleCount-2 : triangleCount)
    {
        // Note: This seems to be an indirect order. Is the triangle clipping correctly set ?
        if(0 == i%2) writeAccess.PushIndices(i+0, i+1, i+2);
        else if(1 == i%2) writeAccess.PushIndices(i+0, i+2, i+1);
    }
    if(closed)
    {
        size_t const i = triangleCount-2;
        SG_ASSERT(0 == i%2);
        writeAccess.PushIndices(i+0, 0, i+1);
        writeAccess.PushIndices(i+1, 0, 1);
    }
    writeAccess.FinishWritingVertex(vertexCount);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CirclePathDrawer::DrawLineProto(DrawContext const& iContext,
                                     float2 const& iA,
                                     float2 const& iB,
                                     DrawParameters const& iDrawParameters,
                                     size_t layer) const
{
    // Using the shader used to draw circles, we can draw line by keeping the
    // same texture coordinates along the line direction. The radius of the
    // hacked circle must be greater than half the stroke thickness for that to
    // work. We use the stroke thickness, arbitrarily.
    float const aaMaxRadius = 1;
    float const radius = iDrawParameters.strokeWidth + aaMaxRadius;
    float const ooRadius = 1.f / radius;
    float const strokeWidthRatio = iDrawParameters.strokeWidth * ooRadius;

    SG_ASSERT(iA != iB);
    float2 const AB = iB - iA;
    float2 const orthoAB = Orthogonal(AB);
    float2 const orthonormalAB = orthoAB.Normalised();
    float const lineExtHalfWidth = 0.5f * iDrawParameters.strokeWidth + aaMaxRadius;
    float2 const extTex = orthonormalAB * (radius + lineExtHalfWidth) * ooRadius;
    float2 const intTex = orthonormalAB * (radius - lineExtHalfWidth) * ooRadius;

    size_t const vertexCount = 4;
    struct PreVertex
    {
        float2 pos;
        float2 tex;
    };
    PreVertex preVertices[vertexCount];
    preVertices[0].pos = iA - orthonormalAB * lineExtHalfWidth;
    preVertices[1].pos = iB - orthonormalAB * lineExtHalfWidth;
    preVertices[2].pos = iA + orthonormalAB * lineExtHalfWidth;
    preVertices[3].pos = iB + orthonormalAB * lineExtHalfWidth;
    preVertices[0].tex = intTex;
    preVertices[1].tex = intTex;
    preVertices[2].tex = extTex;
    preVertices[3].tex = extTex;

    box2f bbox;
    Context::TransformType const transformType = iContext.GetTransformType();
    switch(transformType)
    {
    case Context::TransformType::None:
        {
            for_range(size_t, i, 0, vertexCount)
                bbox.Grow(preVertices[i].pos);
        }
        break;
    case Context::TransformType::Translate2D:
        {
            float4x4 const& transform = iContext.GetTransform();
            float2 const T = transform.Col(3).xy();
            for_range(size_t, i, 0, vertexCount)
            {
                preVertices[i].pos += T;
            }
        }
        break;
    case Context::TransformType::Transform2D:
        {
            float4x4 const& transform = iContext.GetTransform();
            float2x2 const R = transform.SubMatrix<2, 2>(0, 0);
            float2 const T = transform.Col(3).xy();
            for_range(size_t, i, 0, vertexCount)
            {
                preVertices[i].pos = R * preVertices[i].pos + T;
                bbox.Grow(preVertices[i].pos);
            }
        }
        break;
    case Context::TransformType::Transform3D:
        SG_ASSERT_NOT_IMPLEMENTED();
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }

    if(-1 == layer)
    {
        LayerManager* layerManager = iContext.GetLayerManager();
        layer = layerManager->Lock(bbox)[0];
    }

    rendering::TransientRenderBatch::Properties properties;
    properties.indexSize = sizeof(u32);
    properties.vertexSize = sizeof(CirclePathDrawerVertex);
    rendering::TransientRenderBatch* renderBatch = iContext.GetCompositingLayer()->GetOrCreateTransientRenderBatch(
        *m_material,
        properties);

    rendering::TransientRenderBatch::WriteAccess<CirclePathDrawerVertex> writeAccess(*renderBatch, vertexCount, 6, layer);
    CirclePathDrawerVertex* vertices = writeAccess.GetVertexPointerForWriting();
    for_range(size_t, i, 0, vertexCount)
    {
        vertices[i].pos = preVertices[i].pos;
        vertices[i].tex.xy() = preVertices[i].tex;
        vertices[i].tex.z() = strokeWidthRatio;
        vertices[i].col[0] = float4(iDrawParameters.strokeColor);
        vertices[i].col[1] = float4(0,0,0,0);
    }
    writeAccess.PushIndices(0, 2, 1);
    writeAccess.PushIndices(1, 2, 3);
    writeAccess.FinishWritingVertex(vertexCount);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
CirclePathDrawer::Segment::Segment()
{
    type = Segment::Type::Line;
    for_range(size_t,i,0,2)
    {
        position[i] = float2(0);
        direction[i] = float2(0);
        arcAngleIFA[i] = 0;
    }
    centerIFA = float2(0);
    radiusIFA = 0;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CirclePathDrawer::ExplicitPath(std::vector<Segment>& oExplicitPath,
                                    ArrayView<circlepath::Node> iPath) const
{
#if SG_ENABLE_ASSERT
    auto DrawDebugLine = [&oExplicitPath](float2 const& A, float2 const& B) {
        Segment s;
        s.position[0] = A;
        s.position[1] = B;
        s.direction[0] = s.direction[1] = (B-A).Normalised();
        oExplicitPath.push_back(s);
    };
#endif
    float2 pos = float2(0, 0);
    float2 dir = float2(1, 0);
    bool const hasBeginNode = iPath.front().type == circlepath::NodeType::Begin;
    size_t const begin = hasBeginNode ? 1 : 0;
    bool const isClosed = iPath.back().type == circlepath::NodeType::Close;
    size_t const end = isClosed ? iPath.size() - 1 : iPath.size();
    if(hasBeginNode)
        pos = ToFloat2(iPath.front().begin.position);
    for_range(size_t, i, begin, end)
    {
        circlepath::Node const& node = iPath[i];
        Segment segment;
        bool pushSegment = true;
        switch(node.type)
        {
        case circlepath::NodeType::Begin:
            SG_ASSERT_NOT_REACHED();
            break;
        case circlepath::NodeType::LineToAngle:
        {
            circlepath::NodeLineToAngle const& n = node.lineToAngle;
            float2x2 const R = matrix::Rotation(n.unitAngle * float(2 * math::constant::PI));
            dir = R * dir;
            if(0 == node.lineToAngle.length)
                pushSegment = false;
            else
            {
                segment.type = Segment::Type::Line;
                segment.position[0] = pos;
                pos += dir * n.length;
                segment.direction[0] = dir;
                segment.direction[1] = dir;
                segment.position[1] = pos;
            }
            break;
        }
        case circlepath::NodeType::LineToDirection:
        {
            circlepath::NodeLineToDirection const& n = node.lineToDirection;
            float2x2 const R = matrix::Rotation(n.unitDirection * float(2 * math::constant::PI));
            dir = R * float2(1,0);
            if(0 == node.lineToAngle.length)
                pushSegment = false;
            else
            {
                segment.type = Segment::Type::Line;
                segment.position[0] = pos;
                pos += dir * n.length;
                segment.direction[0] = dir;
                segment.direction[1] = dir;
                segment.position[1] = pos;
            }
            break;
        }
        case circlepath::NodeType::LineToPoint:
        {
            segment.type = Segment::Type::Line;
            circlepath::NodeLineToPoint const& n = node.lineToPoint;
            float2 const nextPos = ToFloat2(n.destination);
            SG_ASSERT(nextPos != pos);
            dir = (nextPos - pos).Normalised();
            segment.position[0] = pos;
            pos = nextPos;
            segment.position[1] = nextPos;
            segment.direction[0] = dir;
            segment.direction[1] = dir;
            break;
        }
        case circlepath::NodeType::ArcToAngle:
        {
            segment.type = Segment::Type::Arc;
            circlepath::NodeArcToAngle const& n = node.arcToAngle;
            float2x2 const R = matrix::Rotation(n.unitAngle * float(2 * math::constant::PI));
            float const signAngle = n.unitAngle >= 0 ? 1.f : -1.f;
            float2 const radialDir = Orthogonal(dir) * -1.f * signAngle;
            float const angle = atan2f(radialDir.y(), radialDir.x());
            float const unitAngleBegin = angle * float(0.5 * math::constant::_1_PI);
            float const unitAngleEnd = unitAngleBegin + n.unitAngle;
            float2 const center = pos - radialDir * n.radius;

            segment.position[0] = pos;
            segment.direction[0] = dir;
            segment.arcAngleIFA[0] = unitAngleBegin;
            segment.arcAngleIFA[1] = unitAngleEnd;
            segment.centerIFA = center;
            segment.radiusIFA = n.radius;
            pos = center + R * (pos - center);
            dir = R * dir;

            segment.position[1] = pos;
            segment.direction[1] = dir;
            break;
        }
        case circlepath::NodeType::ArcToDirection:
        {
            segment.type = Segment::Type::Arc;
            circlepath::NodeArcToDirection const& n = node.arcToDirection;
            float2x2 const R = matrix::Rotation(n.unitDirection * float(2 * math::constant::PI));
            float2 const nextDir = R * float2(1,0);
            float const detdirs = det(dir, nextDir);
            float const signAngle = detdirs >= 0 ? 1.f : -1.f;
            float2 const radialDir0 = Orthogonal(dir) * -1.f * signAngle;
            float const angle0 = atan2f(radialDir0.y(), radialDir0.x());
            float2 const center = pos - radialDir0 * n.radius;
            float2 const radialDir1 = Orthogonal(nextDir) * -1.f * signAngle;
            float const angle1 = atan2f(radialDir1.y(), radialDir1.x());
            float unitAngleBegin = angle0 * float(0.5 * math::constant::_1_PI);
            float unitAngleEnd = angle1 * float(0.5 * math::constant::_1_PI);
            if(unitAngleEnd > unitAngleBegin + 0.5f)
                unitAngleBegin += 1.f;
            if(unitAngleBegin > unitAngleEnd + 0.5f)
                unitAngleEnd += 1.f;

            segment.position[0] = pos;
            segment.direction[0] = dir;
            segment.arcAngleIFA[0] = unitAngleBegin;
            segment.arcAngleIFA[1] = unitAngleEnd;
            segment.centerIFA = center;
            segment.radiusIFA = n.radius;

            pos = center + n.radius * radialDir1;
            dir = nextDir;

            segment.position[1] = pos;
            segment.direction[1] = dir;
            break;
        }
        case circlepath::NodeType::ArcToPoint:
        {
            // Let B be the middle of AD where A is pos and D is destination.
            // u is ortho(direction) v is ortho(AD).
            // O, the center of the circular arc is at the intersection of
            // (A, u) and (B, v):
            // det(OA, u) = 0
            // det(OB, v) = 0
            //
            // OA.x * u.y - OA.y * u.x = 0
            // OB.x * v.y - OB.y * v.x = 0
            //
            // A.x * u.y - A.y * u.x - O.x * u.y + O.y * u.x = 0
            // B.x * v.y - B.y * v.x - O.x * v.y + O.y * v.x = 0
            //
            // det(A,u) - O.x * u.y + O.y * u.x = 0 (1)
            // det(B,v) - O.x * v.y + O.y * v.x = 0 (1)
            //
            // (1) * v.x - (2) * u.x :
            // det(A,u) * v.x - det(B,v) * u.x - O.x * (u.y * v.x - v.y * u.x) = 0
            // O.x = (det(B,v) * u.x - det(A,u) * v.x) / det(u,v)
            // (1) * v.y - (2) * u.y :
            // det(Au) * v.y - det(Bv) * u.y + O.y * (u.x * v.y - v.x * u.y) = 0
            // O.y = (det(B,v) * u.y - det(A,u) * v.y) / det(u,v)
            //
            segment.type = Segment::Type::Arc;
            circlepath::NodeArcToPoint const& n = node.arcToPoint;
            float2 const A = pos;
            float2 const D = ToFloat2(n.destination);
            float2 const AD = D-A;
            float const detdirAD = det(dir, AD);
            float const signAngle = detdirAD >= 0 ? 1.f : -1.f;
            float2 const u = -signAngle * Orthogonal(dir);
            float2 const B = 0.5f * (A + D);
            float2 const v = -signAngle * Orthogonal(AD);
            float const detAu = det(A, u);
            float const detBv = det(B, v);
            float const detuv = det(u, v);
            float const detx = detBv * u.x() - detAu * v.x();
            float const dety = detBv * u.y() - detAu * v.y();
            float const oodetuv = 1.f / detuv;
            float2 const O = float2(detx, dety) * oodetuv;
            //DrawDebugLine(O, A);
            //DrawDebugLine(O, B);
            //DrawDebugLine(O, D);

            float const angleBegin = atan2f(u.y(), u.x());
            float const unitAngleBegin = angleBegin * float(0.5 * math::constant::_1_PI);
            float const angleMid = atan2f(v.y(), v.x());
            float const unitAngleMid = angleMid * float(0.5 * math::constant::_1_PI);
            float unitHalfAngleDiff = unitAngleMid - unitAngleBegin;
            if(unitHalfAngleDiff > 0.5f) unitHalfAngleDiff -= 1;
            if(unitHalfAngleDiff < -0.5f) unitHalfAngleDiff += 1;
            float const unitAngleDiff = 2 * unitHalfAngleDiff;
            float const unitAngleEnd = unitAngleBegin + unitAngleDiff;
            float const unitAngle = unitAngleEnd - unitAngleBegin;
            float2x2 const R = matrix::Rotation(unitAngle * float(2 * math::constant::PI));
            float2 const center = O;
            float2 const OA = A - O;
            float radius = sqrt(OA.LengthSq());

            segment.position[0] = pos;
            segment.direction[0] = dir;
            segment.arcAngleIFA[0] = unitAngleBegin;
            segment.arcAngleIFA[1] = unitAngleEnd;
            segment.centerIFA = center;
            segment.radiusIFA = radius;
            pos = D;
            dir = R * dir;

            segment.position[1] = pos;
            segment.direction[1] = dir;
            break;
        }
        case circlepath::NodeType::Close:
            SG_ASSERT_NOT_REACHED();
            break;
        default:
            SG_ASSUME_NOT_REACHED();
        }
        SG_ASSERT(EqualsWithTolerance(dir.LengthSq(), 1.f, 0.000001f));
        if(pushSegment)
            oExplicitPath.push_back(segment);
    }
    if(isClosed)
    {
        SG_ASSERT(EqualsWithTolerance(oExplicitPath.back().position[1], oExplicitPath.front().position[0], 0.001f));
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CirclePathDrawer::ComputeCapsAndEndPoints(std::vector<Segment>& ioExplicitPath,
                                               std::vector<SegmentEndPoints>& oEndPoints,
                                               bool iIsClosed,
                                               GeometryParameters const& iGeometryParameters) const
{
    float const halfWidth = 0.5f * iGeometryParameters.maxStrokeWidth + iGeometryParameters.aaMaxRadius;

    size_t const size = ioExplicitPath.size();
    oEndPoints.resize(size);
    for_range(size_t, i, 0, size)
    {
        size_t j = i+1;
        if(i == size-1)
            if(iIsClosed) j = 0;
            else break;
        ioExplicitPath.reserve(ioExplicitPath.size() + 1);
        oEndPoints.reserve(oEndPoints.size() + 1);
        Segment& prev = ioExplicitPath[i];
        Segment& next = ioExplicitPath[j];
        SegmentEndPoint& prevEndPoint = oEndPoints[i]._[1];
        SegmentEndPoint& nextEndPoint = oEndPoints[j]._[0];
        if(Segment::Type::Line == prev.type && Segment::Type::Line == next.type)
        {
            float2 const A = prev.position[0];
            float2 const B = prev.position[1];
            float2 const C = next.position[1];
            SG_ASSERT(B == next.position[0]);
            float2 const AB = B - A;
            float2 const tAB = AB.Normalised();
            float2 const nAB = Orthogonal(tAB);
            float2 const BC = C - B;
            float2 const tBC = BC.Normalised();
            float2 const nBC = Orthogonal(tBC);
            float2x2 M(uninitialized);
            M.SetRow(0, nAB);
            M.SetRow(1, nBC);
            float2x2 invM;
            bool ok = InvertROK(invM, M, 0.0001f);
            float2 BP = ok ? invM * float2(halfWidth) : nAB * halfWidth;
            SG_ASSERT(EqualsWithTolerance(dot(nAB, BP), halfWidth, 0.001f));
            SG_ASSERT(EqualsWithTolerance(dot(nBC, BP), halfWidth, 0.001f));
            float2 const P0 = B + BP;
            float2 const P1 = B - BP;

            // Remember that we will draw line with circle shader.
            float const maxStrokeWidthRatio = 1.f;
            float const halfMaxStrokeWidthRatio = 0.5f * maxStrokeWidthRatio;
            float const radius = 2 * halfWidth;
            float const ooRadius = 1.f/radius;
            float const unitStrokeWidthRatio = ooRadius;

            prevEndPoint.pointCount = 2;
            prevEndPoint.positions[0] = P0;
            prevEndPoint.positions[1] = P1;
            prevEndPoint.texs[0] = (nAB * (1+halfMaxStrokeWidthRatio)).Append(unitStrokeWidthRatio).Append(1);
            prevEndPoint.texs[1] = (nAB * (1-halfMaxStrokeWidthRatio)).Append(unitStrokeWidthRatio).Append(1);
            nextEndPoint.pointCount = 2;
            nextEndPoint.positions[0] = P0;
            nextEndPoint.positions[1] = P1;
            nextEndPoint.texs[0] = (nBC * (1+halfMaxStrokeWidthRatio)).Append(unitStrokeWidthRatio).Append(1);
            nextEndPoint.texs[1] = (nBC * (1-halfMaxStrokeWidthRatio)).Append(unitStrokeWidthRatio).Append(1);

            size_t const extIndex = det(AB, BC) > 0 ? 1 : 0;
            float const extSign = 0 == extIndex ? 1.f : -1.f;
            LineJoin lineJoin = iGeometryParameters.strokeLineJoin;
            if(LineJoin::Mitter == lineJoin)
            {
                float const mitterLengthSq = (prevEndPoint.positions[extIndex] - B).LengthSq();
                float const maxMitterLengthSq = sq(iGeometryParameters.strokeMitterLimit * iGeometryParameters.maxStrokeWidth);
                if(mitterLengthSq > maxMitterLengthSq)
                    lineJoin = iGeometryParameters.strokeLineJoinForMitterLimit;
            }

            if(LineJoin::Bevel == lineJoin || LineJoin::Round == lineJoin)
            {
                if(LineJoin::Bevel == lineJoin)
                {
                    float2 const prevExtP = prevEndPoint.positions[extIndex];
                    float2 const t = (0.5f * (tAB + tBC)).Normalised();
                    float2 const n = Orthogonal(t);
                    float2x2 M0(uninitialized);
                    M0.SetRow(0, nAB);
                    M0.SetRow(1, n);
                    float2x2 invM0;
                    ok = InvertROK(invM0, M0, 0.0001f);
                    SG_ASSERT(ok);
                    float2 const d0 = ok ? invM0 * float2(halfWidth) : nAB * halfWidth;
                    SG_ASSERT(EqualsWithTolerance(dot(nAB, d0), halfWidth, 0.001f));
                    SG_ASSERT(EqualsWithTolerance(dot(n, d0), halfWidth, 0.001f));
                    float2 const extP0 = B + extSign*d0;
                    float2x2 M1(uninitialized);
                    M1.SetRow(0, nBC);
                    M1.SetRow(1, n);
                    float2x2 invM1;
                    ok = InvertROK(invM1, M1, 0.0001f);
                    SG_ASSERT(ok);
                    float2 const d1 = ok ? invM1 * float2(halfWidth) : nBC * halfWidth;
                    SG_ASSERT(EqualsWithTolerance(dot(nBC, d1), halfWidth, 0.001f));
                    SG_ASSERT(EqualsWithTolerance(dot(n, d1), halfWidth, 0.001f));
                    float2 const extP1 = B + extSign*d1;

                    prevEndPoint.pointCount = 3;
                    prevEndPoint.positions[extIndex] = extP0;
                    prevEndPoint.positions[2] = B;
                    prevEndPoint.texs[2] = nAB.Append(unitStrokeWidthRatio).Append(1);
                    nextEndPoint.pointCount = 3;
                    nextEndPoint.positions[extIndex] = extP1;
                    nextEndPoint.positions[2] = B;
                    nextEndPoint.texs[2] = nBC.Append(unitStrokeWidthRatio).Append(1);

                    ioExplicitPath.emplace_back();
                    Segment& back = ioExplicitPath.back();
                    oEndPoints.emplace_back();
                    SegmentEndPoints& backEndPoints = oEndPoints.back();

                    back.type = Segment::Type::Line;
                    back.direction[0] = back.direction[1] = t;
                    back.position[0] = back.position[1] = B;
                    backEndPoints._[0].pointCount = 2;
                    backEndPoints._[0].positions[extIndex] = prevEndPoint.positions[extIndex];
                    backEndPoints._[0].positions[1-extIndex] = B;
                    backEndPoints._[0].texs[extIndex] = (n * (1+extSign*halfMaxStrokeWidthRatio)).Append(unitStrokeWidthRatio).Append(1);
                    backEndPoints._[0].texs[1-extIndex] = n.Append(unitStrokeWidthRatio).Append(1);
                    if(dot(tAB, tBC) < -0.01f)
                    {
                        backEndPoints._[1].pointCount = 3;
                        float2 const d2 = extSign*nAB + tAB;
                        float2 const d3 = extSign*nBC - tBC;
                        float2 const extP2 = B + d2 * halfWidth;
                        float2 const extP3 = B + d3 * halfWidth;
                        backEndPoints._[1].positions[extIndex] = extP2;
                        backEndPoints._[1].positions[2] = extP3;
                        float const dotd2n = dot(d2, n);
                        float const dotd3n = dot(d3, n);
                        backEndPoints._[1].texs[extIndex] = (n * (1+dotd2n*halfMaxStrokeWidthRatio)).Append(unitStrokeWidthRatio).Append(1);
                        backEndPoints._[1].texs[2] = (n * (1+dotd3n*halfMaxStrokeWidthRatio)).Append(unitStrokeWidthRatio).Append(1);
                    }
                    else
                    {
                        backEndPoints._[1].pointCount = 2;
                        backEndPoints._[1].positions[extIndex] = prevExtP;
                        float const dotBextPn = dot(prevExtP-B, n);
                        backEndPoints._[1].texs[extIndex] = (n * (1+dotBextPn*halfMaxStrokeWidthRatio/halfWidth)).Append(unitStrokeWidthRatio).Append(1);
                    }
                    backEndPoints._[1].positions[1-extIndex] = nextEndPoint.positions[extIndex];
                    backEndPoints._[1].texs[1-extIndex] = backEndPoints._[0].texs[extIndex];
                }
                else
                {
                    float2 const extP = prevEndPoint.positions[extIndex];
                    prevEndPoint.pointCount = 3;
                    prevEndPoint.positions[extIndex] = B + extSign * halfWidth * nAB;
                    prevEndPoint.positions[2] = B;
                    prevEndPoint.texs[2] = nAB.Append(unitStrokeWidthRatio).Append(1);
                    nextEndPoint.pointCount = 3;
                    nextEndPoint.positions[extIndex] = B + extSign * halfWidth * nBC;
                    nextEndPoint.positions[2] = B;
                    nextEndPoint.texs[2] = nBC.Append(unitStrokeWidthRatio).Append(1);

                    ioExplicitPath.emplace_back();
                    Segment& back = ioExplicitPath.back();
                    oEndPoints.emplace_back();
                    SegmentEndPoints& backEndPoints = oEndPoints.back();

                    // TODO
                    SG_UNUSED(back);
                    SG_UNUSED(backEndPoints);
                    SG_ASSERT_NOT_IMPLEMENTED();
                }
            }
        }
        else if(Segment::Type::Line == prev.type && Segment::Type::Arc == next.type)
        {
        // todo
            prevEndPoint.pointCount = 2;
            prevEndPoint.positions[0] = prev.position[1];
            prevEndPoint.positions[1] = prev.position[1] + float2(0.1f, 0.1f);
            prevEndPoint.texs[0] = float4(0,0,1,1);
            prevEndPoint.texs[1] = float4(0,0,1,1);
            nextEndPoint.pointCount = 2;
            nextEndPoint.positions[0] = prev.position[1];
            nextEndPoint.positions[1] = prev.position[1] + float2(0.1f, 0.1f);
            nextEndPoint.texs[0] = float4(0,0,1,1);
            nextEndPoint.texs[1] = float4(0,0,1,1);
        }
        else if(Segment::Type::Arc == prev.type && Segment::Type::Line == next.type)
        {
        // todo
            prevEndPoint.pointCount = 2;
            prevEndPoint.positions[0] = prev.position[1];
            prevEndPoint.positions[1] = prev.position[1] + float2(0.1f, 0.1f);
            prevEndPoint.texs[0] = float4(0,0,1,1);
            prevEndPoint.texs[1] = float4(0,0,1,1);
            nextEndPoint.pointCount = 2;
            nextEndPoint.positions[0] = prev.position[1];
            nextEndPoint.positions[1] = prev.position[1] + float2(0.1f, 0.1f);
            nextEndPoint.texs[0] = float4(0,0,1,1);
            nextEndPoint.texs[1] = float4(0,0,1,1);
        }
        else if(Segment::Type::Arc == prev.type && Segment::Type::Arc == next.type)
        {
        // todo
            prevEndPoint.pointCount = 2;
            prevEndPoint.positions[0] = prev.position[1];
            prevEndPoint.positions[1] = prev.position[1] + float2(0.1f, 0.1f);
            prevEndPoint.texs[0] = float4(0,0,1,1);
            prevEndPoint.texs[1] = float4(0,0,1,1);
            nextEndPoint.pointCount = 2;
            nextEndPoint.positions[0] = prev.position[1];
            nextEndPoint.positions[1] = prev.position[1] + float2(0.1f, 0.1f);
            nextEndPoint.texs[0] = float4(0,0,1,1);
            nextEndPoint.texs[1] = float4(0,0,1,1);
        }
        else
            SG_ASSUME_NOT_REACHED();
    }
    if(!iIsClosed)
    {
        Segment& first = ioExplicitPath[0];
        SegmentEndPoint& firstEndPoint = oEndPoints[0]._[0];
        Segment& last = ioExplicitPath[size-1];
        SegmentEndPoint& lastEndPoint = oEndPoints[size-1]._[1];

        float2 const firstOrthoDir = Orthogonal(first.direction[0]).Normalised();
        float2 const firstOrthoDirXHalfWidth = firstOrthoDir * halfWidth;
        firstEndPoint.pointCount = 2;
        // NB: This is LineCap::Butt mode
        firstEndPoint.positions[0] = first.position[0] + firstOrthoDirXHalfWidth;
        firstEndPoint.positions[1] = first.position[0] - firstOrthoDirXHalfWidth;
        if(Segment::Type::Line == first.type)
        {
            float const maxStrokeWidthRatio = 1.f;
            float const halfMaxStrokeWidthRatio = 0.5f * maxStrokeWidthRatio;
            float const radius = 2 * halfWidth;
            float const ooRadius = 1.f/radius;
            float const unitStrokeWidthRatio = ooRadius;

            firstEndPoint.texs[0] = (firstOrthoDir * (1+halfMaxStrokeWidthRatio)).Append(unitStrokeWidthRatio).Append(1);
            firstEndPoint.texs[1] = (firstOrthoDir * (1-halfMaxStrokeWidthRatio)).Append(unitStrokeWidthRatio).Append(1);
        }
        else if(Segment::Type::Arc == first.type)
        {
             // TODO
            firstEndPoint.texs[0] = float4(0,0,1,1);
            firstEndPoint.texs[1] = float4(0,0,1,1);
        }


        float2 const lastOrthoDir = Orthogonal(last.direction[1]).Normalised();
        float2 const lastOrthoDirXHalfWidth = lastOrthoDir * halfWidth;
        lastEndPoint.pointCount = 2;
        lastEndPoint.positions[0] = last.position[1] + lastOrthoDirXHalfWidth;
        lastEndPoint.positions[1] = last.position[1] - lastOrthoDirXHalfWidth;
        if(Segment::Type::Line == last.type)
        {
            float const maxStrokeWidthRatio = 1.f;
            float const halfMaxStrokeWidthRatio = 0.5f * maxStrokeWidthRatio;
            float const radius = 2 * halfWidth;
            float const ooRadius = 1.f/radius;
            float const unitStrokeWidthRatio = ooRadius;

            lastEndPoint.texs[0] = (lastOrthoDir * (1+halfMaxStrokeWidthRatio)).Append(unitStrokeWidthRatio).Append(1);
            lastEndPoint.texs[1] = (lastOrthoDir * (1-halfMaxStrokeWidthRatio)).Append(unitStrokeWidthRatio).Append(1);
        }
        else if(Segment::Type::Arc == last.type)
        {
             // TODO
            lastEndPoint.texs[0] = float4(0,0,1,1);
            lastEndPoint.texs[1] = float4(0,0,1,1);
        }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CirclePathDrawer::DrawLine(DrawContext const& iContext,
                                float2 const& iPos,
                                Segment const& iSegment,
                                SegmentEndPoints const& iEndPoints,
                                DrawParameters const& iDrawParameters,
                                size_t layer) const
{
    SG_UNUSED(iPos);
    SG_ASSERT_NOT_REACHED();
    SG_ASSERT_AND_UNUSED(Segment::Type::Line == iSegment.type);
    // Using the shader used to draw circles, we can draw line by keeping the
    // same texture coordinates along the line direction. The radius of the
    // hacked circle must be greater than half the stroke thickness for that to
    // work. We use the stroke thickness, arbitrarily.

    SegmentEndPoints endPoints = iEndPoints;

    box2f bbox;
    Context::TransformType const transformType = iContext.GetTransformType();
    switch(transformType)
    {
    case Context::TransformType::None:
        {
            for_range(size_t, e, 0, 2)
                for_range(size_t, ei, 0, endPoints._[e].pointCount)
                    bbox.Grow(endPoints._[e].positions[ei]);
        }
        break;
    case Context::TransformType::Translate2D:
        {
            float4x4 const& transform = iContext.GetTransform();
            float2 const T = transform.Col(3).xy();
            for_range(size_t, e, 0, 2)
                for_range(size_t, ei, 0, endPoints._[e].pointCount)
                {
                    float2& pos = endPoints._[e].positions[ei];
                    pos += T;
                    bbox.Grow(pos);
                }
        }
        break;
    case Context::TransformType::Transform2D:
        {
            float4x4 const& transform = iContext.GetTransform();
            float2x2 const R = transform.SubMatrix<2, 2>(0, 0);
            float2 const T = transform.Col(3).xy();
            for_range(size_t, e, 0, 2)
                for_range(size_t, ei, 0, endPoints._[e].pointCount)
                {
                    float2& pos = endPoints._[e].positions[ei];
                    pos = R * pos + T;
                    bbox.Grow(pos);
                }
        }
        break;
    case Context::TransformType::Transform3D:
        SG_ASSERT_NOT_IMPLEMENTED();
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }

    if(-1 == layer)
    {
        LayerManager* layerManager = iContext.GetLayerManager();
        layer = layerManager->Lock(bbox)[0];
    }

    rendering::TransientRenderBatch::Properties properties;
    properties.indexSize = sizeof(u32);
    properties.vertexSize = sizeof(CirclePathDrawerVertex);
    rendering::TransientRenderBatch* renderBatch = iContext.GetCompositingLayer()->GetOrCreateTransientRenderBatch(
        *m_material,
        properties);

    float const strokeWidth = iDrawParameters.strokeWidth;
    size_t const vertexCount = endPoints._[0].pointCount + endPoints._[1].pointCount;
    size_t const indexCount = (vertexCount - 2) * 3;

    rendering::TransientRenderBatch::WriteAccess<CirclePathDrawerVertex> writeAccess(*renderBatch, vertexCount, indexCount, layer);
    CirclePathDrawerVertex* vertices = writeAccess.GetVertexPointerForWriting();
    size_t i = 0;
    for_range(size_t, e, 0, 2)
        for_range(size_t, ei, 0, endPoints._[e].pointCount)
        {
            float2& pos = endPoints._[e].positions[ei];
            float4 tex = endPoints._[e].texs[ei];
            tex._[2] *= strokeWidth;
            vertices[i].pos = pos;
            vertices[i].tex = tex;
            vertices[i].col[0] = float4(iDrawParameters.strokeColor);
            vertices[i].col[1] = float4(0,0,0,0);
            ++i;
        }
    size_t const n = endPoints._[0].pointCount;
    size_t const m = endPoints._[1].pointCount;
    SG_ASSERT(2 <= n && n <= 3);
    SG_ASSERT(1 <= m && m <= 3);
    bool const showBackFace = true; // For UI
    if(showBackFace)
    {
        writeAccess.PushIndices(0, n, n-1);
        if(m > 1)
            writeAccess.PushIndices(n-1, n, n+m-1);
        if(n > 2)
            writeAccess.PushIndices(1, 2, n+m-1);
        if(m > 2)
            writeAccess.PushIndices(1, n+2, n+1);
    }
    else
    {
        writeAccess.PushIndices(0, n-1, n);
        if(m > 1)
            writeAccess.PushIndices(n, n-1, n+m-1);
        if(n > 2)
            writeAccess.PushIndices(2, 1, n+m-1);
        if(m > 2)
            writeAccess.PushIndices(1, n+1, n+2);
    }
    writeAccess.FinishWritingVertex(vertexCount);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CirclePathDrawer::DrawArc(DrawContext const& iContext,
                               float2 const& iPos,
                               Segment const& iSegment,
                               SegmentEndPoints const& iEndPoints,
                               DrawParameters const& iDrawParameters,
                               size_t layer) const
{
    SG_UNUSED(iPos);
    SG_ASSERT_NOT_REACHED();
    SG_ASSERT(Segment::Type::Arc == iSegment.type);
    SegmentEndPoints endPoints = iEndPoints;
    float const aaMaxRadius = 1;
    float const radius = iSegment.radiusIFA;
    float const extRadius = radius + 0.5f * iDrawParameters.strokeWidth + aaMaxRadius;
    float const intRadius = radius - 0.5f * iDrawParameters.strokeWidth - aaMaxRadius;
    float const ooRadius = 1.f / radius;
    float const strokeWidthRatio = iDrawParameters.strokeWidth * ooRadius;

    //              1               3               5
    // interior     +               +               +
    //                                                  ...
    // exterior     +       +               +
    //              0       2 <-----------> 4
    //                           Sector
    //
    // Having exterior end-points in the middle of a sector is interesting for
    // joining with previous or next arc or line as such point can be placed
    // nearly as possible to the curve.
    size_t const MAX_SECTOR_COUNT = 8;
    size_t const MAX_HALF_SECTOR_COUNT = 2 * MAX_SECTOR_COUNT;
    float const extVertexDistanceRatio = 1.f / cos(float(2 * math::constant::PI / (2 * MAX_SECTOR_COUNT)));
    bool const reverseDirection = iSegment.arcAngleIFA[0] > iSegment.arcAngleIFA[1];
    size_t const begin = reverseDirection ? 1 : 0;
    size_t const end = reverseDirection ? 0 : 1;
    float const unitAngleBegin = iSegment.arcAngleIFA[begin];
    float const unitAngleEnd = iSegment.arcAngleIFA[end];
    float const fullDeltaAngle = unitAngleEnd - unitAngleBegin;
    SG_ASSERT(fullDeltaAngle >= 0.f);
    SG_ASSERT(fullDeltaAngle <= 1.f);
    bool const closed = 1.f == fullDeltaAngle;
    size_t const sectorCount = ceili(fullDeltaAngle * MAX_SECTOR_COUNT);
    size_t const halfSectorCount = sectorCount * 2;
    SG_ASSERT(!closed || halfSectorCount == MAX_HALF_SECTOR_COUNT);
    float const halfSectorAngle = fullDeltaAngle / halfSectorCount;
    float const cos0 = cos(unitAngleBegin * float(2 * math::constant::PI));
    float const sin0 = sin(unitAngleBegin * float(2 * math::constant::PI));
    float2 const dir0 = float2(cos0, sin0);
    float2x2 const RhalfSector = matrix::Rotation(halfSectorAngle * float(2 * math::constant::PI));
    struct PreVertex
    {
        float2 pos;
        float2 tex;
    };
    size_t const MAX_PREVERTEX_COUNT = MAX_HALF_SECTOR_COUNT + 3;
    PreVertex preVertices[MAX_PREVERTEX_COUNT];
    size_t const triangleCount = closed ? MAX_HALF_SECTOR_COUNT : halfSectorCount + 1;
    size_t const vertexCount = closed ? MAX_HALF_SECTOR_COUNT + 1 : halfSectorCount + 3;
    float2 const center = iSegment.centerIFA;
    float2 dir = dir0;
    float radii[2] = { extVertexDistanceRatio * extRadius, intRadius };
    {
        float2 const u = dir * extRadius;
        preVertices[0].pos = center + u;
        preVertices[0].tex = u * ooRadius;
    }
    for_range(size_t, i, 2, vertexCount-2)
    {
        float2 const u = dir * radii[i%2];
        preVertices[i].pos = center + u;
        preVertices[i].tex = u * ooRadius;
        if(closed || i < (vertexCount-2))
            dir = RhalfSector * dir;
    }
    {
        float2 const u = dir * extRadius;
        preVertices[vertexCount-1].pos = center + u;
        preVertices[vertexCount-1].tex = u * ooRadius;
    }

    box2f bbox;
    Context::TransformType const transformType = iContext.GetTransformType();
    switch(transformType)
    {
    case Context::TransformType::None:
        {
            for_range(size_t, i, 0, (vertexCount+1)/2)
                bbox.Grow(preVertices[2*i].pos);
        }
        break;
    case Context::TransformType::Translate2D:
        {
            float4x4 const& transform = iContext.GetTransform();
            float2 const T = transform.Col(3).xy();
            for_range(size_t, i, 0, vertexCount)
            {
                preVertices[i].pos += T;
                bbox.Grow(preVertices[i].pos);
            }
        }
        break;
    case Context::TransformType::Transform2D:
        {
            float4x4 const& transform = iContext.GetTransform();
            float2x2 const R = transform.SubMatrix<2, 2>(0, 0);
            float2 const T = transform.Col(3).xy();
            for_range(size_t, i, 0, vertexCount)
            {
                preVertices[i].pos = R * preVertices[i].pos + T;
                bbox.Grow(preVertices[i].pos);
            }
        }
        break;
    case Context::TransformType::Transform3D:
        SG_ASSERT_NOT_IMPLEMENTED();
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }

    if(-1 == layer)
    {
        LayerManager* layerManager = iContext.GetLayerManager();
        layer = layerManager->Lock(bbox)[0];
    }

    rendering::TransientRenderBatch::Properties properties;
    properties.indexSize = sizeof(u32);
    properties.vertexSize = sizeof(CirclePathDrawerVertex);
    rendering::TransientRenderBatch* renderBatch = iContext.GetCompositingLayer()->GetOrCreateTransientRenderBatch(
        *m_material,
        properties);

    rendering::TransientRenderBatch::WriteAccess<CirclePathDrawerVertex> writeAccess(*renderBatch, vertexCount, triangleCount*3, layer);
    CirclePathDrawerVertex* vertices = writeAccess.GetVertexPointerForWriting();
    for_range(size_t, i, 0, vertexCount)
    {
        vertices[i].pos = preVertices[i].pos;
        vertices[i].tex = preVertices[i].tex.Append(strokeWidthRatio).Append(1);
        vertices[i].col[0] = float4(iDrawParameters.strokeColor);
        vertices[i].col[1] = float4(0,0,0,0);
    }

    bool const showBackFace = true; // For UI
    if((showBackFace && !reverseDirection) || (!showBackFace && reverseDirection) )
    {
        for_range(size_t, i, 0, closed ? triangleCount-2 : triangleCount)
        {
            if(0 == i%2) writeAccess.PushIndices(i+0, i+1, i+2);
            else if(1 == i%2) writeAccess.PushIndices(i+0, i+2, i+1);
        }
        if(closed)
        {
            size_t const i = triangleCount-2;
            SG_ASSERT(0 == i%2);
            writeAccess.PushIndices(i+0, 0, i+1);
            writeAccess.PushIndices(i+1, 0, 1);
        }
    }
    else
    {
        for_range(size_t, i, 0, closed ? triangleCount-2 : triangleCount)
        {
            if(0 == i%2) writeAccess.PushIndices(i+0, i+2, i+1);
            else if(1 == i%2) writeAccess.PushIndices(i+0, i+1, i+2);
        }
        if(closed)
        {
            size_t const i = triangleCount-2;
            SG_ASSERT(0 == i%2);
            writeAccess.PushIndices(i+1, 0, i+0);
            writeAccess.PushIndices(1, 0, i+1);
        }
    }
    writeAccess.FinishWritingVertex(vertexCount);

}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CirclePathDrawer::DrawSegment(DrawContext const& iContext,
                                   float2 const& iPos,
                                   Segment const& iSegment,
                                   SegmentEndPoints const& iEndPoints,
                                   DrawParameters const& iDrawParameters,
                                   size_t layer) const
{
    switch(iSegment.type)
    {
    case Segment::Type::Line:
        DrawLine(iContext, iPos, iSegment, iEndPoints, iDrawParameters, layer);
        break;
    case Segment::Type::Arc:
        DrawArc(iContext, iPos, iSegment, iEndPoints, iDrawParameters, layer);
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CirclePathDrawer::DrawPath(DrawContext const& iContext,
                                float2 const& iPos,
                                ArrayView<circlepath::Node> iPath,
                                GeometryParameters const& iGeometryParameters,
                                DrawParameters const& iDrawParameters,
                                size_t layer) const
{
#if 1
    Tesselation tesselation;
    ComputeTesselation(iPath, tesselation, iGeometryParameters);
    Draw(iContext, iPos, tesselation, iDrawParameters, layer);
#else
    bool const isClosed = iPath.back().type == circlepath::NodeType::Close;
    // Generate tesselation
    std::vector<Segment> explicitPath;
    ExplicitPath(explicitPath, iPath);
    std::vector<SegmentEndPoints> endPoints;
    ComputeCapsAndEndPoints(explicitPath, endPoints, isClosed, iGeometryParameters);

    for_range(size_t, i, 0, explicitPath.size())
    {
        Segment const& segment = explicitPath[i];
#if 1
        DrawSegment(iContext, iPos, segment, endPoints[i], iDrawParameters, layer);
#else
        switch(segment.type)
        {
        case Segment::Type::Line:
        {
            if(segment.position[0] != segment.position[1])
                DrawLineProto(iContext, segment.position[0] + iPos, segment.position[1] + iPos, iDrawParameters, layer);
            break;
        }
        case Segment::Type::Arc:
        {
            float unitAngleBegin = segment.arcAngleIFA[0];
            float unitAngleEnd = segment.arcAngleIFA[1];
            if(unitAngleBegin > unitAngleEnd) { std::swap(unitAngleBegin, unitAngleEnd); }
            DrawArcProto(iContext, segment.centerIFA + iPos, segment.radiusIFA, unitAngleBegin, unitAngleEnd, iDrawParameters, layer);
            break;
        }
        default:
            SG_ASSUME_NOT_REACHED();
        }
#endif
    }
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
namespace {
    // This is the number of points used to represent the intersections at the
    // interior of the joint. In case of an intersection of an arc with a line
    // for instance, the intersection when changing the stroke width is not on
    // a line. Hence we may need multiple points.
    size_t const MAX_JOINT_INTERIOR_POINT_COUNT = 6;
    // This is the total number of points needed at the end of a segment. It
    // comprises the previous number, plus the central point, plus the point at
    // the exterior of the joint that is most often not shared with the
    // following segment.
    size_t const MAX_JOINT_POINT_COUNT = MAX_JOINT_INTERIOR_POINT_COUNT + 2;
    // This is the maximum number of points of the filling polygon that can be
    // generated by a joint.
    size_t const MAX_JOINT_INSIDE_POINT_COUNT = 3;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CirclePathDrawer::ComputeTesselation(ArrayView<circlepath::Node> iPath,
                                          Tesselation& oTesselation,
                                          GeometryParameters const& iGeometryParameters) const
{
    oTesselation.outline.Clear();
    oTesselation.convexHull.clear();
#if SG_ENABLE_ASSERT
    oTesselation.aaMaxRadius = iGeometryParameters.aaMaxRadius;
    oTesselation.maxStrokeWidth = iGeometryParameters.maxStrokeWidth;
#endif

    bool const isClosed = iPath.back().type == circlepath::NodeType::Close;
    oTesselation.isClosed = isClosed;

    std::vector<Segment> explicitPath;
    ExplicitPath(explicitPath, iPath);

    struct VertexData
    {
        enum Type { Interior, Exterior, Cap, None } type;
        VertexData(Type iType) : type(iType) {}
    };
    //auto MakeVertexData = [](VertexData::Type type) { VertexData d; d.type = type; return d; };
    struct PolygonData
    {
        float2 O;
        float2 tex;
        float2x2 dtexdxy;
        float2 texzw;
    };
    geometry::TriangulatorEx<VertexData, PolygonData> triangulator;
    std::vector<u16> interiorIndices;

    float const halfWidth = 0.5f * iGeometryParameters.maxStrokeWidth + iGeometryParameters.aaMaxRadius;
    float const width = 2 * halfWidth;
    float const ooWidth= 1.f/width;

    // Remember that we will draw lines using circle shader to draw a unit
    // circle with a unit stroke. The geometry spans radius range [0.5, 1.5].
    float const lineMaxStrokeWidthRatio = 1.f;
    float const lineHalfMaxStrokeWidthRatio = 0.5f * lineMaxStrokeWidthRatio;
    // NB: the following value will be multiplied by stroke value before being
    // sent to the shader. In case the stroke value is equal to Width, we would
    // want the value sent to the shader to be 1.
    float const lineStrokeWidthFactor = ooWidth;

#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
    size_t const dbgSize = 512;
    float const dbgOccupation = 0.9f;
    image::RGBImage dbgImg((uint2(dbgSize)));
    box2f bbox;
    {
        dbgImg.Fill(ubyte3(0));
        for(auto const& s : explicitPath)
        {
            if(Segment::Type::Arc == s.type)
            {
                box2f const arcbbox = box2f::FromCenterDelta(s.centerIFA, 2*float2(s.radiusIFA));
                bbox.Grow(arcbbox);
            }
            else
            {
                for_range(size_t, i, 0, 2)
                {
                    float2 const& xy = s.position[i];
                    bbox.Grow(xy);
                }
            }
        }
        bbox.min -= float2(halfWidth);
        bbox.max += float2(halfWidth);
    }
    image::WindowToViewport toDbgFrame(bbox, box2f::FromCenterDelta(0.5f * dbgImg.WidthHeight(), 0.96f * dbgImg.WidthHeight()));
    auto brush = image::brush::Blend<image::blend::Classic>();
#endif

#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
    auto DebugDrawPoint = [&triangulator, &dbgImg, &toDbgFrame, &brush](size_t index)
    {
        auto ps = triangulator.GetVertexPositions();
        auto data = triangulator.GetVertexData();
        u32 col = 0xaaaaaa;
        switch(data[index].type)
        {
        case VertexData::Interior: col = 0x88ee88; break;
        case VertexData::Exterior: col = 0xee8888; break;
        }
        float2 const P = ps[index];
        image::DrawRect(dbgImg, box2f::FromCenterDelta(toDbgFrame(P), float2(3)), brush.Stroke(ubyte3((col>>16)&0xff,(col>>8)&0xff,col&0xff),1));
        //image::DrawTextA(dbgImg, toDbgFrame(P) + float2(5, 0), Format("%0", index), brush.Fill(ubyte3((col>>16)&0xff,(col>>8)&0xff,col&0xff)));
    };
    auto DebugDrawLine = [&triangulator, &dbgImg, &toDbgFrame, &brush](size_t indexA, size_t indexB)
    {
        auto ps = triangulator.GetVertexPositions();
        auto data = triangulator.GetVertexData();
        float2 const A = ps[indexA];
        float2 const B = ps[indexB];
        auto const typeA = data[indexA].type;
        auto const typeB = data[indexB].type;
        u32 col = 0x777777;
        if(typeA == typeB)
        {
            switch(typeA)
            {
            case VertexData::Interior: col = 0x66cc66; break;
            case VertexData::Exterior: col = 0xcc6666; break;
            }
        }
        image::DrawLine1pxNoAA(dbgImg, roundi(toDbgFrame(A)), roundi(toDbgFrame(B)), brush.Stroke(ubyte3((col>>16)&0xff,(col>>8)&0xff,col&0xff),1));
    };
#endif

    auto GenerateJoint = [
        halfWidth, lineStrokeWidthFactor, lineHalfMaxStrokeWidthRatio, ooWidth, isClosed,
        &iGeometryParameters, &triangulator, &interiorIndices
        ONLY_WITH_CIRCLE_PATH_DEBUG_IMAGE(SG_COMMA &dbgImg SG_COMMA &DebugDrawPoint SG_COMMA &DebugDrawLine SG_COMMA &toDbgFrame)
    ]
        (Segment const& a, Segment const& b,
         size_t (&points)[2][MAX_JOINT_POINT_COUNT], size_t (&pointCounts)[2],
         size_t (&jointInsidePoints)[MAX_JOINT_INSIDE_POINT_COUNT], size_t& jointInsidePointCount)
    {
        SG_ASSERT(0 == jointInsidePointCount);

        // NB: points will be returned in the order they have on the orientated
        // polygons they are in. e.g. points[0] goes from right to left and
        // points[1] goes the other way.
        float2 const B = a.position[1];
        SG_ASSERT(EqualsWithTolerance(B, b.position[0], 0.0001f));
        float2 tAB(uninitialized);
        float2 tBC(uninitialized);
        if(Segment::Type::Line == a.type)
        {
            float2 const A = a.position[0];
            float2 const AB = B - A;
            tAB = AB.Normalised();
        }
        else if(Segment::Type::Arc == a.type)
        {
            tAB = a.direction[1];
            SG_ASSERT(EqualsWithTolerance(tAB.LengthSq(), 1.f, 0.0001f));
        }
        else
            SG_ASSUME_NOT_REACHED();

        if(Segment::Type::Line == b.type)
        {
            float2 const C = b.position[1];
            float2 const BC = C - B;
            tBC = BC.Normalised();
        }
        else if(Segment::Type::Arc == b.type)
        {
            tBC = b.direction[0];
            SG_ASSERT(EqualsWithTolerance(tBC.LengthSq(), 1.f, 0.0001f));
        }
        else
            SG_ASSUME_NOT_REACHED();

        float2 const nAB = Orthogonal(tAB);
        float2 const nBC = Orthogonal(tBC);

        // TODO: if tAB = tBC, generate left and right point and that's all.
        float const detABC = det(tAB, tBC);
        if(EqualsWithTolerance(detABC, 0.f, 0.01f))
        {
            float2 const BP = 0.5f*(nAB+nBC) * halfWidth;
            float2 const Pl = B + BP;
            float2 const Pr = B - BP;
            size_t const indexPl = triangulator.PushVertex(Pl, VertexData::Interior);
            size_t const indexPr = triangulator.PushVertex(Pr, VertexData::Exterior);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
            DebugDrawPoint(indexPr);
            DebugDrawPoint(indexPl);
#endif
            points[0][0] = indexPr;
            points[0][1] = indexPl;
            points[1][0] = indexPl;
            points[1][1] = indexPr;
            pointCounts[0] = 2;
            pointCounts[1] = 2;
            return;
        }

        // Compute mitter ends
        float2x2 M(uninitialized);
        M.SetRow(0, nAB);
        M.SetRow(1, nBC);
        float2x2 invM;
        bool ok = InvertROK(invM, M, 0.0001f);
        float2 BP = ok ? invM * float2(halfWidth) : nAB * halfWidth;
        SG_ASSERT(EqualsWithTolerance(dot(nAB, BP), halfWidth, 0.001f));
        SG_ASSERT(EqualsWithTolerance(dot(nBC, BP), halfWidth, 0.001f));
        float2 const mitterPl = B + BP;
        float2 const mitterPr = B - BP;

        float2 const mitterPs[] = { mitterPl, mitterPr };

        // ext: exterior of the curve
        size_t const extIndex = detABC > 0 ? 1 : 0;
        size_t const intIndex = 1-extIndex;
        float const extSign = 0 == extIndex ? 1.f : -1.f;
        float const intSign = -extSign;

        VertexData const intVertexData = 0 == intIndex ? VertexData::Interior : VertexData::Exterior;
        VertexData const extVertexData = 0 == extIndex ? VertexData::Interior : VertexData::Exterior;

        // from furthest to nearest
        size_t jointIntPoints[MAX_JOINT_INTERIOR_POINT_COUNT];
        size_t jointIntPointCount = 0;

        if(Segment::Type::Line == a.type && Segment::Type::Line == b.type)
        {
            size_t const index = triangulator.PushVertex(mitterPs[intIndex], intVertexData);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
            DebugDrawPoint(index);
#endif
            jointIntPoints[0] = index;
            jointIntPointCount = 1;
        }
        else if(Segment::Type::Line == a.type && Segment::Type::Arc == b.type
           || Segment::Type::Arc == a.type && Segment::Type::Line == b.type)
        {
            size_t const arcIndex = Segment::Type::Line == a.type ? 1 : 0;
            Segment const& arc = Segment::Type::Line == a.type ? b : a;
            float2 nLine = 1 == arcIndex ? nAB : nBC;
            float2 tLine = 1 == arcIndex ? tAB : tBC;
            float const turnSign = arc.arcAngleIFA[0] > arc.arcAngleIFA[1] ? -1.f : 1.f;
            float const halfMaxStrokeWidth = 0.5f * iGeometryParameters.maxStrokeWidth;
            float2 Ps[MAX_JOINT_INTERIOR_POINT_COUNT];

            // The following is a big heuristic to reduce number of points
            // dependending on what is needed.
            float const dotABC = dot(tAB, tBC);
            float const angleFactor = dotABC < 0 ? 1+sq(dotABC) : sq(detABC); // sq as the circle is locally quadratic.
            float const radiusFactor = halfMaxStrokeWidth / arc.radiusIFA;
            float const reduceCountFactor = saturate(angleFactor * radiusFactor * 6);

            size_t const count = roundi(1 + (MAX_JOINT_INTERIOR_POINT_COUNT-1) * reduceCountFactor);
            float const ooCount = 1.f/count;
            for_range(size_t, i, 0, count)
            {
                float const t = tweening::decelerate((count-i)*ooCount);
                float2 const O = B + t * intSign * nLine * halfMaxStrokeWidth;
                float const R = arc.radiusIFA - t * intSign * turnSign * halfMaxStrokeWidth;
                using namespace geometry;
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
                auto brush = image::brush::Blend<image::blend::PremultipliedRGBA>();
                image::DrawLine1pxNoAA(dbgImg, roundi(toDbgFrame(a.position[0])), roundi(toDbgFrame(B)), brush.Stroke(ubyte4(64, 64, 64, 0), 1));
                image::DrawLine1pxNoAA(dbgImg, roundi(toDbgFrame(b.position[1])), roundi(toDbgFrame(B)), brush.Stroke(ubyte4(64, 64, 64, 0), 1));
                image::DrawCircle1pxNoAA(dbgImg, roundi(toDbgFrame(arc.centerIFA)), roundi(toDbgFrame.Rescale(float2(arc.radiusIFA)).x()), brush.Stroke(ubyte4(32, 32, 32, 0), 1));
                image::DrawCircle1pxNoAA(dbgImg, roundi(toDbgFrame(arc.centerIFA)), roundi(toDbgFrame.Rescale(float2(R)).x()), brush.Stroke(ubyte4(64, 64, 64, 0), 1));
                image::DrawLine1pxNoAA(dbgImg, roundi(toDbgFrame(O)), roundi(toDbgFrame(O+tLine)), brush.Stroke(ubyte4(64, 64, 64, 0), 1));
#endif
                auto const r = LineToCircle(Point(O), Vector(tLine), Point(arc.centerIFA), R);
                SG_ASSERT(r.first);
                float2 const P0 = r.second.first;
                float2 const P1 = r.second.second;
                float const d0 = (P0-B).LengthSq();
                float const d1 = (P1-B).LengthSq();
                float2 const P = d0 < d1 ? P0 : P1;
                Ps[i] = P;
            }
            float2 const Pex = lerp(count > 1 ? Ps[1]: B,Ps[0],1.f+(halfWidth-halfMaxStrokeWidth)/(halfMaxStrokeWidth*ooCount));
            Ps[0] = Pex;
            for_range(size_t, i, 0, count)
            {
                size_t const index = triangulator.PushVertex(Ps[i], intVertexData);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
                DebugDrawPoint(index);
#endif
                jointIntPoints[i] = index;
                ++jointIntPointCount;
            }
        }
        else if(Segment::Type::Arc == a.type && Segment::Type::Arc == b.type)
        {
            float const aTurnSign = a.arcAngleIFA[0] > a.arcAngleIFA[1] ? -1.f : 1.f;
            float const bTurnSign = b.arcAngleIFA[0] > b.arcAngleIFA[1] ? -1.f : 1.f;
            float const halfMaxStrokeWidth = 0.5f * iGeometryParameters.maxStrokeWidth;
            float2 Ps[MAX_JOINT_INTERIOR_POINT_COUNT];

            // The following is a big heuristic to reduce number of points
            // dependending on what is needed.
            float const dotABC = dot(tAB, tBC);
            float const angleFactor = dotABC < 0 ? 1+sq(dotABC) : sq(detABC); // sq as the circle is locally quadratic.
            float const sumRadius = a.radiusIFA + b.radiusIFA;
            float const diffRadius = std::abs(a.radiusIFA - b.radiusIFA);
            float const minRadius = 0.5f * (sumRadius - diffRadius);
            float const diffRadiusFactor = diffRadius / sumRadius;
            float const radiusFactor = halfMaxStrokeWidth / minRadius;
            float const reduceCountFactor = saturate(angleFactor * diffRadiusFactor * radiusFactor * 6);

            size_t const count = roundi(1 + (MAX_JOINT_INTERIOR_POINT_COUNT-1) * reduceCountFactor);
            float const ooCount = 1.f/count;
            for_range(size_t, i, 0, count)
            {
                float const t = tweening::decelerate((count-i)*ooCount);
                float const Ra = a.radiusIFA - t * intSign * aTurnSign * halfMaxStrokeWidth;
                float const Rb = b.radiusIFA - t * intSign * bTurnSign * halfMaxStrokeWidth;
                using namespace geometry;
                auto const r = CircleToCircle(Point(a.centerIFA), Ra, Point(b.centerIFA), Rb);
                SG_ASSERT(r.first);
                float2 const P0 = r.second.first;
                float2 const P1 = r.second.second;
                float const d0 = (P0-B).LengthSq();
                float const d1 = (P1-B).LengthSq();
                float2 const P = d0 < d1 ? P0 : P1;
                Ps[i] = P;
            }
            float2 const Pex = lerp(count > 1 ? Ps[1]: B,Ps[0],1.f+(halfWidth-halfMaxStrokeWidth)/(halfMaxStrokeWidth*ooCount));
            Ps[0] = Pex;
            for_range(size_t, i, 0, count)
            {
                size_t const index = triangulator.PushVertex(Ps[i], intVertexData);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
                DebugDrawPoint(index);
#endif
                jointIntPoints[i] = index;
                ++jointIntPointCount;
            }
        }
        else
            SG_ASSUME_NOT_REACHED();

        LineJoin lineJoin = iGeometryParameters.strokeLineJoin;
        if(LineJoin::Mitter == lineJoin)
        {
            float const mitterLengthSq = (mitterPs[extIndex] - B).LengthSq();
            float const maxMitterLengthSq = sq(iGeometryParameters.strokeMitterLimit * iGeometryParameters.maxStrokeWidth);
            if(mitterLengthSq > maxMitterLengthSq)
                lineJoin = iGeometryParameters.strokeLineJoinForMitterLimit;
        }

        auto PushPoints = [
            &points, &pointCounts, intIndex, extIndex, &jointIntPointCount, &jointIntPoints
        ]
            (size_t aExtPoint, size_t bExtPoint, size_t indexB)
        {
            bool pushB = size_t(-1) != indexB;
            size_t const pointCount = jointIntPointCount + (pushB ? 2 : 1);
            for_range(size_t, i, 0, jointIntPointCount)
            {
                points[intIndex][pointCount-1-i] = jointIntPoints[i];
                points[extIndex][i] = jointIntPoints[i];
            }
            points[0][0 == intIndex ? 0 : pointCount - 1] = aExtPoint;
            points[1][0 == intIndex ? pointCount - 1 : 0] = bExtPoint;
            if(pushB)
            {
                points[intIndex][1] = indexB;
                points[extIndex][pointCount - 2] = indexB;
            }
            pointCounts[0] = pointCount;
            pointCounts[1] = pointCount;
        };

        if(LineJoin::Mitter == lineJoin)
        {
            size_t const indexMitterP = triangulator.PushVertex(mitterPs[extIndex], extVertexData);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
            DebugDrawPoint(indexMitterP);
#endif
            if(Segment::Type::Line == a.type && Segment::Type::Line == b.type)
            {
                PushPoints(indexMitterP, indexMitterP, size_t(-1));
            }
            else
            {
                size_t const indexB = triangulator.PushVertex(B, VertexData::None);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
                DebugDrawPoint(indexB);
#endif
                size_t aExtPoint;
                size_t bExtPoint;
                if(Segment::Type::Line == a.type)
                    aExtPoint = indexMitterP;
                else
                {
                    float2 const extP = B + extSign * nAB * halfWidth;
                    size_t const indexExtP = triangulator.PushVertex(extP, extVertexData);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
                    DebugDrawPoint(indexExtP);
#endif
                    if(isClosed && extIndex == 0)
                        jointInsidePoints[jointInsidePointCount++] = indexExtP;

                    float2 const duvAlongnAB = nAB;
                    PolygonData data;
                    data.O = B;
                    data.tex = -nAB;
                    data.dtexdxy.SetRow(0, duvAlongnAB.x() * nAB * ooWidth);
                    data.dtexdxy.SetRow(1, duvAlongnAB.y() * nAB * ooWidth);
                    data.texzw = float2(lineStrokeWidthFactor, 1);
                    size_t indices[] = { indexB, 0 == extIndex ? indexMitterP : indexExtP, 0 == extIndex ? indexExtP : indexMitterP };
                    triangulator.PushPolygon(data, indices[0], indices[1], indices[2]);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
                    DebugDrawLine(indices[0], indices[1]);
                    DebugDrawLine(indices[1], indices[2]);
                    DebugDrawLine(indices[2], indices[0]);
#endif

                    aExtPoint = indexExtP;
                }
                if(Segment::Type::Line == b.type)
                    bExtPoint = indexMitterP;
                else
                {
                    float2 const extP = B + extSign * nBC * halfWidth;
                    size_t const indexExtP = triangulator.PushVertex(extP, extVertexData);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
                    DebugDrawPoint(indexExtP);
#endif
                    if(isClosed && extIndex == 0)
                        jointInsidePoints[jointInsidePointCount++] = indexMitterP;

                    float2 const duvAlongnBC = nBC;
                    PolygonData data;
                    data.O = B;
                    data.tex = -nBC;
                    data.dtexdxy.SetRow(0, duvAlongnBC.x() * nBC * ooWidth);
                    data.dtexdxy.SetRow(1, duvAlongnBC.y() * nBC * ooWidth);
                    data.texzw = float2(lineStrokeWidthFactor, 1);
                    size_t indices[] = { indexB, 0 == extIndex ? indexExtP : indexMitterP, 0 == extIndex ? indexMitterP : indexExtP };
                    triangulator.PushPolygon(data, indices[0], indices[1], indices[2]);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
                    DebugDrawLine(indices[0], indices[1]);
                    DebugDrawLine(indices[1], indices[2]);
                    DebugDrawLine(indices[2], indices[0]);
#endif
                    bExtPoint = indexExtP;
                }
                PushPoints(aExtPoint, bExtPoint, indexB);
            }
        }
        else
        {
            SG_ASSERT(LineJoin::Bevel == lineJoin || LineJoin::Round == lineJoin);
            size_t const indexB = triangulator.PushVertex(B, VertexData::None);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
            DebugDrawPoint(indexB);
#endif

            float2 const t = (0.5f * (tAB + tBC)).Normalised();
            float2 const n = Orthogonal(t);
            float2x2 M0(uninitialized);
            M0.SetRow(0, nAB);
            M0.SetRow(1, n);
            float2x2 invM0;
            ok = InvertROK(invM0, M0, 0.0001f);
            SG_ASSERT(ok);
            float2 const d0 = ok ? invM0 * float2(halfWidth) : nAB * halfWidth;
            SG_ASSERT(EqualsWithTolerance(dot(nAB, d0), halfWidth, 0.001f));
            SG_ASSERT(EqualsWithTolerance(dot(n, d0), halfWidth, 0.001f));
            float2 const bevelP0 = B + extSign*d0;
            float2x2 M1(uninitialized);
            M1.SetRow(0, nBC);
            M1.SetRow(1, n);
            float2x2 invM1;
            ok = InvertROK(invM1, M1, 0.0001f);
            SG_ASSERT(ok);
            float2 const d1 = ok ? invM1 * float2(halfWidth) : nBC * halfWidth;
            SG_ASSERT(EqualsWithTolerance(dot(nBC, d1), halfWidth, 0.001f));
            SG_ASSERT(EqualsWithTolerance(dot(n, d1), halfWidth, 0.001f));
            float2 const bevelP1 = B + extSign*d1;

            if(LineJoin::Bevel == lineJoin)
            {
                size_t const indexBevelP0 = triangulator.PushVertex(bevelP0, extVertexData);
                size_t const indexBevelP1 = triangulator.PushVertex(bevelP1, extVertexData);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
                DebugDrawPoint(indexBevelP0);
                DebugDrawPoint(indexBevelP1);
#endif
                if(Segment::Type::Line == a.type && Segment::Type::Line == b.type)
                {
                    if(isClosed && extIndex == 0)
                        jointInsidePoints[jointInsidePointCount++] = indexBevelP0;

                    PushPoints(indexBevelP0, indexBevelP1, indexB);
                }
                else
                {
                    size_t aExtPoint;
                    size_t bExtPoint;
                    if(Segment::Type::Line == a.type)
                        aExtPoint = indexBevelP0;
                    else
                    {
                        float2 const extP = B + extSign * nAB * halfWidth;
                        size_t const indexExtP = triangulator.PushVertex(extP, extVertexData);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
                        DebugDrawPoint(indexExtP);
#endif

                        if(isClosed && extIndex == 0)
                        jointInsidePoints[jointInsidePointCount++] = indexExtP;

                        float2 const duvAlongnAB = nAB;
                        PolygonData data;
                        data.O = B;
                        data.tex = -nAB;
                        data.dtexdxy.SetRow(0, duvAlongnAB.x() * nAB * ooWidth);
                        data.dtexdxy.SetRow(1, duvAlongnAB.y() * nAB * ooWidth);
                        data.texzw = float2(lineStrokeWidthFactor, 1);
                        size_t indices[] = { indexB, 0 == extIndex ? indexBevelP0 : indexExtP, 0 == extIndex ? indexExtP : indexBevelP0 };
                        triangulator.PushPolygon(data, indices[0], indices[1], indices[2]);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
                        DebugDrawLine(indices[0], indices[1]);
                        DebugDrawLine(indices[1], indices[2]);
                        DebugDrawLine(indices[2], indices[0]);
#endif
                        aExtPoint = indexExtP;
                    }

                    if(isClosed && extIndex == 0)
                        jointInsidePoints[jointInsidePointCount++] = indexBevelP0;

                    if(Segment::Type::Line == b.type)
                        bExtPoint = indexBevelP1;
                    else
                    {
                        float2 const extP = B + extSign * nBC * halfWidth;
                        size_t const indexExtP = triangulator.PushVertex(extP, extVertexData);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
                        DebugDrawPoint(indexExtP);
#endif
                        if(isClosed && extIndex == 0)
                        jointInsidePoints[jointInsidePointCount++] = indexBevelP1;

                        float2 const duvAlongnBC = nBC;
                        PolygonData data;
                        data.O = B;
                        data.tex = -nBC;
                        data.dtexdxy.SetRow(0, duvAlongnBC.x() * nBC * ooWidth);
                        data.dtexdxy.SetRow(1, duvAlongnBC.y() * nBC * ooWidth);
                        data.texzw = float2(lineStrokeWidthFactor, 1);
                        size_t indices[] = { indexB, 0 == extIndex ? indexExtP : indexBevelP1, 0 == extIndex ? indexBevelP1 : indexExtP };
                        triangulator.PushPolygon(data, indices[0], indices[1], indices[2]);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
                        DebugDrawLine(indices[0], indices[1]);
                        DebugDrawLine(indices[1], indices[2]);
                        DebugDrawLine(indices[2], indices[0]);
#endif

                        bExtPoint = indexExtP;
                    }
                    PushPoints(aExtPoint, bExtPoint, indexB);
                }
                float2 const duvdn = n;
                PolygonData data;
                data.O = B;
                data.tex = float2(-n);
                data.dtexdxy.SetRow(0, duvdn.x() * n * ooWidth);
                data.dtexdxy.SetRow(1, duvdn.y() * n * ooWidth);
                data.texzw = float2(lineStrokeWidthFactor, 1);

                size_t indices[] = { indexB, 0 == extIndex ? indexBevelP1 : indexBevelP0, 0 == extIndex ? indexBevelP0 : indexBevelP1 };
                triangulator.PushPolygon(data, indices[0], indices[1], indices[2]);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
                DebugDrawLine(indices[0], indices[1]);
                DebugDrawLine(indices[1], indices[2]);
                DebugDrawLine(indices[2], indices[0]);
#endif
            }
            else
            {
                SG_ASSERT(LineJoin::Round == lineJoin);

                float2 const extP0 = B + extSign * nAB * halfWidth;
                float2 const extP1 = B + extSign * nBC * halfWidth;

                size_t const indexExtP0 = triangulator.PushVertex(extP0, extVertexData);
                size_t const indexBevelP0 = triangulator.PushVertex(bevelP0, extVertexData);
                size_t const indexBevelP1 = triangulator.PushVertex(bevelP1, extVertexData);
                size_t const indexExtP1 = triangulator.PushVertex(extP1, extVertexData);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
                DebugDrawPoint(indexExtP0);
                DebugDrawPoint(indexExtP1);
                DebugDrawPoint(indexBevelP0);
                DebugDrawPoint(indexBevelP1);
#endif

                if(isClosed && extIndex == 0)
                {
                    jointInsidePoints[jointInsidePointCount++] = indexExtP0;
                    jointInsidePoints[jointInsidePointCount++] = indexBevelP0;
                    jointInsidePoints[jointInsidePointCount++] = indexBevelP1;
                    SG_ASSERT(jointInsidePointCount <= MAX_JOINT_INSIDE_POINT_COUNT);
                }

                PushPoints(indexExtP0, indexExtP1, indexB);

                PolygonData data;
                data.O = B;
                data.tex = float2(0);
                data.dtexdxy.SetRow(0, float2(1,0) * ooWidth);
                data.dtexdxy.SetRow(1, float2(0,1) * ooWidth);
                data.texzw = float2(-lineStrokeWidthFactor, 0 == extIndex ? -1.f : 1.f);

                size_t indices[] = {
                    indexB,
                    indexExtP0,
                    indexBevelP0,
                    indexBevelP1,
                    indexExtP1,
                };
                if(0 == extIndex)
                    triangulator.PushPolygon(data, indices[4], indices[3], indices[2], indices[1], indices[0]);
                else
                    triangulator.PushPolygon(data, indices[0], indices[1], indices[2], indices[3], indices[4]);

#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
                DebugDrawLine(indices[0], indices[1]);
                DebugDrawLine(indices[1], indices[2]);
                DebugDrawLine(indices[2], indices[3]);
                DebugDrawLine(indices[3], indices[4]);
                DebugDrawLine(indices[4], indices[0]);
#endif
            }
        }
    };

    size_t prevPointCount = 0;
    size_t prevPoints[MAX_JOINT_POINT_COUNT];
    size_t lastPointCount = 0;
    size_t lastPoints[MAX_JOINT_POINT_COUNT];

    // first points
    if(isClosed)
    {
        Segment const& last = explicitPath.back();
        Segment const& first = explicitPath.front();
        size_t jointPoints[2][MAX_JOINT_POINT_COUNT];
        size_t jointPointCounts[2] = { 0, 0 };
        size_t jointInsidePoints[MAX_JOINT_INSIDE_POINT_COUNT];
        size_t jointInsidePointCounts = 0;
        GenerateJoint(last, first, jointPoints, jointPointCounts, jointInsidePoints, jointInsidePointCounts);

        lastPointCount = jointPointCounts[0];
        for_range(size_t, i, 0, lastPointCount)
            lastPoints[i] = jointPoints[0][i];
        prevPointCount = jointPointCounts[1];
        for_range(size_t, i, 0, prevPointCount)
            prevPoints[i] = jointPoints[1][i];

        for_range(size_t, j, 0, jointInsidePointCounts)
            interiorIndices.push_back(checked_numcastable(jointInsidePoints[j]));
    }
    else
    {
        Segment const& b = explicitPath.front();
        float2 const& bOrthoDir = Orthogonal(b.direction[0]);
        SG_ASSERT(EqualsWithTolerance(bOrthoDir.LengthSq(), 1.f, 0.001f));
        Segment const& e = explicitPath.back();
        float2 const& eOrthoDir = Orthogonal(e.direction[1]);
        SG_ASSERT(EqualsWithTolerance(eOrthoDir.LengthSq(), 1.f, 0.001f));
        switch(iGeometryParameters.strokeLineCap)
        {
        case LineCap::Butt:
        {
            float2 const bL = b.position[0] + halfWidth * bOrthoDir;
            float2 const bR = b.position[0] - halfWidth * bOrthoDir;
            float2 const eL = e.position[1] + halfWidth * eOrthoDir;
            float2 const eR = e.position[1] - halfWidth * eOrthoDir;
            prevPoints[0] = triangulator.PushVertex(bL, VertexData::Interior);
            prevPoints[1] = triangulator.PushVertex(bR, VertexData::Exterior);
            lastPoints[1] = triangulator.PushVertex(eL, VertexData::Interior);
            lastPoints[0] = triangulator.PushVertex(eR, VertexData::Exterior);
            prevPointCount = 2;
            lastPointCount = 2;
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
            DebugDrawPoint(prevPoints[0]);
            DebugDrawPoint(prevPoints[1]);
            DebugDrawPoint(lastPoints[1]);
            DebugDrawPoint(lastPoints[0]);
#endif
            break;
        }
        case LineCap::Round:
            SG_ASSERT_NOT_IMPLEMENTED();
            break;
        case LineCap::Square:
            SG_ASSERT_NOT_IMPLEMENTED();
            break;
        }
    }

    size_t const size = explicitPath.size();
    for_range(size_t, i, 0, size)
    {
        if(isClosed)
            interiorIndices.push_back(checked_numcastable(prevPoints[0]));
        Segment const& segment = explicitPath[i];

        size_t const ip1 = i + 1;
        size_t jointPoints[2][MAX_JOINT_POINT_COUNT];
        size_t jointPointCounts[2] = { 0, 0 };
        size_t jointInsidePoints[MAX_JOINT_INSIDE_POINT_COUNT];
        size_t jointInsidePointCounts = 0;
        if(ip1 < size)
        {
            Segment const& nextSegment = explicitPath[ip1];
            GenerateJoint(segment, nextSegment, jointPoints, jointPointCounts, jointInsidePoints, jointInsidePointCounts);
        }
        else
        {
            jointPointCounts[0] = lastPointCount;
            for_range(size_t, j, 0, lastPointCount)
                jointPoints[0][j] = lastPoints[j];
        }

        switch (segment.type)
        {
        case Segment::Type::Line:
        {
            float2 const A = segment.position[0];
            float2 const B = segment.position[1];
            float2 const AB = B - A;
            float2 const tAB = AB.Normalised();
            float2 const nAB = Orthogonal(tAB);

            float2 const duvAlongnAB = nAB;

            PolygonData data;
            data.O = segment.position[0];
            data.tex = -nAB;
            data.dtexdxy.SetRow(0, duvAlongnAB.x() * nAB * ooWidth);
            data.dtexdxy.SetRow(1, duvAlongnAB.y() * nAB * ooWidth);
            data.texzw = float2(lineStrokeWidthFactor, 1);

            triangulator.BeginPolygon(data);
            ONLY_WITH_CIRCLE_PATH_DEBUG_IMAGE(size_t dbgA = jointPoints[0][jointPointCounts[0]-1];)
            for_range(size_t, j, 0, prevPointCount)
            {
                triangulator.PushPolygonVertex(prevPoints[j]);
                ONLY_WITH_CIRCLE_PATH_DEBUG_IMAGE(DebugDrawLine(dbgA, prevPoints[j]) SG_COMMA dbgA = prevPoints[j];)
            }
            for_range(size_t, j, 0, jointPointCounts[0])
            {
                triangulator.PushPolygonVertex(jointPoints[0][j]);
                ONLY_WITH_CIRCLE_PATH_DEBUG_IMAGE(DebugDrawLine(dbgA, jointPoints[0][j]) SG_COMMA dbgA = jointPoints[0][j];)
            }
            triangulator.EndPolygon();

            break;
        }
        case Segment::Type::Arc:
        {
            //              1               3               5
            // interior     +               +               +
            //                                                  ...
            // exterior     +       +               +
            //              0       2 <-----------> 4
            //                           Sector
            float2 const A = segment.position[0];
            float2 const B = segment.position[1];

            float const aaMaxRadius = 1;
            float const radius = segment.radiusIFA;
            float const ooRadius = 1.f / radius;
            float const extRadius = radius + halfWidth;
            float const intRadius = radius - halfWidth;
            float const strokeWidthRatio = width * ooRadius;

            size_t constexpr MAX_SECTOR_COUNT = 24;
            size_t constexpr MAX_HALF_SECTOR_COUNT = 2 * MAX_SECTOR_COUNT;
            float const extVertexDistanceRatio = 1.f / cos(float(2 * math::constant::PI / (2 * MAX_SECTOR_COUNT)));
            bool const turnRight = segment.arcAngleIFA[0] > segment.arcAngleIFA[1];
            float const turnSign = turnRight ? -1.f : 1.f;
            float const unitAngleBegin = segment.arcAngleIFA[0];
            float const unitAngleEnd = segment.arcAngleIFA[1];
            float const fullDeltaAngle = turnRight ? unitAngleBegin - unitAngleEnd : unitAngleEnd - unitAngleBegin;
            SG_ASSERT(fullDeltaAngle >= 0.f);
            SG_ASSERT_MSG(fullDeltaAngle < 1.f, "Full disk is not supported");
            size_t const sectorCount = std::max(1, ceili(fullDeltaAngle * MAX_SECTOR_COUNT));
            size_t const halfSectorCount = sectorCount * 2;
            float const halfSectorAngle = fullDeltaAngle / halfSectorCount;
            float const cos0 = cos(unitAngleBegin * float(2 * math::constant::PI));
            float const sin0 = sin(unitAngleBegin * float(2 * math::constant::PI));
            float2 const dir0 = float2(cos0, sin0);
            float2x2 const Rh = matrix::Rotation(turnSign * halfSectorAngle * float(2 * math::constant::PI));
            float2x2 const R = Rh*Rh;

            VertexData const intVertexData = turnRight ? VertexData::Exterior : VertexData::Interior;
            VertexData const extVertexData = turnRight ? VertexData::Interior : VertexData::Exterior;

            float2 const center = segment.centerIFA;
            size_t extIndexBegin = size_t(-1);
            size_t extIndexEnd = size_t(-1);
            {
                size_t const prevJointExtPointIndex = prevPoints[turnRight ? 0 : prevPointCount-1];
                size_t const nextJointExtPointIndex = jointPoints[0][turnRight ? jointPointCounts[0]-1 : 0];
                float2 const prevJointExtPoint = triangulator.GetVertexPositions()[prevJointExtPointIndex];
                float2 const nextJointExtPoint = triangulator.GetVertexPositions()[nextJointExtPointIndex];
                float2 const prevJointDir = prevJointExtPoint - center;
                float2 const nextJointDir = nextJointExtPoint - center;
                float const largeExtRadius = extVertexDistanceRatio * extRadius;
                float2 dir = Rh*dir0;
                size_t j = 0;
                for(; j < sectorCount; ++j)
                {
                    if(turnSign * det(prevJointDir, dir) > 0.f)
                        break;
                    dir = R*dir;
                }

                extIndexBegin = triangulator.PushVertex(center + dir * largeExtRadius, extVertexData);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
                DebugDrawPoint(extIndexBegin);
#endif
                extIndexEnd = extIndexBegin+1;
                for(++j; j < sectorCount; ++j)
                {
                    dir = R*dir;
                    // TODO: handle the case where the arc do more than half a circle.
                    if(turnSign * det(nextJointDir, dir) > 0.f)
                        break;
                    size_t const index = triangulator.PushVertex(center + dir * largeExtRadius, extVertexData);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
                    DebugDrawPoint(index);
#endif
                    SG_ASSERT(extIndexEnd == index);
                    extIndexEnd = index+1;
                }
            }

            size_t intIndexBegin = size_t(-1);
            size_t intIndexEnd = size_t(-1);
            {
                size_t const prevJointIntPointIndex = prevPoints[turnRight ? prevPointCount-1 : 0];
                size_t const nextJointIntPointIndex = jointPoints[0][turnRight ? 0 : jointPointCounts[0]-1];
                float2 const prevJointIntPoint = triangulator.GetVertexPositions()[prevJointIntPointIndex];
                float2 const nextJointIntPoint = triangulator.GetVertexPositions()[nextJointIntPointIndex];
                float2 const prevJointDir = prevJointIntPoint - center;
                float2 const nextJointDir = nextJointIntPoint - center;
                float2 dir = R * dir0;
                size_t j = 0;
                for(; j < sectorCount; ++j)
                {
                    if(turnSign * det(prevJointDir, dir) > 0.f)
                        break;
                    dir = R*dir;
                }
                intIndexBegin = triangulator.PushVertex(center + dir * intRadius, intVertexData);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
                DebugDrawPoint(intIndexBegin);
#endif
                intIndexEnd = intIndexBegin+1;
                for(++j; j < sectorCount-1; ++j)
                {
                    dir = R*dir;
                    // TODO: handle the case where the arc do more than half a circle.
                    if(turnSign * det(nextJointDir, dir) > 0.f)
                        break;
                    size_t const index = triangulator.PushVertex(center + dir * intRadius, intVertexData);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
                    DebugDrawPoint(index);
#endif
                    SG_ASSERT(intIndexEnd == index);
                    intIndexEnd = index+1;
                }
            }

            PolygonData data;
            data.O = center;
            data.tex = float2(0);
            data.dtexdxy.SetRow(0, float2(1,0) * ooRadius);
            data.dtexdxy.SetRow(1, float2(0,1) * ooRadius);
            data.texzw = float2(ooRadius, turnSign);

            triangulator.BeginPolygon(data);
            ONLY_WITH_CIRCLE_PATH_DEBUG_IMAGE(size_t dbgA = jointPoints[0][jointPointCounts[0]-1];)
            if(turnRight)
            {
                reverse_for_range(size_t, j, extIndexBegin, extIndexEnd)
                {
                    triangulator.PushPolygonVertex(j);
                    ONLY_WITH_CIRCLE_PATH_DEBUG_IMAGE(DebugDrawLine(dbgA, j) SG_COMMA dbgA = j;)
                }
                for_range(size_t, j, extIndexBegin, extIndexEnd)
                {
                    interiorIndices.push_back(checked_numcastable(j));
                }
            }
            else
            {
                reverse_for_range(size_t, j, intIndexBegin, intIndexEnd)
                {
                    triangulator.PushPolygonVertex(j);
                    ONLY_WITH_CIRCLE_PATH_DEBUG_IMAGE(DebugDrawLine(dbgA, j) SG_COMMA dbgA = j;)
                }
                for_range(size_t, j, intIndexBegin, intIndexEnd)
                {
                    interiorIndices.push_back(checked_numcastable(j));
                }
            }
            for_range(size_t, j, 0, prevPointCount)
            {
                triangulator.PushPolygonVertex(prevPoints[j]);
                ONLY_WITH_CIRCLE_PATH_DEBUG_IMAGE(DebugDrawLine(dbgA, prevPoints[j]) SG_COMMA dbgA = prevPoints[j];)
            }
            if(turnRight)
            {
                for_range(size_t, j, intIndexBegin, intIndexEnd)
                {
                    triangulator.PushPolygonVertex(j);
                    ONLY_WITH_CIRCLE_PATH_DEBUG_IMAGE(DebugDrawLine(dbgA, j) SG_COMMA dbgA = j;)
                }
            }
            else
            {
                for_range(size_t, j, extIndexBegin, extIndexEnd)
                {
                    triangulator.PushPolygonVertex(j);
                    ONLY_WITH_CIRCLE_PATH_DEBUG_IMAGE(DebugDrawLine(dbgA, j) SG_COMMA dbgA = j;)
                }
            }
            for_range(size_t, j, 0, jointPointCounts[0])
            {
                triangulator.PushPolygonVertex(jointPoints[0][j]);
                ONLY_WITH_CIRCLE_PATH_DEBUG_IMAGE(DebugDrawLine(dbgA, jointPoints[0][j]) SG_COMMA dbgA = jointPoints[0][j];)
            }
            triangulator.EndPolygon();

            break;
        }
        default:
            SG_ASSUME_NOT_REACHED();
        }
        for_range(size_t, j, 0, jointInsidePointCounts)
            interiorIndices.push_back(checked_numcastable(jointInsidePoints[j]));

        for_range(size_t, j, 0, jointPointCounts[1])
        {
            prevPoints[j] = jointPoints[1][j];
            prevPointCount = jointPointCounts[1];
        }
    }

    if(isClosed)
    {
        PolygonData data;
        data.O = float2(0);
        data.tex = float2(0);
        data.dtexdxy = float2x2();
        data.texzw = float2(0,1);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
        size_t prev_i = interiorIndices.back();
#endif
        triangulator.BeginPolygon(data);
        for(u16& i : interiorIndices)
        {
            triangulator.PushPolygonVertex(i);
#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
            DebugDrawLine(prev_i, i);
            prev_i = i;
#endif
        }
        triangulator.EndPolygon();
    }

    triangulator.Run();

    auto const vpos = triangulator.GetVertexPositions();
    auto const verticesdata = triangulator.GetVertexData();
    auto const indices = triangulator.GetIndices();
    auto const polygons = triangulator.GetPolygons();
    auto const polygonsdata = triangulator.GetPolygonsData();
    size_t const vertexCount = vpos.size();
    SG_ASSERT(verticesdata.size() == vertexCount);
    size_t const polygonCount = polygons.size();
    SG_ASSERT(polygonsdata.size() == polygonCount);
    struct CachedVertex
    {
        size_t lastPolygonIndex { size_t(-1) };
        size_t index {};
    };
    std::vector<CachedVertex> vertexCache(vpos.size());
    for_range(size_t, i, 0, polygonCount)
    {
        auto const p = polygons[i];
        auto const& pdata = polygonsdata[i];
        for_range(size_t, j, p.begin, p.end)
        {
            size_t const k = indices[j];
            auto& cache = vertexCache[k];
            if(i != cache.lastPolygonIndex)
            {
                cache.lastPolygonIndex = i;
                cache.index = oTesselation.outline.Data().size();

                float2 const P = vpos[k];
                float2 const texxy = pdata.tex + pdata.dtexdxy * (P-pdata.O);
                float4 const tex = texxy.Append(pdata.texzw);
                OutlineVertex vertex;
                vertex.position = P;
                vertex.tex = tex;
                oTesselation.outline.PushData(vertex);
            }
            oTesselation.outline.PushIndex(checked_numcastable(cache.index));
        }
    }

    std::vector<float2> hull;
    hull.reserve(vertexCount);
    if(isClosed)
    {
        for_range(size_t, i, 0, vertexCount)
        {
            if(verticesdata[i].type == VertexData::Exterior)
                hull.push_back(vpos[i]);
        }
    }
    else
    {
        for_range(size_t, i, 0, vertexCount)
        {
            if(verticesdata[i].type != VertexData::None)
                hull.push_back(vpos[i]);
        }
    }
    std::vector<size_t> convexHullIndices;
    geometry::ComputeConvexHull(convexHullIndices, AsArrayView(hull));
    oTesselation.convexHull.reserve(convexHullIndices.size());
    for(size_t i : convexHullIndices)
        oTesselation.convexHull.push_back(hull[i]);

#if CIRCLE_PATH_ENABLE_DEBUG_IMAGE
    float2 A = oTesselation.convexHull.back();
    for_range(size_t, i, 0, oTesselation.convexHull.size())
    {
        float2 const& B = oTesselation.convexHull[i];
        image::DrawLine1pxNoAA(dbgImg, roundi(toDbgFrame(A)), roundi(toDbgFrame(B)), brush.Stroke(ubyte3(255,255,128),1));
        A = B;
    }
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CirclePathDrawer::Draw(DrawContext const& iContext,
                            float2 const& iPos,
                            Tesselation const& iTesselation,
                            DrawParameters const& iDrawParameters,
                            size_t layer) const
{
    Context::TransformType const transformType = iContext.GetTransformType();
    switch(transformType)
    {
    case Context::TransformType::None:
        DrawImpl<Context::TransformType::None>(iContext, iPos, iTesselation, iDrawParameters, layer);
        break;
    case Context::TransformType::Translate2D:
        DrawImpl<Context::TransformType::Translate2D>(iContext, iPos, iTesselation, iDrawParameters, layer);
        break;
    case Context::TransformType::Transform2D:
        DrawImpl<Context::TransformType::Transform2D>(iContext, iPos, iTesselation, iDrawParameters, layer);
        break;
    case Context::TransformType::Transform3D:
        SG_ASSERT_NOT_REACHED();
        DrawImpl<Context::TransformType::Transform3D>(iContext, iPos, iTesselation, iDrawParameters, layer);
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<Context::TransformType transformType>
void CirclePathDrawer::DrawImpl(DrawContext const& iContext,
                                float2 const& iPos,
                                Tesselation const& iTesselation,
                                DrawParameters const& iDrawParameters,
                                size_t layer) const
{
    float4x4 const& transform = iContext.GetTransform();
    auto Transform = [&iPos, &transform](float2 const& P)
    {
        switch(transformType)
        {
        case Context::TransformType::None:
            return P+iPos;
        case Context::TransformType::Translate2D:
        {
            float2 const T = transform.Col(3).xy() + iPos;
            return P + T;
        }
        case Context::TransformType::Transform2D:
        {
            float2x2 const R = transform.SubMatrix<2, 2>(0, 0);
            float2 const T = transform.Col(3).xy();
            return R * (P + iPos) + T;
        }
        case Context::TransformType::Transform3D:
            SG_ASSERT_NOT_IMPLEMENTED();
            break;
        default:
            SG_ASSUME_NOT_REACHED();
        }
        return P;
    };

    if(-1 == layer)
    {
        box2f bbox;
        for(float2 const P : iTesselation.convexHull)
            bbox.Grow(Transform(P));
        LayerManager* layerManager = iContext.GetLayerManager();
        layer = layerManager->Lock(bbox)[0];
    }

    rendering::TransientRenderBatch::Properties properties;
    properties.indexSize = sizeof(u32);
    properties.vertexSize = sizeof(CirclePathDrawerVertex);
    rendering::TransientRenderBatch* renderBatch = iContext.GetCompositingLayer()->GetOrCreateTransientRenderBatch(
        *m_material,
        properties);
    auto outlineVertices = iTesselation.outline.Data();
    auto outlineIndices = iTesselation.outline.Indices();
    rendering::TransientRenderBatch::WriteAccess<CirclePathDrawerVertex> writeAccess(*renderBatch, outlineVertices.size(), outlineIndices.size(), layer);
    CirclePathDrawerVertex* vertices = writeAccess.GetVertexPointerForWriting();

    float const strokeWidth = iDrawParameters.strokeWidth;

    float4 const strokeColor = float4(iDrawParameters.strokeColor);
    float4 const fillColor = iTesselation.isClosed ? float4(iDrawParameters.fillColor) : float4(0);
    //float4 const fillColor = float4(iDrawParameters.fillColor);
    for_range(size_t, i, 0, outlineVertices.size())
    {
        float2 const& pos = Transform(outlineVertices[i].position);
        float4 tex = outlineVertices[i].tex;
        tex._[2] *= strokeWidth;
        vertices[i].pos = pos;
        vertices[i].tex = tex;
        vertices[i].col[0] = strokeColor;
        vertices[i].col[1] = fillColor;
    }
    SG_ASSERT(outlineIndices.size() % 3 == 0);
    u16 const* p = outlineIndices.data();
    u16 const* e = p + outlineIndices.size();
    bool const isIndirectOrder = true; // for UI
    while(p != e)
    {
        if(isIndirectOrder)
            writeAccess.PushIndices(p[0], p[2], p[1]);
        else
            writeAccess.PushIndices(p[0], p[1], p[2]);
        p += 3;
    }

}
//=============================================================================
CirclePathDrawerDescriptor::CirclePathDrawerDescriptor()
    : m_drawer()
    , m_pixelShader()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
CirclePathDrawerDescriptor::~CirclePathDrawerDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CirclePathDrawerDescriptor::CreateDrawer()
{
    refptr<rendering::VertexShaderDescriptor> vsDesc = new rendering::VertexShaderDescriptor(FilePath("src:/UserInterface/Shaders/V_UI_CirclePathDrawer.hlsl"), "vmain");
    rendering::VertexShaderProxy vs = vsDesc->GetProxy();
    rendering::ShaderInputLayoutProxy layout(ui::CirclePathDrawer::VertexDescriptor(), vs);
    rendering::PixelShaderProxy ps = m_pixelShader->GetProxy();
    rendering::RenderStateName blendState("Premultiplied Alpha Blending"); // TODO: expose blend state as a property
    refptr<rendering::Material> material = new rendering::Material(layout, vs, ps, blendState);
    m_drawer = new ui::CirclePathDrawer(material.get());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, ui), CirclePathDrawerDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(pixelShader, "")
REFLECTION_CLASS_END
//=============================================================================
}
}


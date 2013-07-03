#include "stdafx.h"

#include "Triangulator.h"

#include "Triangulate.h"
#include <Core/For.h>

#if SG_ENABLE_ASSERT
#define SG_TRIANGULATOR_ENABLE_DEBUG_IMAGE 0
#else
#define SG_TRIANGULATOR_ENABLE_DEBUG_IMAGE 0
#endif

#if SG_TRIANGULATOR_ENABLE_DEBUG_IMAGE
// Breaks project dependencies, but only in debug
#include <Image/DebugImage.h>
#endif
#if SG_ENABLE_UNIT_TESTS
#include <Core/FileSystem.h>
#include <Core/StringFormat.h>
#include <Core/TestFramework.h>
#endif

namespace sg {
namespace geometry {
//=============================================================================
Triangulator::Triangulator()
    : m_state(State::Writable)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
size_t Triangulator::PushVertex(float2 const& iPosition)
{
    SG_ASSERT(State::Writable == m_state);
    size_t id = m_vertexPositions.size();
    m_vertexPositions.push_back(iPosition);
    return id;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
size_t Triangulator::BeginPolygon(PolygonType iType)
{
    SG_ASSERT(State::Writable == m_state);
    m_state = State::BuildPolygon;
    size_t id = m_polygons.size();
    m_polygons.emplace_back();
    Polygon& p = m_polygons.back();
    p.type = iType;
    p.begin = m_polygonIndices.size();
    p.end = p.begin;
    p.masterIndexOrSubCount = 0;
    return id;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Triangulator::PushPolygonVertex(size_t iVertexId)
{
    SG_ASSERT(State::BuildPolygon == m_state);
    switch(m_state)
    {
    case State::BuildPolygon:
        SG_ASSERT(!m_polygons.empty());
        ++(m_polygons.back().end);
        break;
    case State::BuildSubPolygon:
        SG_ASSERT_NOT_IMPLEMENTED();
        SG_ASSERT(!m_subPolygons.empty());
        ++(m_subPolygons.back().end);
        break;
    default:
        SG_ASSERT_NOT_REACHED();
    }
    SG_ASSERT(iVertexId < m_vertexPositions.size());
    m_polygonIndices.push_back(iVertexId);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Triangulator::EndPolygon()
{
    SG_ASSERT(State::BuildPolygon == m_state);
    SG_ASSERT(m_polygons.back().end == m_polygonIndices.size());
    m_state = State::Writable;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Triangulator::BeginSubPolygon(size_t iMasterPolygonId, PolygonType iType)
{
    SG_ASSERT(State::Writable == m_state);
    m_state = State::BuildSubPolygon;
    Polygon& master = m_polygons[iMasterPolygonId];
    SG_ASSERT(master.masterIndexOrSubCount <= m_subPolygons.size());
    ++master.masterIndexOrSubCount;
    m_subPolygons.emplace_back();
    Polygon& p = m_polygons.back();
    p.type = iType;
    p.begin = m_polygonIndices.size();
    p.end = p.begin;
    p.masterIndexOrSubCount = iMasterPolygonId;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Triangulator::EndSubPolygon()
{
    SG_ASSERT(State::BuildSubPolygon == m_state);
    SG_ASSERT(m_subPolygons.back().end == m_polygonIndices.size());
    m_state = State::Writable;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Triangulator::Run()
{
    SG_ASSERT(State::Writable == m_state);
    SG_ASSERT(m_indices.empty());
    SG_ASSERT(m_tesselatedPolygons.empty());

#if SG_TRIANGULATOR_ENABLE_DEBUG_IMAGE
    auto brush = image::brush::Blend<image::blend::Classic>().Monospace();
    size_t const dbgSize = 1 << 10;
    image::RGBImage dbgImg((uint2(dbgSize)));
    dbgImg.Fill(ubyte3(0));
    box2f bbox;
    bbox.Grow(AsArrayView(m_vertexPositions));
    image::WindowToViewport toDbgFrame(bbox, box2f::FromCenterDelta(0.5f * dbgImg.WidthHeight(), 0.96f * dbgImg.WidthHeight()));

    for(auto const& p : m_polygons)
    {
        switch(p.type)
        {
        case PolygonType::Polygon:
        {
            float2 A = toDbgFrame(m_vertexPositions[m_polygonIndices[p.end-1]]);
            for_range(size_t, i, p.begin, p.end)
            {
                float2 B = toDbgFrame(m_vertexPositions[m_polygonIndices[i]]);
                image::DrawLine1pxNoAA(dbgImg, roundi(A), roundi(B), brush.Stroke(ubyte3(255), 1));
                A = B;
            }
            break;
        }
        case PolygonType::Ribbon:
        {
            SG_ASSERT_NOT_IMPLEMENTED();
            break;
        }
        default:
            SG_ASSUME_NOT_REACHED();
        }
        SG_UNUSED(p);
    }

    for_range(size_t, i, 0, m_vertexPositions.size())
    {
        auto const& p = toDbgFrame(m_vertexPositions[i]);
        image::DrawRect(dbgImg, box2f::FromCenterDelta(p, float2(5)), brush.Stroke(ubyte3(255), 1));
        //image::DrawText(dbgImg, p + float2(10,3), Format("%0", i), brush.Fill(ubyte3(255)));
    }
#endif
    std::vector<float2> vertices;
    std::vector<size_t> indices;

    m_tesselatedPolygons.reserve(m_polygons.size());
    for(auto const& p : m_polygons)
    {
        switch(p.type)
        {
        case PolygonType::Polygon:
        {
            size_t const size = p.end - p.begin;
            vertices.reserve(size);
            indices.reserve(size);
            for(size_t i = 0, pi = p.begin; pi < p.end; ++i, ++pi)
            {
                size_t const index = m_polygonIndices[pi];
                vertices.emplace_back(m_vertexPositions[index]);
                indices.emplace_back(index);
            }
            break;
        }
        case PolygonType::Ribbon:
        {
            SG_ASSERT_NOT_IMPLEMENTED();
            break;
        }
        default:
            SG_ASSUME_NOT_REACHED();
        }
        if(0 != p.masterIndexOrSubCount)
        {
            SG_ASSERT_NOT_IMPLEMENTED();
            // TODO: add vertices from sub polygons
        }
        size_t const triangleVertexCount = 3 * (vertices.size() - 2);
        size_t const indexCount = m_indices.size();
        TriangleList tl; tl.begin = indexCount; tl.end = indexCount + triangleVertexCount;
        m_tesselatedPolygons.push_back(tl);
        m_indices.resize(tl.end);
        TriangulateReturnCode const rc = Triangulate(AsArrayView(vertices), AsArrayView(m_indices.data() + tl.begin, triangleVertexCount));
        for(size_t i = tl.begin; i < tl.end; ++i)
            m_indices[i] = indices[m_indices[i]];
        vertices.clear();
        indices.clear();
    }


#if SG_TRIANGULATOR_ENABLE_DEBUG_IMAGE
    size_t const N = m_indices.size();
    SG_ASSERT(N % 3 == 0);
    for(size_t t = 0; t < N; t+=3)
    {
        float2 const A = toDbgFrame(m_vertexPositions[m_indices[t]]);
        float2 const B = toDbgFrame(m_vertexPositions[m_indices[t+1]]);
        float2 const C = toDbgFrame(m_vertexPositions[m_indices[t+2]]);
        image::DrawLine1pxNoAA(dbgImg, roundi(A), roundi(B), image::brush::Stroke(ubyte3(128)));
        image::DrawLine1pxNoAA(dbgImg, roundi(B), roundi(C), image::brush::Stroke(ubyte3(128)));
        image::DrawLine1pxNoAA(dbgImg, roundi(C), roundi(A), image::brush::Stroke(ubyte3(128)));
    }

    SG_DEBUG_IMAGE_STEP_INTO(dbgImg, false);
#endif
    m_state = State::Tesselated;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if 0 // Old code, for reference
#if SG_TRIANGULATOR_ENABLE_DEBUG_IMAGE
namespace {
size_t const dbgSize = 1024;
float const dbgOccupation = 0.9f;
struct DbgData;
DbgData* g_dbgData = nullptr;
struct DbgData {
    image::RGBImage* img;
    float2 O;
    float2 scale;
    size_t polygonIndex;
    DbgData() { SG_ASSERT(nullptr == g_dbgData); g_dbgData = this; }
    ~DbgData() { SG_ASSERT(this == g_dbgData); g_dbgData = nullptr; }
    float2 ToDbgFrame(float2 const& xy) { return (xy - O) * scale; };
};
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Triangulator::Run()
{
    SG_ASSERT(State::Writable == m_state);
    SG_ASSERT(m_indices.empty());
    SG_ASSERT(m_tesselatedPolygons.empty());

#if SG_TRIANGULATOR_ENABLE_DEBUG_IMAGE
    image::RGBImage dbgImg((uint2(dbgSize)));
    DbgData dbgData;
    {
        dbgImg.Fill(ubyte3(0));
        float2 lo = float2(std::numeric_limits<float>::infinity());
        float2 hi = float2(-std::numeric_limits<float>::infinity());
        for(auto const& xy : m_vertexPositions)
        {
            lo = componentwise::min(lo, xy);
            hi = componentwise::max(hi, xy);
        }
        float2 const C = 0.5f * (lo + hi);
        float2 const delta = hi - lo;
        float const d = std::max(delta.x(), delta.y());
        float2 const dbgScale = dbgOccupation * dbgSize / d * float2(1, -1);
        float2 const dbgC = float2(0.5f * dbgSize);
        float2 const dbgO = C - dbgC * (1.f/dbgScale);
        dbgData.img = &dbgImg;
        dbgData.O = dbgO;
        dbgData.scale = dbgScale;
        dbgData.polygonIndex = 0;
    }
    auto ToDbgFrame = [](float2 const& xy) { return g_dbgData->ToDbgFrame(xy); };

    auto brush = image::brush::Blend<image::blend::Classic>().Monospace();

    m_tesselatedPolygons.reserve(m_polygons.size());
    for(auto const& p : m_polygons)
    {
        switch(p.type)
        {
        case PolygonType::Polygon:
        {
            float2 A = ToDbgFrame(m_vertexPositions[m_polygonIndices[p.end-1]]);
            for_range(size_t, i, p.begin, p.end)
            {
                float2 B = ToDbgFrame(m_vertexPositions[m_polygonIndices[i]]);
                image::DrawLine1pxNoAA(dbgImg, roundi(A), roundi(B), brush.Stroke(ubyte3(128), 1));
                A = B;
            }
            break;
        }
        case PolygonType::Ribbon:
        {
            SG_ASSERT_NOT_IMPLEMENTED();
            break;
        }
        default:
            SG_ASSUME_NOT_REACHED();
        }
        SG_UNUSED(p);
    }

    for_range(size_t, i, 0, m_vertexPositions.size())
    {
        auto const& p = ToDbgFrame(m_vertexPositions[i]);
        image::DrawRect(dbgImg, box2f::FromCenterDelta(p, float2(5)), brush.Stroke(ubyte3(255), 1));
        image::DrawText(dbgImg, p + float2(10,3), Format("%0", i), brush.Fill(ubyte3(255)));
    }
#endif
    std::vector<Node> vertices;
    std::vector<size_t> rootIndices;

    for(auto const& p : m_polygons)
    {
        switch(p.type)
        {
        case PolygonType::Polygon:
        {
            size_t const size = p.end - p.begin;
            vertices.reserve(size);
            size_t prevIndex = size-1;
            for(size_t i = 0, pi = p.begin; pi < p.end; ++i, ++pi)
            {
                vertices.emplace_back();
                size_t const nodeIndex = vertices.size();
                Node& n = vertices.back();
                n.index = m_polygonIndices[pi];
                n.neighbors[0] = prevIndex;
                n.neighbors[1] = (i+1 == size ? 0 : i+1);
                n.toVisitCount = 1;
                prevIndex = i;
            }
            break;
        }
        case PolygonType::Ribbon:
        {
            SG_ASSERT_NOT_IMPLEMENTED();
            break;
        }
        default:
            SG_ASSUME_NOT_REACHED();
        }
        if(0 != p.masterIndexOrSubCount)
        {
            SG_ASSERT_NOT_IMPLEMENTED();
            // TODO: add vertices from sub polygons
        }
        size_t const indexCount = m_indices.size();
        TriangleList tl; tl.begin = indexCount; tl.end = indexCount;
        m_tesselatedPolygons.push_back(tl);
        TriangulatePolygon(vertices);
        vertices.clear();
#if SG_TRIANGULATOR_ENABLE_DEBUG_IMAGE
        ++dbgData.polygonIndex;
#endif
    }


#if SG_TRIANGULATOR_ENABLE_DEBUG_IMAGE
    SG_DEBUG_IMAGE_STEP_INTO(dbgImg, false);
#endif
    m_state = State::Tesselated;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SG_FORCE_INLINE float2 Triangulator::GetPosition(Node const& iNode) const { return m_vertexPositions[iNode.index]; };
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Triangulator::TriangulatePolygon(std::vector<Node>& iPolygonCanBeGarbaged)
{
#if SG_TRIANGULATOR_ENABLE_DEBUG_IMAGE
    image::RGBImage& dbgImg = *(g_dbgData->img);
    auto ToDbgFrame = [](float2 const& xy) { return g_dbgData->ToDbgFrame(xy); };
    auto brush = image::brush::Blend<image::blend::Classic>().Monospace();
#endif

    // This polygon triangulation algorithm first generate monoton polygons
    // using a sweep line algorithm.
    // Definition: A polygon P is said to be monoton along a line L if, for
    // any line l orthogonal to L, l intersects P at most 2 times.

    std::vector<Node>& nodes = iPolygonCanBeGarbaged;
    std::vector<size_t> sortedNodeIndices;
    for_range(size_t, i, 0, nodes.size())
        sortedNodeIndices.push_back(i);
    auto Less = [this](Node const& a, Node const& b)
    {
        float const aPos = GetSweepPos(a);
        float const bPos = GetSweepPos(b);
        return aPos < bPos || (aPos == bPos && a.index < b.index);
    };
    auto LessI = [&Less, &nodes](size_t a, size_t b)
    {
        return Less(nodes[a], nodes[b]);
    };
    std::sort(sortedNodeIndices.begin(), sortedNodeIndices.end(), LessI);

#if SG_TRIANGULATOR_ENABLE_DEBUG_IMAGE
    for_range(size_t, i, 0, nodes.size())
    {
        Node const& n = nodes[i];
        image::DrawText(dbgImg, ToDbgFrame(GetPosition(n)) + float2(10, -5 - g_dbgData->polygonIndex * 8.f), Format("%0", i), brush.Fill(ubyte3(64,128,160)));
    }
#endif
#if SG_ENABLE_ASSERT
    for(auto const& n : nodes)
    {
        SG_ASSERT(1 == n.toVisitCount);
        SG_ASSERT(n.neighbors[0] < nodes.size());
        SG_ASSERT(n.neighbors[1] < nodes.size());
    }
#endif

    struct Thread
    {
        size_t nodeIndex;
        size_t nextNeighbor;
    };
    struct MonotonPolygon
    {
        Thread threads[2];
    };

    std::vector<MonotonPolygon> monotonPolygons;
    std::vector<size_t> heads;

    size_t const nodeCount = sortedNodeIndices.size();
    size_t const* psweeplineIndex = sortedNodeIndices.data();
    size_t const* const psweeplineIndexEnd = psweeplineIndex + sortedNodeIndices.size();
    size_t loopIndex = size_t(-1);
    while(psweeplineIndex < psweeplineIndexEnd)
    {
        ++loopIndex;
        size_t const sweeplineIndex = *psweeplineIndex;
        SG_ASSERT(sweeplineIndex < nodes.size());
        // try to advance sweep line
        float nextSweepPos = std::numeric_limits<float>::infinity();
        size_t nextNodeIndex = size_t(-1);
        size_t nextAdvancingThread = size_t(-1);
        for_range(size_t, i, 0, monotonPolygons.size()*2)
        {
            MonotonPolygon const& p = monotonPolygons[i/2];
            Thread const& t = p.threads[i%2];
            size_t const tNextNodeIndex = nodes[t.nodeIndex].neighbors[t.nextNeighbor];
            float const tNextSweepPos = GetSweepPos(nodes[tNextNodeIndex]);
            if(tNextSweepPos < nextSweepPos || (tNextSweepPos == nextSweepPos && nodes[tNextNodeIndex].index < nodes[nextNodeIndex].index))
            {
                nextSweepPos = tNextSweepPos;
                nextNodeIndex = tNextNodeIndex;
                nextAdvancingThread = i;
            }
        }

        if(size_t(-1) != nextNodeIndex && nodes[sweeplineIndex].index == nodes[nextNodeIndex].index)
        {
            {
                Node& sweeplineNode = nodes[sweeplineIndex];
                SG_ASSERT(0 < sweeplineNode.toVisitCount);
                --sweeplineNode.toVisitCount;
            }

            // advance thread ifp
            size_t const polygonIndex = nextAdvancingThread/2;
            size_t const threadIndex = nextAdvancingThread%2;
            MonotonPolygon& p = monotonPolygons[polygonIndex];
            Thread& t = p.threads[threadIndex];
            Thread const& secondThread = p.threads[1-threadIndex];
            SG_CODE_FOR_ASSERT(size_t const tNextNodeIndex_dbg = nodes[t.nodeIndex].neighbors[t.nextNeighbor];)
            SG_ASSERT(nextNodeIndex == tNextNodeIndex_dbg);
            Node& node = nodes[nextNodeIndex];

#if SG_TRIANGULATOR_ENABLE_DEBUG_IMAGE
            {
                float2 const& A = GetPosition(nodes[t.nodeIndex]);
                float2 const& B = GetPosition(node);
                image::DrawLine1pxNoAA(dbgImg, roundi(ToDbgFrame(A)), roundi(ToDbgFrame(B)), brush.Stroke(ubyte3(200,200,40)));
                image::DrawText(dbgImg, ToDbgFrame(0.5f*(A+B)) + float2(10,0), Format("%0", loopIndex), brush.Fill(ubyte3(200,200,40)));
            }
#endif
            size_t const neighborIndex = node.neighbors[0] == t.nodeIndex ? 1 : 0;
            SG_ASSERT(node.neighbors[1-neighborIndex] == t.nodeIndex);
            size_t const threadNextNodeIndex = node.neighbors[neighborIndex];
            Node const& threadNextNode = nodes[threadNextNodeIndex];
            if(threadNextNodeIndex == secondThread.nodeIndex)
            {
                // This is the closing point of the polygon.
                SG_ASSERT(0 == threadNextNode.toVisitCount);
                SG_CODE_FOR_ASSERT(size_t const secondThreadNextNodeIndex_dbg = nodes[secondThread.nodeIndex].neighbors[secondThread.nextNeighbor];)
                SG_ASSERT(secondThreadNextNodeIndex_dbg == nextNodeIndex);
#if SG_TRIANGULATOR_ENABLE_DEBUG_IMAGE
                {
                    float2 const& A = GetPosition(node);
                    float2 const& B = GetPosition(nodes[secondThread.nodeIndex]);
                    image::DrawLine1pxNoAA(dbgImg, roundi(ToDbgFrame(A)), roundi(ToDbgFrame(B)), brush.Stroke(ubyte3(120,200,40)));
                    image::DrawText(dbgImg, ToDbgFrame(0.5f*(A+B)) + float2(10,0), Format("%0", loopIndex), brush.Fill(ubyte3(120,200,40)));
                }
#endif
                if(polygonIndex < monotonPolygons.size() - 1)
                    std::swap(p, monotonPolygons.back());
                monotonPolygons.pop_back();
            }
            else if(0 == threadNextNode.toVisitCount)
            {
                SG_ASSERT(Less(threadNextNode, node));

                // This is a merging of two monoton polygons. We need to split.
                // Let's call this point B. Let's call D the point at the end
                // of the other thread. Let's call A the point before B and C
                // the point before D.
                // We will generate a splitting edge with a point E which is
                // either D or a nearest point between B and D (in that case,
                // both departing edges must go in the same direction as the
                // sweep line advances).

                // Generate 2 nodes, here, in order not to invalidate memory
                // later.
                nodes.emplace_back();
                nodes.emplace_back();
                size_t const indexB2 = nodes.size()-2;
                size_t const indexE2 = nodes.size()-1;
                Node& nodeB2 = nodes[indexB2];
                Node& nodeE2 = nodes[indexE2];

                size_t const indexA = t.nodeIndex;
                size_t const indexB = nextNodeIndex;
                size_t const indexC = secondThread.nodeIndex;
                size_t const indexD = nodes[secondThread.nodeIndex].neighbors[secondThread.nextNeighbor];
                Node& nodeA = nodes[indexA];
                Node& nodeB = nodes[indexB];
                Node& nodeC = nodes[indexC];
                Node& nodeD = nodes[indexD];
                float2 const A = GetPosition(nodeA);
                float2 const B = GetPosition(nodeB);
                float2 const C = GetPosition(nodeC);
                float2 const D = GetPosition(nodeD);
                float2 const AB = B-A;
                float2 const CD = D-C;

                // Note that we will generate 2 edges, one for current polygon,
                // the other for the other one. As nodeB is pointed by the
                // other polygon, we will modify it in place for the other
                // polygon and use new nodes for the curent polygon.

                size_t const edgeIndexInB = nodeB.neighbors[0] == indexA ? 0 : 1;
                SG_ASSERT(nodeB.neighbors[edgeIndexInB] == indexA);
                size_t const edgeIndexInD = nodeD.neighbors[0] == indexC ? 0 : 1;
                SG_ASSERT(nodeD.neighbors[edgeIndexInD] == indexC);

                Node* pE = &nodeD;
                size_t edgeIndexInE = edgeIndexInD;

                size_t const* psweeplineIndex2 = psweeplineIndex+1;
                while(psweeplineIndex2 < psweeplineIndexEnd)
                {
                    size_t const sweeplineIndex2 = *psweeplineIndex2;
                    if(sweeplineIndex2 == indexD)
                        break;
                    Node& sweeplineNode2 = nodes[sweeplineIndex2];
                    SG_ASSERT(sweeplineIndex2 < nodes.size());
                    float2 const P = GetPosition(sweeplineNode2);
                    float2 const AP = P-A;
                    float2 const BP = P-B;
                    float2 const CP = P-C;
                    float const detA = det(AB,AP);
                    float const detC = det(CD,CP);

                    if(detA*detC <= 0)
                    {
                        // Now, we have to compute edgeIndexInE so that we
                        // don't generate a crossing.
                        float2 PQ[2];
                        for_range(size_t, i, 0, 2)
                        {
                            float2 const Q = GetPosition(nodes[sweeplineNode2.neighbors[i]]);
                            PQ[i] = Q - P;
                        }
                        float const detPQ01 = det(PQ[0], PQ[1]);
                        pE = &sweeplineNode2;
                        edgeIndexInE = detPQ01*detA > 0 ? 1 : 0;
                        break;
                    }
                    if(&sweeplineNode2 == pE)
                        break;
                    ++psweeplineIndex2;
                }

                size_t const indexE = pE - nodes.data();
                Node& nodeE = *pE;
                SG_ASSERT(0 == nodeB.toVisitCount);
                SG_ASSERT(1 == nodeE.toVisitCount);
                nodeB.toVisitCount = 1;
                nodeE.toVisitCount = 2;
                nodeB2 = nodeB;
                nodeE2 = nodeE;
                nodeB2.toVisitCount = 0;
                nodeE2.toVisitCount = 0;

                nodeB.neighbors[edgeIndexInB] = indexE;
                nodeE.neighbors[edgeIndexInE] = indexB;
                nodeB2.neighbors[1-edgeIndexInB] = indexE2;
                nodeE2.neighbors[1-edgeIndexInE] = indexB2;

                // Fixup previous node in polygon
                size_t const BIndexInA = nodeA.neighbors[0] == indexB ? 0 : 1;
                SG_ASSERT(nodeA.neighbors[BIndexInA] == indexB);
                nodeA.neighbors[BIndexInA] = indexB2;
                // Fixup next node
                Node& nodeF = nodes[nodeE2.neighbors[edgeIndexInE]];
                size_t const EIndexInF = nodeF.neighbors[0] == indexE ? 0 : 1;
                SG_ASSERT(nodeF.neighbors[EIndexInF] == indexE);
                nodeF.neighbors[EIndexInF] = indexE2;

#if SG_TRIANGULATOR_ENABLE_DEBUG_IMAGE
                {
                    float2 const& E = GetPosition(nodeE);
                    float2 const N = Orthogonal(ToDbgFrame(E)-ToDbgFrame(B)).Normalised();
                    image::DrawLine1pxNoAA(dbgImg, roundi(ToDbgFrame(B)+N) + int2(2,1), roundi(ToDbgFrame(E)+N) + int2(2,1), brush.Stroke(ubyte3(200,60,40)));
                    image::DrawText(dbgImg, ToDbgFrame(0.5f*(B+E)) + float2(10,-10), Format("%0", loopIndex), brush.Fill(ubyte3(200,60,40)));
                }
#endif

#if SG_ENABLE_ASSERT
                {
                    // Check that the merging polygon exists
                    size_t mergingThreadIndex = size_t(-1);
                    for_range(size_t, i, 0, monotonPolygons.size()*2)
                    {
                        MonotonPolygon const& pi = monotonPolygons[i/2];
                        Thread const& ti = pi.threads[i%2];
                        size_t const tiNextNodeIndex = nodes[ti.nodeIndex].neighbors[ti.nextNeighbor];
                        if(tiNextNodeIndex == nextNodeIndex && ti.nodeIndex == threadNextNodeIndex)
                        {
                            mergingThreadIndex = i;
                            break;
                        }
                    }
                    SG_ASSERT(size_t(-1) != mergingThreadIndex);
                }
#endif

                // advance thread
                t.nodeIndex = indexB2;
                t.nextNeighbor = 1-edgeIndexInB;
                SG_ASSERT(nodes[t.nodeIndex].neighbors[t.nextNeighbor]  == indexE2);
            }
            else
            {
                // advance thread
                size_t const tNextNodeIndex = nodes[t.nodeIndex].neighbors[t.nextNeighbor];
                t.nodeIndex = tNextNodeIndex;
                t.nextNeighbor = neighborIndex;
                SG_ASSERT(nodes[t.nodeIndex].neighbors[t.nextNeighbor] == threadNextNodeIndex);
            }

            Node& sweeplineNode = nodes[sweeplineIndex];
            if(0 == sweeplineNode.toVisitCount)
                ++psweeplineIndex;
        }
        else
        {
            SG_ASSERT(1 == nodes[sweeplineIndex].toVisitCount);
            SG_ASSERT(size_t(-1) == nextNodeIndex || Less(nodes[sweeplineIndex], nodes[nextNodeIndex]));
            size_t const indexP = sweeplineIndex;
            float2 const P = GetPosition(nodes[sweeplineIndex]);
            size_t indexQ[] = { nodes[sweeplineIndex].neighbors[0], nodes[sweeplineIndex].neighbors[1] };
            float2 const Q[] = { GetPosition(nodes[indexQ[0]]), GetPosition(nodes[indexQ[1]]) };
            float2 const& Q0 = Q[0];
            float2 const& Q1 = Q[1];
            float2 const PQ0 = Q0-P;
            float2 const PQ1 = Q1-P;
            SG_ASSERT(0 <= PQ0.y());
            SG_ASSERT(0 <= PQ1.y());

            // check if it is a split of an existing monoton polygon or not
            size_t splittedPolygonIndex = size_t(-1);
            for_range(size_t, i, 0, monotonPolygons.size())
            {
                MonotonPolygon& pi = monotonPolygons[i];
                size_t const indexA = pi.threads[0].nodeIndex;
                size_t const indexB = nodes[indexA].neighbors[pi.threads[0].nextNeighbor];
                size_t const indexC = pi.threads[1].nodeIndex;
                size_t const indexD = nodes[indexC].neighbors[pi.threads[1].nextNeighbor];
                float2 const A = GetPosition(nodes[indexA]);
                float2 const B = GetPosition(nodes[indexB]);
                float2 const C = GetPosition(nodes[indexC]);
                float2 const D = GetPosition(nodes[indexD]);

                float2 const AB = B-A;
                float2 const CD = D-C;
                float2 const AP = P-A;
                float2 const CP = P-C;
                float const detA = det(AB,AP);
                float const detC = det(CD,CP);

                if(detA*detC <= 0)
                {
                    SG_ASSERT(0 != detA*detC);
                    SG_ASSERT(size_t(-1) == splittedPolygonIndex);
                    // This is a split.
                    // Create two edges from A to P, one by redirecting the
                    // one to B, the other from a new node linking B to A to P.
                    // Create a new polygon from the new node.

                    nodes.emplace_back();
                    nodes.emplace_back();
                    size_t const indexA2 = nodes.size()-2;
                    size_t const indexP2 = nodes.size()-1;
                    Node& nodeA = nodes[indexA];
                    Node& nodeB = nodes[indexB];
                    Node& nodeP = nodes[indexP];
                    Node& nodeA2 = nodes[indexA2];
                    Node& nodeP2 = nodes[indexP2];
                    Node* nodeQ[] = { &nodes[indexQ[0]], &nodes[indexQ[1]] };

                    size_t const edgeIndexInA = nodeA.neighbors[0] == indexB ? 0 : 1;
                    SG_ASSERT(nodeA.neighbors[edgeIndexInA] == indexB);

                    float const detABP = det(AB,AP);
                    SG_ASSERT(detABP * det(CD,CP) < 0.f);
                    float const detPQ0Q1 = det(PQ0,PQ1);
                    size_t const qInNewPolygon = detPQ0Q1 * detABP >= 0 ? 0 : 1;

                    size_t const edgeIndexInP = nodeP.neighbors[0] == indexQ[qInNewPolygon] ? 0 : 1;
                    SG_ASSERT(nodeP.neighbors[edgeIndexInP] == indexQ[qInNewPolygon]);
                    SG_ASSERT(nodeP.neighbors[1-edgeIndexInP] == indexQ[1-qInNewPolygon]);

                    SG_ASSERT(0 == nodeA.toVisitCount);
                    SG_ASSERT(1 == nodeP.toVisitCount);
                    nodeA.toVisitCount = 0;
                    nodeP.toVisitCount = 2;
                    nodeA2 = nodeA;
                    nodeP2 = nodeP;
                    nodeA2.toVisitCount = 0;
                    nodeP2.toVisitCount = 0;

                    nodeA.neighbors[edgeIndexInA] = indexP;
                    nodeP.neighbors[edgeIndexInP] = indexA;
                    nodeA2.neighbors[1-edgeIndexInA] = indexP2; // B?
                    nodeP2.neighbors[1-edgeIndexInP] = indexA2;

                    // Fixup B and Q[qInNewPolygon]
                    size_t const edgeIndexInB = nodeB.neighbors[0] == indexA ? 0 : 1;
                    SG_ASSERT(nodeB.neighbors[edgeIndexInB] == indexA);
                    nodeB.neighbors[edgeIndexInB] = indexA2;
                    Node& nodeQinNewPolygon = *nodeQ[qInNewPolygon];
                    size_t const edgeIndexInQ = nodeQinNewPolygon.neighbors[0] == indexP ? 0 : 1;
                    SG_ASSERT(nodeQinNewPolygon.neighbors[edgeIndexInQ] == indexP);
                    nodeQinNewPolygon.neighbors[edgeIndexInQ] = indexP2;

                    heads.push_back(indexA2);
                    monotonPolygons.emplace_back();
                    MonotonPolygon& newPolygon = monotonPolygons.back();
                    for_range(size_t, j, 0, SG_ARRAYSIZE(newPolygon.threads))
                    {
                        Thread& t = newPolygon.threads[j];
                        t.nodeIndex = indexA2;
                        t.nextNeighbor = j;
                    }
#if SG_TRIANGULATOR_ENABLE_DEBUG_IMAGE
                    {
                        image::DrawLine1pxNoAA(dbgImg, roundi(ToDbgFrame(A)) + int2(2,1), roundi(ToDbgFrame(P)) + int2(2,1), brush.Stroke(ubyte3(200,120,40)));
                        image::DrawText(dbgImg, ToDbgFrame(0.5f*(A+P)) + float2(10,-10), Format("%0", loopIndex), brush.Fill(ubyte3(200,120,40)));
                    }
#endif

                    splittedPolygonIndex = i;
#if !SG_ENABLE_ASSERT
                    break;
#endif
                }
            }

            if(size_t(-1) == splittedPolygonIndex)
            {
                // if not a split, create a head and 2 threads
                Node& sweeplineNode = nodes[sweeplineIndex];
                SG_ASSERT(1 == sweeplineNode.toVisitCount);
                --sweeplineNode.toVisitCount;
                heads.push_back(sweeplineIndex);
                monotonPolygons.emplace_back();
                MonotonPolygon& p = monotonPolygons.back();
                for_range(size_t, j, 0, SG_ARRAYSIZE(p.threads))
                {
                    Thread& t = p.threads[j];
                    t.nodeIndex = sweeplineIndex;
                    t.nextNeighbor = j;
                }
                SG_ASSERT(0 == sweeplineNode.toVisitCount);
                ++psweeplineIndex;
            }
        }
    }

    for(size_t h : heads)
    {
        TriangulateMonotonPolygon(AsArrayView(nodes), h);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Triangulator::TriangulateMonotonPolygon(ArrayView<Node const> iNodes, size_t iHead)
{
#if SG_TRIANGULATOR_ENABLE_DEBUG_IMAGE
    image::RGBImage& dbgImg = *(g_dbgData->img);
    auto ToDbgFrame = [](float2 const& xy) { return g_dbgData->ToDbgFrame(xy); };
    auto brush = image::brush::Blend<image::blend::Classic>().Monospace();
#endif

    TriangleList& triangleList = m_tesselatedPolygons.back();

    struct StackNode
    {
        size_t nodeIndex;
        float2 position;
    };
    struct Chain
    {
        size_t nodeIndex;
        size_t nextNeighbor;
        float2 position;
    };
    std::vector<StackNode> stack;
    size_t stackChainIndex;
    Chain chains[2];
    float chainValidDetSign[2];

    auto AdvanceChain = [&iNodes, this](Chain& chain)
    {
        size_t const nodeIndex = chain.nodeIndex;
        Node const& node = iNodes[chain.nodeIndex];
        size_t const nextNodeIndex = node.neighbors[chain.nextNeighbor];
        Node const& nextNode = iNodes[nextNodeIndex];
        chain.nodeIndex = nextNodeIndex;
        chain.nextNeighbor = nextNode.neighbors[0] == nodeIndex ? 1 : 0;
        SG_ASSERT(nextNode.neighbors[1-chain.nextNeighbor] == nodeIndex);
        chain.position = GetPosition(nextNode);
    };
    auto PushOnStack = [&stack](size_t nodeIndex, float2 position)
    {
        stack.emplace_back();
        StackNode& back = stack.back();
        back.nodeIndex = nodeIndex;
        back.position = position;
    };

#if SG_TRIANGULATOR_ENABLE_DEBUG_IMAGE
    auto DrawTriangle = [&dbgImg, &ToDbgFrame, &brush](float2 const& iA, float2 const& iB, float2 const& iC, ubyte3 const& color)
    {
        auto GetInteriorPoint = [](float2 const A, float2 const N1, float2 const N2)
        {
            float2x2 M;
            M.SetRow(0, N1);
            M.SetRow(1, N2);
            float2x2 const invM = Invert_AssumeInvertible(M);
            return A + invM * float2(2);
        };
        float2 const iV[] = { iA, iB, iC };
        size_t const n = SG_ARRAYSIZE(iV);
        float2 V[n];
        for_range(size_t, i, 0, n)
            V[i] = ToDbgFrame(iV[i]);
        float2 E[n];
        float2 N[n];
        for_range(size_t, i, 0, n)
        {
            E[i] = V[(i+2) % 3] - V[(i+1) % 3];
            N[i] = Orthogonal(E[i]).Normalised();
        }
        float const sign = det(E[0], E[1]) > 0 ? 1.f : -1.f;
        for_range(size_t, i, 0, n)
            SG_ASSERT(0 < sign * dot(N[i], E[(i+1)%3]));
        float2 W[n];
        for_range(size_t, i, 0, n)
            W[i] = GetInteriorPoint(V[i], sign * N[(i+2) % 3], sign * N[(i+1) % 3]);
        for_range(size_t, i, 0, n)
            image::DrawLine1pxNoAA(dbgImg, roundi(W[i]), roundi(W[(i+1) % 3]), brush.Stroke(color));
    };
#endif

    {
        // push first two vertices on stack
        float2 const O = GetPosition(iNodes[iHead]);
        PushOnStack(iHead, O);

        for_range(size_t, i, 0, 2)
        {
            Chain& chain = chains[i];
            chains[i].nodeIndex = iHead;
            chains[i].nextNeighbor = i;
            AdvanceChain(chain);
        }
        float2 const P0 = chains[0].position;
        float2 const P1 = chains[1].position;
        float detOP0P1 = det(P0-O, P1-O);
        SG_ASSERT(0 != detOP0P1);
        chainValidDetSign[0] = detOP0P1 > 0.f ? -1.f : 1.f;
        chainValidDetSign[1] = -chainValidDetSign[0];
        size_t const chainIndex = GetSweepPos(P0) <= GetSweepPos(P1) ? 0 : 1;
        PushOnStack(chains[chainIndex].nodeIndex, chains[chainIndex].position);
        AdvanceChain(chains[chainIndex]);
        stackChainIndex = chainIndex;
    }

    auto AppendTriangle = [this, &triangleList](size_t indexA, size_t indexB, size_t indexC, bool backFacing)
    {
        size_t indices[] = { indexA, indexB, indexC };
        if(backFacing)
            std::swap(indices[1], indices[2]);
#if SG_ENABLE_ASSERT
        float2 Ps[3];
        for_range(size_t, i, 0, 3) { Ps[i] = m_vertexPositions[indices[i]]; }
        float const detABC = det(Ps[1]-Ps[0], Ps[2]-Ps[0]);
        SG_ASSERT(0 <= detABC);
#endif
        for_range(size_t, i, 0, 3)
            m_indices.push_back(indices[i]);
        triangleList.end += 3;
    };

    bool endOfPolygon = false;
    do
    {
        SG_ASSERT(2 <= stack.size());
        endOfPolygon = chains[0].nodeIndex == chains[1].nodeIndex;
        size_t const chainIndex = endOfPolygon ?
            1 - stackChainIndex :
            GetSweepPos(chains[0].position) <= GetSweepPos(chains[1].position) ? 0 : 1;
        Chain& chain = chains[chainIndex];
        if(chainIndex == stackChainIndex)
        {
            // Generate triangles with points on stack as long as edges are
            // valid.
            // Let's call A the current point, B and C the top points of the
            // stack.
            float2 const A = chain.position;
            size_t stackSize = stack.size();
            StackNode* pB = &stack[stackSize-1];
            do
            {
                StackNode* pC = &stack[stackSize-2];
                float2 const B = pB->position;
                float2 const C = pC->position;
                float detABC = det(B-A, C-A);
                if(detABC*chainValidDetSign[chainIndex] <= 0.f)
                    break;

                AppendTriangle(
                    iNodes[chain.nodeIndex].index,
                    iNodes[pB->nodeIndex].index,
                    iNodes[pC->nodeIndex].index,
                    detABC < 0);

#if SG_TRIANGULATOR_ENABLE_DEBUG_IMAGE
                DrawTriangle(A, B, C, ubyte3(40,180,200));
#endif

                stack.pop_back();
                --stackSize;
                SG_ASSERT(stackSize == stack.size());
                pB = pC;
            } while(1 < stackSize);
        }
        else
        {
            // All edges to points of stack are valid. Generate triangles with
            // all points on stack.
            float2 const A = chain.position;
            size_t stackSize = stack.size();
            StackNode* pB = &stack[stackSize-1];
            size_t const indexTop = pB->nodeIndex;
            float2 const top = pB->position;
            do {
                StackNode* pC = &stack[stackSize-2];
                // append triangle ABC
                AppendTriangle(
                    iNodes[chain.nodeIndex].index,
                    iNodes[pB->nodeIndex].index,
                    iNodes[pC->nodeIndex].index,
                    chainValidDetSign[chainIndex] >= 0.f);

#if SG_TRIANGULATOR_ENABLE_DEBUG_IMAGE
                float2 B = stack[stackSize-1].position;
                float2 C = stack[stackSize-2].position;
                DrawTriangle(A, B, C, ubyte3(40,60,200));
#endif

                --stackSize;
                pB = pC;
            } while(1 < stackSize);

            stack.clear();
            PushOnStack(indexTop, top);
            stackChainIndex = chainIndex;
        }
        PushOnStack(chain.nodeIndex, chain.position);
        AdvanceChain(chain);
        SG_ASSERT(2 <= stack.size());
    } while(!endOfPolygon);
}
#endif
//=============================================================================
#if SG_ENABLE_UNIT_TESTS
SG_TEST((sg, geometry), Triangulator, (Geometry, slow))
{
    filesystem::Init();
    filesystem::MountDeclaredMountingPoints();

    {
        Triangulator triangulator;
        float vertices[] = {
            0, 0, // 0
            2, 1,
            6,-1,
            7, 5,
            4, 6,
            3, 5, // 5
            5, 4,
            6, 2,
            2, 3,
            3, 8,
            1, 5, // 10

            0, 5,
            -1,-1,
            3,-1,
            5,-2,

            7,-2, // 15
            8, 7,
            7, 6,
        };
        SG_ASSERT(SG_ARRAYSIZE(vertices) % 2 == 0);
        size_t const vertexCount = SG_ARRAYSIZE(vertices) / 2;
        for_range(size_t, i, 0, vertexCount)
            triangulator.PushVertex(float2(vertices[2*i], vertices[2*i+1]));
        size_t const pid0 = triangulator.BeginPolygon();
        for_range(size_t, i, 0, 11)
            triangulator.PushPolygonVertex(i);
        triangulator.EndPolygon();

#if 0 // not supported yet
        size_t const pid1 = triangulator.BeginPolygon(Triangulator::PolygonType::Ribbon);
        size_t const indices1 [] = { 10, 11, 0, 12, 1, 13, 2, 14 };
        for_range(size_t, i, 0, SG_ARRAYSIZE(indices1))
            triangulator.PushPolygonVertex(indices1[i]);
        triangulator.EndPolygon();
#else
        size_t const indices1 [] = { 10, 11, 12, 13, 14, 2, 1, 0 };
        size_t const pid1 = triangulator.BeginPolygon();
        for_range(size_t, i, 0, SG_ARRAYSIZE(indices1))
            triangulator.PushPolygonVertex(indices1[i]);
        triangulator.EndPolygon();
#endif

        size_t const pid2 = triangulator.PushPolygon(2,14,15,16,17,9,8,7,6,5,4,3);
        size_t const pid3 = triangulator.PushPolygon(9,17,16);

        triangulator.Run();
    }

#if 1
    {
        Triangulator triangulator;
        float vertices[] = {
              0,   0,
             10,  10,
             20,   5,
             30,  10,
             40,   2,
             50,  10,
             60,   4,
             70,  10,
             80,   3,
             90,  10,
            100,   6,
            100,  80,
             70,  70,
             80, 100,
        };
        SG_ASSERT(SG_ARRAYSIZE(vertices) % 2 == 0);
        size_t const vertexCount = SG_ARRAYSIZE(vertices) / 2;
        for_range(size_t, i, 0, vertexCount)
            triangulator.PushVertex(float2(vertices[2*i], vertices[2*i+1]));
        size_t const pid0 = triangulator.BeginPolygon();
        for_range(size_t, i, 0, vertexCount)
            triangulator.PushPolygonVertex(i);
        triangulator.EndPolygon();

        triangulator.Run();
    }
#endif

    filesystem::Shutdown();
}
#endif
//=============================================================================
}
}


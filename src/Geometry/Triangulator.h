#ifndef Geometry_Triangulator_H
#define Geometry_Triangulator_H

#include <Math/Matrix.h>
#include <Math/Vector.h>
#include <Core/Assert.h>
#include <Core/ArrayView.h>
#include <Core/Config.h>
#include <Core/Utils.h>
#include <vector>

// The triangulator is a tool to generate triangle list from a set of polygons
// sharing vertices. It provides a framework to generate clean triangulation.
//
// A polygon can be cut in an hollow shape using sub polygons. However, this
// tesselator is not able to do other polygon operations (add, subtract, ...).
// Also, self-intersecting polygons are not supported.
//
// TO DO: a tool to compose polygons

namespace sg {
namespace geometry {
//=============================================================================
class Triangulator
{
public:
    struct TriangleList { size_t begin; size_t end; };
    enum class PolygonType { Polygon, Ribbon, };
public:
    Triangulator();
    size_t PushVertex(float2 const& iPosition);
    template <typename... vertex_id_type>
    size_t PushPolygon(vertex_id_type... iVertexIds);
    size_t BeginPolygon(PolygonType iType = PolygonType::Polygon);
    void PushPolygonVertex(size_t iVertexId);
    void EndPolygon();
    template <typename... vertex_id_type>
    void PushSubPolygon(size_t iMasterPolygonId, vertex_id_type... iVertexIds);
    void BeginSubPolygon(size_t iMasterPolygonId, PolygonType iType = PolygonType::Polygon);
    void EndSubPolygon();

    void Run();

    ArrayView<float2 const> GetVertexPositions() const { return AsArrayView(m_vertexPositions); }
    ArrayView<size_t const> GetIndices() const { SG_ASSERT(State::Tesselated == m_state); return AsArrayView(m_indices); }
    ArrayView<TriangleList const> GetPolygons() const { SG_ASSERT(State::Tesselated == m_state); return AsArrayView(m_tesselatedPolygons); }
private:
    enum class State {
        Writable,
        BuildPolygon,
        BuildSubPolygon,
        Tesselated,
    };
    struct Polygon
    {
        PolygonType type;
        size_t masterIndexOrSubCount;
        size_t begin;
        size_t end;
    };
    struct Node
    {
        size_t index;
        size_t neighbors[2];
        size_t toVisitCount;
    };
private:
    template <typename... vertex_id_type>
    void PushPolygonVertices(size_t iVertexId, vertex_id_type... iVertexIds);
    void PushPolygonVertices(size_t iVertexId) { PushPolygonVertex(iVertexId); }
private:
    State m_state;
    std::vector<float2> m_vertexPositions;
    std::vector<Polygon> m_polygons;
    std::vector<Polygon> m_subPolygons;
    std::vector<size_t> m_polygonIndices;

    std::vector<TriangleList> m_tesselatedPolygons;
    std::vector<size_t> m_indices;
};
//=============================================================================
template <typename... vertex_id_type>
size_t Triangulator::PushPolygon(vertex_id_type... iVertexIds)
{
    SG_ASSERT(State::Writable == m_state);
    size_t const id = BeginPolygon();
    PushPolygonVertices(iVertexIds...);
    EndPolygon();
    return id;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename... vertex_id_type>
void Triangulator::PushPolygonVertices(size_t iVertexId, vertex_id_type... iVertexIds)
{
    PushPolygonVertex(iVertexId);
    PushPolygonVertices(iVertexIds...);
}
//=============================================================================
template <typename V, typename P>
class TriangulatorEx
{
public:
    typedef V vertex_data_type;
    typedef P polygon_data_type;

    size_t PushVertex(float2 const& iPosition, vertex_data_type const& iData);
    template <typename... vertex_id_type>
    size_t PushPolygon(polygon_data_type const& iData, vertex_id_type... iVertexIds);
    size_t BeginPolygon(polygon_data_type const& iData, Triangulator::PolygonType iType = Triangulator::PolygonType::Polygon);
    void PushPolygonVertex(size_t iVertexId);
    void EndPolygon();
    template <typename... vertex_id_type>
    void PushSubPolygon(size_t iMasterPolygonId, vertex_id_type... iVertexIds);
    void BeginSubPolygon(size_t iMasterPolygonId, Triangulator::PolygonType iType = Triangulator::PolygonType::Polygon);
    void EndSubPolygon();

    void Run();

    ArrayView<float2 const> GetVertexPositions() const { return m_baseTriangulator.GetVertexPositions(); }
    ArrayView<vertex_data_type const> GetVertexData() const { return AsArrayView(m_vertexData); }
    ArrayView<size_t const> GetIndices() const { return m_baseTriangulator.GetIndices(); }
    ArrayView<Triangulator::TriangleList const> GetPolygons() const { return m_baseTriangulator.GetPolygons(); }
    ArrayView<polygon_data_type const> GetPolygonsData() const { return AsArrayView(m_polygonData); }
private:
    std::vector<vertex_data_type> m_vertexData;
    std::vector<polygon_data_type> m_polygonData;
    Triangulator m_baseTriangulator;
};
//=============================================================================
template <typename V, typename P>
size_t TriangulatorEx<V, P>::PushVertex(float2 const& iPosition, vertex_data_type const& iData)
{
    size_t const i = m_baseTriangulator.PushVertex(iPosition);
    SG_ASSERT(i == m_vertexData.size());
    m_vertexData.push_back(iData);
    return i;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename V, typename P>
template <typename... vertex_id_type>
size_t TriangulatorEx<V, P>::PushPolygon(polygon_data_type const& iData, vertex_id_type... iVertexIds)
{
    size_t const i = m_baseTriangulator.PushPolygon(iVertexIds...);
    SG_ASSERT(i == m_polygonData.size());
    m_polygonData.push_back(iData);
    return i;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename V, typename P>
size_t TriangulatorEx<V, P>::BeginPolygon(polygon_data_type const& iData, Triangulator::PolygonType iType = Triangulator::PolygonType::Polygon)
{
    size_t const i = m_baseTriangulator.BeginPolygon(iType);
    SG_ASSERT(i == m_polygonData.size());
    m_polygonData.push_back(iData);
    return i;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename V, typename P>
void TriangulatorEx<V, P>::PushPolygonVertex(size_t iVertexId)
{
    m_baseTriangulator.PushPolygonVertex(iVertexId);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename V, typename P>
void TriangulatorEx<V, P>::EndPolygon()
{
    m_baseTriangulator.EndPolygon();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename V, typename P>
template <typename... vertex_id_type>
void TriangulatorEx<V, P>::PushSubPolygon(size_t iMasterPolygonId, vertex_id_type... iVertexIds)
{
    m_baseTriangulator.PushSubPolygon(iMasterPolygonId, iVertexIds...);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename V, typename P>
void TriangulatorEx<V, P>::BeginSubPolygon(size_t iMasterPolygonId, Triangulator::PolygonType iType = Triangulator::PolygonType::Polygon)
{
    m_baseTriangulator.BeginSubPolygon(iMasterPolygonId, iType);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename V, typename P>
void TriangulatorEx<V, P>::EndSubPolygon()
{
    m_baseTriangulator.EndSubPolygon();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename V, typename P>
void TriangulatorEx<V, P>::Run()
{
    m_baseTriangulator.Run();
}
//=============================================================================
}
}

#endif

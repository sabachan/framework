#ifndef Geometry_Triangulate_H
#define Geometry_Triangulate_H

#include <Math/Vector.h>
#include <Core/Assert.h>
#include <Core/ArrayView.h>
#include <Core/Config.h>
#include <Core/IntTypes.h>
#include <Core/Utils.h>
#include <vector>


namespace sg {
namespace geometry {
//=============================================================================
enum class TriangulateReturnCode
{
    Ok = 0,
    UnknownError = -128,
    SelfIntersecting,
};
struct TriangulateParameters
{
    float epsilon;
};
// iOutlines contains the number of vertices for each outlines. It is expected
// that the first outline is the external outline while others are internal
// holes. Also, vertices should be in the order where interior is on the left,
// ie, they must be in direct order for first outline, and indirect order for
// other outlines.
TriangulateReturnCode Triangulate(ArrayView<float2 const> iVertices,
                                  ArrayView<size_t const> iOutlines,
                                  ArrayView<size_t> ioTriangles,
                                  TriangulateParameters const* iOptionalParameters = nullptr);
TriangulateReturnCode Triangulate(ArrayView<float2 const> iVertices,
                                  ArrayView<size_t> ioTriangles,
                                  TriangulateParameters const* iOptionalParameters = nullptr);
//=============================================================================
}
}

#endif

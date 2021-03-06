#include "stdafx.h"

#include "VertexTypes.h"

#include "WTF/IncludeD3D11.h"

namespace sg {
namespace rendering {
//=============================================================================
//#define BEGIN_VERTEX_IMPL(VERTEX_TYPE) \
//    namespace { \
//        D3D11_INPUT_ELEMENT_DESC const VERTEX_TYPE##_inputEltDesc[] = {
//#define END_VERTEX_IMPL(VERTEX_TYPE) \
//        }; \
//    } \
//    ArrayView<D3D11_INPUT_ELEMENT_DESC const> \
//    VERTEX_TYPE::InputEltDesc() \
//    { \
//        return ArrayView<D3D11_INPUT_ELEMENT_DESC const>( \
//            VERTEX_TYPE##_inputEltDesc, \
//            SG_ARRAYSIZE(VERTEX_TYPE##_inputEltDesc) \
//            ); \
//    }
//=============================================================================
#define BEGIN_VERTEX_IMPL(VERTEX_TYPE) \
    ArrayView<D3D11_INPUT_ELEMENT_DESC const> \
    VERTEX_TYPE::InputEltDesc() \
    { \
        static D3D11_INPUT_ELEMENT_DESC const inputEltDesc[] = {
#define END_VERTEX_IMPL(...) \
        }; \
        return ArrayView<D3D11_INPUT_ELEMENT_DESC const>( \
            inputEltDesc, \
            SG_ARRAYSIZE(inputEltDesc) \
            ); \
    }
//=============================================================================
BEGIN_VERTEX_IMPL(Vertex_Pos2f_Tex2f)
    {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
END_VERTEX_IMPL(Vertex_Pos2f_Tex2f)
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BEGIN_VERTEX_IMPL(Vertex_Pos2f_Tex2f_Col4f)
    {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
END_VERTEX_IMPL(Vertex_Pos2f_Tex2f_Col4f)
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BEGIN_VERTEX_IMPL(Vertex_Pos2f_Tex2f_2Col4f)
    {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
END_VERTEX_IMPL(Vertex_Pos2f_Tex2f_2Col4f)
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BEGIN_VERTEX_IMPL(Vertex_Pos2f_Tex2f_2Col4f_Col4ub)
    {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    2, DXGI_FORMAT_R8G8B8A8_UINT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
END_VERTEX_IMPL(Vertex_Pos2f_Tex2f_2Col4f_Col4ub)
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BEGIN_VERTEX_IMPL(Vertex_Pos2f_Tex2f_Col4ub)
    {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    0, DXGI_FORMAT_R8G8B8A8_UINT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
END_VERTEX_IMPL(Vertex_Pos2f_Tex2f_Col4ub)
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BEGIN_VERTEX_IMPL(Vertex_Pos2f_Tex2f_2Col4ub)
    {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    0, DXGI_FORMAT_R8G8B8A8_UINT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    1, DXGI_FORMAT_R8G8B8A8_UINT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
END_VERTEX_IMPL(Vertex_Pos2f_Tex2f_2Col4ub)
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BEGIN_VERTEX_IMPL(Vertex_Pos2f_Tex2f_3Col4ub)
    {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    0, DXGI_FORMAT_R8G8B8A8_UINT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    1, DXGI_FORMAT_R8G8B8A8_UINT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    2, DXGI_FORMAT_R8G8B8A8_UINT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
END_VERTEX_IMPL(Vertex_Pos2f_Tex2f_3Col4ub)
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BEGIN_VERTEX_IMPL(Vertex_Pos2f_Tex3f_2Col4f)
    {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXTURE",  0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
END_VERTEX_IMPL(Vertex_Pos2f_Tex3f_2Col4f)
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BEGIN_VERTEX_IMPL(Vertex_Pos2f_Tex4f_2Col4f)
    {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXTURE",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
END_VERTEX_IMPL(Vertex_Pos2f_Tex4f_2Col4f)
//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BEGIN_VERTEX_IMPL(Vertex_Pos3f_Pos2f_Tex2f_Col4f_Col4ub)
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"POSITION", 1, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    1, DXGI_FORMAT_R8G8B8A8_UNORM,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
END_VERTEX_IMPL(Vertex_Pos3f_Pos2f_Tex2f_Col4f_Col4ub)
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BEGIN_VERTEX_IMPL(Vertex_Pos3f_Normal3f_Tex2f_Col4f)
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
END_VERTEX_IMPL(Vertex_Pos3f_Normal3f_Tex2f_Col4f)
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BEGIN_VERTEX_IMPL(Vertex_Pos3f_Normal3f_Tex2f_Col4f_BlendIndices2u16_BlendWeights2u16)
    {"POSITION",        0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"NORMAL",          0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXTURE",         0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",           0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"BLENDINDICES",    0, DXGI_FORMAT_R16G16_UINT,        0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"BLENDWEIGHT",     0, DXGI_FORMAT_R16G16_UNORM,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
END_VERTEX_IMPL(Vertex_Pos3f_Normal3f_Tex2f_Col4f_BlendIndices2u16_BlendWeights2u16)
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BEGIN_VERTEX_IMPL(Vertex_Pos3f_2Normal3f_Tex2f_Col4f_BlendIndices2u16_BlendWeights2u16)
    {"POSITION",        0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"NORMAL",          0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"NORMAL",          1, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXTURE",         0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",           0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"BLENDINDICES",    0, DXGI_FORMAT_R16G16_UINT,        0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"BLENDWEIGHT",     0, DXGI_FORMAT_R16G16_UNORM,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
END_VERTEX_IMPL(Vertex_Pos3f_Normal3f_Tex2f_Col4f_BlendIndices2u16_BlendWeights2u16)
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BEGIN_VERTEX_IMPL(Vertex_Pos3f_Tex2f)
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
END_VERTEX_IMPL(Vertex_Pos3f_Tex2f)
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BEGIN_VERTEX_IMPL(Vertex_Pos3f_Tex2f_Col4f)
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
END_VERTEX_IMPL(Vertex_Pos2f_Tex2f_Col4f)
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BEGIN_VERTEX_IMPL(Vertex_Pos3f_Tex2f_2Col4f_Col4ub)
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    2, DXGI_FORMAT_R8G8B8A8_UINT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
END_VERTEX_IMPL(Vertex_Pos3f_Tex2f_2Col4f_Col4ub)
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BEGIN_VERTEX_IMPL(Vertex_Pos3f_Tex2f_Col4ub)
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    0, DXGI_FORMAT_R8G8B8A8_UINT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
END_VERTEX_IMPL(Vertex_Pos3f_Tex2f_Col4ub)
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BEGIN_VERTEX_IMPL(Vertex_Pos3f_Tex2f_2Col4ub)
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    0, DXGI_FORMAT_R8G8B8A8_UINT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    1, DXGI_FORMAT_R8G8B8A8_UINT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
END_VERTEX_IMPL(Vertex_Pos3f_Tex2f_2Col4ub)
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BEGIN_VERTEX_IMPL(Vertex_Pos3f_Tex2f_3Col4ub)
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    0, DXGI_FORMAT_R8G8B8A8_UINT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    1, DXGI_FORMAT_R8G8B8A8_UINT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    2, DXGI_FORMAT_R8G8B8A8_UINT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
END_VERTEX_IMPL(Vertex_Pos3f_Tex2f_3Col4ub)
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BEGIN_VERTEX_IMPL(Vertex_Pos3f_Tex2f_4Col4ub)
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    0, DXGI_FORMAT_R8G8B8A8_UINT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    1, DXGI_FORMAT_R8G8B8A8_UINT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    2, DXGI_FORMAT_R8G8B8A8_UINT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    3, DXGI_FORMAT_R8G8B8A8_UINT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
END_VERTEX_IMPL(Vertex_Pos3f_Tex2f_4Col4ub)
//=============================================================================
#undef BEGIN_VERTEX_IMPL
#undef END_VERTEX_IMPL
//=============================================================================
}
}

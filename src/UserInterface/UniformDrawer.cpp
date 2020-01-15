#include "stdafx.h"

#include "UniformDrawer.h"

#include "Context.h"
#include <Rendering/Material.h>
#include <Rendering/RenderBatchDico.h>
#include <Rendering/RenderDevice.h>
#include <Rendering/TransientRenderBatch.h>
#include <Rendering/VertexTypes.h>
#include <RenderEngine/CompositingLayer.h>
#include <RenderEngine/ShaderDescriptors.h>

namespace sg {
namespace ui {
//=============================================================================
namespace {
    typedef rendering::Vertex_Pos2f_Tex2f_Col4f UniformDrawerVertex;
}
//=============================================================================
size_t UniformDrawer::VertexSize() { return sizeof(UniformDrawerVertex); }
ArrayView<D3D11_INPUT_ELEMENT_DESC const> UniformDrawer::VertexDescriptor() { return UniformDrawerVertex::InputEltDesc(); }
//=============================================================================
UniformDrawer::UniformDrawer(rendering::Material const* iMaterial)
: m_material(iMaterial)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
UniformDrawer::~UniformDrawer()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void UniformDrawer::DrawFrame(DrawContext const& iContext,
                              box2f const& iInBox,
                              box2f const& iOutBox,
                              Color4f const& col,
                              size_t layer) const
{
    SG_ASSERT(iOutBox.Contains(iInBox));
    if(-1 == layer)
    {
        LayerManager* layerManager = iContext.GetLayerManager();
        layer = layerManager->Lock(iContext.TransformBoundingBox(iOutBox))[0];
    }
    box2f const box0 = box2f::FromMinMax(iOutBox.Min(), float2(iOutBox.Max().x(), iInBox.Min().y()));
    DrawQuad(iContext, box0, col, layer);
    box2f const box1 = box2f::FromMinMax(float2(iOutBox.Min().x(), iInBox.Max().y()), iOutBox.Max());
    DrawQuad(iContext, box1, col, layer);
    box2f const box2 = box2f::FromMinMax(float2(iOutBox.Min().x(), iInBox.Min().y()), float2(iInBox.Min().x(), iInBox.Max().y()));
    DrawQuad(iContext, box2, col, layer);
    box2f const box3 = box2f::FromMinMax(float2(iInBox.Max().x(), iInBox.Min().y()), float2(iOutBox.Max().x(), iInBox.Max().y()));
    DrawQuad(iContext, box3, col, layer);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void UniformDrawer::DrawQuad(DrawContext const& iContext,
                             box2f const& iBox,
                             Color4f const& col,
                             size_t layer) const
{
    SG_ASSERT(AllGreaterStrict(iBox.Delta(), float2(0)));
    Context::TransformType const transformType = iContext.GetTransformType();
    float2 corners[4] = { float2(uninitialized), float2(uninitialized), float2(uninitialized), float2(uninitialized)};
    switch(transformType)
    {
    case Context::TransformType::None:
        corners[0] = float2(iBox.min.x(), iBox.min.y());
        corners[1] = float2(iBox.max.x(), iBox.min.y());
        corners[2] = float2(iBox.min.x(), iBox.max.y());
        corners[3] = float2(iBox.max.x(), iBox.max.y());
        break;
    case Context::TransformType::Translate2D:
        {
            float4x4 const& transform = iContext.GetTransform();
            float2 const T = transform.Col(3).xy();
            corners[0] = float2(iBox.min.x(), iBox.min.y()) + T;
            corners[1] = float2(iBox.max.x(), iBox.min.y()) + T;
            corners[2] = float2(iBox.min.x(), iBox.max.y()) + T;
            corners[3] = float2(iBox.max.x(), iBox.max.y()) + T;
        }
        break;
    case Context::TransformType::Transform2D:
        {
            float4x4 const& transform = iContext.GetTransform();
            float2x2 const R = transform.SubMatrix<2, 2>(0, 0);
            float2 const T = transform.Col(3).xy();
            corners[0] = R * float2(iBox.min.x(), iBox.min.y()) + T;
            corners[1] = R * float2(iBox.max.x(), iBox.min.y()) + T;
            corners[2] = R * float2(iBox.min.x(), iBox.max.y()) + T;
            corners[3] = R * float2(iBox.max.x(), iBox.max.y()) + T;
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
        layer = layerManager->Lock(iContext.TransformBoundingBox(iBox))[0];
    }

    rendering::TransientRenderBatch::Properties properties;
    properties.indexSize = sizeof(u32);
    properties.vertexSize = sizeof(UniformDrawerVertex);
    rendering::TransientRenderBatch* renderBatch = iContext.GetCompositingLayer()->GetOrCreateTransientRenderBatch(
        *m_material,
        properties);

    rendering::TransientRenderBatch::WriteAccess<UniformDrawerVertex> writeAccess(*renderBatch, 4, 6, layer);
    UniformDrawerVertex* vertices = writeAccess.GetVertexPointerForWriting();
    vertices[0].pos = corners[0];
    vertices[1].pos = corners[1];
    vertices[2].pos = corners[2];
    vertices[3].pos = corners[3];
    vertices[0].tex = float2(0.f, 0.f);
    vertices[1].tex = float2(1.f, 0.f);
    vertices[2].tex = float2(0.f, 1.f);
    vertices[3].tex = float2(1.f, 1.f);
    vertices[0].col = float4(col);
    vertices[1].col = float4(col);
    vertices[2].col = float4(col);
    vertices[3].col = float4(col);
    writeAccess.PushIndices(0, 2, 1);
    writeAccess.PushIndices(1, 2, 3);
    writeAccess.FinishWritingVertex(4);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void UniformDrawer::DrawLine(DrawContext const& iContext,
                             float2 const& A,
                             float2 const& B,
                             Color4f const& colA,
                             Color4f const& colB,
                             float thicknessA,
                             float thicknessB,
                             size_t layer) const
{
    float2 const AB = B - A;
    float2 const ey = AB.Normalised();
    float2 const ex = -Orthogonal(ey);
    Context::TransformType const transformType = iContext.GetTransformType();
    float2 corners[4] = { A - 0.5f * thicknessA * ex, A + 0.5f * thicknessA * ex, B - 0.5f * thicknessB * ex, B + 0.5f * thicknessB * ex };
    switch(transformType)
    {
    case Context::TransformType::None:
        break;
    case Context::TransformType::Translate2D:
        {
            float4x4 const& transform = iContext.GetTransform();
            float2 const T = transform.Col(3).xy();
            corners[0] += T;
            corners[1] += T;
            corners[2] += T;
            corners[3] += T;
        }
        break;
    case Context::TransformType::Transform2D:
        {
            float4x4 const& transform = iContext.GetTransform();
            float2x2 const R = transform.SubMatrix<2, 2>(0, 0);
            float2 const T = transform.Col(3).xy();
            corners[0] = R * corners[0] + T;
            corners[1] = R * corners[1] + T;
            corners[2] = R * corners[2] + T;
            corners[3] = R * corners[3] + T;
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
        box2f bb;
        for(auto const& c : corners)
            bb.Grow(c);
        LayerManager* layerManager = iContext.GetLayerManager();
        layer = layerManager->Lock(bb)[0];
    }

    rendering::TransientRenderBatch::Properties properties;
    properties.indexSize = sizeof(u32);
    properties.vertexSize = sizeof(UniformDrawerVertex);
    rendering::TransientRenderBatch* renderBatch = iContext.GetCompositingLayer()->GetOrCreateTransientRenderBatch(
        *m_material,
        properties);

    rendering::TransientRenderBatch::WriteAccess<UniformDrawerVertex> writeAccess(*renderBatch, 4, 6, layer);
    UniformDrawerVertex* vertices = writeAccess.GetVertexPointerForWriting();
    vertices[0].pos = corners[0];
    vertices[1].pos = corners[1];
    vertices[2].pos = corners[2];
    vertices[3].pos = corners[3];
    vertices[0].tex = float2(0.f, 0.f);
    vertices[1].tex = float2(1.f, 0.f);
    vertices[2].tex = float2(0.f, 1.f);
    vertices[3].tex = float2(1.f, 1.f);
    vertices[0].col = float4(colA);
    vertices[1].col = float4(colA);
    vertices[2].col = float4(colB);
    vertices[3].col = float4(colB);
    writeAccess.PushIndices(0, 2, 1);
    writeAccess.PushIndices(1, 2, 3);
    writeAccess.FinishWritingVertex(4);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//                 E[4]      iFlags[2]      E[5]
//    iFlags[6]     +--------------------------+    iFlags[7]
//               /  |                          |  \
//         E[8] +---+--------------------------+---+ E[9]
//              |   | P[0]            ^   P[1] |   |
//              |   |       iFlags[1] |        |   |
//              |   |                 |        |   |
//  iFlags[4]   |   |   iFlags[0]     |        |   |   iFlags[5]
//              |   | <----------------------> |   |
//              |   |                 |        |   |
//              |   | P[2]            v   P[3] |   |
//        E[10] +---+--------------------------+---+ E[11]
//               \  |                          |  /
//    iFlags[8]     +--------------------------+    iFlags[9]
//                E[6]     iFlags[3]       E[7]
//
void UniformDrawer::DrawAAQuad(DrawContext const& iContext,
                               float2 const (&iPos)[12],
                               float2 const (&iTex)[12],
                               Color4f const (&iCols)[12],
                               BitSet<10> iFlags,
                               size_t layer) const
{
    size_t pointIndices[12];
    size_t pointCount = 0;
    pointIndices[0]  = pointCount++;
    pointIndices[1]  = iFlags[0] ? pointCount++ : pointIndices[0];
    pointIndices[2]  = iFlags[1] ? pointCount++ : pointIndices[0];
    pointIndices[3]  = iFlags[0] && iFlags[1] ? pointCount++ : pointIndices[1];
    size_t const baseVertexCount = pointCount;
    pointIndices[4]  = iFlags[2] || iFlags[4] ? pointCount++ : pointCount;
    pointIndices[5]  = iFlags[0] && (iFlags[2] || iFlags[7]) ? pointCount++ : pointCount;
    pointIndices[6]  = iFlags[3] || iFlags[8] ? pointCount++ : pointCount;
    pointIndices[7]  = iFlags[0] && (iFlags[3] || iFlags[9]) ? pointCount++ : pointCount;
    pointIndices[8]  = iFlags[6] ? pointCount++ : pointIndices[4];
    pointIndices[9]  = iFlags[7] ? pointCount++ : pointIndices[5];
    pointIndices[10] = iFlags[8] ? pointCount++ : pointIndices[6];
    pointIndices[11] = iFlags[9] ? pointCount++ : pointIndices[7];
    float2 points[12];
    size_t const vertexCount = pointCount;
    for_range(size_t, i, 0, pointCount) points[i] = iPos[i];
    Context::TransformType const transformType = iContext.GetTransformType();
    switch(transformType)
    {
    case Context::TransformType::None:
        break;
    case Context::TransformType::Translate2D:
        {
            float4x4 const& transform = iContext.GetTransform();
            float2 const T = transform.Col(3).xy();
            for_range(size_t, i, 0, vertexCount)
                points[i] = points[i] + T;
        }
        break;
    case Context::TransformType::Transform2D:
        {
            float4x4 const& transform = iContext.GetTransform();
            float2x2 const R = transform.SubMatrix<2, 2>(0, 0);
            float2 const T = transform.Col(3).xy();
            for_range(size_t, i, 0, vertexCount)
                points[i] = R * points[i] + T;
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
        box2f box;
        for_range(size_t, i, 0, vertexCount)
            box.Grow(points[i]);
        LayerManager* layerManager = iContext.GetLayerManager();
        layer = layerManager->Lock(iContext.TransformBoundingBox(box))[0];
    }

    rendering::TransientRenderBatch::Properties properties;
    properties.indexSize = sizeof(u32);
    properties.vertexSize = sizeof(UniformDrawerVertex);
    rendering::TransientRenderBatch* renderBatch = iContext.GetCompositingLayer()->GetOrCreateTransientRenderBatch(
        *m_material,
        properties);

    size_t const maxTriangleCount = 2 + 4 * 2 + 4;
    size_t const maxIndexCount = maxTriangleCount * 3;
    rendering::TransientRenderBatch::WriteAccess<UniformDrawerVertex> writeAccess(*renderBatch, vertexCount, maxIndexCount, layer);
    UniformDrawerVertex* vertices = writeAccess.GetVertexPointerForWriting();
    for_range(size_t, i, 0, vertexCount)
    {
        vertices[i].pos = points[i];
        vertices[i].tex = iTex[i];
        vertices[i].col = float4(iCols[i]);
    }

    writeAccess.PushIndices(0, 2, 1);
    writeAccess.PushIndices(1, 2, 3);

    if(iFlags[0] && iFlags[1])
    {
        writeAccess.PushIndices(pointIndices[0], pointIndices[2], pointIndices[1]);
        writeAccess.PushIndices(pointIndices[1], pointIndices[2], pointIndices[3]);
    }
    if(iFlags[2])
    {
        writeAccess.PushIndices(pointIndices[0], pointIndices[1], pointIndices[5]);
        writeAccess.PushIndices(pointIndices[0], pointIndices[5], pointIndices[4]);
    }
    if(iFlags[3])
    {
        writeAccess.PushIndices(pointIndices[3], pointIndices[2], pointIndices[6]);
        writeAccess.PushIndices(pointIndices[3], pointIndices[6], pointIndices[7]);
    }
    if(iFlags[4])
    {
        writeAccess.PushIndices(pointIndices[2], pointIndices[0], pointIndices[8]);
        writeAccess.PushIndices(pointIndices[2], pointIndices[8], pointIndices[10]);
    }
    if(iFlags[5])
    {
        writeAccess.PushIndices(pointIndices[1], pointIndices[3], pointIndices[11]);
        writeAccess.PushIndices(pointIndices[1], pointIndices[11], pointIndices[9]);
    }
    if(iFlags[6])
    {
        writeAccess.PushIndices(pointIndices[0], pointIndices[4], pointIndices[8]);
    }
    if(iFlags[7])
    {
        writeAccess.PushIndices(pointIndices[1], pointIndices[9], pointIndices[5]);
    }
    if(iFlags[8])
    {
        writeAccess.PushIndices(pointIndices[2], pointIndices[10], pointIndices[6]);
    }
    if(iFlags[9])
    {
        writeAccess.PushIndices(pointIndices[3], pointIndices[7], pointIndices[11]);
    }

    writeAccess.FinishWritingVertex(vertexCount);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void UniformDrawer::DrawAAParalelogram(DrawContext const& iContext,
                                       float2 const& iOrigin,
                                       float2 const& iAxisX,
                                       float2 const& iAxisY,
                                       Color4f const (&iCols) [4],
                                       float const (&iAARadius) [4],
                                       size_t layer) const
{
    SG_UNUSED(iAARadius);
    Context::TransformType const transformType = iContext.GetTransformType();
    float2 corners[4] = { iOrigin, iOrigin + iAxisX, iOrigin + iAxisY, iOrigin + iAxisX + iAxisY};
    switch(transformType)
    {
    case Context::TransformType::None:
        break;
    case Context::TransformType::Translate2D:
        {
            float4x4 const& transform = iContext.GetTransform();
            float2 const T = transform.Col(3).xy();
            for(float2& c : corners)
                c = c + T;
        }
        break;
    case Context::TransformType::Transform2D:
        {
            float4x4 const& transform = iContext.GetTransform();
            float2x2 const R = transform.SubMatrix<2, 2>(0, 0);
            float2 const T = transform.Col(3).xy();
            for(float2& c : corners)
                c = R * c + T;
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
        box2f box;
        for(float2& c : corners)
            box.Grow(c);
        LayerManager* layerManager = iContext.GetLayerManager();
        layer = layerManager->Lock(iContext.TransformBoundingBox(box))[0];
    }

    rendering::TransientRenderBatch::Properties properties;
    properties.indexSize = sizeof(u32);
    properties.vertexSize = sizeof(UniformDrawerVertex);
    rendering::TransientRenderBatch* renderBatch = iContext.GetCompositingLayer()->GetOrCreateTransientRenderBatch(
        *m_material,
        properties);

    rendering::TransientRenderBatch::WriteAccess<UniformDrawerVertex> writeAccess(*renderBatch, 4, 6, layer);
    UniformDrawerVertex* vertices = writeAccess.GetVertexPointerForWriting();
    vertices[0].pos = corners[0];
    vertices[1].pos = corners[1];
    vertices[2].pos = corners[2];
    vertices[3].pos = corners[3];
    vertices[0].tex = float2(0.f, 0.f);
    vertices[1].tex = float2(1.f, 0.f);
    vertices[2].tex = float2(0.f, 1.f);
    vertices[3].tex = float2(1.f, 1.f);
    vertices[0].col = float4(iCols[0]);
    vertices[1].col = float4(iCols[1]);
    vertices[2].col = float4(iCols[2]);
    vertices[3].col = float4(iCols[3]);
    writeAccess.PushIndices(0, 2, 1);
    writeAccess.PushIndices(1, 2, 3);
    writeAccess.FinishWritingVertex(4);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void UniformDrawer::DrawAALine(DrawContext const& iContext,
                               float2 const& A,
                               float2 const& B,
                               Color4f const& colA,
                               Color4f const& colB,
                               float thicknessA,
                               float thicknessB,
                               float aaRadius,
                               size_t layer) const
{
    float2 const AB = B-A;
    float2 const ex = AB.Normalised();
    float2 const ey = Orthogonal(ex);
    float aa = aaRadius;
    Context::TransformType const transformType = iContext.GetTransformType();
    switch(transformType)
    {
    case Context::TransformType::None:
    case Context::TransformType::Translate2D:
        break;
    case Context::TransformType::Transform2D:
        {
            float4x4 const& transform = iContext.GetTransform();
            float2x2 const R = transform.SubMatrix<2, 2>(0, 0);
            float const avgScale = 0.5f * Trace(R);
            aa /= avgScale;
        }
        break;
    case Context::TransformType::Transform3D:
        SG_ASSERT_NOT_IMPLEMENTED();
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }
    Color4f cA0 = colA; cA0.a() = 0;
    Color4f cB0 = colB; cB0.a() = 0;
    if(aa <= 0.f)
    {
        float const wA = 0.5f * thicknessA;
        float const wB = 0.5f * thicknessB;
        float2 const pos[12] = { A+wA*ey, B-wB*ey, A+wA*ey, B+wB*ey };
        float2 const tex[12] = {};
        Color4f const cols[12] = { colA, colB, colA, colB, };
        DrawAAQuad(iContext, pos, tex, cols, BitSet<10>(0b11), layer);
    }
    if(thicknessA <= aa && thicknessB <= aa)
    {
        float const a = 0.5f * aa;
        float2 const pos[12] = { A+a*ex, B-a*ex, A-a*ex-aa*ey, B+a*ex-aa*ey, A-a*ex+aa*ey, B+a*ex+aa*ey };
        float2 const tex[12] = {};
        Color4f cA1 = colA; cA1.a() *= thicknessA / aa;
        Color4f cB1 = colB; cB1.a() *= thicknessB / aa;
        Color4f const cols[12] = { cA1, cB1, cA0, cB0, cA0, cB0 };
        DrawAAQuad(iContext, pos, tex, cols, BitSet<10>(0b111101), layer);
    }
    else
    {
        float const a = 0.5f * aa;
        float const wA = 0.5f * std::max(0.f, thicknessA - aa);
        float const wB = 0.5f * std::max(0.f, thicknessB - aa);
        Color4f cA1 = colA; cA1.a() *= std::min(1.f, thicknessA / aa);
        Color4f cB1 = colB; cB1.a() *= std::min(1.f, thicknessB / aa);
        float2 const pos[12] = { A+a*ex-wA*ey, B-a*ex-wB*ey, A+a*ex+wA*ey, B-a*ex+wB*ey, A-a*ex-(wA+aa)*ey, B+a*ex-(wB+aa)*ey, A-a*ex+(wA+aa)*ey, B+a*ex+(wB+aa)*ey };
        float2 const tex[12] = {};
        Color4f const cols[12] = { cA1, cB1, cA1, cB1, cA0, cB0, cA0, cB0 };
        DrawAAQuad(iContext, pos, tex, cols, BitSet<10>(0b111111), layer);
    }
}
//=============================================================================
UniformDrawerDescriptor::UniformDrawerDescriptor()
    : m_drawer()
    , m_pixelShader()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
UniformDrawerDescriptor::~UniformDrawerDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void UniformDrawerDescriptor::CreateDrawer()
{
    refptr<rendering::VertexShaderDescriptor> vsDesc = new rendering::VertexShaderDescriptor(FilePath("src:/UserInterface/Shaders/V_UI_UniformDrawer.hlsl"), "vmain");
    rendering::VertexShaderProxy vs = vsDesc->GetProxy();
    rendering::ShaderInputLayoutProxy layout(ui::UniformDrawer::VertexDescriptor(), vs);
    rendering::PixelShaderProxy ps = m_pixelShader->GetProxy();
    rendering::RenderStateName blendState("Premultiplied Alpha Blending"); // TODO: expose blend state as a property
    refptr<rendering::Material> material = new rendering::Material(layout, vs, ps, blendState);
    m_drawer = new ui::UniformDrawer(material.get());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, ui), UniformDrawerDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(pixelShader, "")
REFLECTION_CLASS_END
//=============================================================================
}
}

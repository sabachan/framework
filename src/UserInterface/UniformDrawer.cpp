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

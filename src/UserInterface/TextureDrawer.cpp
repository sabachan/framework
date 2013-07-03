#include "stdafx.h"

#include "TextureDrawer.h"

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
    typedef rendering::Vertex_Pos2f_Tex2f_Col4f TextureDrawerVertex;
}
//=============================================================================
size_t TextureDrawer::VertexSize() { return sizeof(TextureDrawerVertex); }
ArrayView<D3D11_INPUT_ELEMENT_DESC const> TextureDrawer::VertexDescriptor() { return TextureDrawerVertex::InputEltDesc(); }
//=============================================================================
TextureDrawer::TextureDrawer(rendering::Material const* iMaterial, rendering::ShaderResourceName iTextureName)
: m_material(iMaterial)
, m_textureName(iTextureName)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TextureDrawer::~TextureDrawer()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TextureDrawer::DrawTexture(DrawContext const& iContext,
                                rendering::IShaderResource const* iTexture,
                                box2f const& iTextureQuad,
                                Color4f const& col,
                                size_t layer) const
{
    Context::TransformType const transformType = iContext.GetTransformType();
    float2 corners[4] = { float2(uninitialized), float2(uninitialized), float2(uninitialized), float2(uninitialized)};
    box2f const iBox = iTextureQuad;
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

    rendering::Material const* material = GetTextureMaterial(iTexture);
    SG_ASSERT(nullptr != material);

    rendering::TransientRenderBatch::Properties properties;
    properties.indexSize = sizeof(u32);
    properties.vertexSize = sizeof(TextureDrawerVertex);
    rendering::TransientRenderBatch* renderBatch = iContext.GetCompositingLayer()->GetOrCreateTransientRenderBatch(
        *material,
        properties);

    rendering::TransientRenderBatch::WriteAccess<TextureDrawerVertex> writeAccess(*renderBatch, 4, 6, layer);
    TextureDrawerVertex* vertices = writeAccess.GetVertexPointerForWriting();
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
void TextureDrawer::UnregisterTexture(rendering::IShaderResource const* iTexture) const
{
    size_t const count = m_materialFromTexture.erase(iTexture);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::Material const* TextureDrawer::GetTextureMaterial(rendering::IShaderResource const* iTexture) const
{
    auto& f = m_materialFromTexture[iTexture];
    if(nullptr == f.material)
    {
        std::pair<rendering::ShaderResourceName, rendering::IShaderResource const*> texture[] = { std::make_pair(m_textureName, iTexture), };
        rendering::Material* material = new rendering::Material(*m_material, rendering::Material::ConstantsView(nullptr), AsArrayView(texture), rendering::Material::SamplersView(nullptr));
        f.material = material;
    }
    f.frameIndex = 0x70D070D0; // TODO
    return f.material.get();
}
//=============================================================================
TextureDrawerDescriptor::TextureDrawerDescriptor()
    : m_drawer()
    , m_pixelShader()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TextureDrawerDescriptor::~TextureDrawerDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TextureDrawerDescriptor::CreateDrawer()
{
    refptr<rendering::VertexShaderDescriptor> vsDesc = new rendering::VertexShaderDescriptor(FilePath("src:/UserInterface/Shaders/V_UI_UniformDrawer.hlsl"), "vmain");
    rendering::VertexShaderProxy vs = vsDesc->GetProxy();
    rendering::ShaderInputLayoutProxy layout(ui::TextureDrawer::VertexDescriptor(), vs);
    rendering::PixelShaderProxy ps = m_pixelShader->GetProxy();
    rendering::RenderStateName blendState("Premultiplied Alpha Blending"); // TODO: expose blend state as a property
    refptr<rendering::Material> material = new rendering::Material(layout, vs, ps, blendState);
    m_drawer = new ui::TextureDrawer(material.get(), m_textureBindName);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, ui), TextureDrawerDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(pixelShader, "")
    REFLECTION_m_PROPERTY_DOC(textureBindName, "name of the texture in shader")
REFLECTION_CLASS_END
//=============================================================================
}
}

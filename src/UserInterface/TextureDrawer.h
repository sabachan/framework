#ifndef UserInterface_TextureDrawer_H
#define UserInterface_TextureDrawer_H

#include <Core/SmartPtr.h>
#include <Core/ArrayView.h>
#include <Math/Box.h>
#include <Math/Matrix.h>
#include <Rendering/Color.h>
#include <Rendering/ShaderResourceDatabase.h>
#include <Reflection/BaseClass.h>

struct D3D11_INPUT_ELEMENT_DESC;
namespace sg {
namespace rendering {
    class IShaderResource;
    class Material;
}
namespace renderengine {
    class PixelShaderDescriptor;
}
}

namespace sg {
namespace ui {
//=============================================================================
class DrawContext;
//=============================================================================
class TextureDrawer : public RefAndSafeCountable
{
public:
    static size_t VertexSize();
    static ArrayView<D3D11_INPUT_ELEMENT_DESC const> VertexDescriptor();
public:
    TextureDrawer(rendering::Material const* iMaterial, rendering::ShaderResourceName iTextureName);
    ~TextureDrawer();
    void UnregisterTexture(rendering::IShaderResource const* iTexture) const;

    void DrawTexture(DrawContext const& iContext,
                     rendering::IShaderResource const* iTexture,
                     box2f const& iTextureQuad,
                     Color4f const& col,
                     size_t layer = -1) const;
    void DrawTexture(DrawContext const& iContext,
                     rendering::IShaderResource const* iTexture,
                     box2f const& iTextureQuad,
                     box2f const& iClipQuad,
                     Color4f const& col,
                     size_t layer = -1) const;
    void DrawTexture(DrawContext const& iContext,
                     rendering::IShaderResource const* iTexture,
                     box2f const& iTextureQuad,
                     float2x3 const& iTransform2D,
                     box2f const& iClipQuad,
                     Color4f const& col,
                     size_t layer = -1) const;
private:
    void DrawTexture(DrawContext const& iContext,
                     rendering::IShaderResource const* iTexture,
                     box2f const& iQuad,
                     float2 const (&iUVs)[4],
                     Color4f const& col,
                     size_t layer) const;
    rendering::Material const* GetTextureMaterial(rendering::IShaderResource const* iTexture) const;
private:
    refptr<rendering::Material const> m_material;
    rendering::ShaderResourceName const m_textureName;
    struct MaterialAndUsage
    {
        refptr<rendering::Material> material;
        size_t frameIndex;
    };
    // TODO: This dico should be emptied once in a while to prevent java leak.
    // It should be great not to prevent the release of the IShaderResource,
    // however, that means we need a weak reference or something like that...
    // Note that the material also keeps a reference on resource.
    mutable std::unordered_map<refptr<rendering::IShaderResource const>, MaterialAndUsage> m_materialFromTexture;
};
//=============================================================================
class TextureDrawerDescriptor : public reflection::BaseClass
{
    REFLECTION_CLASS_HEADER(TextureDrawerDescriptor, reflection::BaseClass)
public:
    TextureDrawerDescriptor();
    virtual ~TextureDrawerDescriptor() override;
    TextureDrawer const* GetDrawer_AssumeAvailable() const { SG_ASSERT(nullptr != m_drawer); return m_drawer.get(); }
    TextureDrawer const* GetDrawer() { if(nullptr == m_drawer) { CreateDrawer(); } return m_drawer.get(); }
private:
    void CreateDrawer();
private:
    refptr<TextureDrawer> m_drawer;
    refptr<renderengine::PixelShaderDescriptor const> m_pixelShader;
    rendering::ShaderResourceName m_textureBindName;
};
//=============================================================================
}
}

#endif

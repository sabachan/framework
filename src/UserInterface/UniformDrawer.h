#ifndef UserInterface_UniformDrawer_H
#define UserInterface_UniformDrawer_H

#include <Core/SmartPtr.h>
#include <Core/ArrayView.h>
#include <Math/Box.h>
#include <Rendering/Color.h>
#include <Reflection/BaseClass.h>

struct D3D11_INPUT_ELEMENT_DESC;
namespace sg {
namespace rendering {
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
class UniformDrawer : public RefAndSafeCountable
{
public:
    static size_t VertexSize();
    static ArrayView<D3D11_INPUT_ELEMENT_DESC const> VertexDescriptor();
public:
    UniformDrawer(rendering::Material const* iMaterial);
    ~UniformDrawer();

    void DrawQuad(DrawContext const& iContext,
                  box2f const& iBox,
                  Color4f const& col,
                  size_t layer = -1) const;
    void DrawFrame(DrawContext const& iContext,
                   box2f const& iInBox,
                   box2f const& iOutBox,
                   Color4f const& col,
                   size_t layer = -1) const;
private:
    refptr<rendering::Material const> m_material;
};
//=============================================================================
class UniformDrawerDescriptor : public reflection::BaseClass
{
    REFLECTION_CLASS_HEADER(UniformDrawerDescriptor, reflection::BaseClass)
public:
    UniformDrawerDescriptor();
    virtual ~UniformDrawerDescriptor() override;
    UniformDrawer const* GetDrawer_AssumeAvailable() const { SG_ASSERT(nullptr != m_drawer); return m_drawer.get(); }
    UniformDrawer const* GetDrawer() { if(nullptr == m_drawer) { CreateDrawer(); } return m_drawer.get(); }
private:
    void CreateDrawer();
private:
    refptr<UniformDrawer> m_drawer;
    refptr<renderengine::PixelShaderDescriptor const> m_pixelShader;
};
//=============================================================================
}
}

#endif

#ifndef UserInterface_TextRenderer_H
#define UserInterface_TextRenderer_H

#include <vector>
#include <Core/SmartPtr.h>
#include <Core/ArrayView.h>
#include <Math/Box.h>
#include <Math/Vector.h>
#include "Context.h"

struct D3D11_INPUT_ELEMENT_DESC;

namespace sg {
namespace rendering {
    class Material;
}
}
namespace sg {
namespace ui {
//=============================================================================
class DrawContext;
class ITextModifier;
class ITypeface;
struct ParagraphStyle;
struct TextStyle;
//=============================================================================
class TextRenderer
{
public:
    class Cache
    {
        friend class TextRenderer;
        friend class TextRenderer_internal;
    public:
        Cache()
            : buffers()
            , bbox()
            , lastBaseline()
            , gridAligned(false)
        {}
        // The origin of the frame in which the box is expressed corresponds,
        // in y, to the first baseline and, in x, to a point that is aligned
        // as the text. For instance, for a text aligned right, the point (0,0)
        // will be at the end of the first baseline.
        // This seems to be the most understandable behaviour when using only
        // a point for positioning the text. However, some tools are available
        // on higher level to place a text relatively to a containing box.
        box2f const& BoundingBox() const { return bbox; }
        float LastBaseline() const { return lastBaseline; }
    private:
        struct Buffer
        {
        public:
            safeptr<rendering::Material> material;
            std::vector<u8> verticesData;
            std::vector<u16> indices;
        public:
            Buffer(rendering::Material* iMaterial)
                : material(iMaterial)
                , verticesData()
                , indices()
            {}
        };
        std::vector<Buffer> buffers;
        box2f bbox;
        float lastBaseline;
        bool gridAligned;
    };
public:
    static ArrayView<D3D11_INPUT_ELEMENT_DESC const> VertexDescriptor();
    static void Render(float2 const& iPos,
                       std::wstring const& iStr,
                       ITypeface const& iTypeface,
                       TextStyle const& iTextStyle,
                       ParagraphStyle const& iParagraphStyle,
                       ArrayView<ITextModifier const*const> iModifiers,
                       ui::DrawContext const& iContext,
                       size_t iLayer = -1);
    static void Prepare(TextRenderer::Cache& oRenderCache,
                        std::wstring const& iStr,
                        ITypeface const& iTypeface,
                        TextStyle const& iTextStyle,
                        ParagraphStyle const& iParagraphStyle,
                        ArrayView<ITextModifier const*const> iModifiers);
    static void Render(float2 const& iPos,
                       TextRenderer::Cache& iRenderCache,
                       ui::DrawContext const& iContext,
                       size_t iLayer = -1);
    static void Render_AssumeNoTransform(float2 const& iPos,
                                         TextRenderer::Cache& iRenderCache,
                                         ui::DrawContext const& iContext,
                                         size_t iLayer);
};
//=============================================================================
}
}

#endif

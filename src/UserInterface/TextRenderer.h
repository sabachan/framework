#ifndef UserInterface_TextRenderer_H
#define UserInterface_TextRenderer_H

#include "Context.h"
#include "Typeface.h"
#include <Math/Box.h>
#include <Math/Vector.h>
#include <Core/ArrayView.h>
#include <Core/SmartPtr.h>
#include <vector>

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
    struct PositionCache
    {
        friend class TextRenderer;
        friend class TextRenderer_internal;
    private:
        struct Glyph
        {
        public:
            float2 pen;
            // The font box represents the box that encloses the width of the
            // character and ascend and descend of the font.
            box2f fontBox;
            u8 flags;
            ubyte4 fillColor;
            ubyte4 strokeColor;
            GlyphInfo glyphInfo;
            size_t materialIndices[SG_ARRAYSIZE(ui::GlyphInfo().materials)];
        public:
            Glyph(float2 const& iPen, u8 iFlags)
                : pen(iPen)
                , flags(iFlags)
                , fillColor(uninitialized)
                , strokeColor(uninitialized)
                , glyphInfo(uninitialized)
            {}
        };
        std::vector<Glyph> glyphs;
        std::vector<std::pair<safeptr<rendering::Material>, size_t>> materialsAndCounts;
        box2f bbox;
        float lastBaseline;
        float2 lastPen;
        bool gridAligned;
    };
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
        bool Empty() const { return bbox.NVolume_NegativeIfNonConvex() <= 0.f; }
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
    static void Prepare(TextRenderer::PositionCache& oPositionCache,
                        TextRenderer::Cache& oRenderCache,
                        std::wstring const& iStr,
                        ITypeface const& iTypeface,
                        TextStyle const& iTextStyle,
                        ParagraphStyle const& iParagraphStyle,
                        ArrayView<ITextModifier const*const> iModifiers);
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
    static size_t GetGlyphCount(TextRenderer::PositionCache const& iPositionCache);
    static box2f GetGlyphBox(TextRenderer::PositionCache const& iPositionCache,size_t i);
    static float2 GetPenPosition(TextRenderer::PositionCache const& iPositionCache, size_t i);
    static void GetSelectionBoxes(TextRenderer::PositionCache const& iPositionCache, std::vector<box2f>& oBoxes, size_t b, size_t e);
    static size_t GetNearestGlyph(TextRenderer::PositionCache const& iPositionCache, float2 const& iPosition);
    static size_t GetNearestInterGlyph(TextRenderer::PositionCache const& iPositionCache, float2 const& iPosition);
};
//=============================================================================
}
}

#endif

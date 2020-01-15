#include "stdafx.h"

#include "TextRenderer.h"

#include "Context.h"
#include "TextModifier.h"
#include "TextStyle.h"
#include "Typeface.h"
#include <Rendering/TransientRenderBatch.h>
#include <Rendering/VertexTypes.h>
#include <RenderEngine/CompositingLayer.h>
#include <Core/For.h>
#include <Core/Utils.h>

namespace sg {
namespace ui {
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
typedef u32 index_t;
typedef rendering::Vertex_Pos3f_Tex2f_4Col4ub TextVertex;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
enum class CharacterFlag : u8
{
    NoRepresentation = 0x01,
    Expandable = 0x02,
    Breakable = 0x04,
    NewLine = 0x08,
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
u8 GetFlags(u32 iCharCode)
{
    struct CharAndFlags
    {
        u32 character;
        u8 flags;
    };
    CharAndFlags const begin[] =
    {
        { ' ' , u8(CharacterFlag::NoRepresentation) | u8(CharacterFlag::Expandable) | u8(CharacterFlag::Breakable) },
        { '\n' , u8(CharacterFlag::NoRepresentation) | u8(CharacterFlag::NewLine) },
        { '\r' , u8(CharacterFlag::NoRepresentation) | u8(CharacterFlag::Breakable) },
        { '\t' , u8(CharacterFlag::NoRepresentation) | u8(CharacterFlag::Expandable) | u8(CharacterFlag::Breakable) },
    };
    CharAndFlags const* end = begin + SG_ARRAYSIZE(begin);
    CharAndFlags const* f = std::find_if(begin, end, [=](CharAndFlags const& cf) { return iCharCode == cf.character; } );
    if(f != end)
        return f->flags;
    else
        return 0;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
size_t const MAX_STYLE_DEPTH = 8;
struct ComputeGlyphPositionsState
{
    ITypeface const& typeface;
    safeptr<TextStyle const> textStyle;
    safeptr<ui::IFont const> font;
    FontInfo fontInfo;
    box2f bbox;
    float2 pen;
    size_t lineExpendableCount;
    size_t lastExpendableEnd;
    size_t lastBreakableBegin;
    size_t lastBreakableEnd;
    size_t lineBegin;
    float lineAscent;
    float lineDescent;
    float lastBreakableAscent;
    float lastBreakableDescent;
    float prevLineBottom;
    char32_t prevCharCode;
    ITextModifier const*const* modifierIt;
    ITextModifier const*const* modifierEnd;
    size_t nextModifierPosition;
    MaxSizeVector<safeptr<TextStyle const>, MAX_STYLE_DEPTH> textStyleStack;
    MaxSizeVector<TextStyle, MAX_STYLE_DEPTH> modifiableTextStyles;
    int hidden = 0;
public:
    ComputeGlyphPositionsState(ITypeface const& iTypeface)
        : typeface(iTypeface)
    { }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
class TextRenderer_internal
{
public:

template <bool isLastLine>
static void ComputeGlyphPositions_NewLine(TextRenderer::PositionCache& positionsCache,
                                          ComputeGlyphPositionsState& state,
                                          ParagraphStyle const& paragraphStyle)
{
    bool const gridAligned = positionsCache.gridAligned;
    float const lineWidth = paragraphStyle.lineWidth;
    float const lineGap = gridAligned ? round(paragraphStyle.lineGap) : paragraphStyle.lineGap;
    float const usedLineWidth = std::max(0.f, state.pen.x() - state.fontInfo.interCharSpace);
    float const lineOffset = -usedLineWidth * paragraphStyle.alignment;
    state.lastBreakableAscent = std::max(state.lastBreakableAscent, state.fontInfo.ascent);
    state.lastBreakableDescent = std::max(state.lastBreakableDescent, state.fontInfo.descent);
    state.lineAscent = std::max(state.lineAscent, state.lastBreakableAscent);
    state.lineDescent = std::max(state.lineDescent, state.lastBreakableDescent);
    float const y = state.lineBegin == 0 ? 0 : state.prevLineBottom + state.lineAscent + lineGap;

    SG_ASSERT(!gridAligned || y == round(y));
    float2 const offsetPrecise(lineOffset, y);
    float2 const offset = gridAligned ? float2(round(offsetPrecise.x()), offsetPrecise.y()) : offsetPrecise;
    size_t const lineEnd = positionsCache.glyphs.size();
    for(size_t j = state.lineBegin; j < lineEnd; ++j)
    {
        positionsCache.glyphs[j].pen += offset;
    }

    if(isLastLine) { positionsCache.lastBaseline = y; positionsCache.lastPen = float2(state.pen.x(), y); }

    float2 const lineBeginPen = offset;
    // To understand next assert: in the case of text ending with new line, last line has no glyph in it.
    SG_ASSERT((isLastLine && state.lineBegin == positionsCache.glyphs.size()) || positionsCache.glyphs[state.lineBegin].pen == lineBeginPen);
    state.bbox.Grow(lineBeginPen + float2(0.f, -state.lineAscent));
    state.bbox.Grow(lineBeginPen + float2(usedLineWidth, state.lineDescent));

    if(!isLastLine)
    {
        state.pen = float2(0.f, 0.f);
        SG_ASSERT(!gridAligned || state.prevLineBottom == round(state.prevLineBottom));
        SG_ASSERT(!gridAligned || state.lineDescent == round(state.lineDescent));
        state.prevLineBottom = y + state.lineDescent;
        state.lineAscent = 0;
        state.lineDescent = 0;
        state.lastBreakableAscent = 0;
        state.lastBreakableDescent = 0;
        state.lineExpendableCount = 0;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
static void ComputeGlyphPositions_UpdateTextStyle(TextRenderer::PositionCache& positionsCache,
                                                  ComputeGlyphPositionsState& state,
                                                  TextStyle const* textStyle)
{
    SG_UNUSED(positionsCache);
    state.textStyle = textStyle;
    state.font = state.typeface.GetFont(*textStyle);
    state.font->GetFontInfo(state.fontInfo, *textStyle);
    SG_ASSERT(!positionsCache.gridAligned || state.fontInfo.ascent == round(state.fontInfo.ascent));
    SG_ASSERT(!positionsCache.gridAligned || state.fontInfo.descent == round(state.fontInfo.descent));
    state.lastBreakableAscent = std::max(state.lastBreakableAscent, state.fontInfo.ascent);
    state.lastBreakableDescent = std::max(state.lastBreakableDescent, state.fontInfo.descent);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
static void ComputeGlyphPositions_ApplyModifier(TextRenderer::PositionCache& positionsCache,
                                                ComputeGlyphPositionsState& state,
                                                size_t& pos,
                                                ParagraphStyle const& paragraphStyle,
                                                ITextModifier const& modifier)
{
    SG_UNUSED(paragraphStyle);
    switch(modifier.GetType())
    {
    case ITextModifier::Type::DynamicColor:
        SG_ASSERT_NOT_IMPLEMENTED();
        break;
    case ITextModifier::Type::Insert:
        SG_ASSERT_NOT_IMPLEMENTED();
        break;
    case ITextModifier::Type::Jump:
        {
            TextModifier_Jump const& m = checked_cast<TextModifier_Jump const&>(modifier);
            size_t const to = m.To();
            SG_ASSERT(pos <= to);
            pos = to;
        }
        break;
    case ITextModifier::Type::MovePen:
        SG_ASSERT_NOT_IMPLEMENTED();
        break;
    case ITextModifier::Type::RelativeStyleChange:
        SG_ASSERT_NOT_IMPLEMENTED();
        break;
    case ITextModifier::Type::StyleChange:
        {
            TextModifier_StyleChange const& m = checked_cast<TextModifier_StyleChange const&>(modifier);
            TextStyle const& textStyle = m.Style();
            ComputeGlyphPositions_UpdateTextStyle(positionsCache, state, &textStyle);
        }
        break;
    case ITextModifier::Type::StylePush:
        {
            SG_ASSERT_MSG(state.textStyleStack.size() < state.textStyleStack.capacity() / 2, "Capacity should be increased");
            state.textStyleStack.emplace_back(state.textStyle);
        }
        break;
    case ITextModifier::Type::StylePop:
        {
            ComputeGlyphPositions_UpdateTextStyle(positionsCache, state, state.textStyleStack.back().get());
            state.textStyleStack.pop_back();
        }
        break;
    case ITextModifier::Type::HidePush:
        {
            state.hidden += 1;
        }
        break;
    case ITextModifier::Type::HidePop:
        {
            state.hidden -= 1;
        }
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
static void ComputeGlyphPositions_ApplyModifiersIFN(TextRenderer::PositionCache& positionsCache,
                                                    ComputeGlyphPositionsState& state,
                                                    size_t& pos,
                                                    ParagraphStyle const& paragraphStyle)
{
    while(state.nextModifierPosition <= pos)
    {
        SG_ASSERT_MSG(state.nextModifierPosition == pos, "Modifiers must be in order and not masked by a jump.");
        ComputeGlyphPositions_ApplyModifier(positionsCache, state, pos, paragraphStyle, **state.modifierIt);
        ++state.modifierIt;
        state.nextModifierPosition = (state.modifierIt != state.modifierEnd) ? (*state.modifierIt)->Position() : -1;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
static void ComputeGlyphPositions(TextRenderer::PositionCache& oPositionsCache,
                                  std::wstring const& iStr,
                                  ITypeface const& iTypeface,
                                  TextStyle const& iTextStyle,
                                  ParagraphStyle const& iParagraphStyle,
                                  ArrayView<ITextModifier const*const> iModifiers)
{
    bool const gridAligned = iTypeface.GridAligned();

    size_t const len = iStr.length();
    float const lineWidth = iParagraphStyle.lineWidth;
    float const lineGap = gridAligned ? round(iParagraphStyle.lineGap) : iParagraphStyle.lineGap;

    TextRenderer::PositionCache& positionsCache = oPositionsCache;
    positionsCache.glyphs.clear();
    positionsCache.glyphs.reserve(len);
    positionsCache.materialsAndCounts.clear();
    positionsCache.gridAligned = gridAligned;

    std::unordered_map<safeptr<rendering::Material>, size_t> indexFromMaterial;
// TODO: Check that it optimizes also when font is more complicated (multiple materials).
#define USE_BUFFER_LIGHT_CACHE 1
#if USE_BUFFER_LIGHT_CACHE
    size_t lastMaterialIndexCache[SG_ARRAYSIZE(ui::GlyphInfo().materials)];
    void const* lastMaterialCache[SG_ARRAYSIZE(ui::GlyphInfo().materials)];
    for_range(size_t, m, 0, SG_ARRAYSIZE(ui::GlyphInfo().materials))
    {
        lastMaterialIndexCache[m] = all_ones;
        lastMaterialCache[m] = nullptr;
    }
#endif

    {
        ComputeGlyphPositionsState state(iTypeface);

        state.font = iTypeface.GetFont(iTextStyle);
        state.font->GetFontInfo(state.fontInfo, iTextStyle);
        SG_ASSERT(!gridAligned || state.fontInfo.ascent == round(state.fontInfo.ascent));
        SG_ASSERT(!gridAligned || state.fontInfo.descent == round(state.fontInfo.descent));

        state.pen = float2(0.f, 0.f);
        state.lineExpendableCount = 0;
        state.lastExpendableEnd = all_ones;
        state.lastBreakableBegin = all_ones;
        state.lastBreakableEnd = all_ones;
        state.lineBegin = 0;
        // NB: There is a question about do we need to use the first input
        // style ascent and descent even if no character is drawn with it.
        // As I decided that style modifications (even if replaced by another
        // before being applied to a visible character) would be visible, then
        // it is consistent to take input style into acount for at least first
        // line.
        state.lineAscent = state.fontInfo.ascent;
        state.lineDescent = state.fontInfo.descent;
        state.lastBreakableAscent = 0;
        state.lastBreakableDescent = 0;
        state.prevLineBottom = 0.f;
        state.prevCharCode = 0;
        state.textStyle = &iTextStyle;

        ITextModifier const*const* modifierBegin = iModifiers.begin();
        state.modifierIt = modifierBegin;
        state.modifierEnd = iModifiers.end();
        state.nextModifierPosition = (state.modifierIt != state.modifierEnd) ? (*state.modifierIt)->Position() : -1;
        state.textStyleStack;
        state.modifiableTextStyles;

        for(size_t i = 0; i < len; ++i)
        {
            ComputeGlyphPositions_ApplyModifiersIFN(positionsCache, state, i, iParagraphStyle);
            if(state.hidden > 0)
                continue;
            SG_ASSERT(i <= len);
            if(len == i)
                break;
            u32 const charCode = iStr[i];
            float const kerning = state.font->GetKerning(state.prevCharCode, charCode, *state.textStyle);
            float const advance = state.font->GetAdvance(charCode, *state.textStyle);
            SG_ASSERT(!gridAligned || kerning == round(kerning));
            SG_ASSERT(!gridAligned || advance == round(advance));
            u8 flags = GetFlags(charCode);
            if(0 == flags)
            {
                float const glyphWidth = advance - state.fontInfo.interCharSpace;
                if(lineWidth >= 0.f && -1 != state.lastBreakableBegin && state.pen.x() + kerning + glyphWidth > lineWidth)
                {
                    // treat line
                    float const usedLineWidth = positionsCache.glyphs[state.lastBreakableBegin].pen.x() - state.fontInfo.interCharSpace;
                    float const unusedWidth = lineWidth - usedLineWidth;
                    float const allSpaceGrow = unusedWidth * iParagraphStyle.justified;
                    float const oneSpaceGrow = allSpaceGrow / (state.lineExpendableCount-1);
                    float const lineOffset = (-usedLineWidth - allSpaceGrow) * iParagraphStyle.alignment;
                    SG_ASSERT(!gridAligned || state.lineAscent == round(state.lineAscent));
                    float const y = state.lineBegin == 0 ? 0 : state.prevLineBottom + state.lineAscent + lineGap;

                    {
                        SG_ASSERT(!gridAligned || y == round(y));
                        float2 offsetPrecise(lineOffset, y);
                        float2 offset = gridAligned ? float2(round(offsetPrecise.x()), offsetPrecise.y()) : offsetPrecise;
                        for(size_t j = state.lineBegin; j < state.lastBreakableBegin; ++j)
                        {
                            TextRenderer::PositionCache::Glyph& glyph = positionsCache.glyphs[j];
                            glyph.pen += offset;
                            if(glyph.flags & u8(CharacterFlag::Expandable))
                            {
                                offsetPrecise.x() += oneSpaceGrow;
                                offset.x() = gridAligned ? round(offsetPrecise.x()) : offsetPrecise.x();
                            }
                        }

                        state.bbox.Grow(positionsCache.glyphs[state.lineBegin].pen + float2(0.f, -state.lineAscent));
                        state.bbox.Grow(positionsCache.glyphs[state.lineBegin].pen + float2(usedLineWidth, state.lineDescent));

                        for(size_t j = state.lastBreakableBegin; j < state.lastBreakableEnd; ++j)
                        {
                            positionsCache.glyphs[j].pen = offset + float2(usedLineWidth, 0.f);
                        }
                    }

                    state.lineBegin = state.lastBreakableEnd;

                    size_t const glyphEnd = positionsCache.glyphs.size();
                    if(state.lineBegin < glyphEnd )
                    {
                        float offset = -positionsCache.glyphs[state.lineBegin].pen.x();
                        for(size_t j = state.lineBegin; j < glyphEnd; ++j)
                        {
                            positionsCache.glyphs[j].pen.x() += offset;
                        }
                        state.pen.x() += offset;
                        state.pen.x() += kerning;
                    }
                    else
                        state.pen = float2(0.f, 0.f);

                    SG_ASSERT(!gridAligned || state.prevLineBottom == round(state.prevLineBottom));
                    SG_ASSERT(!gridAligned || state.lineDescent == round(state.lineDescent));
                    state.prevLineBottom = y + state.lineDescent;
                    state.lineAscent = 0;
                    state.lineDescent = 0;
                    state.lineExpendableCount = 0;
                    state.lastBreakableBegin = all_ones;
                }
                else
                {
                    state.pen.x() += kerning;
                }
                positionsCache.glyphs.emplace_back(state.pen, flags);
                TextRenderer::PositionCache::Glyph& glyph = positionsCache.glyphs.back();
                glyph.fontBox = box2f::FromMinMax(float2(0.f, -state.fontInfo.ascent), float2(glyphWidth, state.fontInfo.descent));
                glyph.fillColor = ubyte4(state.textStyle->fillColor);
                glyph.strokeColor = ubyte4(state.textStyle->strokeColor);
                state.font->GetGlyphInfo(glyph.glyphInfo, charCode, *state.textStyle);
                for_range(size_t, m, 0, SG_ARRAYSIZE(glyph.glyphInfo.materials))
                {
                    rendering::Material* material = glyph.glyphInfo.materials[m].get();
                    if(nullptr == material)
                    {
                        glyph.materialIndices[m] = all_ones;
                        continue;
                    }
#if USE_BUFFER_LIGHT_CACHE
                    size_t materialIndex = lastMaterialIndexCache[m];
                    if(lastMaterialCache[m] != material)
                    {
#endif
                    auto r = indexFromMaterial.emplace(material, -1);
                    if(r.second)
                    {
                        SG_ASSERT(-1 == r.first->second);
                        r.first->second = positionsCache.materialsAndCounts.size();
                        positionsCache.materialsAndCounts.emplace_back(material, 0);
                    }
                    else
                    {
                        SG_ASSERT(-1 != r.first->second);
                    }
#if USE_BUFFER_LIGHT_CACHE
                    materialIndex = r.first->second;
                    lastMaterialIndexCache[m] = materialIndex;
                    lastMaterialCache[m] = material;
                    }
#else
                    size_t const materialIndex = r.first->second;
#endif
#undef USE_BUFFER_LIGHT_CACHE
                    SG_ASSERT(materialIndex < positionsCache.materialsAndCounts.size());
                    ++positionsCache.materialsAndCounts[materialIndex].second;
                    glyph.materialIndices[m] = materialIndex;
                }
                state.pen.x() += advance;
            }
            else if(flags & u8(CharacterFlag::NewLine))
            {
                SG_ASSERT(0.f == kerning);
                SG_ASSERT((flags & u8(CharacterFlag::Expandable)) == 0);
                SG_ASSERT((flags & u8(CharacterFlag::Breakable)) == 0);
                positionsCache.glyphs.emplace_back(state.pen, flags);
                TextRenderer::PositionCache::Glyph& glyph = positionsCache.glyphs.back();
                glyph.fontBox = box2f::FromMinMax(float2(0.f, -state.fontInfo.ascent), float2(0.f, state.fontInfo.descent));
                ComputeGlyphPositions_NewLine<false>(positionsCache, state, iParagraphStyle);
                state.lineBegin = positionsCache.glyphs.size();
            }
            else
            {
                if(flags & u8(CharacterFlag::Expandable))
                {
                    if(state.lastExpendableEnd != positionsCache.glyphs.size())
                    {
                        ++state.lineExpendableCount;
                    }
                    else
                    {
                        // If there are multiple contiguous spaces, it is better
                        // if they are considered one space when expended.
                        flags &= ~u8(CharacterFlag::Expandable);
                    }
                    state.lastExpendableEnd = positionsCache.glyphs.size()+1;
                }
                if(flags & u8(CharacterFlag::Breakable))
                {
                    SG_ASSERT(0.f == kerning); // difficult to handle if it breaks here
                    if(state.lastBreakableEnd != positionsCache.glyphs.size())
                    {
                        state.lastBreakableBegin = positionsCache.glyphs.size();
                    }
                    state.lastBreakableEnd = positionsCache.glyphs.size()+1;
                    // NB: We could have updated line ascent and descent at each character, but it would
                    // have cost a little bit more.
                    // Instead, we prefer to update it only at sapce, new line, and any style modification,
                    // even if the style is not used for one character.
                    state.lineAscent = std::max(state.lineAscent, state.lastBreakableAscent);
                    state.lineDescent = std::max(state.lineDescent, state.lastBreakableDescent);
                    state.lineAscent = std::max(state.lineAscent, state.fontInfo.ascent);
                    state.lineDescent = std::max(state.lineDescent, state.fontInfo.descent);
                    state.lastBreakableAscent = 0;
                    state.lastBreakableDescent = 0;
                }
                positionsCache.glyphs.emplace_back(state.pen, flags);
                TextRenderer::PositionCache::Glyph& glyph = positionsCache.glyphs.back();
                glyph.fontBox = box2f::FromMinMax(float2(0.f, -state.fontInfo.ascent), float2(advance, state.fontInfo.descent));
                state.pen.x() += advance;
            }

            state.prevCharCode = charCode;
        }

        size_t pos = len;
        ComputeGlyphPositions_ApplyModifiersIFN(positionsCache, state, pos, iParagraphStyle);
        SG_ASSERT(pos == len);
        ComputeGlyphPositions_NewLine<true>(positionsCache, state, iParagraphStyle);

        oPositionsCache.bbox = state.bbox;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
static void GenerateRenderCache(TextRenderer::Cache& oRenderCache,
                                TextRenderer::PositionCache const& iPositionsCache,
                                std::wstring const& iStr,
                                ITypeface const& iTypeface,
                                TextStyle const& iTextStyle,
                                ParagraphStyle const& iParagraphStyle,
                                ArrayView<ITextModifier const*const> iModifiers)
{
    SG_UNUSED((iStr, iTypeface, iTextStyle, iParagraphStyle, iModifiers));
    oRenderCache.bbox = iPositionsCache.bbox;
    oRenderCache.lastBaseline = iPositionsCache.lastBaseline;
    oRenderCache.gridAligned = iPositionsCache.gridAligned;

    auto const& positionsCacheCharacters = iPositionsCache.glyphs;

    size_t const materialCount = iPositionsCache.materialsAndCounts.size();
    for_range(size_t, i, 0, materialCount)
    {

        rendering::Material* material = iPositionsCache.materialsAndCounts[i].first.get();
        size_t const count = iPositionsCache.materialsAndCounts[i].second;
        oRenderCache.buffers.emplace_back(material);
        oRenderCache.buffers.back().verticesData.resize(4*count*sizeof(TextVertex));
        oRenderCache.buffers.back().indices.resize(6*count);
    }
    std::vector<size_t> vertexCounts(materialCount, 0);
    std::vector<size_t> indexCounts(materialCount, 0);

    size_t glyphCount = iPositionsCache.glyphs.size();
    for(size_t i = 0; i < glyphCount; ++i)
    {
        TextRenderer::PositionCache::Glyph const& glyph = iPositionsCache.glyphs[i];
        u8 const flags = glyph.flags;
        if(flags & u8(CharacterFlag::NoRepresentation))
            continue;
        ui::GlyphInfo const& glyphInfo = glyph.glyphInfo;
        ubyte4 const fillCol = glyph.fillColor;
        ubyte4 const strokeCol = glyph.strokeColor;
        ubyte4 const quadVertexData1 = glyphInfo.quadVertexData1;
        ubyte4 const quadVertexData2 = glyphInfo.quadVertexData2;
        box2f const renderbox = positionsCacheCharacters[i].pen + glyphInfo.renderbox;

        for_range(size_t, m, 0, SG_ARRAYSIZE(glyphInfo.materials))
        {
            size_t const materialIndex = glyph.materialIndices[m];
            if(-1 == materialIndex)
                continue;

            size_t const bufferIndex = materialIndex;
            TextRenderer::Cache::Buffer& buffer = oRenderCache.buffers[bufferIndex];
            u16 const firstIndex = checked_numcastable(vertexCounts[materialIndex]);
            vertexCounts[materialIndex] += 4;
            SG_ASSERT(vertexCounts[materialIndex] * sizeof(TextVertex) <= buffer.verticesData.size());
            TextVertex* vertices = reinterpret_cast<TextVertex*>(buffer.verticesData.data())+firstIndex;

            vertices[0].pos.xy() = renderbox.min;
            vertices[1].pos.xy() = float2(renderbox.max.x(), renderbox.min.y());
            vertices[2].pos.xy() = float2(renderbox.min.x(), renderbox.max.y());
            vertices[3].pos.xy() = renderbox.max;

            vertices[0].pos.z() = 0.f;
            vertices[1].pos.z() = 0.f;
            vertices[2].pos.z() = 0.f;
            vertices[3].pos.z() = 0.f;

            box2f const uvbox = glyphInfo.uvBox;
            vertices[0].tex = uvbox.min;
            vertices[1].tex = float2(uvbox.max.x(), uvbox.min.y());
            vertices[2].tex = float2(uvbox.min.x(), uvbox.max.y());
            vertices[3].tex = uvbox.max;

            vertices[0].col[0] = fillCol;
            vertices[1].col[0] = fillCol;
            vertices[2].col[0] = fillCol;
            vertices[3].col[0] = fillCol;

            vertices[0].col[1] = strokeCol;
            vertices[1].col[1] = strokeCol;
            vertices[2].col[1] = strokeCol;
            vertices[3].col[1] = strokeCol;

            vertices[0].col[2] = quadVertexData1;
            vertices[1].col[2] = quadVertexData1;
            vertices[2].col[2] = quadVertexData1;
            vertices[3].col[2] = quadVertexData1;

            vertices[0].col[3] = quadVertexData2;
            vertices[1].col[3] = quadVertexData2;
            vertices[2].col[3] = quadVertexData2;
            vertices[3].col[3] = quadVertexData2;

            size_t const indexCount = indexCounts[materialIndex];
            indexCounts[materialIndex] += 6;
            SG_ASSERT(indexCounts[materialIndex] <= buffer.indices.size());
            u16* indices = &buffer.indices[indexCount];
            indices[0] = firstIndex+0;
            indices[1] = firstIndex+2;
            indices[2] = firstIndex+1;
            indices[3] = firstIndex+1;
            indices[4] = firstIndex+2;
            indices[5] = firstIndex+3;
        }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SG_FORCE_INLINE static void WriteVertex(TextVertex& dst, TextVertex const& src, float2 const& pos)
{
#if 0
    dst.pos    = src.pos + pos.xy0();
    dst.col[0] = src.col[0];
    dst.col[1] = src.col[1];
    dst.col[2] = src.col[2];
    dst.col[3] = src.col[3];
    dst.tex    = src.tex;
#else // Perf
    dst.pos._[0] = src.pos._[0] + pos._[0];
    dst.pos._[1] = src.pos._[1] + pos._[1];
    dst.pos._[2] = src.pos._[2];
    memcpy(&dst.tex, &src.tex, sizeof(TextVertex) - SG_OFFSET_OF(TextVertex, tex));
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <Context::TransformType transformType>
SG_FORCE_INLINE static void WriteVertex(TextVertex& dst, TextVertex const& src, float4x4 const& transform)
{
    switch(transformType)
    {
    case Context::TransformType::None:
    case Context::TransformType::Translate2D:
        SG_ASSUME_NOT_REACHED();
        break;
    case Context::TransformType::Transform2D:
        dst.pos.xy() = (transform * src.pos.xy01()).xy();
        dst.pos._[2] = src.pos._[2];
        break;
    case Context::TransformType::Transform3D:
        SG_ASSERT_NOT_IMPLEMENTED();
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }
#if 0
    dst.col[0] = src.col[0];
    dst.col[1] = src.col[1];
    dst.col[2] = src.col[2];
    dst.col[3] = src.col[3];
    dst.tex    = src.tex;
#else // Perf
    memcpy(&dst.tex, &src.tex, sizeof(TextVertex) - SG_OFFSET_OF(TextVertex, tex));
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <Context::TransformType transformType>
static void Render_AssumeTransformType(float2 const& iPos,
                                       TextRenderer::Cache& iRenderCache,
                                       ui::DrawContext const& iContext,
                                       size_t iLayer)
{
    float2 const pos = iRenderCache.gridAligned ? round(iPos) : iPos;
    float4x4 const& transform = iContext.GetTransform();
    float4x4 transformAndPos = transform;
    float2 translateAndPos = pos;
    switch(transformType)
    {
    case Context::TransformType::None:
        SG_ASSERT(Context::TransformType::None == iContext.GetTransformType() ||
               Context::TransformType::Translate2D == iContext.GetTransformType());
        break;
    case Context::TransformType::Translate2D:
        SG_ASSUME_NOT_REACHED();
        break;
    case Context::TransformType::Transform2D:
        SG_ASSERT(Context::TransformType::Transform2D == iContext.GetTransformType());
        transformAndPos = transformAndPos * matrix::HomogeneousTranslation(pos.xy00());
        break;
    case Context::TransformType::Transform3D:
        SG_ASSERT(Context::TransformType::Transform3D == iContext.GetTransformType());
        SG_ASSERT_NOT_IMPLEMENTED();
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }

    size_t layer = iLayer;
    if(-1 == layer)
    {
        box2f const box = iRenderCache.bbox + pos;
        box2f const boxToLock = Context::TransformType::None == transformType ? box : iContext.TransformBoundingBox(box);
        ui::LayerRange const range = iContext.GetLayerManager()->Lock(boxToLock);
        layer = range[0];
    }

    for(auto const& buffer : iRenderCache.buffers)
    {
        rendering::Material* material = buffer.material.get();
        SG_ASSERT(nullptr != material);
        rendering::TransientRenderBatch::Properties properties;
        properties.vertexSize = sizeof(TextVertex);
        properties.indexSize = sizeof(index_t);
        rendering::TransientRenderBatch* renderBatch = iContext.GetCompositingLayer()->GetOrCreateTransientRenderBatch(
            *material,
            properties);

        size_t const dataSize = buffer.verticesData.size();
        SG_ASSERT(0 == dataSize % sizeof(TextVertex));
        size_t const vertexCount = dataSize / sizeof(TextVertex);
        size_t const indexCount = buffer.indices.size();
        rendering::TransientRenderBatch::WriteAccess<TextVertex> writeAccess(*renderBatch, vertexCount, indexCount, layer);
        TextVertex* vertices = writeAccess.GetVertexPointerForWriting();
        TextVertex const* src = reinterpret_cast<TextVertex const*>(buffer.verticesData.data());

        for_range(size_t, i, 0, vertexCount)
        {
            switch(transformType)
            {
            case Context::TransformType::None:
            case Context::TransformType::Translate2D:
                TextRenderer_internal::WriteVertex(vertices[i], src[i], translateAndPos);
                break;
            case Context::TransformType::Transform2D:
            case Context::TransformType::Transform3D:
                TextRenderer_internal::WriteVertex<transformType>(vertices[i], src[i], transformAndPos);
                break;
            default:
                SG_ASSUME_NOT_REACHED();
            }
        }

#if 0
        writeAccess.PushIndices_AssumeIndexSize<sizeof(index_t)>(buffer.indices.begin(), buffer.indices.end());
#else // For debug perf
        u16 const* srcBegin = buffer.indices.data();
        u16 const* srcEnd = srcBegin + buffer.indices.size();
        writeAccess.PushIndices_AssumeIndexSize<sizeof(index_t)>(srcBegin, srcEnd);
#endif
        writeAccess.FinishWritingVertex(vertexCount);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
};
//=============================================================================
ArrayView<D3D11_INPUT_ELEMENT_DESC const> TextRenderer::VertexDescriptor()
{
    return TextVertex::InputEltDesc();
}
//=============================================================================
void TextRenderer::Prepare(TextRenderer::PositionCache& oPositionCache,
                           TextRenderer::Cache& oRenderCache,
                           std::wstring const& iStr,
                           ITypeface const& iTypeface,
                           TextStyle const& iTextStyle,
                           ParagraphStyle const& iParagraphStyle,
                           ArrayView<ITextModifier const*const> iModifiers)
{
    oRenderCache.buffers.clear();
    oPositionCache.glyphs.clear();
    oPositionCache.materialsAndCounts.clear();

    TextRenderer_internal::ComputeGlyphPositions(oPositionCache, iStr, iTypeface, iTextStyle, iParagraphStyle, iModifiers);

    TextRenderer_internal::GenerateRenderCache(oRenderCache, oPositionCache, iStr, iTypeface, iTextStyle, iParagraphStyle, iModifiers);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TextRenderer::Prepare(TextRenderer::Cache& oRenderCache,
                           std::wstring const& iStr,
                           ITypeface const& iTypeface,
                           TextStyle const& iTextStyle,
                           ParagraphStyle const& iParagraphStyle,
                           ArrayView<ITextModifier const*const> iModifiers)
{
    TextRenderer::PositionCache positionsCache;
    TextRenderer::Prepare(positionsCache, oRenderCache, iStr, iTypeface, iTextStyle, iParagraphStyle, iModifiers);
}
//=============================================================================
void TextRenderer::Render_AssumeNoTransform(float2 const& iPos,
                                     TextRenderer::Cache& iRenderCache,
                                     ui::DrawContext const& iContext,
                                     size_t iLayer)
{
    TextRenderer_internal::Render_AssumeTransformType<Context::TransformType::None>(iPos, iRenderCache, iContext, iLayer);

}
//=============================================================================
void TextRenderer::Render(float2 const& iPos,
                          TextRenderer::Cache& iRenderCache,
                          ui::DrawContext const& iContext,
                          size_t iLayer)
{
#if 1
    if(iRenderCache.Empty())
        return;
    Context::TransformType const transformType = iContext.GetTransformType();
    switch(transformType)
    {
    case Context::TransformType::None:
        TextRenderer_internal::Render_AssumeTransformType<Context::TransformType::None>(iPos, iRenderCache, iContext, iLayer);
        break;
    case Context::TransformType::Translate2D:
        {
            float2 const T = iContext.GetTransform().Col(3).xy();
            float2 const pos = iPos + T;
            TextRenderer_internal::Render_AssumeTransformType<Context::TransformType::None>(pos, iRenderCache, iContext, iLayer);
        }
        break;
    case Context::TransformType::Transform2D:
        TextRenderer_internal::Render_AssumeTransformType<Context::TransformType::Transform2D>(iPos, iRenderCache, iContext, iLayer);
        break;
    case Context::TransformType::Transform3D:
        TextRenderer_internal::Render_AssumeTransformType<Context::TransformType::Transform3D>(iPos, iRenderCache, iContext, iLayer);
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }
#elif 1
    TextRenderer_internal::Render_AssumeTransformType<Context::TransformType::None>(iPos, iRenderCache, iContext, iLayer);
#else
    float2 const pos = iRenderCache.gridAligned ? round(iPos) : iPos;

    size_t layer = iLayer;
    if(-1 == layer)
    {
        box2f const box = iRenderCache.bbox + pos;
        ui::LayerRange const range = iContext.GetLayerManager()->Lock(box);
        layer = range[0];
    }

    for(auto const& buffer : iRenderCache.buffers)
    {
        rendering::Material* material = buffer.material.get();
        SG_ASSERT(nullptr != material);
        rendering::MultiLayerTransientRenderBatch::Properties properties;
        properties.vertexSize = sizeof(TextVertex);
        properties.indexSize = sizeof(index_t);
        rendering::MultiLayerTransientRenderBatch* renderBatch = iContext.GetCompositingLayer()->GetOrCreateMultiLayerBatch(
            *material,
            properties);

        size_t const dataSize = buffer.verticesData.size();
        SG_ASSERT(0 == dataSize % sizeof(TextVertex));
        size_t const vertexCount = dataSize / sizeof(TextVertex);
        size_t const indexCount = buffer.indices.size();
        rendering::MultiLayerTransientRenderBatch::WriteAccess<TextVertex> writeAccess(*renderBatch, vertexCount, indexCount, layer);
        TextVertex* vertices = writeAccess.GetVertexPointerForWriting();
        TextVertex const* src = reinterpret_cast<TextVertex const*>(buffer.verticesData.data());

        for_range(size_t, i, 0, vertexCount)
        {
            TextRenderer_internal::WriteVertex(vertices[i], src[i], pos);
        }

#if 0
        writeAccess.PushIndices_AssumeIndexSize<sizeof(index_t)>(buffer.indices.begin(), buffer.indices.end());
#else // For debug perf
        u16 const* srcBegin = buffer.indices.data();
        u16 const* srcEnd = srcBegin + buffer.indices.size();
        writeAccess.PushIndices_AssumeIndexSize<sizeof(index_t)>(srcBegin, srcEnd);
#endif
        writeAccess.FinishWritingVertex(vertexCount);
    }
#endif
}
//=============================================================================
void TextRenderer::Render(float2 const& iPos,
                          std::wstring const& iStr,
                          ITypeface const& iTypeface,
                          TextStyle const& iTextStyle,
                          ParagraphStyle const& iParagraphStyle,
                          ArrayView<ITextModifier const*const> iModifiers,
                          ui::DrawContext const& iContext,
                          size_t iLayer)
{
    TextRenderer::Cache cache;
    Prepare(cache, iStr, iTypeface, iTextStyle, iParagraphStyle, iModifiers);
    Render(iPos, cache, iContext, iLayer);
}
//=============================================================================
size_t TextRenderer::GetGlyphCount(TextRenderer::PositionCache const& iPositionCache)
{
    auto const& glyphs = iPositionCache.glyphs;
    size_t const glyphCount = glyphs.size();
    return glyphCount;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
box2f TextRenderer::GetGlyphBox(TextRenderer::PositionCache const& iPositionCache, size_t i)
{
    auto const& glyphs = iPositionCache.glyphs;
    size_t const glyphCount = glyphs.size();
    SG_ASSERT(i < glyphCount);
    if(i >= glyphCount)
        return box2f();
    box2f const box = glyphs[i].pen + glyphs[i].fontBox;
    return box;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float2 TextRenderer::GetPenPosition(TextRenderer::PositionCache const& iPositionCache, size_t i)
{
    auto const& glyphs = iPositionCache.glyphs;
    size_t const glyphCount = glyphs.size();
    SG_ASSERT(i <= glyphCount);
    if(i >= glyphCount)
        return iPositionCache.lastPen;
    return  glyphs[i].pen;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TextRenderer::GetSelectionBoxes(TextRenderer::PositionCache const& iPositionCache, std::vector<box2f>& oBoxes, size_t b, size_t e)
{
    SG_ASSERT(oBoxes.empty());
    auto const& glyphs = iPositionCache.glyphs;
    size_t const glyphCount = glyphs.size();
    SG_ASSERT(b <= glyphCount);
    SG_ASSERT(e <= glyphCount);
    if(b >= glyphCount)
        return;
    oBoxes.emplace_back(glyphs[b].pen + glyphs[b].fontBox);
    for_range(size_t, i, b+1, e)
    {
        if(i >= glyphCount)
            break;
        box2f const glyphBox = glyphs[i].pen + glyphs[i].fontBox;
        box2f const& lineBox = oBoxes.back();
        bool const isOnLine = lineBox.SubBox<1>(1).Contains(glyphBox.Center().y());
        if(isOnLine)
            oBoxes.back().Grow(glyphBox);
        else
            oBoxes.push_back(glyphBox);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
size_t TextRenderer::GetNearestGlyph(TextRenderer::PositionCache const& iPositionCache, float2 const& iPosition)
{
    auto const& glyphs = iPositionCache.glyphs;
    size_t const glyphCount = glyphs.size();
    float2 bestd = float2(FLT_MAX);
    float2 bestabsd = float2(FLT_MAX);
    size_t bestIndex = all_ones;
    for_range(size_t, i, 0, glyphCount)
    {
        auto const& glyph = glyphs[i];
        float2 d = glyph.fontBox.VectorToPoint(iPosition - glyph.pen);
        float2 const absd = componentwise::abs(d);
        if(absd.y() < bestabsd.y())
        {
            bestd = d;
            bestabsd = absd;
            bestIndex = i;
        }
        else if(absd.y() == bestabsd.y() && absd.x() <= bestabsd.x())
        {
            bestd = d;
            bestabsd = absd;
            bestIndex = i;
        }
    }
    return bestIndex;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
size_t TextRenderer::GetNearestInterGlyph(TextRenderer::PositionCache const& iPositionCache, float2 const& iPosition)
{
    auto const& glyphs = iPositionCache.glyphs;
    size_t const glyphCount = glyphs.size();
    if(0 == glyphCount)
        return 0;
    size_t const nearestGlyphIndex = GetNearestGlyph(iPositionCache, iPosition);
    auto const& glyph = glyphs[nearestGlyphIndex];

    float2 const d = glyph.fontBox.VectorToPoint(iPosition - glyph.pen);
    float2 const absd = componentwise::abs(d);
    float2 const dend = iPosition - iPositionCache.lastPen;
    float2 const absdend = componentwise::abs(dend);
    if(absdend.y() < absd.y())
        return glyphCount;

    float dcenter = iPosition.x() - glyph.pen.x() - glyph.fontBox.Center().x();
    if(dcenter >= 0.f)
    {
        if(nearestGlyphIndex+1 < glyphCount)
        {
            // In the case the next character is in another line, then it is
            // preferable to put the cursor at the end of the line than at the
            // begining of the next one.
            auto const& nextGlyph = glyphs[nearestGlyphIndex+1];
            float2 nextd = nextGlyph.fontBox.VectorToPoint(iPosition - nextGlyph.pen);
            if(nextd.x() > d.x())
                return nearestGlyphIndex;
            else
                return nearestGlyphIndex+1;
        }
        else
        {
            if(dend.x() > d.x())
                return nearestGlyphIndex;
            else
                return nearestGlyphIndex+1;
        }
    }
    return nearestGlyphIndex;
}
//=============================================================================
}
}

#ifndef UserInterface_Text_H
#define UserInterface_Text_H

#include "TextRenderer.h"
#include <Core/SmartPtr.h>
#include <Math/Box.h>
#include <Math/Vector.h>
#include <string>

namespace sg {
namespace ui {
//=============================================================================
struct ParagraphStyle;
struct TextStyle;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class DrawContext;
class ITypeface;
class TextFormatScript;
//=============================================================================
struct TextBoxAlignment
{
    float2 alignementToAnchor;
    float useTextAlignmentInX;
    float useBaselinesInY;
};
//=============================================================================
float2 ComputeTextBoxOffset(TextRenderer::Cache const& iTextCache, TextBoxAlignment const& iAlignment);
//=============================================================================
class Text
{
public:
    typedef TextBoxAlignment Alignment;
    typedef std::wstring lstring; // TODO: implement lstring, localized string
    Text();
    Text(lstring const& iText, ITypeface const* iTypeface, TextStyle const* iTextStyle, ParagraphStyle const* iParagraphStyle);
    Text(lstring const& iText, ITypeface const* iTypeface, TextFormatScript const* iTFS, ParagraphStyle const* iParagraphStyle);
    Text(lstring const& iText, ITypeface const* iTypeface, TextStyle const* iTextStyle, TextFormatScript const* iTFS, ParagraphStyle const* iParagraphStyle);
    ~Text() {}
    void SetParagraphStyle(ParagraphStyle const* iParagraphStyle);
    void SetStyles(ITypeface const* iTypeface, TextStyle const* iTextStyle, ParagraphStyle const* iParagraphStyle);
    void SetStyles(ITypeface const* iTypeface, TextFormatScript const* iTFS, ParagraphStyle const* iParagraphStyle);
    void SetStyles(ITypeface const* iTypeface, TextStyle const* iTextStyle, TextFormatScript const* iTFS, ParagraphStyle const* iParagraphStyle);
    void SetText(lstring const& iText);

    ITypeface const* GetTypeface() const { return m_typeface.get(); }
    TextStyle const* GetTextStyle() const { return m_textStyle.get(); }
    TextFormatScript const* GetTFS() const { return m_tfs.get(); } 
    ParagraphStyle const* GetParagraphStyle() const { return m_paragraphStyle.get(); }

    box2f Box();
    void Draw(ui::DrawContext const& iContext, float2 const& iPosition, size_t iLayer = -1);
    void Draw(ui::DrawContext const& iContext, float2 const& iPosition, TextBoxAlignment const& iAlignment, size_t iLayer = -1);
    float2 ComputeOffset(TextBoxAlignment const& iAlignment);

    size_t GetGlyphCount();
    box2f GetGlyphBox(size_t i);
    float2 GetPenPosition(size_t i);
    size_t GetNearestGlyph(float2 const& iPosition);
    size_t GetNearestInterGlyph(float2 const& iPosition);
    void GetSelectionBoxes(std::vector<box2f>& oBoxes, size_t b, size_t e);
private:
    void SetStyles_Impl(ITypeface const* iTypeface, TextStyle const* iTextStyle, TextFormatScript const* iTFS, ParagraphStyle const* iParagraphStyle);
    void PrepareIFN() { if(!m_isUpToDate) Prepare(); SG_ASSERT(m_isUpToDate); }
    void Prepare();
private:
    lstring m_text;
    safeptr<TextFormatScript const> m_tfs;
    safeptr<TextStyle const> m_textStyle;
    safeptr<ParagraphStyle const> m_paragraphStyle;
    safeptr<ITypeface const> m_typeface;
    ui::TextRenderer::PositionCache m_positionCache;
    ui::TextRenderer::Cache m_textCache;
    bool m_isUpToDate;
};
//=============================================================================
}
}

#endif

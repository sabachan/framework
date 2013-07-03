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
class Text
{
public:
    struct Alignment
    {
        float2 alignementToAnchor;
        float useTextAlignmentInX;
        float useBaselinesInY;
    };
public:
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

    box2f Box();
    void Draw(ui::DrawContext const& iContext, float2 const& iPosition, size_t iLayer = -1);
    void Draw(ui::DrawContext const& iContext, float2 const& iPosition, Alignment const& iAlignment, size_t iLayer = -1);
    float2 ComputeOffset(Alignment const& iAlignment);
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
    ui::TextRenderer::Cache m_textCache;
    bool m_isUpToDate;
};
//=============================================================================
}
}

#endif

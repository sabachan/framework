#ifndef UserInterface_GenericStyleGuide_H
#define UserInterface_GenericStyleGuide_H

#include "CirclePathDrawer.h"
#include "Length.h"
#include "UniformDrawer.h"
#include "TextureDrawer.h"
#include "TextFormatScript.h"
#include "Typeface.h"
#include <Reflection/BaseClass.h>
#include <Rendering/Color.h>

namespace sg {
namespace ui {
//=============================================================================
class ITypeface;
//=============================================================================
class GenericStyleGuide : public reflection::BaseClass
{
    REFLECTION_CLASS_HEADER(GenericStyleGuide, reflection::BaseClass)
    typedef reflection::BaseClass reflection_parent_type;
public:
    GenericStyleGuide();
    ~GenericStyleGuide();
    // The R8G8B8A8 is assumed to be sRGB with no alpha pre-multiplication:
    //     (r^gamma, g^gamma, b^gamma, a)
    // This choice is made in order to ease the writing of colors as alpha
    // premultiplication should be made before gamma:
    //     (r, g, b, a) -> ((r*a)^gamma, (g*a)^gamma, (b*a)^gamma, a)
    R8G8B8A8 GetColor(FastSymbol iName) const { auto r = m_colors.find(iName); SG_ASSERT(r != m_colors.end()); return r->second; }
    // The linear colors are returned with alpha pre-multiplication:
    //     (r*a, g*a, b*a, a)
    Color4f GetLinearColor(FastSymbol iName) const { auto r = m_genLinearColors.find(iName); SG_ASSERT(r != m_genLinearColors.end()); return r->second; }
    Length GetLength(FastSymbol iName) const { auto r = m_lengths.find(iName); SG_ASSERT(r != m_lengths.end()); return r->second; }
    Length2 GetVector(FastSymbol iName) const { auto r = m_vectors.find(iName); SG_ASSERT(r != m_vectors.end()); return r->second; }
    UniformDrawer const* GetUniformDrawer(FastSymbol iName) const { auto r = m_uniformDrawers.find(iName); SG_ASSERT(r != m_uniformDrawers.end()); return r->second->GetDrawer_AssumeAvailable(); }
    TextureDrawer const* GetTextureDrawer(FastSymbol iName) const { auto r = m_textureDrawers.find(iName); SG_ASSERT(r != m_textureDrawers.end()); return r->second->GetDrawer_AssumeAvailable(); }
    CirclePathDrawer const* GetCirclePathDrawer(FastSymbol iName) const { auto r = m_circlePathDrawers.find(iName); SG_ASSERT(r != m_circlePathDrawers.end()); return r->second->GetDrawer_AssumeAvailable(); }
    TextFormatScript const* GetTFS(FastSymbol iName) const { auto r = m_tfs.find(iName); SG_ASSERT(r != m_tfs.end()); return r->second.get(); }
    TextStyle const& GetTextStyle(FastSymbol iName) const { auto r = m_textStyles.find(iName); SG_ASSERT(r != m_textStyles.end()); return r->second; }
    ParagraphStyle const& GetParagraphStyle(FastSymbol iName) const { auto r = m_paragraphStyles.find(iName); SG_ASSERT(r != m_paragraphStyles.end()); return r->second; }
    ITypeface const* GetTypeface(FastSymbol iName) const { auto r = m_typefaces.find(iName); SG_ASSERT(r != m_typefaces.end()); return r->second.get(); }
protected:
    virtual void VirtualOnCreated(reflection::ObjectCreationContext& iContext) override
    {
        reflection_parent_type::VirtualOnCreated(iContext);
        OnCreatedOrModified();
    }
    virtual void VirtualOnModified(reflection::ObjectModificationContext& iContext) override
    {
        reflection_parent_type::VirtualOnModified(iContext);
        OnCreatedOrModified();
    }
private:
    void OnCreatedOrModified();
private:
    std::unordered_map<FastSymbol, R8G8B8A8> m_colors;
    std::unordered_map<FastSymbol, Color4f> m_genLinearColors;
    std::unordered_map<FastSymbol, ui::Length> m_lengths;
    std::unordered_map<FastSymbol, Length2> m_vectors;
    std::unordered_map<FastSymbol, refptr<UniformDrawerDescriptor>> m_uniformDrawers;
    std::unordered_map<FastSymbol, refptr<TextureDrawerDescriptor>> m_textureDrawers;
    std::unordered_map<FastSymbol, refptr<CirclePathDrawerDescriptor>> m_circlePathDrawers;
    std::unordered_map<FastSymbol, refptr<TextFormatScript>> m_tfs;
    std::unordered_map<FastSymbol, TextStyle> m_textStyles;
    std::unordered_map<FastSymbol, ParagraphStyle> m_paragraphStyles;
    std::unordered_map<FastSymbol, refptr<ITypeface>> m_typefaces;
};
//=============================================================================
}
}

#endif

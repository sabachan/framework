#include "stdafx.h"

#include "ClippingContainer.h"

#include "Context.h"
#include "LayerManager.h"
#include "PointerEvent.h"
#include "TextureDrawer.h"
#include <Core/Log.h>
#include <Reflection/ObjectDatabase.h>
#include <Rendering/RenderDevice.h>
#include <Rendering/Surface.h>
#include <RenderEngine/Compositing.h>
#include <RenderEngine/CompositingLayer.h>
#include <ObjectScript/Reader.h>

namespace sg {
namespace ui {
//=============================================================================
ClippingContainer::ClippingContainer(ui::TextureDrawer const* iTextureDrawer)
: parent_type()
, m_textureDrawer(iTextureDrawer)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ClippingContainer::~ClippingContainer()
{
    rendering::IShaderResource const* offscreenTexture = GetOffscreenTexture();
    m_textureDrawer->UnregisterTexture(offscreenTexture);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ClippingContainer::VirtualOnDraw(DrawContext const& iContext)
{
    parent_type::VirtualOnDraw(iContext);
    box2f const& box = PlacementBox_AssumeUpToDate();
    if(box.NVolume_NegativeIfNonConvex() > 0)
    {
        rendering::IShaderResource const* offscreenTexture = GetOffscreenTexture();
        m_textureDrawer->DrawTexture(iContext, offscreenTexture, box, Color4f(1,1,1,1));
    }
}
//=============================================================================
}
}

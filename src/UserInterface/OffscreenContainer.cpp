#include "stdafx.h"

#include "OffscreenContainer.h"

#include "Area.h"
#include "Context.h"
#include "LayerManager.h"
#include "PointerEventClipper.h"
#include <Core/Log.h>
#include <Reflection/ObjectDatabase.h>
#include <Rendering/RenderDevice.h>
#include <Rendering/Surface.h>
#include <RenderEngine/Compositing.h>
#include <RenderEngine/CompositingLayer.h>
#include <ObjectScript/Reader.h>

// Implementation notes:
// In order for the render commands to be created, the compositing must be
// executed. Either it is done (1) by the Offscreen Container itself, or
// (2) it can be postponed to when the "parent" compositing is executed, or
// (3) just before the layer in the RenderBatch is executed.
// implementation of (1) is easy. Is there any problem ?
// implementation of (2) should be easy, as long as we can register to be
// executed before the compositing, or even one of its instructions. This
// enables the control of when the temporary surface is in use, allowing
// a potential reuse.
// implemetation of (3) would allow the reuse of temporary surface by other
// Offscreen containers, as long as they are not nested. To do that, the
// compositing can be register in the CompositingLayer behind a IRenderBatch
// interface. Also, care must be taken if OffscreenContainers are rendered
// in the same layer.

namespace sg {
namespace ui {
//=============================================================================
OffscreenCommon::OffscreenCommon()
    : name_viewport_resolution("viewport_resolution")
    , m_compositingDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg,ui), OffscreenCommon)
    REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(compositingDescriptor, "")
REFLECTION_CLASS_END
//=============================================================================
namespace {
    uint2 const MIN_OFFSCREEN_SURFACE_WIDTH_HEIGHT = uint2(8,8);
}
//=============================================================================
OffscreenContainer::OffscreenContainer(rendering::SurfaceProperties const* iOptionnalProperties)
: parent_type()
, m_resolution(MIN_OFFSCREEN_SURFACE_WIDTH_HEIGHT)
, m_shaderConstantDatabase()
, m_surface()
, m_compositing()
, m_layer()
, m_layerManager(new LayerManager)
, m_mayNeedUpdateResolution(false)
{
    OffscreenCommon* common = OffscreenCommon::GetIFP();
    SG_ASSERT(nullptr != common);
    rendering::RenderDevice* renderDevice = rendering::RenderDevice::GetIFP();
    SG_ASSERT(nullptr != renderDevice);

    // TODO: How not to create this database, but use the same as main UI?
    m_shaderConstantDatabase = new rendering::ShaderConstantDatabase;
    m_shaderConstantDatabase->AddVariable(common->name_viewport_resolution, new rendering::ShaderVariable<float2>());

    rendering::SurfaceProperties prop;
    if(nullptr != iOptionnalProperties)
    {
        SG_ASSERT(1 == iOptionnalProperties->mipLevels);
        prop = *iOptionnalProperties;
    }
    else
    {
        prop.baseFormat = rendering::SurfaceFormat::R8G8B8A8_TYPELESS;
        prop.readFormat = rendering::SurfaceFormat::R8G8B8A8_UNORM_SRGB;
        prop.writeFormat = rendering::SurfaceFormat::R8G8B8A8_UNORM_SRGB;
        prop.mipLevels = 1;
    }
    m_surface = new rendering::Surface(renderDevice, &m_resolution, &prop);
    rendering::IRenderTarget* renderTargets[] = { m_surface.get() };

    renderengine::CompositingDescriptor const* compositingDesc = common->GetCompositingDescriptor();
    ArrayView<rendering::IShaderResource*> const inputSurfaces;
    ArrayView<rendering::IRenderTarget*> const outputSurfaces = AsArrayView(renderTargets);
    rendering::IShaderConstantDatabase const* constantDatabases_data[] = { m_shaderConstantDatabase.get() };
    ArrayView<rendering::IShaderConstantDatabase const*> const constantDatabases = AsArrayView(constantDatabases_data);
    ArrayView<rendering::IShaderResourceDatabase const*> const resourceDatabases;
    refptr<renderengine::ICompositing> compositing = compositingDesc->CreateInstance(renderDevice, inputSurfaces, outputSurfaces, constantDatabases, resourceDatabases);
    m_compositing = compositing;

    renderengine::CompositingLayer* layer = compositing->GetLayer(FastSymbol("Draw"), FastSymbol("gui2D"));
    SG_ASSERT(nullptr != layer);
    m_layer = layer;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
OffscreenContainer::~OffscreenContainer()
{
    m_surface->Terminate();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void OffscreenContainer::VirtualOnDraw(DrawContext const& iContext)
{
    SG_UNUSED(iContext);
    if(m_mayNeedUpdateResolution)
        UpdateResolutionIFN();
    box2f const& box = PlacementBox_AssumeUpToDate();
    SG_ASSERT(m_resolution.Get() == uint2(componentwise::max(ceili(box.Delta()), int2(MIN_OFFSCREEN_SURFACE_WIDTH_HEIGHT))));
    SG_ASSERT(iContext.GetTransformType() == Context::TransformType::None
           || iContext.GetTransformType() == Context::TransformType::Translate2D); // else, not impl (can be done better than naive impl).
    DrawContext preContext(*m_layerManager, m_layer.get(), box2f::FromMinDelta(float2(0,0), float2(m_resolution.Get())));
    DrawContext context(preContext, matrix::HomogeneousTranslation(-box.min.xy0()), matrix::HomogeneousTranslation(box.min.xy0()), Context::TransformType::Translate2D);
    parent_type::VirtualOnDraw(context);
    m_layerManager->Clear();
    m_compositing->Execute(FastSymbol("Draw"));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void OffscreenContainer::VirtualOnPointerEvent(PointerEventContext const& iContext, PointerEvent const& iPointerEvent)
{
    if(m_mayNeedUpdateResolution)
        UpdateResolutionIFN();
    box2f const& box = PlacementBox_AssumeUpToDate();
    SG_ASSERT(m_resolution.Get() == uint2(componentwise::max(ceili(box.Delta()), int2(MIN_OFFSCREEN_SURFACE_WIDTH_HEIGHT))));
    BoxArea area(box);
    PointerEventClipper clipper(iContext, iPointerEvent, area);
    parent_type::VirtualOnPointerEvent(iContext, iPointerEvent);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void OffscreenContainer::UpdateResolutionIFN()
{
    SG_ASSERT(m_mayNeedUpdateResolution);
    box2f const& box = PlacementBox_AssumeUpToDate();
    uint2 res = uint2(componentwise::max(ceili(box.Delta()), int2(MIN_OFFSCREEN_SURFACE_WIDTH_HEIGHT)));
    if(res != m_resolution.Get())
    {
        m_resolution.Set(res);

        OffscreenCommon* common = OffscreenCommon::GetIFP();
        SG_ASSERT(common);
        rendering::IShaderVariable* ivar = m_shaderConstantDatabase->GetConstantForWriting(common->name_viewport_resolution);
        rendering::ShaderVariable<float2>* var = checked_cast<rendering::ShaderVariable<float2>*>(ivar);
        var->Set(float2(res));
    }
    m_mayNeedUpdateResolution = false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void OffscreenContainer::VirtualInvalidateChildrenPlacement()
{
    // VirtualInvalidateChildrenPlacement() est appelée par Component sur un
    // SetPlacementBox(). Placer ce code ici permet qu'il soit appelé lorsqu'il
    // le faut, à la différence de VirtualUpatePlacement qui peut ne pas être
    // appelé.
    m_mayNeedUpdateResolution = true;

    parent_type::VirtualInvalidateChildrenPlacement();
}
//=============================================================================
}
}

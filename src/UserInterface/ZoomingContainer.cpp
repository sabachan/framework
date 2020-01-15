#include "stdafx.h"

#include "ZoomingContainer.h"

#include "Context.h"
#include "PointerEvent.h"
#include "Movable.h"
#include "WheelHelper.h"
#include <Math/Vector.h>
#include <Math/Matrix.h>
#include <System/MouseUtils.h>
#include <System/UserInputEvent.h>

#if SG_ENABLE_ASSERT
#include "RootContainer.h"
#endif

namespace sg {
namespace ui {
//=============================================================================
ZoomingContainer::ZoomingContainer(Properties const& iProperties)
    : m_properties(iProperties)
{
    m_subContainer = new SubContainer();
    RequestAddToFront(m_subContainer.get());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ZoomingContainer::ZoomingContainer()
    : ZoomingContainer(Properties())
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ZoomingContainer::SetContent(Component* iContent)
{
    SG_ASSERT(nullptr == m_content || nullptr == iContent);
    if(nullptr != m_content)
    {
        Component* c = m_content.get();
        m_content = nullptr;
        m_subContainer->RequestRemove(c);
    }
    if(nullptr != iContent)
    {
        m_content = iContent;
#if SG_ENABLE_ASSERT
        refptr<ui::Component> stayAlive = iContent;
        CheckConstraintsOnItem(iContent);
#endif
        m_subContainer->RequestAddToFront(iContent);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ZoomingContainer::OnWheelEvent(SG_UI_WHEEL_LISTENER_PARAMETERS)
{
    SG_UI_WHEEL_LISTENER_UNUSED_PARAMETERS;
    box2f const box = PlacementBox_AssumeUpToDate();
    float prevZoom = m_zoom;
    float const newZoom = std::max(m_zoom * std::exp(0.1f * iWheelDelta), 1.e-16f);
    float2 const prevZoomCenter = iPointerLocalPosition.xy() - box.min;
    float2 const newZoomCenter = prevZoomCenter * newZoom / prevZoom;
    m_offset -= newZoomCenter - prevZoomCenter;
    m_zoom = newZoom;
    InvalidatePlacement();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ZoomingContainer::ComputeTransforms(float4x4& oTransform, float4x4& oInvTransform)
{
    if(nullptr == m_content)
    {
        oTransform = float4x4::Identity();
        oInvTransform = float4x4::Identity();
        return;
    }
    box2f const box = PlacementBox_AssumeUpToDate();
    box2f const contentBox = m_content->PlacementBox_AssumeUpToDate();
    float2 const boxSize = box.Delta();
    float2 const contentBoxSize = contentBox.Delta();
    float const zoom = m_zoom;
    float const ooZoom = 1.f / zoom;
    SG_ASSERT(EqualsWithTolerance(boxSize, contentBoxSize * zoom, 1.e-2f * zoom));
    float2 offset = box.min - contentBox.min;
    oTransform = matrix::HomogeneousScale(float3(zoom, zoom, 1));
    oTransform.SetCol(3, offset.Append(float2(0, 1)));
    oInvTransform = matrix::HomogeneousScale(float3(ooZoom, ooZoom, 1));
    oInvTransform.SetCol(3, (-ooZoom*offset).Append(float2(0, 1)));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ZoomingContainer::VirtualOnPointerEvent(ui::PointerEventContext const& iContext, ui::PointerEvent const& iPointerEvent)
{
    ui::BoxArea wheelArea(Parent()->PlacementBox_AssumeUpToDate());
    ui::WheelHelper wheelHelper;
    wheelHelper.OnPointerEventPreChildren(iContext, iPointerEvent, wheelArea);
    float4x4 transform;
    float4x4 invTransform;
    ComputeTransforms(transform, invTransform);
    ui::PointerEventContext context(iContext, transform, invTransform, ui::Context::TransformType::Transform2D);
    parent_type::VirtualOnPointerEvent(context, iPointerEvent);
    wheelHelper.OnPointerEventPostChildren(iContext, iPointerEvent, *this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ZoomingContainer::VirtualOnDraw(DrawContext const& iContext)
{
    float4x4 transform;
    float4x4 invTransform;
    ComputeTransforms(transform, invTransform);
    ui::DrawContext context(iContext, transform, invTransform, ui::Context::TransformType::Transform2D);
    parent_type::VirtualOnDraw(context);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ZoomingContainer::VirtualUpdatePlacement()
{
    if(nullptr == m_content)
    {
        SetPlacementBox(box2f::FromMinDelta(float2(0,0), float2(0,0)) + m_offset);
        m_subContainer->SetPlacementBox(box2f::FromMinDelta(float2(0,0), float2(0,0)));
        return;
    }
    box2f const& parentBox = Parent()->PlacementBox_AssumeUpToDate();
    SetPlacementBox(box2f::FromMinDelta(float2(0,0), float2(0,0)));
    m_subContainer->SetPlacementBox(box2f::FromMinDelta(float2(0,0), parentBox.Delta()));
    box2f const contentBox = m_content->PlacementBox();
    box2f const box = box2f::FromMinDelta(parentBox.Min() + m_offset - m_zoom * contentBox.Min(), m_zoom * contentBox.Delta());
    SetPlacementBox(box);
    m_subContainer->SetPlacementBox(box2f::FromMinDelta(float2(0,0), parentBox.Delta()));
#if SG_ENABLE_ASSERT
    box2f const newContentBox = m_content->PlacementBox();
    SG_ASSERT(EqualsWithTolerance(newContentBox.Delta(), contentBox.Delta(), 1.e-3f));
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if SG_ENABLE_ASSERT
void ZoomingContainer::CheckConstraintsOnItem(Component* iContent)
{
    SG_ASSERT(iContent->IsRefCounted_ForAssert());
    if(nullptr != iContent->Parent())
        return;

    ui::RootContainer testContainer;
    testContainer.RequestAddToFront(iContent);
    SG_ASSERT(iContent->Parent() == &testContainer);
    testContainer.SetPlacementBox(box2f::FromMinMax(float2(0), float2(0)));
    box2f const box0 = iContent->PlacementBox();
    testContainer.SetPlacementBox(box2f::FromMinMax(float2(0), 0.5f*box0.Delta()));
    box2f const box1 = iContent->PlacementBox();
    SG_ASSERT_MSG(box1.Delta() == box0.Delta(),
               "The minimum box of the content should be constant when the parent is smaller along the 2 dimensions.");
    testContainer.SetPlacementBox(box2f::FromMinMax(float2(0), box0.Delta()));
    box2f const box2 = iContent->PlacementBox();
    SG_ASSERT_MSG(box2.Delta() == box0.Delta(),
               "The minimum box of the content should be constant when the parent is smaller along the 2 dimensions.");

    testContainer.RequestRemove(iContent);
    SG_ASSERT(nullptr == iContent->Parent());
}
#endif
//=============================================================================
}
}

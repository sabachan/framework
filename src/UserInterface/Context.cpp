#include "stdafx.h"

#include "Context.h"

#include <Rendering/RenderBatchDico.h>
#include <RenderEngine/CompositingLayer.h>

namespace sg {
namespace ui {
//=============================================================================
Context::Context()
: m_transform(float4x4::Identity())
, m_transformType(TransformType::None)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Context::Context(Context const& iParent, float4x4 const& iTransform, TransformType iTransformType)
: m_transform(iParent.m_transform * iTransform)
, m_transformType(ComputeTransformType(iParent.m_transformType, iTransformType))
{
#if SG_ENABLE_ASSERT
    SG_ASSERT_MSG(TransformType::None != iTransformType, "There is no need to create a new context, then.");
    switch(iTransformType)
    {
    case TransformType::None:
        SG_ASSUME_NOT_REACHED();
        break;
    case TransformType::Translate2D:
        SG_ASSERT(float4(1,0,0,0) == iTransform.Col(0));
        SG_ASSERT(float4(0,1,0,0) == iTransform.Col(1));
        SG_ASSERT(float4(0,0,1,0) == iTransform.Col(2));
        SG_ASSERT(float4(0,0,0,1) == iTransform.Row(3));
        break;
    case TransformType::Transform2D:
        SG_ASSERT(float4(0,0,1,0) == iTransform.Row(2));
        SG_ASSERT(float4(0,0,0,1) == iTransform.Row(3));
        break;
    case TransformType::Transform3D:
        SG_ASSERT(float4(0,0,0,1) == iTransform.Row(3));
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Context::~Context()
{
}
//=============================================================================
DrawContext::DrawContext(LayerManager& iLayerManager, renderengine::CompositingLayer* iLayer, box2f const& iBoundingClippingBox)
: Context()
, m_layerManager(&iLayerManager)
, m_layer(iLayer)
, m_boundingClippingBox(iBoundingClippingBox)
{
    SG_ASSERT(nullptr != iLayer);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
DrawContext::DrawContext(DrawContext const& iParent, float4x4 const& iTransform, float4x4 const& iInverseTransform, TransformType iTransformType)
: Context(iParent, iTransform, iTransformType)
, m_layerManager(iParent.m_layerManager)
, m_layer(iParent.m_layer)
, m_boundingClippingBox()
{
    SG_ASSERT(EqualsWithTolerance(iTransform * iInverseTransform, float4x4::Identity(), 1.e-1f));
    switch(iTransformType)
    {
    case TransformType::None:
        m_boundingClippingBox = iParent.m_boundingClippingBox;
        break;
    case TransformType::Translate2D:
    {
        float2 const T = iTransform.Col(3).xy();
        m_boundingClippingBox = iParent.m_boundingClippingBox - T;
        break;
    }
    case TransformType::Transform2D:
    {
        float2x2 const R = iInverseTransform.SubMatrix<2, 2>(0, 0);
        float2 const T = iInverseTransform.Col(3).xy();
        m_boundingClippingBox.Grow(R * iParent.m_boundingClippingBox.Corner(BitSet<2>(0)) + T);
        m_boundingClippingBox.Grow(R * iParent.m_boundingClippingBox.Corner(BitSet<2>(1)) + T);
        m_boundingClippingBox.Grow(R * iParent.m_boundingClippingBox.Corner(BitSet<2>(2)) + T);
        m_boundingClippingBox.Grow(R * iParent.m_boundingClippingBox.Corner(BitSet<2>(3)) + T);
        break;
    }
    case TransformType::Transform3D:
        // No bounding box
        m_boundingClippingBox = box2f::FromMinMax(float2(-INFINITY), float2(INFINITY));
        break;
    }
    SG_ASSERT(0 == errno);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
DrawContext::~DrawContext()
{
}
//=============================================================================
PointerEventContext::PointerEventContext()
: Context()
, m_inverseTransform(float4x4::Identity())
, m_postRenderTranformation(nullptr)
, m_previousPostRenderTranformationContext(nullptr)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
PointerEventContext::PointerEventContext(PointerEventContext const& iParent, float4x4 const& iTransform, float4x4 const& iInverseTransform, TransformType iTransformType)
: Context(iParent, iTransform, iTransformType)
, m_inverseTransform(iInverseTransform * iParent.m_inverseTransform)
, m_postRenderTranformation(iParent.m_postRenderTranformation)
, m_previousPostRenderTranformationContext(iParent.m_previousPostRenderTranformationContext)
{
#if SG_ENABLE_ASSERT
    SG_ASSERT(EqualsWithTolerance(iTransform * iInverseTransform, float4x4::Identity(), 1.e-1f));
    SG_ASSERT_MSG(TransformType::None != iTransformType, "There is no need to create a new context, then.");
    switch(iTransformType)
    {
    case TransformType::None:
        SG_ASSUME_NOT_REACHED();
        break;
    case TransformType::Translate2D:
        SG_ASSERT(float4(0) == iInverseTransform.Col(0));
        SG_ASSERT(float4(0) == iInverseTransform.Col(1));
        SG_ASSERT(float4(0) == iInverseTransform.Col(2));
        SG_ASSERT(float4(0,0,0,1) == iInverseTransform.Row(3));
        break;
    case TransformType::Transform2D:
        SG_ASSERT(float4(0,0,1,0) == iInverseTransform.Row(2));
        SG_ASSERT(float4(0,0,0,1) == iInverseTransform.Row(3));
        SG_ASSERT(float4(0,0,1,0) == iInverseTransform.Col(2));
        break;
    case TransformType::Transform3D:
        SG_ASSERT(float4(0,0,0,1) == iInverseTransform.Row(3));
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
PointerEventContext::PointerEventContext(PointerEventContext const& iParent, IPostRenderTransformationForPointer const* iPostRenderTransformation)
: Context(iParent)
, m_inverseTransform(iParent.m_inverseTransform)
, m_postRenderTranformation(iPostRenderTransformation)
, m_previousPostRenderTranformationContext(&iParent)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
PointerEventContext::~PointerEventContext()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void PointerEventContext::TransformPointerPosition(float3& oHomogenousPointerPosition, float2 const& iPointerPosition) const
{
    SG_ASSERT(nullptr == m_postRenderTranformation); // not impl
    SG_ASSERT(nullptr == m_previousPostRenderTranformationContext); // not impl
    TransformType const transformType = GetTransformType();
    switch(transformType)
    {
    case TransformType::None:
        oHomogenousPointerPosition.xy() = iPointerPosition;
        oHomogenousPointerPosition.z() = 1.f;
        break;
    case TransformType::Translate2D:
        SG_ASSERT_NOT_IMPLEMENTED();
        break;
    case TransformType::Transform2D:
        {
            float2x2 const R = m_inverseTransform.SubMatrix<2, 2>(0, 0);
            float2 const T = m_inverseTransform.Col(3).xy();
            oHomogenousPointerPosition.xy() = R * iPointerPosition + T;
            oHomogenousPointerPosition.z() = 1.f;
        }
        break;
    case TransformType::Transform3D:
        SG_ASSERT_NOT_IMPLEMENTED();
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }
}
//=============================================================================
}
}

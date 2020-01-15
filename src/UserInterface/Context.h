#ifndef UserInterface_Context_H
#define UserInterface_Context_H

#include "LayerManager.h"
#include <Core/SmartPtr.h>
#include <Math/Box.h>
#include <Math/Matrix.h>

namespace sg {
namespace renderengine {
    class CompositingLayer;
}
}

namespace sg {
namespace ui {
//=============================================================================
class Context : public SafeCountable
{
public:
    enum class TransformType : u8 { None, Translate2D, Transform2D, Transform3D };
    TransformType GetTransformType() const { return m_transformType; }
    float4x4 const& GetTransform() const { return m_transform; }
    box2f TransformBoundingBox(box2f const& iBox) const;
protected:
    Context();
    Context(Context const& iParent, float4x4 const& iTransform, TransformType iTransformType);
    ~Context();
private:
    static inline TransformType ComputeTransformType(TransformType a, TransformType b)
    {
        return static_cast<TransformType>(std::max(static_cast<u8>(a), static_cast<u8>(b)));
    }
private:
    float4x4 m_transform;
    TransformType m_transformType;
};
//=============================================================================
inline box2f Context::TransformBoundingBox(box2f const& box) const
{
    switch(m_transformType)
    {
    case TransformType::None:
        return box;
    case TransformType::Translate2D:
        return box + m_transform.Col(3).xy();
    case TransformType::Transform2D:
        {
            float2x2 const R = m_transform.SubMatrix<2, 2>(0, 0);
            float2 const T = m_transform.Col(3).xy();
            float2 const center = R * box.Center() + T;
            float2 const delta1 = R * box.Delta();
            float2 const delta2 = R * (box.Delta() * float2(1, -1));
            float2 const delta = componentwise::max(componentwise::abs(delta1), componentwise::abs(delta2));
            return box2f::FromCenterDelta(center, delta);
        }
    case TransformType::Transform3D:
        SG_ASSERT_NOT_REACHED();
        return box;
    default:
        SG_ASSUME_NOT_REACHED();
    }
    SG_ASSUME_NOT_REACHED();
    return box;
}
//=============================================================================
class DrawContext : public Context
{
public:
    DrawContext(LayerManager& iLayerManager, renderengine::CompositingLayer* iLayer, box2f const& iBoundingClippingBox);
    DrawContext(DrawContext const& iParent, float4x4 const& iTransform, float4x4 const& iInverseTransform, TransformType iTransformType);
    ~DrawContext();
    box2f const& BoundingClippingBox() const { return m_boundingClippingBox; }
    LayerManager* GetLayerManager() const { return m_layerManager.get(); }
    renderengine::CompositingLayer* GetCompositingLayer() const { return m_layer.get(); }
private:
    safeptr<LayerManager> m_layerManager;
    safeptr<renderengine::CompositingLayer> m_layer;
    box2f m_boundingClippingBox;
};
//=============================================================================
class IPostRenderTransformationForPointer : public SafeCountable
{
    // oPointerPosition is in homogeneous coordinates, in order to be able to
    // represent invalid position as infinites, with direction information.
    // A valid position will have z = 1, while an invalid position will have
    // a z = 0, the vector xy pointing to the "nearest" direction.
    virtual void VirtualTransformPointerPosition(float3 const& oHomogenousPointerPosition, float2 const& iPointerPosition) const = 0;
};
//=============================================================================
class PointerEventContext : public Context
{
public:
    PointerEventContext();
    PointerEventContext(PointerEventContext const& iParent, float4x4 const& iTransform, float4x4 const& iInverseTransform, TransformType iTransformType);
    PointerEventContext(PointerEventContext const& iParent, IPostRenderTransformationForPointer const* iPostRenderTransformation);
    ~PointerEventContext();
    float4x4 const& GetInverseTransform() const { return m_inverseTransform; }
    void TransformPointerPosition(float3& oHomogenousPointerPosition, float2 const& iPointerPosition) const;
private:
    float4x4 m_inverseTransform;
    safeptr<IPostRenderTransformationForPointer const> m_postRenderTranformation;
    safeptr<PointerEventContext const> m_previousPostRenderTranformationContext;
};
//=============================================================================
}
}

#endif

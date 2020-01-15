#ifndef UserInterface_ZoomingContainer_H
#define UserInterface_ZoomingContainer_H

#include "Container.h"
#include "Movable.h"
//#include "PointerEvent.h"
#include "SensitiveArea.h"
#include "WheelHelper.h"
#include <Math/Matrix.h>
#include <System/MouseUtils.h>
#include <System/UserInputEvent.h>


namespace sg {
namespace ui {
//=============================================================================
// This class is dedicated to be set as content to a ScrollingContainer
class ZoomingContainer : public Container
                       , public IMovable
                       , private ISensitiveAreaListener
                       , private IWheelListener
{
    typedef Container parent_type;
public:
    struct Properties
    {
        float zoomSpeed = 10.f;
        float minZoom = 0;
        float maxZoom = INFINITY;
    };
public:
    ZoomingContainer();
    ZoomingContainer(Properties const& iProperties);
    void SetContent(Component* iContent);
protected:
    virtual void VirtualOnPointerEvent(PointerEventContext const& iContext, PointerEvent const& iPointerEvent) override;
    virtual void VirtualOnDraw(DrawContext const& iContext) override;
    virtual void OnWheelEvent(SG_UI_WHEEL_LISTENER_PARAMETERS) override;
    virtual void VirtualUpdatePlacement() override;
    virtual Component* VirtualAsComponent() { return this; }
    virtual void VirtualResetOffset() { m_offset = float2(0.f, 0.f); }
    virtual void VirtualAddOffset(float2 const& iOffset) { m_offset += iOffset; }
#if SG_ENABLE_ASSERT
    void CheckConstraintsOnItem(Component* iContent);
#endif
private:
    void ComputeTransforms(float4x4& oTransform, float4x4& oInvTransform);
private:
    Properties m_properties;
    refptr<SubContainer> m_subContainer;
    safeptr<Component> m_content;
    float m_zoom = 1.f;
    float2 m_offset = float2(0.f, 0.f);
};
//=============================================================================
}
}

#endif

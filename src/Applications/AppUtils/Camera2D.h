#ifndef Applications_AppUtils_Camera2D_H
#define Applications_AppUtils_Camera2D_H

#include <Core/SmartPtr.h>
#include <Math/Matrix.h>

namespace sg {
namespace apputils {
//=============================================================================
class Camera2D : public RefAndSafeCountable
{
public:
    Camera2D() : m_worldToView(), m_viewToWorld() { m_worldToView.SetSubMatrix(0,0, float2x2::Identity()); UpdateViewToWorld(); }
    float2x3 const& WorldToView() const { return m_worldToView; }
    float2x3 const& ViewToWorld() const { return m_viewToWorld; }
    void TranslateWorldInCameraSpace(float2 const& iTranslationInCameraSpace);
    void ApplyZoomInCameraSpace(float2 const& iCenterInCameraSpace, float iZoomFactor);
private:
    void UpdateViewToWorld();
private:
    float2x3 m_worldToView;
    float2x3 m_viewToWorld;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline float4x4 ToFloat4x4(float2x3 const& m)
{
    float4x4 r;
    r.SetSubMatrix(0,0,m.SubMatrix<2,2>(0,0));
    r(0,2) = 0;
    r(1,2) = 0;
    r.SetSubMatrix(0,3,m.SubMatrix<2,1>(0,2));
    r.SetRow(2,float4(0,0,1,0));
    r.SetRow(3,float4(0,0,0,1));
    return r;
}
//=============================================================================
}
}

#endif

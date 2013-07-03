#include "stdafx.h"

#include "Camera2D.h"

namespace sg {
namespace apputils {
//=============================================================================
void Camera2D::TranslateWorldInCameraSpace(float2 const& iTranslationInCameraSpace)
{
    float2 const t = m_worldToView.Col(2) + iTranslationInCameraSpace;
    m_worldToView.SetCol(2, t);
    UpdateViewToWorld();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Camera2D::ApplyZoomInCameraSpace(float2 const& iCenterInCameraSpace, float iZoomFactor)
{
    float scale = std::exp(iZoomFactor);
    float2 const t = (m_worldToView.Col(2) - iCenterInCameraSpace) * scale + iCenterInCameraSpace;
    float2x2 const M = m_worldToView.SubMatrix<2,2>(0,0) *= scale;
    m_worldToView.SetSubMatrix(0,0, M);
    m_worldToView.SetCol(2, t);
    UpdateViewToWorld();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Camera2D::UpdateViewToWorld()
{
    float2x2 const M = m_worldToView.SubMatrix<2,2>(0,0);
    float2x2 const invM = Invert_AssumeInvertible(M);
    float2 const t = m_worldToView.Col(2);
    float2 const invt = -(invM * t);
    m_viewToWorld.SetSubMatrix(0,0, invM);
    m_viewToWorld.SetCol(2, invt);
}
//=============================================================================
}
}


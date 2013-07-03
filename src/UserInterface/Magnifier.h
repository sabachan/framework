#ifndef UserInterface_Magnifier_H
#define UserInterface_Magnifier_H

#include <Core/SmartPtr.h>

namespace sg {
namespace ui {
//=============================================================================
class Magnifier : public SafeCountable
{
public:
    Magnifier(float iMagnification = 1.f) : m_magnification(iMagnification) {}
    SG_FORCE_INLINE void SetMagnification(float iMagnification) { m_magnification = iMagnification; }
    SG_FORCE_INLINE float Magnification() const { return m_magnification; }
private:
    float m_magnification;
};
//=============================================================================
class IMagnifiable
{
public:
    IMagnifiable(Magnifier const& iMagnifier) : m_magnifier(&iMagnifier) {}
    SG_FORCE_INLINE void SetMagnifier(Magnifier const& iMagnifier);
    SG_FORCE_INLINE Magnifier const& GetMagnifier() const { return *m_magnifier; }
    SG_FORCE_INLINE float Magnification() const { return m_magnifier->Magnification(); }
protected:
    virtual void VirtualOnMagnifierChange(Magnifier const& iMagnifier) { SG_UNUSED(iMagnifier); }
private:
    safeptr<Magnifier const> m_magnifier;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void IMagnifiable::SetMagnifier(Magnifier const& iMagnifier)
{
    m_magnifier = &iMagnifier;
    VirtualOnMagnifierChange(iMagnifier);
}
//=============================================================================
}
}

#endif

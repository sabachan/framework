#ifndef UserInterface_PointerEvent_H
#define UserInterface_PointerEvent_H

#include <System/UserInputEvent.h>

namespace sg {
namespace ui {
//=============================================================================
class PointerEvent
{
    SG_NON_COPYABLE(PointerEvent)
public:
    PointerEvent(system::UserInputEvent const& iEvent SG_CODE_FOR_ASSERT(SG_COMMA size_t iIndex))
        : m_event(&iEvent)
        SG_CODE_FOR_ASSERT(SG_COMMA m_index(iIndex))
    {}
    ~PointerEvent() {}

    system::UserInputEvent const& Event() const { return *m_event; }
#if SG_ENABLE_ASSERT
    size_t Index() const { return m_index; }
#endif
private:
    SG_CODE_FOR_ASSERT(size_t m_index);
    safeptr<system::UserInputEvent const> m_event;
};
//=============================================================================
}
}

#endif

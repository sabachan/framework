#ifndef System_UserInputListener_H
#define System_UserInputListener_H

#include <Core/SmartPtr.h>
#include "UserInputEvent.h"

namespace sg {
namespace system {
//=============================================================================
class IUserInputListener : public SafeCountable
{
public:
    virtual void OnUserInputEvent(UserInputEvent const& iEvent) = 0;
};
//=============================================================================
}
}

#endif

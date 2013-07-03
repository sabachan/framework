#ifndef System_UserInputManager_H
#define System_UserInputManager_H

#include <memory>
#include <vector>
#include <Core/SmartPtr.h>
#include "UserInputEvent.h"
#include "UserInputListener.h"

namespace sg {
namespace system {
//=============================================================================
class Window;
//=============================================================================
class UserInputListenerList : public SafeCountable
{
public:
    UserInputListenerList();
    ~UserInputListenerList();
    void RegisterListener(IUserInputListener* iListener, size_t iPriority);
    void UnregisterListener(IUserInputListener* iListener);
    void ProcessEvent(UserInputEvent const& iEvent);
private:
    void Update();
private:
    typedef std::pair<size_t, safeptr<IUserInputListener> > PriorityAndListener;
    std::vector<PriorityAndListener> m_listeners;
    bool m_modified;
};
//=============================================================================
class UserInputManager
{
public:
    UserInputManager();
    ~UserInputManager();

    void RegisterListener(IUserInputListener* iListener, size_t iPriority) { m_listeners.RegisterListener(iListener, iPriority); }
    void UnregisterListener(IUserInputListener* iListener) { m_listeners.UnregisterListener(iListener); }

    UserInputEvent& CreatePushAndGetEvent(Window* iWnd, UserInputListenerList* iWndListeners);
    void RunEvents();
private:
    UserInputListenerList m_listeners;
    std::vector<std::pair<safeptr<UserInputListenerList>, std::unique_ptr<UserInputEvent> > > m_events;
    //max_sized_array<UserInputEvent, 64> m_events;

#if SG_ENABLE_ASSERT
public:
    size_t BatchIndex() const { return m_batchIndex; }
private:
    size_t m_batchIndex;
    size_t m_eventIndex;
#endif
};
//=============================================================================
}
}

#endif

#include "stdafx.h"

#include "UserInputManager.h"

#include <algorithm>
#include "UserInputEvent.h"
#include "Window.h"

namespace sg {
namespace system {
//=============================================================================
UserInputListenerList::UserInputListenerList()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
UserInputListenerList::~UserInputListenerList()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void UserInputListenerList::RegisterListener(IUserInputListener* iListener, size_t iPriority)
{
#if SG_ENABLE_ASSERT
    for(auto const& it : m_listeners)
        SG_ASSERT_MSG(it.first != iPriority, "A listener with same priority is already present.");
#endif
    m_listeners.emplace_back(iPriority, iListener);
    m_modified = true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void UserInputListenerList::UnregisterListener(IUserInputListener* iListener)
{
    SG_CODE_FOR_ASSERT(bool found = false;)
    for(auto& it : m_listeners)
    {
        if(it.second == iListener)
        {
            SG_CODE_FOR_ASSERT(found = true;)
            if(&it != &m_listeners.back())
            {
                using std::swap;
                swap(it, m_listeners.back());
            }
            break;
        }
    }
    SG_ASSERT(found);
    m_listeners.pop_back();
    m_modified = true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void UserInputListenerList::ProcessEvent(UserInputEvent const& iEvent)
{
    if(m_modified)
        Update();

    SG_CODE_FOR_ASSERT(size_t priority = 0;)
    for(auto const& it : m_listeners)
    {
        SG_ASSERT(it.first >= priority);
        SG_ASSERT(nullptr != it.second);
        it.second->OnUserInputEvent(iEvent);
        SG_CODE_FOR_ASSERT(priority = it.first + 1;)
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void UserInputListenerList::Update()
{
    std::sort(m_listeners.begin(), m_listeners.end(), [](PriorityAndListener const& a, PriorityAndListener const& b) { return a.first < b.first; });
}
//=============================================================================
UserInputManager::UserInputManager()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
UserInputManager::~UserInputManager()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
UserInputEvent& UserInputManager::CreatePushAndGetEvent(Window* iWnd, UserInputListenerList* iWndListeners)
{
    m_events.emplace_back();
    m_events.back().first = iWndListeners;
#if SG_ENABLE_ASSERT
    m_events.back().second.reset(new UserInputEvent(iWnd, m_eventIndex++, m_batchIndex++));
#else
    m_events.back().second.reset(new UserInputEvent(iWnd));
#endif
    return *m_events.back().second;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void UserInputManager::RunEvents()
{
    for(auto const& it : m_events)
    {
        UserInputEvent const& event = *it.second;
        m_listeners.ProcessEvent(event);
        if(nullptr != it.first)
            it.first->ProcessEvent(event);
    }
    m_events.clear();
}
//=============================================================================
}
}

#ifndef System_Window_H
#define System_Window_H

#include <Core/Observer.h>
#include <Core/SmartPtr.h>
#include <Math/Box.h>
#include <Math/Vector.h>
#include <vector>
#include "UserInputAdapter_Win.h"
#include "UserInputManager.h"

namespace sg {
namespace system {
//=============================================================================
class WindowedApplication;
//=============================================================================
class Window : public RefAndSafeCountable
{
    SG_NON_COPYABLE(Window)
    friend class WindowedApplication;
public:
    Window();
    ~Window();
    void RegisterUserInputListener(IUserInputListener* iListener, size_t iPriority) { m_userInputListeners.RegisterListener(iListener, iPriority); }
    void UnregisterUserInputListener(IUserInputListener* iListener) { m_userInputListeners.UnregisterListener(iListener); }
    ObservableValue<uint2> const& ClientSize() const { return m_clientSize; }
    void Show(bool iShow);
    bool IsVisible();
    void CloseIFP();
    bool IsClosed();
    void SetClientSize(uint2 const& iSize);
    void SetClientPlacement(box2i const& iPlacement);
    int2 GetClientTopLeft();
    box2i GetClientPlacement();
public: // win specific
    HWND hWnd() const { return m_hWnd; }
    LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UserInputManager& iUserInputManager);
private:
    void InitInstance();
private:
    HWND m_hWnd;
    ObservableValue<uint2> m_clientSize;
    bool m_isClosed;
    win::UserInputAdapter m_userInputAdapter;
    UserInputListenerList m_userInputListeners;
};
//=============================================================================
}
}

#endif


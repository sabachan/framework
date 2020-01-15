#ifndef System_WindowedApplication_H
#define System_WindowedApplication_H

#include <Core/Platform.h>
#if !SG_PLATFORM_IS_WIN
// TODO: move win specific in sg::system::win::WindowedApplication as a derived class.
#error "Windows only !"
#endif

#include "UserInputManager.h"
#include <Core/SmartPtr.h>
#include <Math/Vector.h>
#include <vector>

namespace sg {
namespace system {
//=============================================================================
class IUserInputListener;
class Window;
//=============================================================================
class WindowedApplication
{
    friend Window;
public:
    WindowedApplication();
    virtual ~WindowedApplication();

    void Run();
    void RegisterUserInputListener(IUserInputListener* iListener, size_t iPriority) { m_userInputManager.RegisterListener(iListener, iPriority); }
    void UnregisterUserInputListener(IUserInputListener* iListener) { m_userInputManager.UnregisterListener(iListener); }
public: // win specific
    LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    ATOM GetWindowClass() { return m_windowClassAtom; }
protected:
    virtual void VirtualOneTurn();
    virtual void VirtualOnDropFile(char const* iFilePath, Window* iWindow, uint2 const& iPosition);
    // TODO: Port clients on UserInputListeners and remove.
    virtual LRESULT VirtualWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
    void OneTurn();
    void RegisterWindowClass();
    size_t GetWindowIndex(HWND hWnd);
    Window* GetWindow(HWND hWnd);
    void RegisterWindow(Window* iWindow);
    void UnregisterWindow(Window* iWindow);
private:
    HINSTANCE m_hinst;
    ATOM m_windowClassAtom;
    std::vector<safeptr<Window> > m_windows;
    UserInputManager m_userInputManager;
    SG_CODE_FOR_ASSERT(bool m_parentVirtualOneTurnCalled);
};
//=============================================================================
WindowedApplication* GetWindowedApplicationIFP();
//=============================================================================
}
}

#endif


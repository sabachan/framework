#include "stdafx.h"

#include "Window.h"

#include <Core/Platform.h>
#include <Core/WinUtils.h>
#include "WindowedApplication.h"

#if SG_PLATFORM_IS_WIN
#include <Core/WindowsH.h>
#endif

namespace sg {
namespace system {
//=============================================================================
Window::Window()
    : m_hWnd(nullptr)
    , m_clientSize(uint2(400,300))
    , m_isClosed(false)
    , m_userInputAdapter()
    , m_userInputListeners()
{
    WindowedApplication* app = GetWindowedApplicationIFP();
    SG_ASSERT(nullptr != app);

    app->RegisterWindow(this);

    Show(true);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Window::~Window()
{
    if(!IsClosed())
        Close();

    WindowedApplication* app = GetWindowedApplicationIFP();
    SG_ASSERT_AND_UNUSED(nullptr == app->GetWindow(m_hWnd));

    BOOL rc = DestroyWindow(m_hWnd);
    if(!rc)
        winutils::PrintWinLastError("DestroyWindow");
    SG_ASSERT(rc);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::InitInstance()
{
    WindowedApplication* app = GetWindowedApplicationIFP();
    // TODO: Replace constant WindowClass by properties.
    ATOM windowClass = app->GetWindowClass();
    DWORD const style = WS_OVERLAPPEDWINDOW;
    bool const hasMenu = false;
    uint2 const wh = m_clientSize.Get();
    RECT rect = {0,0,LONG(wh.x()),LONG(wh.y())};
    BOOL rc = AdjustWindowRect(&rect, style, hasMenu);
    SG_ASSERT_AND_UNUSED(rc);

    m_hWnd = CreateWindow(
        LPCTSTR(windowClass),
        "Dev Application",
        style,
        CW_USEDEFAULT,
        0,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,
        NULL,
        NULL,
        NULL);

   if (nullptr == m_hWnd)
   {
       SG_ASSERT_NOT_REACHED();
   }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Window::IsVisible()
{
    return nullptr != m_hWnd && 0 != IsWindowVisible(m_hWnd);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::Show(bool iShow)
{
    SG_ASSERT(!m_isClosed);
    if(iShow == IsVisible())
        return;
    if(iShow)
    {
        if(nullptr == m_hWnd)
        {
            InitInstance();
            SG_ASSERT(nullptr != m_hWnd);
        }
        ShowWindow(m_hWnd, SW_SHOWDEFAULT);
    }
    else
    {
        ShowWindow(m_hWnd, SW_HIDE);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::Close()
{
    // Can't destroy window like that today, as clients like RenderWindow do
    // not expect that.
    // For now, window is kept alive, but not shown.
    if(IsVisible())
        ShowWindow(m_hWnd, SW_HIDE);

    WindowedApplication* app = GetWindowedApplicationIFP();
    SG_ASSERT(nullptr != app);
    app->UnregisterWindow(this);

    m_isClosed = true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Window::IsClosed()
{
    return m_isClosed;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::SetClientSize(uint2 const& iSize)
{
    RECT prevRect;
    GetWindowRect(m_hWnd, &prevRect);
    int2 const topleft = int2(prevRect.left, prevRect.top);

    DWORD const style = WS_OVERLAPPEDWINDOW;
    bool const hasMenu = false;
    uint2 const wh = iSize;
    RECT rect = {0,0,LONG(wh.x()),LONG(wh.y())};
    BOOL rc = AdjustWindowRect(&rect, style, hasMenu);
    SG_ASSERT_AND_UNUSED(rc);
    int2 const delta = int2(rect.right - rect.left, rect.bottom - rect.top);
    MoveWindow(m_hWnd, topleft.x(), topleft.y(), delta.x(), delta.y(), true);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::SetClientPlacement(box2i const& iPlacement)
{
    DWORD const style = WS_OVERLAPPEDWINDOW;
    bool const hasMenu = false;
    uint2 const wh = uint2(iPlacement.Delta());
    RECT rect = {0,0,LONG(wh.x()),LONG(wh.y())};
    BOOL rc = AdjustWindowRect(&rect, style, hasMenu);
    SG_ASSERT_AND_UNUSED(rc);
    int2 const adjustMin = int2(rect.left, rect.top);
    int2 const adjustMax = int2(rect.right, rect.bottom);
    int2 const topleft = int2(iPlacement.Min()) + adjustMin;
    int2 const bottomright = int2(iPlacement.Min()) + adjustMax;
    int2 const delta = bottomright - topleft;
    MoveWindow(m_hWnd, topleft.x(), topleft.y(), delta.x(), delta.y(), true);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
int2 Window::GetClientTopLeft()
{
    POINT p;
    p.x = 0;
    p.y = 0;
    BOOL rc = ClientToScreen(m_hWnd, &p);
    SG_ASSERT_AND_UNUSED(rc);
    return int2(p.x, p.y);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
box2i Window::GetClientPlacement()
{
    int2 const topleft = Window::GetClientTopLeft();
    RECT rect;
    GetClientRect(m_hWnd, &rect);
    SG_ASSERT(0 == rect.left);
    SG_ASSERT(0 == rect.top);
    box2i const box = box2i::FromMinMax(topleft, int2(rect.right, rect.bottom));
    return box;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LRESULT Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UserInputManager& iManager)
{
    SG_ASSERT_AND_UNUSED(m_hWnd == hWnd);
    switch (message)
    {
    case WM_SIZE:
        {
            switch(wParam)
            {
            case SIZE_MAXHIDE:
                SG_ASSERT_NOT_REACHED();
            case SIZE_MAXSHOW:
                SG_ASSERT_NOT_REACHED();
            case SIZE_MINIMIZED:
                // Do nothing for now (buffers of size 0 are not really supported).
                // The best would be to disable unusefull rendering.
                // TODO: signal to the window that it is minimized.
                return 0;
            case SIZE_MAXIMIZED:
                // Here, we could go fullscreen
                break;
            case SIZE_RESTORED:
                break;
            default:
                SG_ASSERT_NOT_REACHED();
            }
            u32 const w = LOWORD(lParam);
            u32 const h = HIWORD(lParam);
            SG_ASSERT(w > 0);
            SG_ASSERT(h > 0);
            m_clientSize.Set(uint2(w,h));
        }
        break;
    default:
        return m_userInputAdapter.WndProc(this, message, wParam, lParam, m_userInputListeners, iManager);
    }
    return 0;
}
//=============================================================================
}
}

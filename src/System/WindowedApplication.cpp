#include "stdafx.h"

#include "WindowedApplication.h"

#include "Window.h"

#include <Core/PerfLog.h>
#include <Core/Platform.h>
#if SG_PLATFORM_IS_WIN
#include <Core/WinUtils.h>
#endif

namespace sg {
namespace system {
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
WindowedApplication* g_pWindowedApplication= nullptr;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LRESULT CALLBACK WindowedApplication_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    SG_ASSERT(nullptr != g_pWindowedApplication);
    return g_pWindowedApplication->WndProc(hWnd, message, wParam, lParam);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
WindowedApplication* GetWindowedApplicationIFP()
{
    return g_pWindowedApplication;
}
//=============================================================================
WindowedApplication::WindowedApplication()
: m_hinst(nullptr)
, m_windowClassAtom()
, m_windows()
SG_CODE_FOR_ASSERT(SG_COMMA m_parentVirtualOneTurnCalled(false))
{
    SG_ASSERT(nullptr == g_pWindowedApplication);
    g_pWindowedApplication = this;

    RegisterWindowClass();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
WindowedApplication::~WindowedApplication()
{
    MSG msg;
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    SG_ASSERT(m_windows.empty());
    g_pWindowedApplication = nullptr;

    BOOL rc = UnregisterClass(
        LPCTSTR(m_windowClassAtom),
        m_hinst
    );
    if(!rc)
    {
        winutils::PrintWinLastError("UnregisterClass");
    }
    SG_ASSERT(rc);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void WindowedApplication::RegisterWindowClass(/*TODO: window properties and name ?*/)
{
    size_t const CLASS_NAME_MAX_SIZE = 256;
    char className[CLASS_NAME_MAX_SIZE] = "WindowedApplication_WndClass";

    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WindowedApplication_WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = m_hinst;
    wcex.hIcon          = nullptr;
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = className;
    wcex.hIconSm        = nullptr;
    m_windowClassAtom = RegisterClassEx(&wcex);
    if(0 == m_windowClassAtom)
    {
        SG_ASSERT_NOT_REACHED();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
size_t WindowedApplication::GetWindowIndex(HWND hWnd)
{
    size_t const windowCount = m_windows.size();
    for(size_t i = 0; i < windowCount; ++i)
    {
        Window const* window = m_windows[i].get();
        if(window->hWnd() == hWnd)
            return i;
    }
    return all_ones;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Window* WindowedApplication::GetWindow(HWND hWnd)
{
    size_t i = GetWindowIndex(hWnd);
    if(-1 == i)
        return nullptr;
    SG_ASSERT(i < m_windows.size());
    return m_windows[i].get();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void WindowedApplication::RegisterWindow(Window* iWindow)
{
    SG_ASSERT_MSG(m_windows.end() == std::find(m_windows.begin(), m_windows.end(), iWindow), "Window already registered");
    m_windows.push_back(iWindow);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void WindowedApplication::UnregisterWindow(Window* iWindow)
{
    auto f = std::find(m_windows.begin(), m_windows.end(), iWindow);
    SG_ASSERT_MSG(m_windows.end() != f, "Window was not registered");
    using std::swap;
    if(m_windows.end() - f > 1)
        swap(m_windows[f-m_windows.begin()], m_windows.back());
    m_windows.resize(m_windows.size() - 1);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void WindowedApplication::Run()
{
    MSG msg;

    bool quitResquested = false;
    while(!quitResquested)
    {
#if SG_ENABLE_PERF_LOG
        perflog::OnNewFrame();
#endif
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if(WM_QUIT == msg.message)
            {
                quitResquested = true;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if(!quitResquested)
        {
            m_userInputManager.RunEvents();

            OneTurn();
        }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void WindowedApplication::OneTurn()
{
    SG_CODE_FOR_ASSERT(m_parentVirtualOneTurnCalled = false);
    VirtualOneTurn();
    SG_ASSERT_MSG(m_parentVirtualOneTurnCalled, "A derivative of WindowedApplication did not called parent VirtualOneTurn()!");
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void WindowedApplication::VirtualOneTurn()
{
    SG_CODE_FOR_ASSERT(m_parentVirtualOneTurnCalled = true);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LRESULT WindowedApplication::VirtualWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CLOSE:
        {
            Window* window = GetWindow(hWnd);
            // provisory code (client should be able to ask user confirmation)
            window->Close();
            if(m_windows.empty())
                PostQuitMessage(0);
        }
        break;
    case WM_DESTROY:
        {
            Window* window = GetWindow(hWnd);
            // provisory assert (code is currently done in WM_CLOSE)
            SG_ASSERT_AND_UNUSED(nullptr == window);
        }
        break;
    default:
        {
            SG_ASSERT(nullptr != hWnd);
            Window* window = GetWindow(hWnd);
            //SG_ASSERT(nullptr != window);
            if(nullptr != window)
                return window->WndProc(hWnd, message, wParam, lParam, m_userInputManager);
            else
                return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    return 0;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LRESULT WindowedApplication::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return VirtualWndProc(hWnd, message, wParam, lParam);
}
//=============================================================================
}
}

#pragma once

#include "system/app-interface.h"
#include <Windows.h>

/**
 * Windows アプリ
 */
class WindowsApp : public IApp
{
    struct Private;
    normal_unique_ptr<Private> p;

    static LRESULT WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    LRESULT OnWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

public:
    WindowsApp(int screenWidth, int screenHeight);
    ~WindowsApp();

    int Run(HINSTANCE hInstance, int nCmdShow) const;
    HWND GetWindowsHandle() const;

protected:
    std::tuple<int, int> GetScreenSize() const override;
    void OnSetup() override { }
    void OnCleanup() override { }
    void OnUpdate() override { }
};

extern normal_unique_ptr<WindowsApp> CreateWindowsApp();

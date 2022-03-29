#pragma once

#include <dxgi.h>
#include <windows.h>

class Window
{
public:
    static const int swapChainLength = 2;

    Window();
    ~Window();
    void init(HINSTANCE hInstance, int nCmdShow, IUnknown* device);
    IDXGISwapChain* getSwapChain();
    void present();

private:
    HINSTANCE m_instanceHandle = nullptr;
    HWND m_windowHandle = nullptr;
    LONG m_width = 800;
    LONG m_height = 600;
    IDXGISwapChain* m_swapChain = nullptr;
};

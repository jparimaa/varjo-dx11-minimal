#pragma once

#include "Utility.h"
#include <winerror.h>
#include <comdef.h>
#include <iostream>

void checkVarjoError(varjo_Session* session)
{
    varjo_Error err = varjo_GetError(session);
    if (err != varjo_NoError)
    {
        std::cerr << "Varjo error: " << varjo_GetErrorDesc(err) << "\n";
        abort();
    }
}

void checkHresult(HRESULT hr)
{
    if (FAILED(hr))
    {
        std::cerr << "HRESULT error: " << _com_error(hr).ErrorMessage() << "\n";
    }
}

Microsoft::WRL::ComPtr<IDXGIAdapter> getAdapter(varjo_Luid luid)
{
    Microsoft::WRL::ComPtr<IDXGIFactory> factory = nullptr;

    const HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    if (SUCCEEDED(hr))
    {
        UINT i = 0;
        while (true)
        {
            Microsoft::WRL::ComPtr<IDXGIAdapter> adapter = nullptr;
            if (factory->EnumAdapters(i++, &adapter) == DXGI_ERROR_NOT_FOUND)
            {
                break;
            }

            DXGI_ADAPTER_DESC desc;
            if (SUCCEEDED(adapter->GetDesc(&desc)) && desc.AdapterLuid.HighPart == luid.high && desc.AdapterLuid.LowPart == luid.low)
            {
                return adapter;
            }
        }
    }
    return nullptr;
}

bool isEscDown()
{
    HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
    INPUT_RECORD input;
    DWORD count = 0;
    while (GetNumberOfConsoleInputEvents(in, &count) && count > 0)
    {
        if (ReadConsoleInputA(in, &input, 1, &count) && count > 0 && input.EventType == KEY_EVENT && input.Event.KeyEvent.bKeyDown)
        {
            if (input.Event.KeyEvent.uChar.AsciiChar == '\033') // ESC
            {
                return true;
            }
        }
    }
    return false;
}

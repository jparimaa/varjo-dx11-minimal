#pragma once

#include <Varjo.h>
#include <Varjo_types_d3d11.h>
#include <wrl/client.h>

template<typename T>
void releaseDXPtr(T*& ptr)
{
    if (ptr != nullptr)
    {
        ptr->Release();
        ptr = nullptr;
    }
}

void checkVarjoError(varjo_Session* session);
void checkHresult(HRESULT hr);

Microsoft::WRL::ComPtr<IDXGIAdapter> getAdapter(varjo_Luid luid);
bool isEscDown();

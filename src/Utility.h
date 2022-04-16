#pragma once

#include <Varjo.h>
#include <Varjo_types_d3d11.h>
#include <glm/glm.hpp>
#include <wrl/client.h>
#include <vector>

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
glm::mat4 toGlmMat4(const double* matrix);

// clang-format off
const std::vector<float> c_backgroundVertexData
{
    -20.0f,  20.0f, -15.0f, 0.0f, 0.0f,
     20.0f,  20.0f, -15.0f, 1.0f, 0.0f,
    -20.0f, -20.0f, -15.0f, 0.0f, 1.0f,
	 20.0f, -20.0f, -15.0f, 1.0f, 1.0f
};
// clang-format on

const std::vector<uint32_t> c_backgroundIndexData{0, 1, 2, 3};

const UINT c_vertexStride = 20;
const UINT c_vertexOffset = 0;
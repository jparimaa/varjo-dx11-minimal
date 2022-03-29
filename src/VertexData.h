#pragma once

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

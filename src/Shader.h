#pragma once

#include <d3d11.h>

#include <vector>

class Shader
{
public:
    Shader();
    ~Shader();

    void init(ID3D11Device* device, ID3DBlob* vertexBlob, ID3DBlob* pixelBlob, const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputLayout);
    void init(ID3D11Device* device, ID3DBlob* vertexBlob, const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputLayout);
    ID3D11VertexShader* getVertexShader();
    ID3D11PixelShader* getPixelShader();
    ID3D11InputLayout* getIputLayout();
    void reset();

private:
    ID3D11VertexShader* m_vertexShader = nullptr;
    ID3D11PixelShader* m_pixelShader = nullptr;
    ID3D11InputLayout* m_inputLayout = nullptr;
};
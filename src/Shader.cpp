#include "Shader.h"
#include "Log.h"
#include "Utility.h"

Shader::Shader()
{
}

Shader::~Shader()
{
    reset();
}

void Shader::init(ID3D11Device* device, ID3DBlob* vertexBlob, ID3DBlob* pixelBlob, const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputLayout)
{
    HRESULT result = device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, &m_vertexShader);
    checkHresult(result);

    result = device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), nullptr, &m_pixelShader);
    checkHresult(result);

    result = device->CreateInputLayout(inputLayout.data(), (UINT)inputLayout.size(), vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), &m_inputLayout);
    checkHresult(result);
}

void Shader::init(ID3D11Device* device, ID3DBlob* vertexBlob, const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputLayout)
{
    HRESULT result = device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, &m_vertexShader);
    checkHresult(result);

    result = device->CreateInputLayout(inputLayout.data(), (UINT)inputLayout.size(), vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), &m_inputLayout);
    checkHresult(result);
}

ID3D11VertexShader* Shader::getVertexShader()
{
    return m_vertexShader;
}

ID3D11PixelShader* Shader::getPixelShader()
{
    return m_pixelShader;
}

ID3D11InputLayout* Shader::getIputLayout()
{
    return m_inputLayout;
}

void Shader::reset()
{
    releaseDXPtr(m_vertexShader);
    releaseDXPtr(m_pixelShader);
    releaseDXPtr(m_inputLayout);
}

#include "DX11Renderer.h"
#include "Log.h"
#include "Utility.h"

#include <Varjo_d3d11.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>

DX11Renderer::DX11Renderer()
{
}

DX11Renderer::~DX11Renderer()
{
    for (int i = 0; i < m_textures.size(); ++i)
    {
        releaseDXPtr(m_textures[i]);
        releaseDXPtr(m_textureSrvs[i]);
    }

    releaseDXPtr(m_linearSampler);

    for (RenderTarget& renderTarget : m_renderTargets)
    {
        releaseDXPtr(renderTarget.rtv);
    }

    releaseDXPtr(m_windowRtv);

    releaseDXPtr(m_rasterState);
    releaseDXPtr(m_matrixBuffer);
    releaseDXPtr(m_backgroundVertexBuffer);

    releaseDXPtr(m_deviceContext);
    releaseDXPtr(m_device);
}

void DX11Renderer::createDevice(Microsoft::WRL::ComPtr<IDXGIAdapter> adapter)
{
    D3D_DRIVER_TYPE driverType = adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE;
    UINT flags = 0;
#ifdef _DEBUG
    flags = D3D11_CREATE_DEVICE_DEBUG;
#endif
    HRESULT hr = D3D11CreateDevice(adapter.Get(), driverType, 0, flags, nullptr, 0, D3D11_SDK_VERSION, &m_device, nullptr, &m_deviceContext);
    checkHresult(hr);
}

void DX11Renderer::createSwapChain(varjo_Session* session, int length, int width, int height, varjo_SwapChain*& colorSwapChainOut)
{
    varjo_SwapChainConfig2 swapChainConfig{};
    swapChainConfig.numberOfTextures = length;
    swapChainConfig.textureArraySize = 1;
    swapChainConfig.textureFormat = varjo_TextureFormat_R8G8B8A8_SRGB;
    swapChainConfig.textureWidth = width;
    swapChainConfig.textureHeight = height;

    colorSwapChainOut = varjo_D3D11CreateSwapChain(session, m_device, &swapChainConfig);

    checkVarjoError(session);

    for (int i = 0; i < length; ++i)
    {
        const varjo_Texture colorTexture = varjo_GetSwapChainImage(colorSwapChainOut, i);
        ID3D11Texture2D* d3dColorTexture = varjo_ToD3D11Texture(colorTexture);

        addRenderTarget(d3dColorTexture);
    }
}

void DX11Renderer::createWindow(HINSTANCE hInstance, int nCmdShow)
{
    m_window.init(hInstance, nCmdShow, m_device);

    IDXGISwapChain* swapChain = m_window.getSwapChain();
    ID3D11Texture2D* backBuffer = nullptr;
    HRESULT result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    checkHresult(result);

    result = m_device->CreateRenderTargetView(backBuffer, nullptr, &m_windowRtv);
    backBuffer->Release();
    checkHresult(result);
}

void DX11Renderer::createDefaultShader()
{
    const std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayout{
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}};
    m_shader.init(m_device, inputLayout);

    D3D11_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.FrontCounterClockwise = false;
    rasterizerDesc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
    rasterizerDesc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizerDesc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizerDesc.DepthClipEnable = true;
    rasterizerDesc.ScissorEnable = false;
    rasterizerDesc.MultisampleEnable = true;
    rasterizerDesc.AntialiasedLineEnable = false;

    HRESULT result = m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterState);
    checkHresult(result);
}

void DX11Renderer::createVertexBuffers()
{
    {
        D3D11_BUFFER_DESC bd{};
        bd.ByteWidth = sizeof(glm::mat4) * 3;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bd.Usage = D3D11_USAGE_DYNAMIC;

        HRESULT result = m_device->CreateBuffer(&bd, nullptr, &m_matrixBuffer);
        checkHresult(result);
    }

    {
        D3D11_BUFFER_DESC bd{};
        bd.Usage = D3D11_USAGE_IMMUTABLE;
        bd.ByteWidth = static_cast<UINT>(sizeof(float) * c_backgroundVertexData.size());
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA data{};
        data.pSysMem = c_backgroundVertexData.data();

        HRESULT result = m_device->CreateBuffer(&bd, &data, &m_backgroundVertexBuffer);
        checkHresult(result);
    }
}

void DX11Renderer::createTextures()
{
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = 0;
    desc.Height = 0;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    const std::string assetsPath = ASSETS_PATH;
    const std::vector<std::string> textureNames{assetsPath + "logo.png"};

    const size_t numTextures = textureNames.size();
    m_textures.resize(numTextures);
    m_textureSrvs.resize(numTextures);
    for (size_t i = 0; i < numTextures; ++i)
    {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(textureNames[i].c_str(), &texWidth, &texHeight, &texChannels, 4);
        assert(pixels != nullptr);

        desc.Width = texWidth;
        desc.Height = texHeight;
        UINT rowSizeInBytes = texWidth * texChannels;
        UINT imageSizeInBytes = texWidth * texHeight * texChannels;

        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = pixels;
        initData.SysMemPitch = rowSizeInBytes;
        initData.SysMemSlicePitch = imageSizeInBytes;

        HRESULT result = m_device->CreateTexture2D(&desc, &initData, &m_textures[i]);

        stbi_image_free(pixels);

        checkHresult(result);

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        result = m_device->CreateShaderResourceView(m_textures[i], &srvDesc, &m_textureSrvs[i]);
        checkHresult(result);
    }

    D3D11_SAMPLER_DESC sampDesc{};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    HRESULT result = m_device->CreateSamplerState(&sampDesc, &m_linearSampler);
    checkHresult(result);
}

void DX11Renderer::setSwapchainIndex(int index)
{
    m_currentIndex = index;
}

void DX11Renderer::bindRenderTarget()
{
    m_currentRenderTarget = &m_renderTargets[m_currentIndex];
    m_deviceContext->OMSetRenderTargets(1, &m_currentRenderTarget->rtv, nullptr);
}

void DX11Renderer::clear(float r, float g, float b, float a)
{
    const float color[] = {r, g, b, a};
    m_deviceContext->ClearRenderTargetView(m_currentRenderTarget->rtv, color);
}

void DX11Renderer::setViewport(int /*index*/, int32_t x, int32_t y, int32_t width, int32_t height)
{
    D3D11_VIEWPORT viewport{};
    viewport.TopLeftX = static_cast<float>(x);
    viewport.TopLeftY = static_cast<float>(y);
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1;

    m_deviceContext->RSSetViewports(1, &viewport);
}

void DX11Renderer::setViewAndProjectionMatrix(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    m_deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
    glm::mat4* matrixData = (glm::mat4*)MappedResource.pData;
    matrixData[0] = glm::mat4(1.0f);
    matrixData[1] = viewMatrix;
    matrixData[2] = projectionMatrix;
    m_deviceContext->Unmap(m_matrixBuffer, 0);
}

void DX11Renderer::render()
{
    m_deviceContext->IASetInputLayout(m_shader.getIputLayout());
    m_deviceContext->IASetVertexBuffers(0, 1, &m_backgroundVertexBuffer, &c_vertexStride, &c_vertexOffset);
    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    m_deviceContext->VSSetShader(m_shader.getVertexShader(), nullptr, 0);
    m_deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);
    m_deviceContext->RSSetState(m_rasterState);
    m_deviceContext->PSSetShader(m_shader.getPixelShader(), nullptr, 0);
    m_deviceContext->PSSetShaderResources(0, static_cast<UINT>(m_textureSrvs.size()), m_textureSrvs.data());
    m_deviceContext->PSSetSamplers(0, 1, &m_linearSampler);
    m_deviceContext->Draw(4, 0);
}

void DX11Renderer::updateWindow()
{
    const float color[] = {0.2f, 0.2f, 0.2f, 1.0f};
    m_deviceContext->ClearRenderTargetView(m_windowRtv, color);
    m_window.present();
}

void DX11Renderer::addRenderTarget(ID3D11Texture2D* rtvTexture)
{
    m_renderTargets.resize(m_renderTargets.size() + 1);
    RenderTarget& renderTarget = m_renderTargets.back();

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;

    renderTarget.rtvTexture = rtvTexture;
    HRESULT hr = m_device->CreateRenderTargetView(rtvTexture, &rtvDesc, &renderTarget.rtv);
    checkHresult(hr);
}

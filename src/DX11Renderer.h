#pragma once

#include "Window.h"
#include "Shader.h"

#include <glm/glm.hpp>
#include <Varjo.h>
#include <Varjo_Layers.h>

#include <d3d11.h>
#include <wrl/client.h>

#include <vector>

class DX11Renderer
{
public:
    DX11Renderer();
    ~DX11Renderer();

    void createDevice(Microsoft::WRL::ComPtr<IDXGIAdapter> adapter);
    void createSwapChain(varjo_Session* session, int length, int width, int height, varjo_SwapChain*& colorSwapChainOut);
    void createWindow(HINSTANCE hInstance, int nCmdShow);
    void createDefaultShader();
    void createVertexBuffers();
    void createTextures();

    void setSwapchainIndex(int index);
    void bindRenderTarget();
    void clear(float r, float g, float b, float a);
    void setViewport(int index, int32_t x, int32_t y, int32_t width, int32_t height);
    void setViewAndProjectionMatrix(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
    void render();
    void updateWindow();

private:
    struct RenderTarget
    {
        ID3D11Texture2D* rtvTexture = nullptr;
        ID3D11RenderTargetView* rtv = nullptr;
    };

    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_deviceContext = nullptr;

    Window m_window;
    ID3D11RenderTargetView* m_windowRtv = nullptr;

    Shader m_shader;
    ID3D11RasterizerState* m_rasterState = nullptr;
    ID3D11Buffer* m_matrixBuffer = nullptr;
    ID3D11Buffer* m_backgroundVertexBuffer = nullptr;

    int m_currentIndex = -1;
    std::vector<RenderTarget> m_renderTargets;
    RenderTarget* m_currentRenderTarget = nullptr;

    std::vector<ID3D11Texture2D*> m_textures;
    std::vector<ID3D11ShaderResourceView*> m_textureSrvs;
    ID3D11SamplerState* m_linearSampler;

    void addRenderTarget(ID3D11Texture2D* rtvTexture);
};
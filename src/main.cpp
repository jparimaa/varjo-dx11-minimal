#include "Log.h"
#include "Utility.h"
#include "DX11Renderer.h"

#include <Varjo.h>
#include <Varjo_d3d11.h>
#include <Varjo_types_d3d11.h>
#include <Varjo_types_layers.h>
#include <Varjo_layers.h>
#include <Varjo_math.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <wrl/client.h>
#include <stdlib.h>
#include <crtdbg.h>

#include <cstdlib>
#include <iostream>
#include <vector>

const int SWAP_CHAIN_SIZE = 3;
const double NEAR_CLIP_DISTANCE = 0.1;
const double FAR_CLIP_DISTANCE = 1000.0;

std::vector<varjo_Viewport> createViewportsWithViewDescription(varjo_Session* session)
{
    std::vector<varjo_Viewport> viewports;
    const int32_t viewCount = varjo_GetViewCount(session);
    int x = 0;
    int y = 0;
    for (int32_t i = 0; i < viewCount; ++i)
    {
        const varjo_ViewDescription viewDesc = varjo_GetViewDescription(session, i);
        const varjo_Viewport viewport = varjo_Viewport{x, y, viewDesc.width, viewDesc.height};
        viewports.push_back(viewport);
        x += viewport.width;
        if (i == 1)
        {
            x = 0;
            y += viewport.height;
        }
    }
    return viewports;
}

std::vector<varjo_LayerMultiProjView> getMultiProjViews(varjo_Session* session,
                                                        varjo_SwapChain* swapChain,
                                                        const std::vector<varjo_Viewport>& viewports)
{
    std::vector<varjo_LayerMultiProjView> multiProjectionViews;
    const int32_t viewCount = varjo_GetViewCount(session);
    multiProjectionViews.resize(viewCount);
    for (int32_t i = 0; i < viewCount; ++i)
    {
        multiProjectionViews[i].viewport = varjo_SwapChainViewport{swapChain, viewports[i].x, viewports[i].y, viewports[i].width, viewports[i].height, 0};
    }

    return multiProjectionViews;
}

glm::mat4 toGlmMat4(const double* matrix)
{
    glm::mat4 result;
    float* dst = glm::value_ptr(result);
    for (int i = 0; i < 16; ++i)
    {
        dst[i] = static_cast<float>(matrix[i]);
    }
    return result;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPWSTR /*lpCmdLine*/, int nCmdShow)
{
    ConsoleOutput consoleOutput;

    if (!varjo_IsAvailable())
    {
        std::cerr << "ERROR: Varjo system not available.\n";
        abort();
    }

    varjo_Session* session = varjo_SessionInit();

    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter = getAdapter(varjo_D3D11GetLuid(session));

    const std::vector<varjo_Viewport> viewports = createViewportsWithViewDescription(session);

    varjo_SwapChain* colorSwapChain = nullptr;
    int width = viewports[0].width + viewports[1].width;
    int height = viewports[0].height + viewports[2].height;

    DX11Renderer renderer;
    renderer.createDevice(adapter);
    renderer.createSwapChain(session, SWAP_CHAIN_SIZE, width, height, colorSwapChain);
    renderer.createWindow(hInstance, nCmdShow);
    renderer.createDefaultShader();
    renderer.createVertexBuffers();
    renderer.createTextures();

    std::vector<varjo_LayerMultiProjView> renderLayerViews = getMultiProjViews(session, colorSwapChain, viewports);

    varjo_FrameInfo* frameInfo = varjo_CreateFrameInfo(session);
    int32_t swapChainIndex;
    const int32_t viewCount = varjo_GetViewCount(session);

    MSG msg{};
    while (!isEscDown() && WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        varjo_WaitSync(session, frameInfo);
        varjo_BeginFrameWithLayers(session);

        varjo_AcquireSwapChainImage(colorSwapChain, &swapChainIndex);

        renderer.setSwapchainIndex(swapChainIndex);
        renderer.bindRenderTarget();
        renderer.clear(0.0f, 0.0f, 0.5f, 1.0f);

        for (int32_t i = 0; i < viewCount; ++i)
        {
            varjo_ViewInfo& view = frameInfo->views[i];
            const varjo_Viewport viewport = viewports[i];
            renderer.setViewport(i, viewport.x, viewport.y, viewport.width, viewport.height);

            varjo_UpdateNearFarPlanes(view.projectionMatrix, varjo_ClipRangeZeroToOne, NEAR_CLIP_DISTANCE, FAR_CLIP_DISTANCE);

            glm::mat4 viewMatrix = toGlmMat4(view.viewMatrix);
            glm::mat4 projectionMatrix = toGlmMat4(view.projectionMatrix);

            renderer.bindRenderTarget();
            renderer.setViewAndProjectionMatrix(glm::transpose(viewMatrix), glm::transpose(projectionMatrix));
            renderer.render();

            std::copy(view.projectionMatrix, view.projectionMatrix + 16, renderLayerViews[i].projection.value);
            std::copy(view.viewMatrix, view.viewMatrix + 16, renderLayerViews[i].view.value);
        }

        renderer.updateWindow();

        varjo_ReleaseSwapChainImage(colorSwapChain);

        varjo_LayerMultiProj renderLayer;
        renderLayer.header.type = varjo_LayerMultiProjType;
        renderLayer.header.flags = 0;
        renderLayer.space = varjo_SpaceLocal;
        renderLayer.viewCount = viewCount;
        renderLayer.views = renderLayerViews.data();

        std::vector<varjo_LayerHeader*> layers{&renderLayer.header};

        varjo_SubmitInfoLayers submitInfoLayers{};
        submitInfoLayers.frameNumber = frameInfo->frameNumber;
        submitInfoLayers.layerCount = static_cast<int32_t>(layers.size());
        submitInfoLayers.layers = layers.data();

        varjo_EndFrameWithLayers(session, &submitInfoLayers);

        checkVarjoError(session);
    }

    varjo_FreeFrameInfo(frameInfo);
    varjo_FreeSwapChain(colorSwapChain);
    varjo_SessionShutDown(session);

    return 0;
}
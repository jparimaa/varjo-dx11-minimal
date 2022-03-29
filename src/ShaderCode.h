#pragma once

#include "Log.h"
#include "Utility.h"

#include <d3dcompiler.h>

inline ID3DBlob* compileShader(const char* src, const char* target, const char* name)
{
    ID3DBlob* compiledShader = nullptr;
    ID3DBlob* compilerMsgs = nullptr;
    HRESULT result = D3DCompile(src, strlen(src) + 1, name, nullptr, nullptr, "main", target, D3DCOMPILE_SKIP_OPTIMIZATION, 0, &compiledShader, &compilerMsgs);

    if (compilerMsgs != nullptr)
    {
        printf("Compiler errors: '%.*s'\n", static_cast<int>(compilerMsgs->GetBufferSize()), reinterpret_cast<char*>(compilerMsgs->GetBufferPointer()));
        compilerMsgs->Release();
    }

    checkHresult(result);

    return compiledShader;
}

inline ID3DBlob* getVertexShader()
{
    return compileShader(
        R"(
			cbuffer ConstantBuffer : register(b0)
			{
				matrix World;
				matrix View;
				matrix Projection;
			}

			struct VSInput
			{
				float4 Position : POSITION;
				float2 UV : TEXCOORD;
			};

			struct VSOutput
			{
				float4 Position : SV_POSITION;
				float2 UV : TEXCOORD;
			};

			VSOutput main(VSInput input)
			{
				VSOutput output;
				output.Position = mul(input.Position, World);
				output.Position = mul(output.Position, View);
				output.Position = mul(output.Position, Projection);	
				output.UV = input.UV;
				return output;
			}
		)",
        "vs_4_0",
        "vertex");
}

inline ID3DBlob* getPixelShader()
{
    return compileShader(
        R"(
			Texture2D g_texture : register(t0);
			SamplerState g_sampler : register(s0);

			struct PSInput
			{
				float4 Position : SV_POSITION;
				float2 UV : TEXCOORD;
			};

			float4 main(PSInput input) : SV_Target
			{
				return g_texture.Sample(g_sampler, input.UV);
			}
		)",
        "ps_4_0",
        "pixel");
}

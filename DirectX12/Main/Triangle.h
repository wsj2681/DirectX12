#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"

using namespace Microsoft::WRL;
using namespace DirectX;

class Triangle 
{
public:
    Triangle(ID3D12Device* device);
    ~Triangle();

    void Initialize();
    void Render(ID3D12GraphicsCommandList* commandList);

private:
    void CompileShaders();
    void CreatePipelineState(ID3D12Device* device);

    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;

    ComPtr<ID3D12Device> device;
    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> pipelineState;
};
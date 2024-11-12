#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <vector>
#include <string>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

using namespace Microsoft::WRL;
using namespace DirectX;

template<typename VertexType>
class Mesh {
public:
    Mesh(ID3D12Device* device, const std::vector<VertexType>& vertices, const std::vector<uint16_t>& indices);
    ~Mesh() = default;

    bool Initialize(const std::wstring& vertexShaderPath, const std::wstring& pixelShaderPath);
    void Render(ID3D12GraphicsCommandList* commandList);

private:
    void CreateVertexBuffer(ID3D12Device* device, const std::vector<VertexType>& vertices);
    void CreateIndexBuffer(ID3D12Device* device, const std::vector<uint16_t>& indices);
    bool CompileShadersAndCreatePipelineState(ID3D12Device* device, const std::wstring& vertexShaderPath, const std::wstring& pixelShaderPath);

    ComPtr<ID3D12Device> device;

    ComPtr<ID3D12Resource> vertexBuffer;
    ComPtr<ID3D12Resource> indexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;
    UINT indexCount;

    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> pipelineState;
};

template<typename VertexType>
Mesh<VertexType>::Mesh(ID3D12Device* device, const std::vector<VertexType>& vertices, const std::vector<uint16_t>& indices)
    : device(device), indexCount(static_cast<UINT>(indices.size()))
{
    CreateVertexBuffer(device, vertices);
    CreateIndexBuffer(device, indices);
}

template<typename VertexType>
void Mesh<VertexType>::CreateVertexBuffer(ID3D12Device* device, const std::vector<VertexType>& vertices)
{
    const UINT bufferSize = static_cast<UINT>(sizeof(VertexType) * vertices.size());

    CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertexBuffer));

    void* mappedData = nullptr;
    CD3DX12_RANGE readRange(0, 0);
    vertexBuffer->Map(0, &readRange, &mappedData);
    memcpy(mappedData, vertices.data(), bufferSize);
    vertexBuffer->Unmap(0, nullptr);

    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = sizeof(VertexType);
    vertexBufferView.SizeInBytes = bufferSize;
}

template<typename VertexType>
void Mesh<VertexType>::CreateIndexBuffer(ID3D12Device* device, const std::vector<uint16_t>& indices)
{
    const UINT bufferSize = static_cast<UINT>(sizeof(uint16_t) * indices.size());

    CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&indexBuffer));

    void* mappedData = nullptr;
    CD3DX12_RANGE readRange(0, 0);
    indexBuffer->Map(0, &readRange, &mappedData);
    memcpy(mappedData, indices.data(), bufferSize);
    indexBuffer->Unmap(0, nullptr);

    indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
    indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    indexBufferView.SizeInBytes = bufferSize;
}

template<typename VertexType>
bool Mesh<VertexType>::CompileShadersAndCreatePipelineState(ID3D12Device* device, const std::wstring& vertexShaderPath, const std::wstring& pixelShaderPath)
{
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;
    ComPtr<ID3DBlob> error;

    HRESULT hr = D3DCompileFromFile(vertexShaderPath.c_str(), nullptr, nullptr, "VSMain", "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vertexShader, &error);
    if (FAILED(hr)) {
        if (error) OutputDebugStringA((char*)error->GetBufferPointer());
        return false;
    }

    hr = D3DCompileFromFile(pixelShaderPath.c_str(), nullptr, nullptr, "PSMain", "ps_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelShader, &error);
    if (FAILED(hr)) {
        if (error) OutputDebugStringA((char*)error->GetBufferPointer());
        return false;
    }

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signature;
    hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
    if (FAILED(hr)) return false;

    hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature));
    if (FAILED(hr)) return false;

    D3D12_INPUT_ELEMENT_DESC inputLayout[] = 
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.pRootSignature = rootSignature.Get();
    psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));
    return SUCCEEDED(hr);
}

template<typename VertexType>
bool Mesh<VertexType>::Initialize(const std::wstring& vertexShaderPath, const std::wstring& pixelShaderPath)
{
    return CompileShadersAndCreatePipelineState(device.Get(), vertexShaderPath, pixelShaderPath);
}

template<typename VertexType>
void Mesh<VertexType>::Render(ID3D12GraphicsCommandList* commandList)
{
    commandList->SetGraphicsRootSignature(rootSignature.Get());
    commandList->SetPipelineState(pipelineState.Get());
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
    commandList->IASetIndexBuffer(&indexBufferView);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
}

#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <vector>
#include "d3dx12.h"

using namespace Microsoft::WRL;
using namespace DirectX;

struct Vertex 
{
    XMFLOAT3 position; // 정점 위치
    XMFLOAT3 color;    // 정점 색상
};

class Mesh 
{
public:
    Mesh() = default;
    Mesh(ID3D12Device* device, const std::vector<Vertex>& vertices);
    ~Mesh();

    void Render(ID3D12GraphicsCommandList* commandList);

private:
    void CreateVertexBuffer(ID3D12Device* device, const std::vector<Vertex>& vertices);
    bool CompileShadersAndCreatePipelineState(ID3D12Device* device);

    ComPtr<ID3D12Resource> vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    UINT vertexCount;

    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> pipelineState;

};
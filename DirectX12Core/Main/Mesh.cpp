#include "framework.h"
#include "Mesh.h"

Mesh::Mesh(ID3D12Device* device, const vector<Vertex>& vertices, const vector<uint16_t>& indices)
    : indexCount_(static_cast<UINT>(indices.size())) 
{
    CreateVertexBuffer(device, vertices);
    CreateIndexBuffer(device, indices);
}

void Mesh::CreateVertexBuffer(ID3D12Device* device, const vector<Vertex>& vertices) 
{
    // ���ؽ� ���� ���� �� �ʱ�ȭ
    UINT bufferSize = static_cast<UINT>(vertices.size() * sizeof(Vertex));
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertexBuffer));

    // ���� ������ ����
    Vertex* mappedData = nullptr;
    CD3DX12_RANGE readRange(0, 0);
    vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedData));
    memcpy(mappedData, vertices.data(), bufferSize);
    vertexBuffer->Unmap(0, nullptr);

    // �� ����
    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = sizeof(Vertex);
    vertexBufferView.SizeInBytes = bufferSize;
}

void Mesh::CreateIndexBuffer(ID3D12Device* device, const vector<uint16_t>& indices) 
{
    // �ε��� ���� ���� �� �ʱ�ȭ
    UINT bufferSize = static_cast<UINT>(indices.size() * sizeof(uint16_t));
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&indexBuffer));

    // ���� ������ ����
    uint16_t* mappedData = nullptr;
    CD3DX12_RANGE readRange(0, 0);
    indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedData));
    memcpy(mappedData, indices.data(), bufferSize);
    indexBuffer->Unmap(0, nullptr);

    // �� ����
    indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
    indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    indexBufferView.SizeInBytes = bufferSize;
}

void Mesh::Render(ID3D12GraphicsCommandList* commandList) 
{
    // ���� �� ���� �� ��ο� ȣ��
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
    commandList->IASetIndexBuffer(&indexBufferView);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawIndexedInstanced(indexCount_, 1, 0, 0, 0);
}
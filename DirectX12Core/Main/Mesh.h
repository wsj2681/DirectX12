#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <vector>
#include "d3dx12.h"

using Microsoft::WRL::ComPtr;
using namespace std;

struct Vertex {
    float position[3];
    float color[4];
};

class Mesh {
public:
    Mesh(ID3D12Device* device, const vector<Vertex>& vertices, const vector<uint16_t>& indices);
    ~Mesh() = default;

    void Render(ID3D12GraphicsCommandList* commandList);

private:
    void CreateVertexBuffer(ID3D12Device* device, const vector<Vertex>& vertices);
    void CreateIndexBuffer(ID3D12Device* device, const vector<uint16_t>& indices);

    ComPtr<ID3D12Resource> vertexBuffer;
    ComPtr<ID3D12Resource> indexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;
    UINT indexCount_;
};

//HR
#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x) \
      if (FAILED((x))) \
      { \
         LPVOID errorLog = nullptr; \
         FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, \
            nullptr, (x), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), \
            reinterpret_cast<LPWSTR>(&errorLog), 0, nullptr); \
         fprintf(stderr, "%s", static_cast<char*>(errorLog)); \
         LocalFree(errorLog); \
         __debugbreak(); \
      }
#endif
#else
#ifndef HR
#define   HR(x) (x);
#endif
#endif
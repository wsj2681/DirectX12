#pragma once

#include "DX12Device.h"
#include "Mesh.h"
#include "Camera.h"
#include <wrl.h>
#include <d3d12.h>
#include <d3dcompiler.h>

using Microsoft::WRL::ComPtr;

class DX12Renderer 
{
public:
    DX12Renderer(DX12Device* device);
    ~DX12Renderer();

    bool Initialize();
    void Render();

private:
    bool CreateCommandObjects();
    bool CreateRenderTarget();
    bool CreatePipelineState();
    bool CreateConstantBuffer();

    struct MVPMatrix
    {
        XMMATRIX world;
        XMMATRIX view;
        XMMATRIX projection;
    };

    DX12Device* device;                       // DX12 디바이스 참조
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ComPtr<ID3D12GraphicsCommandList> commandList;
    ComPtr<ID3D12PipelineState> pipelineState_;
    ComPtr<ID3D12RootSignature> rootSignature_;

    ComPtr<ID3D12Resource> renderTargets[2];
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    UINT rtvDescriptorSize;
    UINT currentBackBufferIndex;

    unique_ptr<Mesh> mesh;
    unique_ptr<Camera> camera;

    XMMATRIX worldMatrix;
    ComPtr<ID3D12Resource> mvpBuffer;

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
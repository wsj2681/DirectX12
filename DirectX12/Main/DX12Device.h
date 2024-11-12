#pragma once
#define FRAMEBUFFER_WIDTH 800
#define FRAMEBUFFER_HEIGHT 600
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <string>
#include "d3dx12.h"

using Microsoft::WRL::ComPtr;

class DX12Device
{
public:
    DX12Device(HWND hWnd);
    ~DX12Device();

    bool Initialize();
    void Present();

    ComPtr<IDXGISwapChain4>& GetSwapChain();
    ComPtr<ID3D12Device>& GetDevice();
    ComPtr<ID3D12CommandQueue>& GetCommandQueue();

private:
    bool CreateDevice();
    bool CreateCommandQueue();
    bool CreateSwapChain();
    bool CreateRenderTargetViews();

    HWND hWnd;
    ComPtr<ID3D12Device> device;
    ComPtr<ID3D12CommandQueue> commandQueue;
    ComPtr<IDXGISwapChain4> swapChain;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ComPtr<ID3D12Resource> renderTargets[2];
    UINT rtvDescriptorSize;
    UINT currentBackBufferIndex;
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
#pragma once

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
#pragma once

class DX12Device
{
public:
    DX12Device(HWND hWnd);
    ~DX12Device();

    void Initialize(HWND hWnd);
    void ClearBackBuffer();
    void Render();

private:
    static constexpr UINT FrameCount = 2;

    ComPtr<ID3D12Device> device;
    ComPtr<ID3D12CommandQueue> commandQueue;
    ComPtr<IDXGISwapChain4> swapchain;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ComPtr<ID3D12Resource> renderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ComPtr<ID3D12GraphicsCommandList> commandList;

    ComPtr<ID3D12Fence> fence;
    UINT64 fenceValue = 0;
    HANDLE fenceEvent = nullptr;

    UINT rtvDescriptorSize = 0;
    UINT frameIndex = 0;

    void WaitForGpu();
};
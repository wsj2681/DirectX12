#include "DX12Device.h"
#include <stdexcept>

DX12Device::DX12Device(HWND hWnd)
    : hWnd(hWnd), rtvDescriptorSize(0), currentBackBufferIndex(0)
{
}

DX12Device::~DX12Device()
{
    // DirectX 자원을 해제하는 소멸자 처리
}

bool DX12Device::Initialize()
{
    if (!CreateDevice()) return false;
    if (!CreateCommandQueue()) return false;
    if (!CreateSwapChain()) return false;
    if (!CreateRenderTargetViews()) return false;
    return true;
}

bool DX12Device::CreateDevice()
{
    // DXGI 팩토리 생성
    ComPtr<IDXGIFactory4> dxgiFactory;
    HR(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

    // 디바이스 생성
    HR(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));

    return true;
}

bool DX12Device::CreateCommandQueue()
{
    // 커맨드 큐 생성
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    HR(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&this->commandQueue)));

    return true;
}

bool DX12Device::CreateSwapChain()
{
    // DXGI 팩토리 생성
    ComPtr<IDXGIFactory4> dxgiFactory;
    HR(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

    // 스왑 체인 설정
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Width = FRAMEBUFFER_WIDTH;  // 기본 너비 (사용자 설정 가능)
    swapChainDesc.Height = FRAMEBUFFER_HEIGHT; // 기본 높이 (사용자 설정 가능)
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    ComPtr<IDXGISwapChain1> tempswapChain;
    HR(dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &swapChainDesc, nullptr, nullptr, &tempswapChain));
    dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

    // IDXGISwapChain4로 캐스팅
    HR(tempswapChain.As(&this->swapChain));

    currentBackBufferIndex = this->swapChain->GetCurrentBackBufferIndex();
    return true;
}

bool DX12Device::CreateRenderTargetViews()
{
    // RTV 힙 생성
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HR(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)));

    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // 백 버퍼에 대한 RTV 생성
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < 2; ++i)
    {
        HR(swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i])));

        device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, rtvDescriptorSize);
    }

    return true;
}

void DX12Device::Present()
{
    swapChain->Present(1, 0);
    currentBackBufferIndex = swapChain->GetCurrentBackBufferIndex();
}

ComPtr<IDXGISwapChain4>& DX12Device::GetSwapChain()
{
    return this->swapChain;
}

ComPtr<ID3D12Device>& DX12Device::GetDevice()
{
    return this->device;
}

ComPtr<ID3D12CommandQueue>& DX12Device::GetCommandQueue()
{
    return commandQueue;
}

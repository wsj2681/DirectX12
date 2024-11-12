#include "DX12Device.h"
#include <stdexcept>

DX12Device::DX12Device(HWND hWnd)
    : hWnd(hWnd), rtvDescriptorSize(0), currentBackBufferIndex(0) 
{
}

DX12Device::~DX12Device() 
{
    // DirectX �ڿ��� �����ϴ� �Ҹ��� ó��
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
    // DXGI ���丮 ����
    ComPtr<IDXGIFactory4> dxgiFactory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)))) 
    {
        return false;
    }

    // ����̽� ����
    if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)))) 
    {
        return false;
    }

    return true;
}

bool DX12Device::CreateCommandQueue() 
{
    // Ŀ�ǵ� ť ����
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    if (FAILED(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&this->commandQueue)))) 
    {
        return false;
    }

    return true;
}

bool DX12Device::CreateSwapChain() 
{
    // DXGI ���丮 ����
    ComPtr<IDXGIFactory4> dxgiFactory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)))) 
    {
        return false;
    }

    // ���� ü�� ����
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Width = 800;  // �⺻ �ʺ� (����� ���� ����)
    swapChainDesc.Height = 600; // �⺻ ���� (����� ���� ����)
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> tempswapChain;
    if (FAILED(dxgiFactory->CreateSwapChainForHwnd(
        commandQueue.Get(), hWnd, &swapChainDesc, nullptr, nullptr, &tempswapChain))) 
    {
        return false;
    }

    // IDXGISwapChain4�� ĳ����
    if (FAILED(tempswapChain.As(&this->swapChain))) 
    {
        return false;
    }

    currentBackBufferIndex = this->swapChain->GetCurrentBackBufferIndex();
    return true;
}

bool DX12Device::CreateRenderTargetViews() 
{
    // RTV �� ����
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    if (FAILED(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)))) 
    {
        return false;
    }

    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // �� ���ۿ� ���� RTV ����
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < 2; ++i) 
    {
        if (FAILED(swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i])))) 
        {
            return false;
        }
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
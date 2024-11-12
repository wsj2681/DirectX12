#include "DX12Renderer.h"

DX12Renderer::DX12Renderer(DX12Device* device)
    : device_(device), rtvDescriptorSize_(0), currentBackBufferIndex_(0) 
{
}

DX12Renderer::~DX12Renderer() 
{
    // ������ ���ҽ� ����
}

bool DX12Renderer::Initialize() 
{
    if (!CreateCommandObjects()) return false;
    if (!CreateRenderTarget()) return false;
    return true;
}

bool DX12Renderer::CreateCommandObjects() 
{
    // Ŀ�ǵ� �Ҵ��� ����
    HR(device_->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_)));

    // �׷��� Ŀ�ǵ� ����Ʈ ����
    HR(device_->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_)));

    commandList_->Close(); // �ʱ�ȭ �� Ŀ�ǵ� ����Ʈ�� �ݾƵӴϴ�.
    return true;
}

bool DX12Renderer::CreateRenderTarget() 
{
    // RTV �� ����
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    HR(device_->GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap_)));

    rtvDescriptorSize_ = device_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // ���� ü���� �� ���ۿ� ���� ���� Ÿ�� �� ����
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap_->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < 2; ++i) 
    {
        device_->GetSwapChain()->GetBuffer(i, IID_PPV_ARGS(&renderTargets_[i]));
        device_->GetDevice()->CreateRenderTargetView(renderTargets_[i].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, rtvDescriptorSize_);
    }

    return true;
}

void DX12Renderer::Render() 
{
    // Ŀ�ǵ� �Ҵ��ڿ� Ŀ�ǵ� ����Ʈ �ʱ�ȭ
    commandAllocator_->Reset();
    commandList_->Reset(commandAllocator_.Get(), nullptr);

    // ���� Ÿ�� ���� �� Ŭ����
    currentBackBufferIndex_ = device_->GetSwapChain()->GetCurrentBackBufferIndex();
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets_[currentBackBufferIndex_].Get(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList_->ResourceBarrier(1, &barrier);

    // ���� Ÿ�� �� ����
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap_->GetCPUDescriptorHandleForHeapStart(),
        currentBackBufferIndex_, rtvDescriptorSize_);
    commandList_->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // ȭ�� Ŭ����
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    commandList_->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // ���� Ÿ���� �ٽ� Present ���·� ��ȯ
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets_[currentBackBufferIndex_].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    commandList_->ResourceBarrier(1, &barrier);

    // Ŀ�ǵ� ����Ʈ �ݰ� ����
    commandList_->Close();
    ID3D12CommandList* ppCommandLists[] = { commandList_.Get() };
    device_->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // ȭ�鿡 ǥ��
    device_->GetSwapChain()->Present(1, 0);
}
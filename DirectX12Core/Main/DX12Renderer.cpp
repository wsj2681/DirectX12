#include "DX12Renderer.h"

DX12Renderer::DX12Renderer(DX12Device* device)
    : device(device), rtvDescriptorSize(0), currentBackBufferIndex(0) 
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
    HR(device->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));

    // �׷��� Ŀ�ǵ� ����Ʈ ����
    HR(device->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

    commandList->Close(); // �ʱ�ȭ �� Ŀ�ǵ� ����Ʈ�� �ݾƵӴϴ�.
    return true;
}

bool DX12Renderer::CreateRenderTarget() 
{
    // RTV �� ����
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    HR(device->GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)));

    rtvDescriptorSize = device->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // ���� ü���� �� ���ۿ� ���� ���� Ÿ�� �� ����
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < 2; ++i) 
    {
        device->GetSwapChain()->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
        device->GetDevice()->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, rtvDescriptorSize);
    }

    return true;
}

void DX12Renderer::Render() 
{
    // Ŀ�ǵ� �Ҵ��ڿ� Ŀ�ǵ� ����Ʈ �ʱ�ȭ
    commandAllocator->Reset();
    commandList->Reset(commandAllocator.Get(), nullptr);

    // ���� Ÿ�� ���� �� Ŭ����
    currentBackBufferIndex = device->GetSwapChain()->GetCurrentBackBufferIndex();
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets[currentBackBufferIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &barrier);

    // ���� Ÿ�� �� ����
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(),
        currentBackBufferIndex, rtvDescriptorSize);
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // ȭ�� Ŭ����
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // ���� Ÿ���� �ٽ� Present ���·� ��ȯ
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets[currentBackBufferIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &barrier);

    // Ŀ�ǵ� ����Ʈ �ݰ� ����
    commandList->Close();
    ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
    device->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // ȭ�鿡 ǥ��
    device->GetSwapChain()->Present(1, 0);
}
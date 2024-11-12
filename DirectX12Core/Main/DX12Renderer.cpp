#include "DX12Renderer.h"

DX12Renderer::DX12Renderer(DX12Device* device)
    : device(device), rtvDescriptorSize(0), currentBackBufferIndex(0) 
{
}

DX12Renderer::~DX12Renderer() 
{
    // 렌더링 리소스 정리
}

bool DX12Renderer::Initialize() 
{
    if (!CreateCommandObjects()) return false;
    if (!CreateRenderTarget()) return false;
    return true;
}

bool DX12Renderer::CreateCommandObjects() 
{
    // 커맨드 할당자 생성
    HR(device->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));

    // 그래픽 커맨드 리스트 생성
    HR(device->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

    commandList->Close(); // 초기화 시 커맨드 리스트를 닫아둡니다.
    return true;
}

bool DX12Renderer::CreateRenderTarget() 
{
    // RTV 힙 생성
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    HR(device->GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)));

    rtvDescriptorSize = device->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // 스왑 체인의 백 버퍼에 대한 렌더 타겟 뷰 생성
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
    // 커맨드 할당자와 커맨드 리스트 초기화
    commandAllocator->Reset();
    commandList->Reset(commandAllocator.Get(), nullptr);

    // 렌더 타겟 설정 및 클리어
    currentBackBufferIndex = device->GetSwapChain()->GetCurrentBackBufferIndex();
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets[currentBackBufferIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &barrier);

    // 렌더 타겟 뷰 설정
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(),
        currentBackBufferIndex, rtvDescriptorSize);
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // 화면 클리어
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // 렌더 타겟을 다시 Present 상태로 전환
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets[currentBackBufferIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &barrier);

    // 커맨드 리스트 닫고 제출
    commandList->Close();
    ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
    device->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // 화면에 표시
    device->GetSwapChain()->Present(1, 0);
}
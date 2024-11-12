#include "DX12Renderer.h"

DX12Renderer::DX12Renderer(DX12Device* device)
    : device_(device), rtvDescriptorSize_(0), currentBackBufferIndex_(0) 
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
    HR(device_->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_)));

    // 그래픽 커맨드 리스트 생성
    HR(device_->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_)));

    commandList_->Close(); // 초기화 시 커맨드 리스트를 닫아둡니다.
    return true;
}

bool DX12Renderer::CreateRenderTarget() 
{
    // RTV 힙 생성
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    HR(device_->GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap_)));

    rtvDescriptorSize_ = device_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // 스왑 체인의 백 버퍼에 대한 렌더 타겟 뷰 생성
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
    // 커맨드 할당자와 커맨드 리스트 초기화
    commandAllocator_->Reset();
    commandList_->Reset(commandAllocator_.Get(), nullptr);

    // 렌더 타겟 설정 및 클리어
    currentBackBufferIndex_ = device_->GetSwapChain()->GetCurrentBackBufferIndex();
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets_[currentBackBufferIndex_].Get(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList_->ResourceBarrier(1, &barrier);

    // 렌더 타겟 뷰 설정
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap_->GetCPUDescriptorHandleForHeapStart(),
        currentBackBufferIndex_, rtvDescriptorSize_);
    commandList_->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // 화면 클리어
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    commandList_->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // 렌더 타겟을 다시 Present 상태로 전환
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets_[currentBackBufferIndex_].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    commandList_->ResourceBarrier(1, &barrier);

    // 커맨드 리스트 닫고 제출
    commandList_->Close();
    ID3D12CommandList* ppCommandLists[] = { commandList_.Get() };
    device_->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // 화면에 표시
    device_->GetSwapChain()->Present(1, 0);
}
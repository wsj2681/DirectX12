#include "DX12Renderer.h"
#include "d3dcompiler.h"

#pragma comment(lib, "d3dcompiler.lib")

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
    if (!CreateCommandObjects() || !CreateRenderTarget())
    {
        return false;
    }
    if (!CompileShadersAndCreatePipelineState())
    {
        return false;
    }
    return true;
}

bool DX12Renderer::CompileShadersAndCreatePipelineState()
{
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;
    ComPtr<ID3DBlob> error;

    // Compile Vertex Shader
    HRESULT hr = D3DCompileFromFile(L"Shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vertexShader, &error);
    if (FAILED(hr))
    {
        if (error) OutputDebugStringA((char*)error->GetBufferPointer());
        return false;
    }

    // Compile Pixel Shader
    hr = D3DCompileFromFile(L"Shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelShader, &error);
    if (FAILED(hr))
    {
        if (error) OutputDebugStringA((char*)error->GetBufferPointer());
        return false;
    }

    // Describe and create the root signature
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signature;
    D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
    device->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature));

    // Create the pipeline state, which includes compiling and loading shaders
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { nullptr, 0 }; // No input layout
    psoDesc.pRootSignature = rootSignature.Get();
    psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    return SUCCEEDED(device->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));
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
        if (renderTargets[i]) 
        {
            renderTargets[i]->Release();
            renderTargets[i].Reset();
        }

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
    //commandList->Reset(commandAllocator.Get(), nullptr);
    commandList->Reset(commandAllocator.Get(), pipelineState.Get());

    D3D12_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(FRAMEBUFFER_WIDTH); // 프레임버퍼 너비
    viewport.Height = static_cast<float>(FRAMEBUFFER_HEIGHT); // 프레임버퍼 높이
    viewport.MaxDepth = 1.0f;

    D3D12_RECT scissorRect = {};
    scissorRect.right = FRAMEBUFFER_WIDTH;
    scissorRect.bottom = FRAMEBUFFER_HEIGHT;

    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

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

    
    commandList->SetGraphicsRootSignature(rootSignature.Get());
    commandList->SetPipelineState(pipelineState.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList->DrawInstanced(3, 1, 0, 0);

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

void DX12Renderer::ToggleFullscreen()
{
    // 현재 상태를 반전하여 전체 화면 상태 변경
    isFullscreen = !isFullscreen;
    if (device && device->GetSwapChain())
    {
        // 전체 화면 상태를 전환
        device->GetSwapChain()->SetFullscreenState(isFullscreen, nullptr);

        // 전체 화면 모드 전환 시 해상도 조정
        DXGI_MODE_DESC fullscreenDesc = {};
        fullscreenDesc.Width = FRAMEBUFFER_WIDTH;         // 원하는 해상도 설정
        fullscreenDesc.Height = FRAMEBUFFER_HEIGHT;
        fullscreenDesc.RefreshRate.Numerator = 60;        // 새로 고침 속도 설정
        fullscreenDesc.RefreshRate.Denominator = 1;
        fullscreenDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        fullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        fullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

        // 전체 화면 모드로 전환할 때 해상도 조정
        device->GetSwapChain()->ResizeTarget(&fullscreenDesc);

        // 스왑 체인 버퍼 크기 재조정 (백 버퍼 초기화)
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        device->GetSwapChain()->GetDesc(&swapChainDesc);
        device->GetSwapChain()->ResizeBuffers(
            2, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT,
            swapChainDesc.BufferDesc.Format, swapChainDesc.Flags
        );

        // 렌더 타겟 뷰 생성 (백 버퍼가 변경되었기 때문에 다시 생성 필요)
        CreateRenderTarget();
    }
}

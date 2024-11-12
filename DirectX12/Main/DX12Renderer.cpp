#include "DX12Renderer.h"
#include "d3dcompiler.h"

#pragma comment(lib, "d3dcompiler.lib")

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
    // Ŀ�ǵ� �Ҵ��ڿ� Ŀ�ǵ� ����Ʈ �ʱ�ȭ
    commandAllocator->Reset();
    //commandList->Reset(commandAllocator.Get(), nullptr);
    commandList->Reset(commandAllocator.Get(), pipelineState.Get());

    D3D12_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(FRAMEBUFFER_WIDTH); // �����ӹ��� �ʺ�
    viewport.Height = static_cast<float>(FRAMEBUFFER_HEIGHT); // �����ӹ��� ����
    viewport.MaxDepth = 1.0f;

    D3D12_RECT scissorRect = {};
    scissorRect.right = FRAMEBUFFER_WIDTH;
    scissorRect.bottom = FRAMEBUFFER_HEIGHT;

    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

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

    
    commandList->SetGraphicsRootSignature(rootSignature.Get());
    commandList->SetPipelineState(pipelineState.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList->DrawInstanced(3, 1, 0, 0);

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

void DX12Renderer::ToggleFullscreen()
{
    // ���� ���¸� �����Ͽ� ��ü ȭ�� ���� ����
    isFullscreen = !isFullscreen;
    if (device && device->GetSwapChain())
    {
        // ��ü ȭ�� ���¸� ��ȯ
        device->GetSwapChain()->SetFullscreenState(isFullscreen, nullptr);

        // ��ü ȭ�� ��� ��ȯ �� �ػ� ����
        DXGI_MODE_DESC fullscreenDesc = {};
        fullscreenDesc.Width = FRAMEBUFFER_WIDTH;         // ���ϴ� �ػ� ����
        fullscreenDesc.Height = FRAMEBUFFER_HEIGHT;
        fullscreenDesc.RefreshRate.Numerator = 60;        // ���� ��ħ �ӵ� ����
        fullscreenDesc.RefreshRate.Denominator = 1;
        fullscreenDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        fullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        fullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

        // ��ü ȭ�� ���� ��ȯ�� �� �ػ� ����
        device->GetSwapChain()->ResizeTarget(&fullscreenDesc);

        // ���� ü�� ���� ũ�� ������ (�� ���� �ʱ�ȭ)
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        device->GetSwapChain()->GetDesc(&swapChainDesc);
        device->GetSwapChain()->ResizeBuffers(
            2, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT,
            swapChainDesc.BufferDesc.Format, swapChainDesc.Flags
        );

        // ���� Ÿ�� �� ���� (�� ���۰� ����Ǿ��� ������ �ٽ� ���� �ʿ�)
        CreateRenderTarget();
    }
}

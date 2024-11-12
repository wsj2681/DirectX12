#include "framework.h"
#include "DX12Renderer.h"
#include "VertexData.h"

#pragma comment(lib, "d3dcompiler.lib")

DX12Renderer::DX12Renderer(DX12Device* device)
    : device(device), rtvDescriptorSize(0), currentBackBufferIndex(0) 
{
    worldMatrix = XMMatrixIdentity();
}

DX12Renderer::~DX12Renderer() 
{
    // ������ ���ҽ� ����
}

bool DX12Renderer::Initialize() 
{
    if (!CreateCommandObjects()) return false;
    if (!CreateRenderTarget()) return false; 

    if (!CreatePipelineState()) 
    {
        MessageBox(nullptr, L"Failed to create pipeline state", L"Error", MB_OK);
        return false;
    }

    worldMatrix = XMMatrixIdentity();

    if (!CreateConstantBuffer()) {
        MessageBox(nullptr, L"Failed to create constant buffer", L"Error", MB_OK);
        return false;
    }

    this->camera = make_unique<Camera>();
    this->camera->SetPosition(0.f, 2.f, -5.f);
    std::vector<Vertex> vertices = GetCubeVertices();
    std::vector<uint16_t> indices = GetCubeIndices();

    this->mesh = make_unique<Mesh>(device->GetDevice().Get(), vertices, indices);

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

bool DX12Renderer::CreatePipelineState()
{
    // ��Ʈ ���� ����
    CD3DX12_ROOT_PARAMETER rootParams[1];
    rootParams[0].InitAsConstantBufferView(0); // b0 ���Կ� ��� ���� ���ε�

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, rootParams, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HR(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &errorBlob));
    HR(device->GetDevice()->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&rootSignature_)));

    // Shader Compile
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;
    HR(D3DCompileFromFile(L"cube.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vertexShader, nullptr));
    HR(D3DCompileFromFile(L"cube.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pixelShader,  nullptr));

    D3D12_INPUT_ELEMENT_DESC inputLayout[] = 
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.pRootSignature = rootSignature_.Get();
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

    return SUCCEEDED(device->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_)));
}

bool DX12Renderer::CreateConstantBuffer()
{
    UINT bufferSize = (sizeof(XMMATRIX) + 255) & ~255;

    // ��� ���� ���ҽ� ����
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
    HRESULT hr = device->GetDevice()->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&mvpBuffer));

    if (FAILED(hr)) 
    {
        OutputDebugString(L"Failed to create constant buffer (mvpBuffer)\n");
        return false;
    }

    return SUCCEEDED(hr);
}

void DX12Renderer::Render() 
{
    // Ŀ�ǵ� �Ҵ��ڿ� Ŀ�ǵ� ����Ʈ �ʱ�ȭ
    commandAllocator->Reset();

    //commandList->Reset(commandAllocator.Get(), nullptr);
    commandList->Reset(commandAllocator.Get(), pipelineState_.Get());

    // ���������� ���� �� ��Ʈ ���� ����
    commandList->SetGraphicsRootSignature(rootSignature_.Get());
    commandList->SetPipelineState(pipelineState_.Get());

    // ��-�������� ��� ��� �� ��� ���ۿ� ���ε�
    MVPMatrix mvpData;
    mvpData.world = XMMatrixTranspose(worldMatrix);
    mvpData.view = XMMatrixTranspose(camera->GetViewMatrix());
    mvpData.projection = XMMatrixTranspose(camera->GetProjectionMatrix());

    // MVP ����� ��� ���ۿ� ����
    D3D12_RANGE readRange = {}; // �б� ���� ���� ����
    void* pData = nullptr;
    mvpBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pData));
    memcpy(pData, &mvpData, sizeof(MVPMatrix));
    mvpBuffer->Unmap(0, nullptr);

    commandList->SetGraphicsRootConstantBufferView(0, mvpBuffer->GetGPUVirtualAddress());

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
    const float clearColor[] = { 1.0f, 1.2f, 1.4f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    mesh->Render(commandList.Get());

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
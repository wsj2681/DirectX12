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
    // 렌더링 리소스 정리
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

bool DX12Renderer::CreatePipelineState()
{
    // 루트 서명 생성
    CD3DX12_ROOT_PARAMETER rootParams[1];
    rootParams[0].InitAsConstantBufferView(0); // b0 슬롯에 상수 버퍼 바인딩

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

    // 상수 버퍼 리소스 생성
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
    // 커맨드 할당자와 커맨드 리스트 초기화
    commandAllocator->Reset();

    //commandList->Reset(commandAllocator.Get(), nullptr);
    commandList->Reset(commandAllocator.Get(), pipelineState_.Get());

    // 파이프라인 상태 및 루트 서명 설정
    commandList->SetGraphicsRootSignature(rootSignature_.Get());
    commandList->SetPipelineState(pipelineState_.Get());

    // 뷰-프로젝션 행렬 계산 및 상수 버퍼에 업로드
    MVPMatrix mvpData;
    mvpData.world = XMMatrixTranspose(worldMatrix);
    mvpData.view = XMMatrixTranspose(camera->GetViewMatrix());
    mvpData.projection = XMMatrixTranspose(camera->GetProjectionMatrix());

    // MVP 행렬을 상수 버퍼에 복사
    D3D12_RANGE readRange = {}; // 읽기 범위 설정 없음
    void* pData = nullptr;
    mvpBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pData));
    memcpy(pData, &mvpData, sizeof(MVPMatrix));
    mvpBuffer->Unmap(0, nullptr);

    commandList->SetGraphicsRootConstantBufferView(0, mvpBuffer->GetGPUVirtualAddress());

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
    const float clearColor[] = { 1.0f, 1.2f, 1.4f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    mesh->Render(commandList.Get());

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
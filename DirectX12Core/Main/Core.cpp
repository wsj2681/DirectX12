#include "Core.h"
#include "resource.h"

Core::Core(HINSTANCE hInstance) : hInstance(hInstance), hWnd(nullptr) 
{
    LoadStringW(this->hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(this->hInstance, IDC_MAIN, szWindowClass, MAX_LOADSTRING);
}

Core::~Core() 
{

}

bool Core::Initialize(int nCmdShow) 
{
    if (!RegisterWindowClass()) return false;
    if (!InitInstance(nCmdShow)) return false;

    dx12Device = make_unique<DX12Device>(hWnd);
    if (!dx12Device->Initialize())
    {
        MessageBox(hWnd, L"DirectX 12 디바이스 초기화에 실패했습니다.", L"오류", MB_OK);
        return false;
    }

    return true;
}

int Core::Run() 
{
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) 
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            dx12Device->Present();
        }
    }
    return static_cast<int>(msg.wParam);
}

bool Core::RegisterWindowClass() 
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

bool Core::InitInstance(int nCmdShow) 
{
    hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd) return false;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return true;
}

LRESULT CALLBACK Core::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
    switch (message) 
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
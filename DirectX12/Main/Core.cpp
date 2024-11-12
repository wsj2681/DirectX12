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
    if (!RegisterWindowClass())
    {
        return false;
    }

    if (!InitInstance(nCmdShow))
    {
        return false;
    }

    dx12Device = make_unique<DX12Device>(hWnd);
    if (!dx12Device->Initialize())
    {
        MessageBox(hWnd, L"DirectX 12 디바이스 초기화에 실패했습니다.", L"오류", MB_OK);
        return false;
    }

    dx12Renderer = make_unique<DX12Renderer>(dx12Device.get());
    if (!dx12Renderer->Initialize())
    {
        MessageBox(hWnd, L"DirectX 12 Renderer 초기화에 실패했습니다.", L"오류", MB_OK);
        return false;
    }

    return true;
}

int Core::Run() {
    MSG msg;
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MAIN));

    // 기본 메세지 루프
    while (true) 
    {
        timer.tick(0.0f);

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
            dx12Renderer->Render();
        }
        
        timer.getFrameRate(&pszFrameRate);
        ::SetWindowText(hWnd, pszFrameRate.c_str());
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
    wcex.cbWndExtra = sizeof(LONG_PTR);
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDC_MAIN));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

bool Core::InitInstance(int nCmdShow) 
{
    DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_BORDER;
    RECT rc = { 0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT };

    hWnd = CreateWindowW(
        szWindowClass, 
        szTitle, 
        dwStyle,
        CW_USEDEFAULT, 
        CW_USEDEFAULT,
        rc.right - rc.left, 
        rc.bottom - rc.top, 
        nullptr, 
        nullptr, 
        hInstance, 
        this);

    if (!hWnd)
    {
        return false;
    }

    SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(dx12Renderer.get()));

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return true;
}

LRESULT CALLBACK Core::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
    static Core* pCore = nullptr;

    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pCreate = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pCore = reinterpret_cast<Core*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCore));
        return 0;
    }

    if (pCore)
    {
        switch (message)
        {
        case WM_KEYUP:
            if (wParam == VK_F9) 
            {
                if (pCore->dx12Renderer) 
                {
                    pCore->dx12Renderer->ToggleFullscreen(); // 전체 화면 토글
                }
            }
            else if (wParam == VK_ESCAPE)
            {
                PostQuitMessage(0);
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
#pragma once
#include "framework.h"
#include "DX12Device.h"
#include "DX12Renderer.h"

class Core {
public:
    Core(HINSTANCE hInstance);
    ~Core();

    bool Initialize(int nCmdShow);
    int Run();

protected:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    bool RegisterWindowClass();
    bool InitInstance(int nCmdShow);

    HINSTANCE hInstance;
    HWND hWnd;
    WCHAR szTitle[MAX_LOADSTRING];
    WCHAR szWindowClass[MAX_LOADSTRING];

    unique_ptr<DX12Device> dx12Device;
    unique_ptr<DX12Renderer> dx12Renderer;
};
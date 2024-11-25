#pragma once
#include "DX12Device.h"

#define MAX_LOADSTRING 100

class Core
{
private:

    static Core* core;
    static once_flag initFlag;
    static HINSTANCE initParam;

public:
    static Core& getInstance()
    {
        call_once(initFlag, []()
            {
                core = new Core(initParam);
            });

        return *core;
    }
    static void CreateCore(const HINSTANCE& hInstance)
    {
        initParam = hInstance;
    }

    Core(HINSTANCE hInstance);

    Core(const Core&) = delete;
    Core& operator=(const Core&) = delete;
    Core(Core&&) = delete;
    Core& operator=(Core&&) = delete;

    ~Core() = default;

    bool Initialize(int nCmdShow);
    int Run();

protected:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    bool RegisterWindowClass() const;
    bool InitInstance(int nCmdShow);

    HINSTANCE hInstance;
    HWND hWnd;
    WCHAR szTitle[MAX_LOADSTRING];
    WCHAR szWindowClass[MAX_LOADSTRING];

    unique_ptr<DX12Device> device;
    //unique_ptr<Scene> scene;
};


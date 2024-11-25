// header.h: 표준 시스템 포함 파일
// 또는 프로젝트 특정 포함 파일이 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
// Windows 헤더 파일
#include <windows.h>
// C 런타임 헤더 파일입니다.
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <wincodec.h>
#include <commdlg.h>

#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <DirectXCollision.h>
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include "d3dx12.h"

using namespace std;
using namespace DirectX;
using Microsoft::WRL::ComPtr;

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

#define FRAMEBUFFER_WIDTH 800
#define FRAMEBUFFER_HEIGHT 600
constexpr float ASPECTRATIO(static_cast<float>(FRAMEBUFFER_WIDTH) / FRAMEBUFFER_HEIGHT);

//HR
#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x) \
      if (FAILED((x))) \
      { \
         LPVOID errorLog = nullptr; \
         FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, \
            nullptr, (x), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), \
            reinterpret_cast<LPWSTR>(&errorLog), 0, nullptr); \
         fprintf(stderr, "%s", static_cast<char*>(errorLog)); \
         LocalFree(errorLog); \
         __debugbreak(); \
      }
#endif
#else
#ifndef HR
#define   HR(x) (x);
#endif
#endif

inline void ThrowIfFailed(HRESULT hr, const std::string & errorMessage = "")
{
    if (FAILED(hr))
    {
        std::string message = "HRESULT failed with error code: 0x" + std::to_string(hr);
        if (!errorMessage.empty())
        {
            message += "\nDetails: " + errorMessage;
        }


        throw std::runtime_error(message);
    }
}

#define SAFE_RESET(x) if((x)) x.Reset()

#define TOWSTRING(x) wstring((x).begin(), (x).end()).c_str()
#pragma once

#include <Windows.h>
#include <string>
#include <array>

class Timer {
public:
    static constexpr ULONG MAX_SAMPLE_COUNT = 50; // 50회의 프레임 처리시간을 누적하여 평균 계산

    Timer();
    ~Timer() = default;

    void tick(float lockFPS = 0.0f); // 타이머 갱신 함수
    unsigned long getFrameRate(std::wstring* frameRateStr = nullptr); // 프레임 레이트 반환
    float getTimeElapsed() const; // 프레임 경과 시간 반환

private:
    bool hardwareHasPerformanceCounter = false;           // 성능 카운터 사용 가능 여부
    float timeScale = 0.0f;                               // 타이머 스케일
    float timeElapsed = 0.0f;                             // 이전 프레임에서 경과된 시간
    __int64 currentTime = 0;                              // 현재 시간
    __int64 lastTime = 0;                                 // 마지막 시간
    __int64 performanceFrequency = 0;                     // 성능 카운터 주파수

    std::array<float, MAX_SAMPLE_COUNT> frameTimes = {};  // 프레임 시간 누적 배열
    ULONG sampleCount = 0;                                // 누적된 프레임 횟수
    unsigned long currentFrameRate = 0;                   // 현재 프레임 레이트
    unsigned long framesPerSecond = 0;                    // 초당 프레임 수
    float fpsTimeElapsed = 0.0f;                          // 프레임 레이트 계산에 소요된 시간
};

inline ULONGLONG timeGetTime() 
{
    return GetTickCount64();
}
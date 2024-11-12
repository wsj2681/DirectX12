#include "framework.h"
#include "Timer.h"


Timer::Timer() 
{
    if (::QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&performanceFrequency))) 
    {
        hardwareHasPerformanceCounter = true;
        ::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&lastTime));
        timeScale = 1.0f / performanceFrequency;
    }
    else {
        hardwareHasPerformanceCounter = false;
        lastTime = ::timeGetTime();
        timeScale = 0.001f; // 밀리초 단위
    }
}

void Timer::tick(float lockFPS) 
{
    // 현재 시간 갱신
    if (hardwareHasPerformanceCounter) 
    {
        ::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currentTime));
    }
    else {
        currentTime = ::timeGetTime();
    }

    // 경과 시간 계산
    float frameTimeElapsed = (currentTime - lastTime) * timeScale;

    // 지정된 FPS로 프레임 제한
    if (lockFPS > 0.0f) 
    {
        while (frameTimeElapsed < (1.0f / lockFPS)) 
        {
            if (hardwareHasPerformanceCounter) 
            {
                ::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currentTime));
            }
            else 
            {
                currentTime = ::timeGetTime();
            }
            frameTimeElapsed = (currentTime - lastTime) * timeScale;
        }
    }

    // 시간 업데이트
    lastTime = currentTime;

    // 프레임 시간 누적 및 평균 계산
    if (fabsf(frameTimeElapsed - timeElapsed) < 1.0f) 
    {
        std::move(frameTimes.begin(), frameTimes.end() - 1, frameTimes.begin() + 1);
        frameTimes[0] = frameTimeElapsed;
        if (sampleCount < MAX_SAMPLE_COUNT)
        {
            sampleCount++;
        }
    }

    // 초당 프레임 수 증가 및 경과 시간 누적
    framesPerSecond++;
    fpsTimeElapsed += frameTimeElapsed;
    if (fpsTimeElapsed > 1.0f) 
    {
        currentFrameRate = framesPerSecond;
        framesPerSecond = 0;
        fpsTimeElapsed = 0.0f;
    }

    // 누적된 프레임 시간의 평균을 계산
    timeElapsed = std::accumulate(frameTimes.begin(), frameTimes.begin() + sampleCount, 0.0f) / sampleCount;
}

unsigned long Timer::getFrameRate(std::wstring* frameRateStr) 
{
    if (frameRateStr) 
    {
        *frameRateStr = L"Core (FPS " + std::to_wstring(currentFrameRate) + L")";
    }
    return currentFrameRate;
}

float Timer::getTimeElapsed() const 
{
    return timeElapsed;
}
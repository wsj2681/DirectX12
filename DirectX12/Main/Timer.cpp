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
        timeScale = 0.001f; // �и��� ����
    }
}

void Timer::tick(float lockFPS) 
{
    // ���� �ð� ����
    if (hardwareHasPerformanceCounter) 
    {
        ::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currentTime));
    }
    else {
        currentTime = ::timeGetTime();
    }

    // ��� �ð� ���
    float frameTimeElapsed = (currentTime - lastTime) * timeScale;

    // ������ FPS�� ������ ����
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

    // �ð� ������Ʈ
    lastTime = currentTime;

    // ������ �ð� ���� �� ��� ���
    if (fabsf(frameTimeElapsed - timeElapsed) < 1.0f) 
    {
        std::move(frameTimes.begin(), frameTimes.end() - 1, frameTimes.begin() + 1);
        frameTimes[0] = frameTimeElapsed;
        if (sampleCount < MAX_SAMPLE_COUNT)
        {
            sampleCount++;
        }
    }

    // �ʴ� ������ �� ���� �� ��� �ð� ����
    framesPerSecond++;
    fpsTimeElapsed += frameTimeElapsed;
    if (fpsTimeElapsed > 1.0f) 
    {
        currentFrameRate = framesPerSecond;
        framesPerSecond = 0;
        fpsTimeElapsed = 0.0f;
    }

    // ������ ������ �ð��� ����� ���
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
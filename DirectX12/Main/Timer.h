#pragma once

#include <Windows.h>
#include <string>
#include <array>

class Timer {
public:
    static constexpr ULONG MAX_SAMPLE_COUNT = 50; // 50ȸ�� ������ ó���ð��� �����Ͽ� ��� ���

    Timer();
    ~Timer() = default;

    void tick(float lockFPS = 0.0f); // Ÿ�̸� ���� �Լ�
    unsigned long getFrameRate(std::wstring* frameRateStr = nullptr); // ������ ����Ʈ ��ȯ
    float getTimeElapsed() const; // ������ ��� �ð� ��ȯ

private:
    bool hardwareHasPerformanceCounter = false;           // ���� ī���� ��� ���� ����
    float timeScale = 0.0f;                               // Ÿ�̸� ������
    float timeElapsed = 0.0f;                             // ���� �����ӿ��� ����� �ð�
    __int64 currentTime = 0;                              // ���� �ð�
    __int64 lastTime = 0;                                 // ������ �ð�
    __int64 performanceFrequency = 0;                     // ���� ī���� ���ļ�

    std::array<float, MAX_SAMPLE_COUNT> frameTimes = {};  // ������ �ð� ���� �迭
    ULONG sampleCount = 0;                                // ������ ������ Ƚ��
    unsigned long currentFrameRate = 0;                   // ���� ������ ����Ʈ
    unsigned long framesPerSecond = 0;                    // �ʴ� ������ ��
    float fpsTimeElapsed = 0.0f;                          // ������ ����Ʈ ��꿡 �ҿ�� �ð�
};

inline ULONGLONG timeGetTime() 
{
    return GetTickCount64();
}
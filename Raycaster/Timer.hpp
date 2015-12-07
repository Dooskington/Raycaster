#pragma once
#include "PCH.hpp"

class Timer
{
public:
    Timer();

    void Start();
    void Stop();
    void Pause();
    void Unpause();
    Uint32 GetTime();
    bool IsStarted();
    bool IsPaused();

private:
    Uint32 m_startTime;
    Uint32 m_pausedTime;
    bool m_isStarted;
    bool m_isPaused;
};
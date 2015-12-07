#include "PCH.hpp"
#include "Timer.hpp"

Timer::Timer() :
    m_startTime(0),
    m_pausedTime(0),
    m_isStarted(false),
    m_isPaused(false)
{
}

void Timer::Start()
{
    m_isStarted = true;
    m_isPaused = false;
    m_startTime = SDL_GetTicks();
    m_pausedTime = 0;
}

void Timer::Stop()
{
    m_isStarted = false;
    m_isPaused = false;
    m_startTime = 0;
    m_pausedTime = 0;
}

void Timer::Pause()
{
    if (m_isStarted && !m_isPaused)
    {
        // Pause the timer
        m_isPaused = true;

        // Calculate the paused time
        m_pausedTime = SDL_GetTicks() - m_startTime;

        // Reset the start time
        m_startTime = 0;
    }
}

void Timer::Unpause()
{
    if (m_isStarted && m_isPaused)
    {
        // Unpause the timer
        m_isPaused = false;

        // Reset the starting time
        m_startTime = SDL_GetTicks() - m_pausedTime;

        // Reset the paused time
        m_pausedTime = 0;
    }
}

Uint32 Timer::GetTime()
{
    Uint32 time = 0;

    if (m_isStarted)
    {
        if (m_isPaused)
        {
            time = m_pausedTime;
        }
        else
        {
            time = SDL_GetTicks() - m_startTime;
        }
    }

    return time;
}

bool Timer::IsStarted()
{
    return m_isStarted;
}

bool Timer::IsPaused()
{
    return m_isPaused && m_isStarted;
}
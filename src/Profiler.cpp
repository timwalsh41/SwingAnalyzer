#include <Profiler.h>

namespace Utility
{

Profiler::Profiler(std::string name) : m_name(name), m_logger(Logger::get())
{
}

void Profiler::tick()
{
    if (m_in_tick_period)
    {
        m_logger.warn("Profiler %s tick called before tock", m_name.c_str());
        m_in_tick_period = false;
        return;
    }

    auto tick_time = std::chrono::high_resolution_clock::now();

    if (!m_v_durations.empty())
    {
        auto interval = std::chrono::duration_cast<std::chrono::duration<double>>(tick_time - m_tick_time);
        m_v_intervals.push_back(interval);
    }

    m_tick_time = tick_time;
    m_in_tick_period = true;
}

void Profiler::tock()
{
    if (!m_in_tick_period)
    {
        m_logger.warn("Profiler %s tock called before tick", m_name.c_str());
        return;
    }

    auto tock_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(tock_time - m_tick_time);

    m_v_durations.push_back(duration);

    m_in_tick_period = false;
}

void Profiler::reset()
{
    m_v_durations.clear();
    m_v_intervals.clear();
}

void Profiler::dump()
{
    if (m_v_durations.empty())
    {
        m_logger.debug("Profiler %s: No duration data", m_name.c_str());
    }
    else
    {
        m_logger.debug("Profiler %s duration data:", m_name.c_str());
        for (auto &duration : m_v_durations)
        {
            m_logger.debug("  Profile data: %f", duration.count());
        }
    }

    if (m_v_intervals.empty())
    {
        m_logger.debug("Profiler %s: No interval data", m_name.c_str());
    }
    else
    {
        m_logger.debug("Profiler %s interval data:", m_name.c_str());
        for (auto &interval : m_v_intervals)
        {
            m_logger.debug("  Profile data: %f", interval.count());
        }
    }
}


} // namespace Utility
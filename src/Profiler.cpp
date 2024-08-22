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

    // we can record the tick to tick interval once we've completed one tick to
    // tock cycle
    if (!m_v_durations.empty())
    {
        if (!m_skipped_first)
        {
            m_skipped_first = true;
            return;
        }

        // we have completed at least one tick-to-tock cycle, so we can record interval
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

void Profiler::reset(bool skip_first)
{
    m_v_durations.clear();
    m_v_intervals.clear();

    m_in_tick_period = false;

    // if we are skipping the first data point, then we need to set the flag
    // to false so that we will skip
    m_skipped_first = !skip_first;
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
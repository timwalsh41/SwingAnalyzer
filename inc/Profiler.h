#ifndef PROFILER_H
#define PROFILER_H

#include <Logger.h>

#include <chrono>
#include <string>
#include <vector>

namespace Utility
{

class Profiler
{
  public:
    Profiler(std::string name);
    ~Profiler() = default;

    // start a new measurement cycle
    void tick();

    // end the current duration measurement cycle
    void tock();

    // clear the current profiler data
    void reset();

    // dump the profiler data to the logger
    void dump();

    std::chrono::time_point<std::chrono::high_resolution_clock> getTickTime() const
    {
        return m_tick_time;
    }

  private:
    // unique identifier for the profiler
    std::string m_name;

    // time point for the start of the current profiler interval
    std::chrono::time_point<std::chrono::high_resolution_clock> m_tick_time;

    // storage of the durations (tick to tock) and intervals (tick to tick)
    std::vector<std::chrono::duration<double>> m_v_durations{};
    std::vector<std::chrono::duration<double>> m_v_intervals{};

    // flag to indicate if we are in a tick period, e.g. we've gotten tick call
    // but not yet a tock call
    bool m_in_tick_period{false};

    Logger &m_logger;
};

} // namespace Utility
#endif // PROFILER_H
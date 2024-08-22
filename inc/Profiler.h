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
    void reset(bool skip_first = false);

    // dump the profiler data to the logger
    void dump();

    double getLastInterval() const
    {
      if (m_v_intervals.empty())
        return 0.;

      return m_v_intervals.back().count();
    }

    double getLastDuration() const
    {
      if (m_v_durations.empty())
        return 0.;

      return m_v_durations.back().count();
    }

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

    // flag to skip the first data point, if necessary - the purpose of this is that I
    // noticed that the the first interval for the image profiler was consistently very
    // short, so this way we can exclude it from the data analysis
    // we default to true here so that we would not skip by default
    bool m_skipped_first{true};

    Logger &m_logger;
};

} // namespace Utility
#endif // PROFILER_H
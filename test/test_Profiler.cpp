#include <Profiler.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>

using namespace Utility;

TEST(ProfilerTest, TickTockCycle)
{
    Profiler profiler("TestProfiler");

    profiler.tick();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    profiler.tock();

    profiler.dump(); // Ensure no crash during dump
}

TEST(ProfilerTest, ResetFunctionality)
{
    Profiler profiler("TestProfiler");

    profiler.tick();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    profiler.tock();

    profiler.reset();

    profiler.dump(); // Ensure no crash during dump
}

TEST(ProfilerTest, SkipFirstFlag)
{
    Profiler profiler("TestProfiler");

    profiler.reset(true);

    profiler.tick();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    profiler.tock();

    profiler.tick();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    profiler.tock();

    profiler.dump(); // Ensure no crash during dump
}

TEST(ProfilerTest, TickBeforeTockWarning)
{
    Profiler profiler("TestProfiler");

    profiler.tick();
    profiler.tick(); // Should log a warning

    profiler.dump(); // Ensure no crash during dump
}

TEST(ProfilerTest, TockBeforeTickWarning)
{
    Profiler profiler("TestProfiler");

    profiler.tock(); // Should log a warning

    profiler.dump(); // Ensure no crash during dump
}

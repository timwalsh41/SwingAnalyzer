// Unit tests for VideoPlaybackController
#include <VideoPlaybackController.h>
#include <gtest/gtest.h>
#include <thread>
#include <atomic>
#include <chrono>

using namespace VideoPlayback;

class VideoPlaybackControllerTest : public ::testing::Test {
protected:
    VideoPlaybackController controller;
};

TEST_F(VideoPlaybackControllerTest, InitialState)
{
    EXPECT_FALSE(controller.isPlaying());
    EXPECT_EQ(controller.getFrameInterval(), std::chrono::microseconds(0));
}

TEST_F(VideoPlaybackControllerTest, StartAndStopPlayback)
{
    std::atomic<bool> playbackFunctionCalled(false);
    controller.startPlayback([&]() {
        playbackFunctionCalled = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_TRUE(controller.isPlaying());
    EXPECT_TRUE(playbackFunctionCalled.load());

    controller.stopPlayback();
    EXPECT_FALSE(controller.isPlaying());
}

TEST_F(VideoPlaybackControllerTest, SetFramerate)
{
    controller.setFramerate(30.0f, 1.0f);
    EXPECT_EQ(controller.getFrameInterval(), std::chrono::microseconds(static_cast<long int>(1.0f / 30.0f * 1e6f)));

    controller.setFramerate(-1.0f, 1.0f); // Invalid
    EXPECT_EQ(controller.getFrameInterval(), std::chrono::microseconds(static_cast<long int>(1.0f / 30.0f * 1e6f)));

    controller.setFramerate(30.0f, -1.0f); // Invalid
    EXPECT_EQ(controller.getFrameInterval(), std::chrono::microseconds(static_cast<long int>(1.0f / 30.0f * 1e6f)));
}

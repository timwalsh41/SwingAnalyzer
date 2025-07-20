// Unit tests for VideoChannel
#include <VideoChannel.h>

#include <gtest/gtest.h>

#include <string>

using namespace VideoPlayback;

class VideoChannelTest : public ::testing::Test
{
    protected:
        VideoChannelTest()
            : channel(WhichVideoChannel::LEFT_CHANNEL) {channel.registerFrameUpdateCallback([](int, WhichVideoChannel){});}
        VideoChannel channel;
};

TEST_F(VideoChannelTest, InitialStateIsUnloaded)
{
    EXPECT_FALSE(channel.isLoaded());
    EXPECT_FALSE(channel.isPlaying());
    EXPECT_EQ(channel.getState(), VideoChannelState::UNLOADED);
    EXPECT_EQ(channel.getCurrentFrame(), 0);
    EXPECT_EQ(channel.getTotalFrames(), 0);
}

TEST_F(VideoChannelTest, LoadInvalidVideoReturnsError)
{
    VideoLoadStatus status = channel.loadVideo("invalid_path.mp4");
    EXPECT_EQ(status, VIDEO_LOAD_FILE_NOT_FOUND);
    EXPECT_FALSE(channel.isLoaded());
}

TEST_F(VideoChannelTest, LoadAndUnloadVideoTransitions)
{
    channel.loadVideo("invalid_path.mp4"); // Should fail, but state should remain
    EXPECT_FALSE(channel.isLoaded());
    channel.unloadVideo(); // Should not crash
    EXPECT_FALSE(channel.isLoaded());
}

TEST_F(VideoChannelTest, PlayPauseTransitions)
{
    channel.loadVideo("invalid_path.mp4");
    channel.playVideo(); // Should not play
    EXPECT_FALSE(channel.isPlaying());
    channel.pauseVideo(); // Should not crash
    EXPECT_FALSE(channel.isPlaying());
}

TEST_F(VideoChannelTest, StepAndSeekFrameNoVideo)
{
    channel.stepFrame(1); // Should not crash
    channel.seekToFrame(5); // Should not crash
    EXPECT_EQ(channel.getCurrentFrame(), 0);
}

TEST_F(VideoChannelTest, CompatibilityMethodsReturnDefaults)
{
    EXPECT_FALSE(channel.IsVideoLoaded());
    EXPECT_EQ(channel.getActiveFrame(), 0);
    EXPECT_EQ(channel.getFrameCount(), 0);
    EXPECT_FLOAT_EQ(channel.getRawFramerate_fps(), 0.0f);
    EXPECT_EQ(channel.getPlaybackStatus(), PAUSED);
    EXPECT_EQ(channel.getEffectiveVideoLengthFrames(), 0);
}

TEST_F(VideoChannelTest, SetFramerateMultiplierInvalid)
{
    channel.setFramerateMultiplier(100.0f); // Should not crash
    channel.setFramerateMultiplier(-1.0f); // Should not crash
}

TEST_F(VideoChannelTest, SetVideoSizeInvalid)
{
    channel.setVideoSize(5, 5); // Should not crash
}

TEST_F(VideoChannelTest, RegisterPlaybackCompleteCallback)
{
    bool called = false;
    channel.registerVideoPlaybackCompleteCallback([&called](WhichVideoChannel){ called = true; });
    // No playback, so callback should not be called
    EXPECT_FALSE(called);
}

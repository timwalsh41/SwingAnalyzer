#include <SynchronizerStatePlaybackPaused.h>
#include <SynchronizerStatePlaybackPlaying.h>
#include <SynchronizerStateSync.h>
#include <MockSynchronizerActions.h>
#include <VideoPlaybackConstants.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace VideoPlayback;
using ::testing::_;
using ::testing::StrictMock;

// Tests for SynchronizerStatePlaybackPaused
class SynchronizerStatePlaybackPausedTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mock_synchronizer = std::make_unique<StrictMock<MockSynchronizerActions>>();
        state = std::make_unique<SynchronizerStatePlaybackPaused>(mock_synchronizer.get());
    }

    std::unique_ptr<StrictMock<MockSynchronizerActions>> mock_synchronizer;
    std::unique_ptr<SynchronizerStatePlaybackPaused> state;
};

TEST_F(SynchronizerStatePlaybackPausedTest, PlayOrPauseEvent_CallsPlayVideos_TransitionsToPlayingState)
{
    // Arrange
    EXPECT_CALL(*mock_synchronizer, playVideos()).Times(1);

    // Act
    auto next_state = state->handleEvent(PLAY_OR_PAUSE, NO_CHANNEL);

    // Assert
    ASSERT_NE(next_state, nullptr);
    EXPECT_EQ(next_state->getName(), "PLAYBACK_PLAYING");
}

TEST_F(SynchronizerStatePlaybackPausedTest, StepForwardLarge_CallsStepVideosWithCorrectSize_StaysInSameState)
{
    // Arrange
    EXPECT_CALL(*mock_synchronizer, stepVideos(VideoPlayback::FRAME_STEP_LARGE)).Times(1);

    // Act
    auto next_state = state->handleEvent(STEP_FORWARD_LARGE, NO_CHANNEL);

    // Assert
    EXPECT_EQ(next_state, nullptr); // Should stay in same state
}

TEST_F(SynchronizerStatePlaybackPausedTest, StepBackwardSmall_CallsStepVideosWithNegativeSize_StaysInSameState)
{
    // Arrange
    EXPECT_CALL(*mock_synchronizer, stepVideos(-VideoPlayback::FRAME_STEP_SMALL)).Times(1);

    // Act
    auto next_state = state->handleEvent(STEP_BACKWARD_SMALL, NO_CHANNEL);

    // Assert
    EXPECT_EQ(next_state, nullptr);
}

TEST_F(SynchronizerStatePlaybackPausedTest, VideoPositionUpdate_CallsVideoPositionUpdateWithBothChannels)
{
    // Arrange
    EXPECT_CALL(*mock_synchronizer, videoPositionUpdate(BOTH_CHANNELS)).Times(1);

    // Act
    // Note: We pass LEFT_CHANNEL as input, but since this event is triggered from the GUI, the input channel
    // actually doesn't matter, PLAYBACK_PAUSED state should always call videoPositionUpdate(BOTH_CHANNELS) 
    // because in playback mode, both channels must be updated on this event
    auto next_state = state->handleEvent(VIDEO_POSITION_UPDATE, LEFT_CHANNEL);

    // Assert
    EXPECT_EQ(next_state, nullptr);
}

TEST_F(SynchronizerStatePlaybackPausedTest, VideoFrameUpdate_CallsVideoFrameUpdateWithCorrectParameters)
{
    // Arrange
    EXPECT_CALL(*mock_synchronizer, videoFrameUpdate(RIGHT_CHANNEL, true)).Times(1);

    // Act
    auto next_state = state->handleEvent(VIDEO_FRAME_UPDATE, RIGHT_CHANNEL);

    // Assert
    EXPECT_EQ(next_state, nullptr);
}

TEST_F(SynchronizerStatePlaybackPausedTest, LoadVideo_CallsCalculateFramerateRatio)
{
    // Arrange
    EXPECT_CALL(*mock_synchronizer, calculateFramerateRatio()).Times(1);

    // Act
    // Channel doesn't matter here...
    auto next_state = state->handleEvent(LOAD_VIDEO, NO_CHANNEL);

    // Assert
    EXPECT_EQ(next_state, nullptr);
}

TEST_F(SynchronizerStatePlaybackPausedTest, SetVideoToStart_CallsSetVideosToStartPosition)
{
    // Arrange
    EXPECT_CALL(*mock_synchronizer, setVideosToStartPosition()).Times(1);

    // Act
    // Channel doesn't matter here...
    auto next_state = state->handleEvent(SET_VIDEO_TO_START, NO_CHANNEL);

    // Assert
    EXPECT_EQ(next_state, nullptr);
}

TEST_F(SynchronizerStatePlaybackPausedTest, SyncOn_TransitionsToSyncState)
{
    // Arrange - expect calls that will be made by SynchronizerStateSync constructor
    // Tightly coupled to SynchronizerStateStateSync implementation
    EXPECT_CALL(*mock_synchronizer, setVideoFrameOffset(0)).Times(1);
    EXPECT_CALL(*mock_synchronizer, clearStartAndEndFrames()).Times(1);

    // Act
    auto next_state = state->handleEvent(SYNC_ON, NO_CHANNEL);

    // Assert
    ASSERT_NE(next_state, nullptr);
    EXPECT_EQ(next_state->getName(), "SYNC");

    // Tick the SYNC state to get the video position update
    EXPECT_CALL(*mock_synchronizer, videoPositionUpdate(BOTH_CHANNELS)).Times(1);

    // Act
    next_state = next_state->handleEvent(TICK, NO_CHANNEL);

    // Assert
    ASSERT_EQ(next_state, nullptr);
}

TEST_F(SynchronizerStatePlaybackPausedTest, TickEvent_DoesNothing_StaysInSameState)
{
    // Act
    auto next_state = state->handleEvent(TICK, NO_CHANNEL);

    // Assert
    EXPECT_EQ(next_state, nullptr);
}

// Tests for SynchronizerStatePlaybackPlaying
class SynchronizerStatePlaybackPlayingTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mock_synchronizer = std::make_unique<StrictMock<MockSynchronizerActions>>();
        state = std::make_unique<SynchronizerStatePlaybackPlaying>(mock_synchronizer.get());
    }

    std::unique_ptr<StrictMock<MockSynchronizerActions>> mock_synchronizer;
    std::unique_ptr<SynchronizerStatePlaybackPlaying> state;
};

TEST_F(SynchronizerStatePlaybackPlayingTest, PlayOrPauseEvent_CallsPauseVideos_TransitionsToPausedState)
{
    // Arrange
    EXPECT_CALL(*mock_synchronizer, pauseVideos()).Times(1);

    // Act
    auto next_state = state->handleEvent(PLAY_OR_PAUSE, NO_CHANNEL);

    // Assert
    ASSERT_NE(next_state, nullptr);
    EXPECT_EQ(next_state->getName(), "PLAYBACK_PAUSED");
}

TEST_F(SynchronizerStatePlaybackPlayingTest, StepForwardLarge_PausesAndSteps_TransitionsToPausedState)
{
    // Arrange
    EXPECT_CALL(*mock_synchronizer, pauseVideos()).Times(1);
    EXPECT_CALL(*mock_synchronizer, stepVideos(VideoPlayback::FRAME_STEP_LARGE)).Times(1);

    // Act
    auto next_state = state->handleEvent(STEP_FORWARD_LARGE, NO_CHANNEL);

    // Assert
    ASSERT_NE(next_state, nullptr);
    EXPECT_EQ(next_state->getName(), "PLAYBACK_PAUSED");
}

TEST_F(SynchronizerStatePlaybackPlayingTest, PlaybackComplete_PausesAndResetsPosition_TransitionsToPausedState)
{
    // Arrange
    EXPECT_CALL(*mock_synchronizer, pauseVideos()).Times(1);
    EXPECT_CALL(*mock_synchronizer, setVideosToStartPosition()).Times(1);
    EXPECT_CALL(*mock_synchronizer, getVideoPositionUpdateChannel()).WillOnce(::testing::Return(LEFT_CHANNEL));
    EXPECT_CALL(*mock_synchronizer, videoFrameUpdate(LEFT_CHANNEL, true)).Times(1);

    // Act - channel doesn't matter here
    auto next_state = state->handleEvent(PLAYBACK_COMPLETE, NO_CHANNEL);

    // Assert
    ASSERT_NE(next_state, nullptr);
    EXPECT_EQ(next_state->getName(), "PLAYBACK_PAUSED");
}

TEST_F(SynchronizerStatePlaybackPlayingTest, SyncOn_PausesVideos_TransitionsToSyncState)
{
    // Arrange
    EXPECT_CALL(*mock_synchronizer, pauseVideos()).Times(1);
    
    // Expect calls that will be made by SynchronizerStateSync constructor
    EXPECT_CALL(*mock_synchronizer, setVideoFrameOffset(0)).Times(1);
    EXPECT_CALL(*mock_synchronizer, clearStartAndEndFrames()).Times(1);
    
    // Act
    auto next_state = state->handleEvent(SYNC_ON, NO_CHANNEL);

    // Assert
    ASSERT_NE(next_state, nullptr);
    EXPECT_EQ(next_state->getName(), "SYNC");
}

TEST_F(SynchronizerStatePlaybackPlayingTest, VideoFrameUpdate_PassesThroughToSynchronizer_StaysInSameState)
{
    // Arrange
    EXPECT_CALL(*mock_synchronizer, videoFrameUpdate(RIGHT_CHANNEL, true)).Times(1);

    // Act
    auto next_state = state->handleEvent(VIDEO_FRAME_UPDATE, RIGHT_CHANNEL);

    // Assert
    EXPECT_EQ(next_state, nullptr);
}

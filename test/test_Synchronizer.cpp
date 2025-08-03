#include "MockVideoChannel.h"
#include <Synchronizer.h>
#include <VideoPlaybackTypes.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace VideoPlayback;
using ::testing::_;
using ::testing::Return;
using ::testing::InSequence;
using ::testing::NiceMock;

class SynchronizerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mock_left_channel = std::make_shared<NiceMock<MockVideoChannel>>();
        mock_right_channel = std::make_shared<NiceMock<MockVideoChannel>>();
        synchronizer = std::make_unique<Synchronizer>(mock_left_channel, mock_right_channel);
    }

    std::shared_ptr<NiceMock<MockVideoChannel>> mock_left_channel;
    std::shared_ptr<NiceMock<MockVideoChannel>> mock_right_channel;
    std::unique_ptr<Synchronizer> synchronizer;
};

// ============================================================================
// Basic Video Control Tests
// ============================================================================

TEST_F(SynchronizerTest, PlayVideos_CallsPlayVideoOnBothChannels)
{
    // Arrange
    EXPECT_CALL(*mock_left_channel, playVideo()).Times(1);
    EXPECT_CALL(*mock_right_channel, playVideo()).Times(1);

    // Act
    synchronizer->playVideos();

    // Assert - expectations verified by mock
}

TEST_F(SynchronizerTest, PauseVideos_CallsPauseVideoOnBothChannels)
{
    // Arrange
    EXPECT_CALL(*mock_left_channel, pauseVideo()).Times(1);
    EXPECT_CALL(*mock_right_channel, pauseVideo()).Times(1);

    // Act
    synchronizer->pauseVideos();

    // Assert - expectations verified by mock
}

TEST_F(SynchronizerTest, PlayVideo_LeftChannel_CallsPlayVideoOnLeftChannelOnly)
{
    // Arrange
    EXPECT_CALL(*mock_left_channel, playVideo()).Times(1);
    EXPECT_CALL(*mock_right_channel, playVideo()).Times(0);

    // Act
    synchronizer->playVideo(LEFT_CHANNEL);

    // Assert - expectations verified by mock
}

TEST_F(SynchronizerTest, PlayVideo_RightChannel_CallsPlayVideoOnRightChannelOnly)
{
    // Arrange
    EXPECT_CALL(*mock_left_channel, playVideo()).Times(0);
    EXPECT_CALL(*mock_right_channel, playVideo()).Times(1);

    // Act
    synchronizer->playVideo(RIGHT_CHANNEL);

    // Assert - expectations verified by mock
}

TEST_F(SynchronizerTest, PlayVideo_BothChannels_CallsPlayVideos)
{
    // Arrange
    EXPECT_CALL(*mock_left_channel, playVideo()).Times(1);
    EXPECT_CALL(*mock_right_channel, playVideo()).Times(1);

    // Act
    synchronizer->playVideo(BOTH_CHANNELS);

    // Assert - expectations verified by mock
}

TEST_F(SynchronizerTest, PauseVideo_LeftChannel_CallsPauseVideoOnLeftChannelOnly)
{
    // Arrange
    EXPECT_CALL(*mock_left_channel, pauseVideo()).Times(1);
    EXPECT_CALL(*mock_right_channel, pauseVideo()).Times(0);

    // Act
    synchronizer->pauseVideo(LEFT_CHANNEL);

    // Assert - expectations verified by mock
}

TEST_F(SynchronizerTest, PauseVideo_BothChannels_CallsPauseVideos)
{
    // Arrange
    EXPECT_CALL(*mock_left_channel, pauseVideo()).Times(1);
    EXPECT_CALL(*mock_right_channel, pauseVideo()).Times(1);

    // Act
    synchronizer->pauseVideo(BOTH_CHANNELS);

    // Assert - expectations verified by mock
}

// ============================================================================
// Step Video Tests
// ============================================================================

TEST_F(SynchronizerTest, StepVideo_LeftChannel_CallsStepFrameOnLeftChannelOnly)
{
    // Arrange
    const int step_size = 5;
    EXPECT_CALL(*mock_left_channel, stepFrame(step_size)).Times(1);
    EXPECT_CALL(*mock_right_channel, stepFrame(_)).Times(0);

    // Act
    synchronizer->stepVideo(LEFT_CHANNEL, step_size);

    // Assert - expectations verified by mock
}

TEST_F(SynchronizerTest, StepVideo_RightChannel_CallsStepFrameOnRightChannelOnly)
{
    // Arrange
    const int step_size = -3;
    EXPECT_CALL(*mock_left_channel, stepFrame(_)).Times(0);
    EXPECT_CALL(*mock_right_channel, stepFrame(step_size)).Times(1);

    // Act
    synchronizer->stepVideo(RIGHT_CHANNEL, step_size);

    // Assert - expectations verified by mock
}

// ============================================================================
// Video Playback Status Tests
// ============================================================================

TEST_F(SynchronizerTest, GetVideoPlaybackStatus_LeftChannel_ReturnsLeftChannelStatus)
{
    // Arrange
    EXPECT_CALL(*mock_left_channel, getPlaybackStatus())
        .WillOnce(Return(VideoPlaybackStatus::PLAYING));

    // Act
    VideoPlaybackStatus status = synchronizer->getVideoPlaybackStatus(LEFT_CHANNEL);

    // Assert
    EXPECT_EQ(status, VideoPlaybackStatus::PLAYING);
}

TEST_F(SynchronizerTest, GetVideoPlaybackStatus_RightChannel_ReturnsRightChannelStatus)
{
    // Arrange
    EXPECT_CALL(*mock_right_channel, getPlaybackStatus())
        .WillOnce(Return(VideoPlaybackStatus::PAUSED));

    // Act
    VideoPlaybackStatus status = synchronizer->getVideoPlaybackStatus(RIGHT_CHANNEL);

    // Assert
    EXPECT_EQ(status, VideoPlaybackStatus::PAUSED);
}

TEST_F(SynchronizerTest, GetVideoPlaybackStatus_InvalidChannel_ReturnsUnknown)
{
    // Act
    VideoPlaybackStatus status = synchronizer->getVideoPlaybackStatus(static_cast<WhichVideoChannel>(999));

    // Assert
    EXPECT_EQ(status, VideoPlaybackStatus::UNKNOWN);
}

// ============================================================================
// Frame Management Tests
// ============================================================================

TEST_F(SynchronizerTest, SetVideoFrameOffset_SetsOffset)
{
    // Arrange
    const int offset = 42;

    // Act
    synchronizer->setVideoFrameOffset(offset);

    // Assert
    EXPECT_EQ(synchronizer->getVideoFrameOffset(), offset);
}

TEST_F(SynchronizerTest, ClearStartAndEndFrames_CallsClearOnBothChannels)
{
    // Arrange
    EXPECT_CALL(*mock_left_channel, setStartFrame(0)).Times(1);
    EXPECT_CALL(*mock_left_channel, setEndFrame(-1)).Times(1);
    EXPECT_CALL(*mock_right_channel, setStartFrame(0)).Times(1);
    EXPECT_CALL(*mock_right_channel, setEndFrame(-1)).Times(1);

    // Act
    synchronizer->clearStartAndEndFrames();

    // Assert - expectations verified by mock
}

TEST_F(SynchronizerTest, SetVideosToStartPosition_CallsSetVideoToStartOnBothChannels)
{
    // Arrange
    EXPECT_CALL(*mock_left_channel, setVideoToStart()).Times(1);
    EXPECT_CALL(*mock_right_channel, setVideoToStart()).Times(1);

    // Act
    synchronizer->setVideosToStartPosition();

    // Assert - expectations verified by mock
}

// ============================================================================
// Video Synchronization Setup Tests
// ============================================================================

TEST_F(SynchronizerTest, SetupVideoSync_CalculatesOffsetAndSetsParameters)
{
    // Arrange
    EXPECT_CALL(*mock_left_channel, getActiveFrame()).WillOnce(Return(150));
    EXPECT_CALL(*mock_left_channel, getStartFrame()).WillRepeatedly(Return(100));
    EXPECT_CALL(*mock_right_channel, getActiveFrame()).WillOnce(Return(220));
    EXPECT_CALL(*mock_right_channel, getStartFrame()).WillRepeatedly(Return(200));
    
    EXPECT_CALL(*mock_left_channel, getEndFrame()).WillRepeatedly(Return(300));
    EXPECT_CALL(*mock_right_channel, getEndFrame()).WillRepeatedly(Return(400));

    // Offset should be (150-100) - (220-200) = 50 - 20 = 30
    // Since offset > 0, left start frame should be adjusted
    EXPECT_CALL(*mock_left_channel, setStartFrame(130)).Times(1); // 100 + 30

    // Act
    synchronizer->setupVideoSync();

    // Assert
    EXPECT_EQ(synchronizer->getVideoFrameOffset(), 30);
}

TEST_F(SynchronizerTest, SetupVideoSync_NegativeOffset_AdjustsRightChannel)
{
    // Arrange
    EXPECT_CALL(*mock_left_channel, getActiveFrame()).WillOnce(Return(110));
    EXPECT_CALL(*mock_left_channel, getStartFrame()).WillRepeatedly(Return(100));
    EXPECT_CALL(*mock_right_channel, getActiveFrame()).WillOnce(Return(250));
    EXPECT_CALL(*mock_right_channel, getStartFrame()).WillRepeatedly(Return(200));
    
    EXPECT_CALL(*mock_left_channel, getEndFrame()).WillRepeatedly(Return(300));
    EXPECT_CALL(*mock_right_channel, getEndFrame()).WillRepeatedly(Return(400));

    // Left offset should be (110-100) - (250-200) = 10 - 50 = -40
    // Since offset < 0, right start frame should be adjusted
    EXPECT_CALL(*mock_right_channel, setStartFrame(240)).Times(1); // 200 - (-40)

    // Act
    synchronizer->setupVideoSync();

    // Assert
    EXPECT_EQ(synchronizer->getVideoFrameOffset(), -40);
}

TEST_F(SynchronizerTest, SetupVideoSync_SetsVideoPositionUpdateChannel)
{
    // Arrange - make left channel shorter
    EXPECT_CALL(*mock_left_channel, getActiveFrame()).WillOnce(Return(100));
    EXPECT_CALL(*mock_left_channel, getStartFrame()).WillRepeatedly(Return(100));
    EXPECT_CALL(*mock_right_channel, getActiveFrame()).WillOnce(Return(200));
    EXPECT_CALL(*mock_right_channel, getStartFrame()).WillRepeatedly(Return(200));
    
    EXPECT_CALL(*mock_left_channel, getEndFrame()).WillRepeatedly(Return(200)); // length = 100
    EXPECT_CALL(*mock_right_channel, getEndFrame()).WillRepeatedly(Return(350)); // length = 150

    // Act
    synchronizer->setupVideoSync();

    // Assert - left channel is shorter, so it should be the update channel
    EXPECT_EQ(synchronizer->getVideoPositionUpdateChannel(), LEFT_CHANNEL);
}

// ============================================================================
// Framerate Calculation Tests
// ============================================================================

TEST_F(SynchronizerTest, CalculateFramerateRatio_BothVideosLoaded_CalculatesRatio)
{
    // Arrange
    EXPECT_CALL(*mock_left_channel, IsVideoLoaded()).WillOnce(Return(true));
    EXPECT_CALL(*mock_right_channel, IsVideoLoaded()).WillOnce(Return(true));
    EXPECT_CALL(*mock_left_channel, getFramerate_fps()).WillOnce(Return(60.0f));
    EXPECT_CALL(*mock_right_channel, getFramerate_fps()).WillOnce(Return(30.0f));

    // Act
    synchronizer->calculateFramerateRatio();

    // Assert
    EXPECT_EQ(synchronizer->getFramerateRatio(), 2.0f);
}

TEST_F(SynchronizerTest, CalculateFramerateRatio_LeftVideoNotLoaded_DoesNotCalculate)
{
    // Arrange
    EXPECT_CALL(*mock_left_channel, IsVideoLoaded()).WillOnce(Return(false));
    // Note: Due to short-circuit evaluation with 'or', right channel is not checked if left is false
    // Should not call getFramerate_fps on either channel

    // Act
    synchronizer->calculateFramerateRatio();

    // Assert - no crash, method handles gracefully
}

// ============================================================================
// Step Videos with Framerate Ratio Tests
// ============================================================================

TEST_F(SynchronizerTest, StepVideos_WithFramerateRatio_AdjustsStepSizes)
{
    // Arrange - set up framerate ratio of 2.0 (left faster than right)
    EXPECT_CALL(*mock_left_channel, IsVideoLoaded()).WillOnce(Return(true));
    EXPECT_CALL(*mock_right_channel, IsVideoLoaded()).WillOnce(Return(true));
    EXPECT_CALL(*mock_left_channel, getFramerate_fps()).WillOnce(Return(60.0f));
    EXPECT_CALL(*mock_right_channel, getFramerate_fps()).WillOnce(Return(30.0f));

    synchronizer->calculateFramerateRatio();

    // Expect adjusted step sizes: left = 5 * 2.0 = 10, right = 5
    EXPECT_CALL(*mock_left_channel, stepFrame(10)).Times(1);
    EXPECT_CALL(*mock_right_channel, stepFrame(5)).Times(1);

    // Act
    synchronizer->stepVideos(5);

    // Assert - expectations verified by mock
}

// ============================================================================
// Video Position Update Tests
// ============================================================================

TEST_F(SynchronizerTest, VideoPositionUpdate_LeftChannel_SeeksLeftChannelToCalculatedFrame)
{
    // Arrange
    synchronizer->setNewVideoPosition(0.5f); // 50%
    
    EXPECT_CALL(*mock_left_channel, getStartFrame()).WillRepeatedly(Return(100));
    EXPECT_CALL(*mock_left_channel, getEndFrame()).WillOnce(Return(200));
    EXPECT_CALL(*mock_left_channel, seekToFrame(150)).Times(1); // 100 + 0.5 * (200-100)

    // Act
    synchronizer->videoPositionUpdate(LEFT_CHANNEL);

    // Assert - expectations verified by mock
}

TEST_F(SynchronizerTest, VideoPositionUpdate_RightChannel_SeeksRightChannelToCalculatedFrame)
{
    // Arrange
    synchronizer->setNewVideoPosition(0.25f); // 25%
    
    EXPECT_CALL(*mock_right_channel, getStartFrame()).WillRepeatedly(Return(50));
    EXPECT_CALL(*mock_right_channel, getEndFrame()).WillOnce(Return(150));
    EXPECT_CALL(*mock_right_channel, seekToFrame(75)).Times(1); // 50 + 0.25 * (150-50)

    // Act
    synchronizer->videoPositionUpdate(RIGHT_CHANNEL);

    // Assert - expectations verified by mock
}

TEST_F(SynchronizerTest, VideoPositionUpdate_BothChannels_SeeksBothChannelsBasedOnCombinedLength)
{
    // First set up combined length through setupVideoSync
    EXPECT_CALL(*mock_left_channel, getActiveFrame()).WillOnce(Return(100));
    EXPECT_CALL(*mock_left_channel, getStartFrame()).WillRepeatedly(Return(100));
    EXPECT_CALL(*mock_right_channel, getActiveFrame()).WillOnce(Return(200));
    EXPECT_CALL(*mock_right_channel, getStartFrame()).WillRepeatedly(Return(200));
    
    EXPECT_CALL(*mock_left_channel, getEndFrame()).WillRepeatedly(Return(300)); // length = 200
    EXPECT_CALL(*mock_right_channel, getEndFrame()).WillRepeatedly(Return(350)); // length = 150
    
    synchronizer->setupVideoSync();
    
    // Arrange for position update
    synchronizer->setNewVideoPosition(0.5f); // 50%
    
    // Combined length should be min(200, 150) = 150
    // Expected frames: left = 100 + 0.5 * 150 = 175, right = 200 + 0.5 * 150 = 275
    EXPECT_CALL(*mock_left_channel, seekToFrame(175)).Times(1);
    EXPECT_CALL(*mock_right_channel, seekToFrame(275)).Times(1);

    // Act
    synchronizer->videoPositionUpdate(BOTH_CHANNELS);

    // Assert - expectations verified by mock
}

TEST_F(SynchronizerTest, VideoPositionUpdate_InvalidPosition_DoesNotSeek)
{
    // Arrange
    synchronizer->setNewVideoPosition(1.5f); // Invalid - greater than 1.0

    // Act
    synchronizer->videoPositionUpdate(LEFT_CHANNEL);

    // Assert
    EXPECT_NE(synchronizer->getNewVideoPosition(), 1.5f); // Should not change position
}

// ============================================================================
// Video Frame Update Tests
// ============================================================================

TEST_F(SynchronizerTest, VideoFrameUpdate_CalculatesNewPosition)
{
    // Arrange
    EXPECT_CALL(*mock_left_channel, getStartFrame()).WillRepeatedly(Return(100));
    EXPECT_CALL(*mock_left_channel, getEndFrame()).WillRepeatedly(Return(200));
    EXPECT_CALL(*mock_left_channel, getActiveFrame()).WillOnce(Return(150));

    // Act
    synchronizer->videoFrameUpdate(LEFT_CHANNEL, false);

    // Assert
    float expected_position = (150.0f - 100.0f) / (200.0f - 100.0f); // 0.5
    EXPECT_FLOAT_EQ(synchronizer->getNewVideoPosition(), expected_position);
}

TEST_F(SynchronizerTest, VideoFrameUpdate_ZeroVideoLength_SetsPositionToZero)
{
    // Arrange
    EXPECT_CALL(*mock_right_channel, getStartFrame()).WillOnce(Return(100));
    EXPECT_CALL(*mock_right_channel, getEndFrame()).WillOnce(Return(100)); // Same start and end

    // Act
    synchronizer->videoFrameUpdate(RIGHT_CHANNEL, false);

    // Assert
    EXPECT_FLOAT_EQ(synchronizer->getNewVideoPosition(), 0.0f);
}

TEST_F(SynchronizerTest, VideoFrameUpdate_WithUpdateJointSlider_FiltersBasedOnUpdateChannel)
{
    // Arrange - set up so LEFT_CHANNEL is the update channel
    EXPECT_CALL(*mock_left_channel, getActiveFrame()).WillOnce(Return(100));
    EXPECT_CALL(*mock_left_channel, getStartFrame()).WillRepeatedly(Return(100));
    EXPECT_CALL(*mock_right_channel, getActiveFrame()).WillOnce(Return(200));
    EXPECT_CALL(*mock_right_channel, getStartFrame()).WillRepeatedly(Return(200));
    
    EXPECT_CALL(*mock_left_channel, getEndFrame()).WillRepeatedly(Return(200)); // shorter
    EXPECT_CALL(*mock_right_channel, getEndFrame()).WillRepeatedly(Return(400)); // longer
    
    synchronizer->setupVideoSync(); // This should set LEFT_CHANNEL as update channel

    // Act - call with RIGHT_CHANNEL but update_joint_slider = true
    // This should be filtered out since RIGHT_CHANNEL != update channel (LEFT_CHANNEL)
    synchronizer->videoFrameUpdate(RIGHT_CHANNEL, true);

    // Assert - position should not change from initial value
    EXPECT_FLOAT_EQ(synchronizer->getNewVideoPosition(), 0.0f);
}

// ============================================================================
// Position Management Tests
// ============================================================================

TEST_F(SynchronizerTest, SetNewVideoPosition_ValidPosition_SetsPosition)
{
    // Act
    synchronizer->setNewVideoPosition(0.75f);

    // Assert
    EXPECT_FLOAT_EQ(synchronizer->getNewVideoPosition(), 0.75f);
}

TEST_F(SynchronizerTest, SetNewVideoPosition_InvalidPosition_DoesNotSetPosition)
{
    // Arrange - set a valid position first
    synchronizer->setNewVideoPosition(0.5f);

    // Act - try to set invalid position
    synchronizer->setNewVideoPosition(-0.1f);

    // Assert - position should remain unchanged
    EXPECT_FLOAT_EQ(synchronizer->getNewVideoPosition(), 0.5f);
}

TEST_F(SynchronizerTest, SetNewVideoPosition_BoundaryValues_HandledCorrectly)
{
    // Test 0.0
    synchronizer->setNewVideoPosition(0.0f);
    EXPECT_FLOAT_EQ(synchronizer->getNewVideoPosition(), 0.0f);

    // Test 1.0
    synchronizer->setNewVideoPosition(1.0f);
    EXPECT_FLOAT_EQ(synchronizer->getNewVideoPosition(), 1.0f);
}

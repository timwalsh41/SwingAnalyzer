#ifndef MOCKSYNCHRONIZERACTIONS_H
#define MOCKSYNCHRONIZERACTIONS_H

#include <ISynchronizerActions.h>
#include <VideoPlaybackTypes.h>

#include <gmock/gmock.h>

namespace VideoPlayback
{

class MockSynchronizerActions : public ISynchronizerActions
{
public:
    MOCK_METHOD(void, playVideos, (), (override));
    MOCK_METHOD(void, pauseVideos, (), (override));
    MOCK_METHOD(void, setVideosToStartPosition, (), (override));
    MOCK_METHOD(void, stepVideos, (int step_size), (override));
    
    MOCK_METHOD(void, playVideo, (WhichVideoChannel channel), (override));
    MOCK_METHOD(void, pauseVideo, (WhichVideoChannel channel), (override));
    MOCK_METHOD(void, stepVideo, (WhichVideoChannel channel, int step_size), (override));
    MOCK_METHOD(VideoPlaybackStatus, getVideoPlaybackStatus, (WhichVideoChannel channel), (const, override));
    
    MOCK_METHOD(void, setupVideoSync, (), (override));
    MOCK_METHOD(void, calculateFramerateRatio, (), (override));
    MOCK_METHOD(void, clearStartAndEndFrames, (), (override));
    MOCK_METHOD(void, setVideoFrameOffset, (int offset), (override));
    MOCK_METHOD(void, videoPositionUpdate, (WhichVideoChannel channel), (override));
    MOCK_METHOD(void, videoFrameUpdate, (WhichVideoChannel channel, bool update_joint_slider), (override));
    
    MOCK_METHOD(WhichVideoChannel, getVideoPositionUpdateChannel, (), (const, override));
    MOCK_METHOD(float, getNewVideoPosition, (), (const, override));
    MOCK_METHOD(void, setNewVideoPosition, (float position), (override));
    MOCK_METHOD(int, getNewActiveFrame, (), (const, override));
    MOCK_METHOD(void, setNewActiveFrame, (int frame), (override));
    MOCK_METHOD(void, notifyVideoPositionChanged, (WhichVideoChannel channel, float position), (override));
    MOCK_METHOD(int, getVideoFrameOffset, (), (const, override));
};

} // namespace VideoPlayback

#endif // MOCKSYNCHRONIZERACTIONS_H

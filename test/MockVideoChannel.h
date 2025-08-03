#ifndef MOCKVIDEOCHANNEL_H
#define MOCKVIDEOCHANNEL_H

#include <IVideoChannel.h>
#include <VideoPlaybackTypes.h>

#include <gmock/gmock.h>
#include <goocanvasmm.h>

namespace VideoPlayback
{

class MockVideoChannel : public IVideoChannel
{
public:
    // Video loading and unloading
    MOCK_METHOD(VideoLoadStatus, loadVideo, (const std::string& path), (override));
    MOCK_METHOD(void, unloadVideo, (), (override));
    MOCK_METHOD(bool, IsVideoLoaded, (), (const, override));
    
    // Playback control
    MOCK_METHOD(void, playVideo, (), (override));
    MOCK_METHOD(void, pauseVideo, (), (override));
    MOCK_METHOD(void, stepFrame, (int offset), (override));
    MOCK_METHOD(void, seekToFrame, (int frame), (override));
    MOCK_METHOD(void, setVideoToStart, (), (override));
    
    // Frame range management
    MOCK_METHOD(void, setStartFrame, (int start), (override));
    MOCK_METHOD(void, setEndFrame, (int end), (override));
    MOCK_METHOD(int, getStartFrame, (), (const, override));
    MOCK_METHOD(int, getEndFrame, (), (const, override));
    
    // Current state queries
    MOCK_METHOD(int, getActiveFrame, (), (const, override));
    MOCK_METHOD(VideoPlaybackStatus, getPlaybackStatus, (), (const, override));
    MOCK_METHOD(float, getFramerate_fps, (), (const, override));
    
    // Playback rate control
    MOCK_METHOD(void, setFramerateMultiplier, (float multiplier), (override));
    MOCK_METHOD(float, getFramerateMultiplier, (), (const, override));
    
    // Video size management
    MOCK_METHOD(VideoSize, getVideoSize, (), (const, override));
    MOCK_METHOD(void, setVideoSize, (int width, int height), (override));
    
    // Callback registration
    MOCK_METHOD(void, registerFrameUpdateCallback, (std::function<void(int, WhichVideoChannel)> callback), (override));
    MOCK_METHOD(void, registerVideoPlaybackCompleteCallback, (std::function<void(WhichVideoChannel)> callback), (override));
    
    // GUI integration
    MOCK_METHOD(const Glib::RefPtr<Goocanvas::Image>&, GetImage, (), (override));
};

} // namespace VideoPlayback

#endif // MOCKVIDEOCHANNEL_H

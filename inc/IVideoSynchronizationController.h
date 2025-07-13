#ifndef IVIDEOSYNCHRONIZATIONCONTROLLER_H
#define IVIDEOSYNCHRONIZATIONCONTROLLER_H

#include <VideoPlaybackTypes.h>

#include <string>

namespace VideoPlayback
{

class IVideoSynchronizationController
{
public:
    virtual ~IVideoSynchronizationController() = default;
    
    // Video playback control
    virtual void playVideo(WhichVideoChannel channel) = 0;
    virtual void pauseVideo(WhichVideoChannel channel) = 0;
    virtual void stepVideo(WhichVideoChannel channel, int frames) = 0;
    virtual void setVideoToStart(WhichVideoChannel channel) = 0;
    
    // Video positioning
    virtual void setVideoPosition(WhichVideoChannel channel, float position) = 0;
    
    // Video file management
    virtual void loadVideo(WhichVideoChannel channel, const std::string& path) = 0;
    virtual void unloadVideo(WhichVideoChannel channel) = 0;
    
    // Synchronization mode control
    virtual void setSyncMode(bool enabled) = 0;
    
    // Playback rate control
    virtual void setPlaybackRate(float multiplier) = 0;
};

} // namespace VideoPlayback

#endif // IVIDEOSYNCHRONIZATIONCONTROLLER_H

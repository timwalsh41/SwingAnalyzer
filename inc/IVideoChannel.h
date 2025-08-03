#ifndef IVIDEOCHANNEL_H
#define IVIDEOCHANNEL_H

#include <VideoPlaybackTypes.h>

#include <goocanvasmm.h>

#include <functional>
#include <string>

namespace VideoPlayback
{

class IVideoChannel
{
public:
    virtual ~IVideoChannel() = default;
    
    // Video loading and unloading
    virtual VideoLoadStatus loadVideo(const std::string& path) = 0;
    virtual void unloadVideo() = 0;
    virtual bool IsVideoLoaded() const = 0;
    
    // Playback control
    virtual void playVideo() = 0;
    virtual void pauseVideo() = 0;
    virtual void stepFrame(int offset) = 0;
    virtual void seekToFrame(int frame) = 0;
    virtual void setVideoToStart() = 0;
    
    // Frame range management
    virtual void setStartFrame(int start) = 0;
    virtual void setEndFrame(int end) = 0;
    virtual int getStartFrame() const = 0;
    virtual int getEndFrame() const = 0;
    
    // Current state queries
    virtual int getActiveFrame() const = 0;
    virtual VideoPlaybackStatus getPlaybackStatus() const = 0;
    virtual float getFramerate_fps() const = 0;
    
    // Playback rate control
    virtual void setFramerateMultiplier(float multiplier) = 0;
    virtual float getFramerateMultiplier() const = 0;

    // Video size management
    virtual VideoSize getVideoSize() const = 0;
    virtual void setVideoSize(int width, int height) = 0;
    
    // Callback registration
    virtual void registerFrameUpdateCallback(std::function<void(int, WhichVideoChannel)> callback) = 0;
    virtual void registerVideoPlaybackCompleteCallback(std::function<void(WhichVideoChannel)> callback) = 0;

    // Sharing image reference with GUI
    virtual const Glib::RefPtr<Goocanvas::Image>& GetImage() = 0;
};

} // namespace VideoPlayback

#endif // IVIDEOCHANNEL_H

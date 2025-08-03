#ifndef ISYNCHRONIZERACTIONS_H
#define ISYNCHRONIZERACTIONS_H

#include <VideoPlaybackTypes.h>

namespace VideoPlayback
{

class ISynchronizerActions
{
public:
    virtual ~ISynchronizerActions() = default;
    
    // Video control actions
    virtual void playVideos() = 0;
    virtual void pauseVideos() = 0;
    virtual void setVideosToStartPosition() = 0;
    virtual void stepVideos(int step_size) = 0;
    
    // Individual channel control (for sync state)
    virtual void playVideo(WhichVideoChannel channel) = 0;
    virtual void pauseVideo(WhichVideoChannel channel) = 0;
    virtual void stepVideo(WhichVideoChannel channel, int step_size) = 0;
    virtual VideoPlaybackStatus getVideoPlaybackStatus(WhichVideoChannel channel) const = 0;
    
    // Synchronization setup
    virtual void setupVideoSync() = 0;
    virtual void calculateFramerateRatio() = 0;
    virtual void clearStartAndEndFrames() = 0;
    virtual void setVideoFrameOffset(int offset) = 0;
    
    // Position updates
    virtual void videoPositionUpdate(WhichVideoChannel channel) = 0;
    virtual void videoFrameUpdate(WhichVideoChannel channel, bool update_joint_slider) = 0;
    
    // Getters for state decisions
    virtual WhichVideoChannel getVideoPositionUpdateChannel() const = 0;
    virtual float getNewVideoPosition() const = 0;
    virtual void setNewVideoPosition(float position) = 0;
    virtual int getNewActiveFrame() const = 0;
    virtual void setNewActiveFrame(int frame) = 0;
    
    // Observer management - needed for SynchronizerStateSyncToPlayback
    virtual void notifyVideoPositionChanged(WhichVideoChannel channel, float position) = 0;
    virtual int getVideoFrameOffset() const = 0;
};

} // namespace VideoPlayback

#endif // ISYNCHRONIZERACTIONS_H

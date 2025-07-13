#ifndef IVIDEOSYNCHRONIZATIONOBSERVER_H
#define IVIDEOSYNCHRONIZATIONOBSERVER_H

#include <VideoPlaybackTypes.h>

namespace VideoPlayback
{

class IVideoSynchronizationObserver
{
public:
    virtual ~IVideoSynchronizationObserver() = default;
    
    // Video position updates (normalized 0.0 to 1.0)
    virtual void onVideoPositionChanged(WhichVideoChannel channel, float position) = 0;
    
    // Playback state changes
    virtual void onPlaybackStateChanged(WhichVideoChannel channel, VideoPlaybackStatus status) = 0;
    
    // Sync mode changes
    virtual void onSyncModeChanged(bool syncMode) = 0;
    
    // Video loading events
    virtual void onVideoLoaded(WhichVideoChannel channel, bool success) = 0;
    virtual void onVideoUnloaded(WhichVideoChannel channel) = 0;
};

} // namespace VideoPlayback

#endif // IVIDEOSYNCHRONIZATIONOBSERVER_H

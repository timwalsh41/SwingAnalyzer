#ifndef VIDEOSYNCHRONIZATIONENGINE_H
#define VIDEOSYNCHRONIZATIONENGINE_H

#include <IVideoSynchronizationObserver.h>
#include <Logger.h>
#include <SyncManager.h>
#include <VideoChannel.h>

#include <memory>
#include <vector>

namespace VideoPlayback
{

class VideoSynchronizationEngine
{
public:
    VideoSynchronizationEngine();
    ~VideoSynchronizationEngine() = default;
    
    // Observer management
    void addObserver(std::weak_ptr<IVideoSynchronizationObserver> observer);
    void removeObserver(std::weak_ptr<IVideoSynchronizationObserver> observer);
    
    // Video playback control
    void playVideo(WhichVideoChannel channel);
    void pauseVideo(WhichVideoChannel channel);
    void stepVideo(WhichVideoChannel channel, int frames);
    void setVideoToStart(WhichVideoChannel channel);
    
    // Video positioning
    void setVideoPosition(WhichVideoChannel channel, float position);
    
    // Video file management
    void loadVideo(WhichVideoChannel channel, const std::string& path);
    void unloadVideo(WhichVideoChannel channel);
    
    // Synchronization mode control
    void setSyncMode(bool enabled);
    
    // Playback rate control
    void setPlaybackRate(float multiplier);
    
    // Access to video channels for canvas setup
    VideoChannel* getVideoChannel(WhichVideoChannel channel);
    
    // Engine lifecycle
    void start();

private:
    std::unique_ptr<SyncManager> m_syncManager;
    std::vector<std::weak_ptr<IVideoSynchronizationObserver>> m_observers;
    Logger& m_logger;
    
    // Notification helpers
    void notifyVideoPositionChanged(WhichVideoChannel channel, float position);
    void notifyPlaybackStateChanged(WhichVideoChannel channel, VideoPlaybackStatus status);
    void notifySyncModeChanged(bool syncMode);
    void notifyVideoLoaded(WhichVideoChannel channel, bool success);
    void notifyVideoUnloaded(WhichVideoChannel channel);
    
    // Clean up expired weak_ptr observers
    void cleanupExpiredObservers();
};

} // namespace VideoPlayback

#endif // VIDEOSYNCHRONIZATIONENGINE_H

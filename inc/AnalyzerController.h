#ifndef ANALYZERCONTROLLER_H
#define ANALYZERCONTROLLER_H

#include <IVideoSynchronizationController.h>
#include <IVideoSynchronizationObserver.h>
#include <Logger.h>
#include <VideoSynchronizationEngine.h>

#include <memory>

class AnalyzerGui;

class AnalyzerController : public VideoPlayback::IVideoSynchronizationController,
                          public VideoPlayback::IVideoSynchronizationObserver,
                          public std::enable_shared_from_this<AnalyzerController>
{
public:
    AnalyzerController(AnalyzerGui* gui);
    ~AnalyzerController() = default;
    
    // Initialize the controller after construction
    void initialize();
    
    // IVideoSynchronizationController implementation
    void playVideo(VideoPlayback::WhichVideoChannel channel) override;
    void pauseVideo(VideoPlayback::WhichVideoChannel channel) override;
    void stepVideo(VideoPlayback::WhichVideoChannel channel, int frames) override;
    void setVideoToStart(VideoPlayback::WhichVideoChannel channel) override;
    void setVideoPosition(VideoPlayback::WhichVideoChannel channel, float position) override;
    void loadVideo(VideoPlayback::WhichVideoChannel channel, const std::string& path) override;
    void unloadVideo(VideoPlayback::WhichVideoChannel channel) override;
    void setSyncMode(bool enabled) override;
    void setPlaybackRate(float multiplier) override;
    
    // IVideoSynchronizationObserver implementation
    void onVideoPositionChanged(VideoPlayback::WhichVideoChannel channel, float position) override;
    void onPlaybackStateChanged(VideoPlayback::WhichVideoChannel channel, VideoPlayback::VideoPlaybackStatus status) override;
    void onSyncModeChanged(bool syncMode) override;
    void onVideoLoaded(VideoPlayback::WhichVideoChannel channel, bool success) override;
    void onVideoUnloaded(VideoPlayback::WhichVideoChannel channel) override;

private:
    AnalyzerGui* m_gui;
    std::shared_ptr<VideoPlayback::VideoSynchronizationEngine> m_engine;
    Logger& m_logger;
};

#endif // ANALYZERCONTROLLER_H

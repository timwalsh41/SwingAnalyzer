#ifndef VIDEOCHANNEL_H
#define VIDEOCHANNEL_H

#include <IGUIImageUpdater.h>
#include <IVideoChannel.h>
#include <IVideoDecoder.h>
#include <Logger.h>
#include <Profiler.h>
#include <ThreadSafeFrameBuffer.h>
#include <VideoChannelState.h>
#include <VideoFrameManager.h>
#include <VideoPlaybackController.h>
#include <VideoPlaybackTypes.h>

#include <glibmm/dispatcher.h>
#include <glibmm/main.h>
#include <goocanvasmm.h>

#include <functional>
#include <memory>

namespace VideoPlayback
{

class VideoChannel : public IVideoChannel {
public:
    VideoChannel(WhichVideoChannel channel);
    ~VideoChannel();
    
    // ================ IVideoChannel interface implementation ================
    // Video operations
    VideoLoadStatus loadVideo(const std::string& path) override;
    void unloadVideo() override;
    void playVideo() override;
    void pauseVideo() override;
    void stepFrame(int offset) override;
    void seekToFrame(int frame) override;
    void setVideoToStart() override;

    // Frame range management
    void setStartFrame(int start) override;
    void setEndFrame(int end) override;
    int getStartFrame() const override;
    int getEndFrame() const override;
    
    // Playback control
    void setFramerateMultiplier(float multiplier) override;
    float getFramerateMultiplier() const override;

    // State queries
    bool isLoaded() const;
    bool isPlaying() const;
    float getFramerate_fps() const override;
    VideoChannelState getState() const;
    bool IsVideoLoaded() const override;
    int getActiveFrame() const override;
    VideoPlaybackStatus getPlaybackStatus() const override;

    // Video size
    VideoSize getVideoSize() const override;
    void setVideoSize(int width, int height) override;

    // Callbacks
    void registerFrameUpdateCallback(std::function<void(int, WhichVideoChannel)> callback) override;
    void registerVideoPlaybackCompleteCallback(std::function<void(WhichVideoChannel)> callback) override;

    // GUI integration
    const Glib::RefPtr<Goocanvas::Image>& GetImage() override;
    

    // ================ Other public methods ================
    // (mainly used for testing)
    int getEffectiveLength() const;
    int getCurrentFrame() const;
    int getTotalFrames() const;
    int getFrameCount() const;
    void enableProfiler();
    void disableProfiler();

private:
    // Core components
    std::unique_ptr<VideoChannelStateMachine> m_state_machine;
    std::unique_ptr<IVideoDecoder> m_video_decoder;
    std::unique_ptr<IGUIImageUpdater> m_gui_updater;
    std::unique_ptr<VideoFrameManager> m_frame_manager;
    std::unique_ptr<VideoPlaybackController> m_playback_controller;
    std::unique_ptr<ThreadSafeFrameBuffer> m_frame_buffer;
    
    // Configuration
    WhichVideoChannel m_channel_id;
    std::function<void(int, WhichVideoChannel)> m_position_callback;
    std::function<void(WhichVideoChannel)> m_complete_callback;
    
    // Current state
    int m_current_frame;
    float m_framerate_multiplier;
    int m_video_width;
    int m_video_height;
    
    // Threading
    Glib::Dispatcher m_frame_update_dispatcher;
    Glib::Dispatcher m_playback_complete_dispatcher;
    
    // Profiling
    Utility::Profiler m_profiler_frame;
    Utility::Profiler m_profiler_image;
    
    // Logger
    Logger& m_logger;
    
    // Private methods
    void playbackWorkerFunction();
    void onFrameUpdate();
    void onPlaybackComplete();
    bool transitionState(VideoChannelEvent event);
    void updateVideoSize();
};

} // namespace VideoPlayback

#endif // VIDEOCHANNEL_H

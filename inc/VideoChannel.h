#ifndef VIDEOCHANNEL_H
#define VIDEOCHANNEL_H

#include <IGUIImageUpdater.h>
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

class VideoChannel {
public:
    VideoChannel(WhichVideoChannel channel, 
                    std::function<void(int, WhichVideoChannel)> update_callback);
    ~VideoChannel();
    
    // State queries
    bool isLoaded() const;
    bool isPlaying() const;
    VideoChannelState getState() const;
    
    // Video operations
    VideoLoadStatus loadVideo(const std::string& path);
    void unloadVideo();
    void playVideo();
    void pauseVideo();
    void stepFrame(int offset);
    void seekToFrame(int frame);
    void setVideoToStart();
    
    // Frame range management
    void setStartFrame(int start);
    void setEndFrame(int end);
    int getStartFrame() const;
    int getEndFrame() const;
    int getEffectiveLength() const;
    
    // Frame position
    int getCurrentFrame() const;
    int getTotalFrames() const;
    
    // Playback control
    void setFramerateMultiplier(float multiplier);
    float getFramerateMultiplier() const;
    float getFramerate() const;
    
    // Video properties
    VideoSize getVideoSize() const;
    void setVideoSize(int width, int height);
    
    // GUI integration
    void* getImageObject();
    const Glib::RefPtr<Goocanvas::Image>& GetImage(); // Old API compatibility
    
    // Compatibility methods with old VideoChannel API
    const bool IsVideoLoaded() const;
    const int getActiveFrame() const;
    const int getFrameCount() const;
    const float getRawFramerate_fps() const;
    const VideoPlaybackStatus getPlaybackStatus() const;
    const int getEffectiveVideoLengthFrames() const;
    void registerVideoPlaybackCompleteCallback(std::function<void(WhichVideoChannel)> callback);
    
    // Callbacks
    void registerPlaybackCompleteCallback(std::function<void(WhichVideoChannel)> callback);
    
    // Profiling
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

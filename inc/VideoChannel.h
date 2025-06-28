#ifndef VIDEOCHANNEL_H
#define VIDEOCHANNEL_H

#include <VideoChannelState.h>
#include <IVideoDecoder.h>
#include <IGUIImageUpdater.h>
#include <VideoFrameManager.h>
#include <VideoPlaybackController.h>
#include <ThreadSafeFrameBuffer.h>
#include <VideoPlaybackTypes.h>
#include <Profiler.h>
#include <Logger.h>

#include <glibmm/main.h>
#include <glibmm/dispatcher.h>
#include <goocanvasmm.h>
#include <memory>
#include <functional>

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
    void setVideoToFrame(int frame_index);
    void registerVideoPlaybackCompleteCallback(std::function<void(WhichVideoChannel)> callback);
    
    // Callbacks
    void registerPlaybackCompleteCallback(std::function<void(WhichVideoChannel)> callback);
    
    // Profiling
    void enableProfiler();
    void disableProfiler();

private:
    // Core components
    std::unique_ptr<VideoChannelStateMachine> state_machine_;
    std::unique_ptr<IVideoDecoder> video_decoder_;
    std::unique_ptr<IGUIImageUpdater> gui_updater_;
    std::unique_ptr<VideoFrameManager> frame_manager_;
    std::unique_ptr<VideoPlaybackController> playback_controller_;
    std::unique_ptr<ThreadSafeFrameBuffer> frame_buffer_;
    
    // Configuration
    WhichVideoChannel channel_id_;
    std::function<void(int, WhichVideoChannel)> position_callback_;
    std::function<void(WhichVideoChannel)> complete_callback_;
    
    // Current state
    int current_frame_;
    float framerate_multiplier_;
    int video_width_;
    int video_height_;
    
    // Threading
    Glib::Dispatcher frame_update_dispatcher_;
    Glib::Dispatcher playback_complete_dispatcher_;
    
    // Profiling
    Utility::Profiler profiler_frame_;
    Utility::Profiler profiler_image_;
    
    // Logger
    Logger& logger_;
    
    // Private methods
    void playbackWorkerFunction();
    void onFrameUpdate();
    void onPlaybackComplete();
    bool transitionState(VideoChannelEvent event);
    void updateVideoSize();
};

} // namespace VideoPlayback

#endif // VIDEOCHANNEL_H

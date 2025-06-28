#ifndef VIDEOPLAYBACKCONTROLLER_H
#define VIDEOPLAYBACKCONTROLLER_H

#include <VideoPlaybackConstants.h>
#include <Logger.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <functional>

namespace VideoPlayback
{

class VideoPlaybackController {
public:
    VideoPlaybackController();
    ~VideoPlaybackController();
    
    void startPlayback(std::function<void()> playback_function);
    void stopPlayback();
    bool isPlaying() const;
    
    void setFramerate(float fps, float multiplier = 1.0f);
    std::chrono::microseconds getFrameInterval() const;
    
private:
    std::atomic<bool> should_continue_;
    std::thread playback_thread_;
    float frame_time_us_;
    Logger& logger_;
    
    void cleanup();
};

} // namespace VideoPlayback

#endif // VIDEOPLAYBACKCONTROLLER_H

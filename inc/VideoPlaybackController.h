#ifndef VIDEOPLAYBACKCONTROLLER_H
#define VIDEOPLAYBACKCONTROLLER_H

#include <Logger.h>
#include <VideoPlaybackConstants.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>

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
    std::atomic<bool> m_should_continue;
    std::thread m_playback_thread;
    float m_frame_time_us;
    Logger& m_logger;

    void cleanup();
};

} // namespace VideoPlayback

#endif // VIDEOPLAYBACKCONTROLLER_H

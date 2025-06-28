#include <VideoPlaybackController.h>

namespace VideoPlayback
{

VideoPlaybackController::VideoPlaybackController() 
    : should_continue_(false), frame_time_us_(0), logger_(Logger::get()) {
}

VideoPlaybackController::~VideoPlaybackController() {
    stopPlayback();
}

void VideoPlaybackController::startPlayback(std::function<void()> playback_function) {
    if (isPlaying()) {
        logger_.warn("Playback already active, stopping previous playback");
        stopPlayback();
    }
    
    should_continue_ = true;
    playback_thread_ = std::thread([this, playback_function]() {
        logger_.debug("Playback thread started");
        playback_function();
        logger_.debug("Playback thread finished");
    });
}

void VideoPlaybackController::stopPlayback() {
    should_continue_ = false;
    cleanup();
}

bool VideoPlaybackController::isPlaying() const {
    return should_continue_.load() && playback_thread_.joinable();
}

void VideoPlaybackController::setFramerate(float fps, float multiplier) {
    if (fps <= 0 || multiplier <= 0) {
        logger_.warn("Invalid framerate parameters: fps=%.2f, multiplier=%.2f", fps, multiplier);
        return;
    }
    
    if (multiplier > MAX_FRAMERATE_MULTIPLIER || multiplier < MIN_FRAMERATE_MULTIPLIER) {
        logger_.warn("Framerate multiplier %.3f out of range (%.3f-%.3f)", 
                     multiplier, MIN_FRAMERATE_MULTIPLIER, MAX_FRAMERATE_MULTIPLIER);
        return;
    }
    
    frame_time_us_ = 1.0f / multiplier / fps * 1e6f;
    logger_.debug("Frame time set to %.2f ms (fps=%.2f, multiplier=%.2f)", 
                  frame_time_us_ / 1000.0f, fps, multiplier);
}

std::chrono::microseconds VideoPlaybackController::getFrameInterval() const {
    return std::chrono::microseconds(static_cast<long int>(frame_time_us_));
}

void VideoPlaybackController::cleanup() {
    if (playback_thread_.joinable()) {
        playback_thread_.join();
    }
}

} // namespace VideoPlayback
